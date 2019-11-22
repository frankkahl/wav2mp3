//
// exports function convert_all_wav_files_in_directory to do the actual conversion work
// all other functions are declared static since they are helper functions internally used only
// Design decisions:
//     - being a nitpicker with respect to the format assumptions about a WAV file:
//         - a WAV file is a special RIFF file which besides the mandatory "RIFF" chunk
//           must contain the chunks "fmt " and "data"
//         - BUT a RIFF file does NOT guarantee any order of the chunks.
//           Therefore assuming the chunk sequence "RIFF", "fmt ", "data"
//           is strictly speaking incorrect and is therefore NOT assumed
//         - all chunks with ids other than "fmt " and "data" are ignored
//           but do not cause an error
//     - clear and detailed warning and error messages

#include "convert_wav_files.h"

#include "chunk_descriptor.h"
#include "configuration.h"
#include "lame_init.h"
#include "return_code.h"
#include "riff_format.h"
#include "signal_handler.h"
#include "thread_pool.h"
#include "tiostream.h"

#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <tuple>

#define ERROR_PREFIX "   [ ERROR ] "
#define OK_PREFIX "   [  OK   ] "
#define SPACES_PREFIX "             "

namespace fs = std::filesystem;
using namespace std;

/*!
 * Read all chunk descriptors from file starting at stream position start to at max the stream position start +
 * max_data_size Returns a tuple of:
 *     - ChunkDescriptorMap: maps the FOURCC chunk ids of all valid chunks found to
 *       the positions and sizes of their data blocks.
 *       If empty then either the file is not a RIFF file or it is corrupt.
 *     - string: is empty if everything went fine, otherwise it contains a warning or error message.
 */

static tuple<ChunkDescriptorMap, string> read_all_chunk_descriptors(ifstream &file, const streampos &start,
                                                                    const streamsize &max_data_size) {
    // now inspect all chunks and store their positions and sizes
    ostringstream      ss;
    ChunkDescriptorMap chunk_descriptors;
    file.seekg(start);

    auto pad_data_size = [](const streamsize chunk_data_size) {
        streamsize padded_data_size = chunk_data_size / sizeof(uint16_t) * sizeof(uint16_t);
        if (chunk_data_size % sizeof(uint16_t)) {
            padded_data_size += sizeof(uint16_t);
        }
        return padded_data_size;
    };

    streamsize padded_max_data_size = pad_data_size(max_data_size);
    while (!(file.eof() || file.fail())) {
        char     chunk_id_raw[4];
        uint32_t chunk_data_size;

        file.read(chunk_id_raw, sizeof(chunk_id_raw));                 // first read FOURCC id of chunk
        file.read((char *)&chunk_data_size, sizeof(chunk_data_size));  // then the size of valid data in the data block
        if (file.eof() || file.fail()) {
            ss << "Reading chunk id and size failed.";
            break;
        }
        streamsize padded_chunk_data_size = pad_data_size(chunk_data_size);

        string chunk_id(chunk_id_raw, sizeof(chunk_id_raw));
        // store start of data position in ChunkDescriptor struct first
        // but do NOT store it yet...
        ChunkDescriptor chunk_pos = {file.tellg(), chunk_data_size};

        // ... first advance pointer to the position of the last byte of the valid data block according to
        // padded_chunk_data_size
        file.seekg(chunk_data_size - 1, ios_base::cur);
        // then peek for the byte to enforce the eofbit or failbit to be set if not enough data is available
        file.peek();
        // check also if the chunk claims to reach beyond the padded_max_data_size
        if (file.eof() || file.fail() || (chunk_pos.start + padded_chunk_data_size > start + padded_max_data_size)) {
            ss << "less data available as claimed in chunk \"" << chunk_id << "\" => discard it";
            break;
        }
        // only if the claimed data is really there consider the chunk valid
        // an add it to the chunk_descriptors
        if (chunk_descriptors.count(chunk_id)) {
            ss << "multiple \"" << chunk_id << "\" chunks found, using the latest one. ";
        }
        chunk_descriptors[chunk_id] = chunk_pos;

        // now forward by the number of padding bytes plus 1 to set the pointer to the beginning of the next chunk
        // adding 1 is needed since the pointer currently points to the last byte of the current data block,
        // not one position behind it
        file.seekg(padded_chunk_data_size - chunk_data_size + 1, ios_base::cur);
        file.peek();  // if peeking the next byte just sets the eofbit without setting the failbit or badbit
                      // the file does not extend beyond the chunk with some unexpected data
                      // the same is true if the end of the chunk id identical with the max_data_size
                      // => break the loop gracefully
        if ((file.eof() && !file.fail())
            || (chunk_pos.start + padded_chunk_data_size == start + padded_max_data_size)) {
            long long a = chunk_pos.start + padded_chunk_data_size;
            long long b = start + padded_max_data_size;
            break;
        }
    }
    //// leave this debug code in for now
    // cout << "--- found chunks: ";
    // for (auto const &[key, value] : chunk_descriptors) {
    //    cout << '"' << key << '"' << "  ";
    //}
    // cout << endl;

    return make_tuple(chunk_descriptors, ss.str());
}

/*!
 * Checks if the passed chunks contain a chunk with FOURCC chunk_name_fourcc
 * and if at the very beginning of the data block the FOURCC format_type_fourcc is stored
 * If yes try to interpret the residual data block as a list of sub-chunks
 * If that fails return an empty ChunkDescriptorMap together with an error string
 * Returns a tuple of:
 *     - ChunkDescriptorMap: maps the FOURCC chunk ids of all valid chunks found to
 *       the positions and sizes of their data blocks.
 *       If empty then either the file is not a RIFF file or it is corrupt.
 *     - string: is empty if everything went fine, otherwise it contains a warning or error message.
 */
static tuple<ChunkDescriptorMap, string> get_chunk_descriptors_of_all_subschunks_of_chunk_of_passed_id_and_format_type(
    ifstream &file, const ChunkDescriptorMap &chunks, const std::string &chunk_name_fourcc,
    const std::string &format_type_fourcc = std::string()) {
    ChunkDescriptorMap riff_sub_chunks;
    ostringstream      ss;
    if (!chunks.count(chunk_name_fourcc)) {
        ss << "No " << chunk_name_fourcc << " chunk found";
        return make_tuple(riff_sub_chunks, ss.str());
    }
    const ChunkDescriptor &riff_chunk = chunks.find(chunk_name_fourcc)->second;
    file.seekg(riff_chunk.start);

    size_t size_of_format_type = 0;
    if (!format_type_fourcc.empty()) {
        char format_raw[4];
        size_of_format_type = sizeof(format_raw);
        file.read(format_raw, sizeof(format_raw));
        string format(format_raw, sizeof(format_raw));
        if (format != format_type_fourcc) {
            ss << "unsupported format \"" << format << "\" specifier instead of \"" << format_type_fourcc
               << "\"; try to ";
            return make_tuple(riff_sub_chunks, ss.str());
        }
    }
    return read_all_chunk_descriptors(file, file.tellg(), riff_chunk.data_size - size_of_format_type);
}

/*! Checks if the chunks contain a valid "LIST" chunk of fomrmat type "INFO", extract its sub-chunks containing the
 *  meta data and adds them to the passed meta_info_chunks. In case a certain sub-chunk is already present in
 *  meta_info_chunks it is overwritten with the new one
 */
void aggregate_meta_data(ifstream &infile, ChunkDescriptorMap &chunks, ChunkDescriptorMap &meta_info_chunks,
                         const std::string &chunk_fourcc, const std::string &format_type_fourcc = std::string()) {
    auto const &[new_meta_info_chunks, message] =
        get_chunk_descriptors_of_all_subschunks_of_chunk_of_passed_id_and_format_type(infile, chunks, chunk_fourcc,
                                                                                      format_type_fourcc);
    meta_info_chunks.insert(new_meta_info_chunks.begin(), new_meta_info_chunks.end());
}

void aggregate_id3_tags(const ChunkDescriptorMap &chunks, ChunkDescriptorMap &meta_info_chunks) {
}

    /*!
     *  Performs all consistency checks for PCM format header to be valid,
     *  see comments of FormatHeader in riff_format.h
     *  and "https://msdn.microsoft.com/en-us/library/windows/desktop/dd390970(v=vs.85).aspx"
     *  returns: if successful: tuple(true, <info string>)
     *				<info_string>: describes found audio data, e.g. "41.0 kHz, 16 bit, stereo"
     *                             should be used for info output to the user.
     *           on failure:    tuple(false, <undefined format_header, <error_message>)
     *  remark: bits per sample are not enforced to 8 or 16 as long as the value is an integer multiple of 8
     */
    static tuple<bool, string> check_sane_pcm_or_ieee_float_format_header(
        const FormatHeaderExtensible &header_extensible) {
    stringstream        ss;
    const FormatHeader &header = header_extensible.header;

    ss << right << setfill('0') << setw(4) << hex;  // switch to an output style suited for FOURCC values
    if (!(header.audio_format == WAVE_FORMAT_PCM || header.audio_format == WAVE_FORMAT_IEEE_FLOAT
          || header.audio_format == WAVE_FORMAT_EXTENSIBLE)) {
        ss << "unsupported audio format ";
        if (audio_format_uint16_to_names.count(header.audio_format)) {
            ss << "\"" << audio_format_uint16_to_names[header.audio_format];
            ss << "\" (0x" << header.audio_format << ")";
        } else {
            ss << "0x" << header.audio_format;
        }
        ss << " instead of \"PCM\" (0x" << WAVE_FORMAT_PCM << ")"
           << " \"WAVE_FORMAT_EXTENSIBLE\" (0x" << WAVE_FORMAT_EXTENSIBLE << ")"
           << " or \"IEEE FLOAT\" (0x" << WAVE_FORMAT_IEEE_FLOAT << ")";
        return make_tuple(false, ss.str());
    }
    if (header.audio_format == WAVE_FORMAT_EXTENSIBLE
        && !(header_extensible.sub_format == KSDATAFORMAT_SUBTYPE_PCM
             || header_extensible.sub_format == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {
        ss << "unsupported sub-format in WAVE_FORMAT_EXTENSIBLE header ";
        if (audio_format_guid_to_names.count(header_extensible.sub_format)) {
            ss << "\"" << audio_format_guid_to_names[header_extensible.sub_format] << "\" ({"
               << header_extensible.sub_format.string() << "})";
        } else {
            ss << "{" << header_extensible.sub_format.string() << "}";
        }
        ss << " instead of \"KSDATAFORMAT_SUBTYPE_PCM\"  or \"KSDATAFORMAT_SUBTYPE_IEEE_FLOAT\")";
        return make_tuple(false, ss.str());
    }
    // now check for certain dependencies of the format properties a PCM file must fulfil
    ss << setfill(' ') << setw(0) << dec;  // switch back to decimal output with no field width and filling
    if (header.samples_per_second * header.block_align != header.bytes_per_second) {
        ss << setfill(' ') << setw(0) << dec;
        ss << "bytes per second (" << header.bytes_per_second << ") != ";
        ss << "samples per second (" << header.samples_per_second << ") * ";
        ss << "block align (" << header.block_align << ")";
        return make_tuple(false, ss.str());
    }
    auto bps = header.bits_per_sample;
    if (header.audio_format == WAVE_FORMAT_PCM
        || (header.audio_format == WAVE_FORMAT_EXTENSIBLE
            && header_extensible.sub_format == KSDATAFORMAT_SUBTYPE_PCM)) {
        if (bps % 8) {
            ss << setfill(' ') << setw(0) << dec;
            ss << "bits per sample of " << bps
               << " illegal for \"PCM\"; should always be a multiple of 8 with a maximum of " << 8 * sizeof(int32_t);
            bps = (bps + 7) / 8 * 8;  // correct container size to the next larger multiple of 8
            ss << ", adjusting to " << bps << "bits per sample";
        }
        if (bps > 8 * sizeof(int32_t)) {
            ss << setfill(' ') << setw(0) << dec;
            ss << "bits per sample of " << bps << " illegal for \"PCM\"; must be <= 32";
        }
    }
    if (header.audio_format == WAVE_FORMAT_IEEE_FLOAT
        || (header.audio_format == WAVE_FORMAT_EXTENSIBLE
            && header_extensible.sub_format == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {
        if (!(bps == 8 * sizeof(float) || bps == 8 * sizeof(double))) {
            ss << "bits per sample of " << dec << bps << " illegal for \"IEEE FLOAT\"; must be 32 or 64";
            return make_tuple(false, ss.str());
        }
    }
    if (header.audio_format == WAVE_FORMAT_EXTENSIBLE && header_extensible.sub_format == KSDATAFORMAT_SUBTYPE_PCM) {
        if (header_extensible.samples.valid_bits_per_sample > bps) {
            ss << "the valid bits per sample (" << header_extensible.samples.valid_bits_per_sample
               << ") in the extensible part of WAVE_FORMAT_EXTENSIBLE "
               << "must always be smaller or equal to the value for bits per sample (" << bps << ")";
            return make_tuple(false, ss.str());
        }
    }
    if (header.block_align != header.num_channels * bps / 8) {
        ss << "block align (" << header.block_align << ") != ";
        ss << "num of channels (" << header.num_channels << ") * ";
        ss << "bits per sample (" << bps << ")/8";
        return make_tuple(false, ss.str());
    }
    // since the header is consistent, generate a descriptive string
    // for the found data
    ss << setfill(' ') << setw(0) << dec;
    ss << setprecision(4) << (double)header.samples_per_second / 1000.0 << " kHz, " << bps << " bit"
       << ", ";
    switch (header.num_channels) {
        case 1:
            ss << "mono";
            break;
        case 2:
            ss << "stereo";
            break;
        default:
            ss << "illegal number " << header.num_channels << " for channels, must be 1 or 2";
            return make_tuple(false, ss.str());
            break;
    }
    return make_tuple(true, ss.str());
}

/*!
 *  Checks if the chunks in chunk_descriptors form a valid WAV file.
 *  The chunk positions to pass can be obtained by calling is_valid_riff_file()
 *  Checks for a valid WAV file:
 *      - "fmt " and "data " chunks present
 *      - enough data present to read the number of bytes claimed
 *        by the data size identifier of the chunk
 *      - data payload of "fmt " chunk must be larger than the size of
 *        FormatHeader (see "riff_format.h")
 *      - The format header must describe a valid PCM file
 *        ( see check_sane_pcm_format_header() )
 *  If successful positions the stream "file" to the beginning of the PCM audio data
 *  returns: if successful: tuple(true, <valid pcm format header>,
 *                                <position and length of PCM data as ChunkDescriptor>,
 *								  <info string>)
 *		<info_string>: describes found audio data, e.g. "41.0 kHz, 16 bit, stereo"
 *                             should be used for info output to the user.
 *           on failure:    tuple(false, <undefined format_header>,
 *                                <undefined ChunkDescriptor object>, <error_message>)
 */
static tuple<bool, FormatHeaderExtensible, ChunkDescriptor, string> is_valid_wav_file(
    ifstream &file, ChunkDescriptorMap &chunk_descriptors) {
    ostringstream          ss;
    FormatHeaderExtensible format_header;
    ChunkDescriptor        data_chunk_payload;
    // check first if there is a "fmt " chunk
    if (!chunk_descriptors.count("fmt ")) {
        ss << "no \"fmt \" chunk found";
        return make_tuple(false, format_header, data_chunk_payload, ss.str());
    }
    // then make sure there is a data chunk
    if (!chunk_descriptors.count("data")) {
        ss << "no \"data\" chunk found";
        return make_tuple(false, format_header, data_chunk_payload, ss.str());
    }
    auto size      = sizeof(format_header);
    auto data_size = chunk_descriptors["fmt "].data_size;
    // and if it contains enough data to be the format header we expect
    if (chunk_descriptors["fmt "].data_size < sizeof(FormatHeader)) {
        ss << "not enough bytes to read the base format header";
        return make_tuple(false, format_header, data_chunk_payload, ss.str());
    }
    // then move pointer to start of data
    // and read it into the FormatHeader struct
    file.seekg(chunk_descriptors["fmt "].start);
    file.read((char *)&format_header.header, sizeof(format_header.header));
    if (format_header.header.audio_format == WAVE_FORMAT_EXTENSIBLE) {
        if (chunk_descriptors["fmt "].data_size < sizeof(FormatHeaderExtensible)) {
            ss << "not enough bytes to read the extensible part of the format header";
            return make_tuple(false, format_header, data_chunk_payload, ss.str());
        }
        file.read((char *)&format_header.size, sizeof(FormatHeaderExtensible) - sizeof(FormatHeader));
    }
    auto [is_header_valid, info_string] = check_sane_pcm_or_ieee_float_format_header(format_header);
    if (!is_header_valid) {
        return make_tuple(false, format_header, data_chunk_payload, info_string);
    }
    // if this point is reached then the stream points to a valid WAV file
    // with PCM or IEEE FLOAT content
    // so return the validated FormatHeader structure and the position and length
    // of the PCM data in the stream in a ChunkDescriptor structure
    return make_tuple(true, format_header, chunk_descriptors["data"], info_string);
}

/*!
 *  case insensitive string compare
 */
bool case_insensitive_compare(const string &a, const string &b) {
    return std::equal(a.begin(), a.end(), b.begin(), b.end(),
                      // a lambda function as predicate will do
                      [](char a, char b) { return tolower(a) == tolower(b); });
}

/*!
 *  helper function for output formatting
 */
static string get_path_relative_to_top_level(const fs::path &in_file_name) {
    std::error_code ec;
    auto            output_path = fs::proximate(in_file_name, fs::path(Configuration::directory_path()), ec);
    if (ec) {
        output_path = in_file_name;  // if getting the relative path fails in any way use the absolute path
    }
    return output_path.string();
}

/*!
 *  Creates the MP3 file and associates it with the passed stream "out_file".
 *  The path of the MP3 file is generated using the following rules:
 *     - if in_file_name has a ".wav" file ending replace it by ".mp3",
 *       if not then just add ".mp3"
 *       The resulting path is referred to as "<mp3_pathname_base>.mp3"
 *     - if the path "<mp3_pathname_base>.mp3" already exists
 *       and Configuration::overwrite_existing_mp3() == false,
 *       then use "<mp3_pathname_base> (1).mp3",
 *       if that file already exists
 *       then use "<mp3_pathname_base> (2).mp3" and so on
 *  if successful:
 *       - "out_file" is a valid open stream
 *       - return: tuple(true, <conversion info string>)
 *         where <conversion info string> has the form:
 *           line 1 (if the case) :     "WARNING: <in_file_name> does not end with .wav.\n"
 *           line 2               :     "<relative path of in_file_name> (<in_file_info>) -> <basename of in_file_name
 **(<n>).mp3
 *       - on failure: "out_file" is closed and invalid
 *       - return: tuple(false, <error message string>)
 *  examples for <conversion info string>:
 *  1. Precondition: "wav_folder\test.mp3" does not exist
 *     Arguments:
 *		 in_file_name: "wav_folder\test.wav"
 *       in_file_info: "41.0 kHz, 16 bit, stereo"
 *     <conversion info string> = "\"test.wav\" (41.0 kHz, 16 bit, stereo) -> \"test.mp3\"
 *  2. like 1. but
 *     Precondition: "wav_folder\test.mp3" exists
 *     <conversion info string> = "\"test.wav\" (41.0 kHz, 16 bit, stereo) -> \"test (1).mp3\"
 *  3. Precondition: "wav_folder\test._wav_.mp3" does not exist
 *     Arguments:
 *		 in_file_name: "wav_folder\test._wav_"
 *       in_file_info: "41.0 kHz, 16 bit, stereo"
 *     <conversion info string> = "WARNING: \"test._wav_\" does not end with .wav but is a valid WAV file.\n
 *                                 \"test._wav_\" (41.0 kHz, 16 bit, stereo) -> \"test._wav_.mp3\"
 */
static tuple<bool, string> open_output_stream(const fs::path &in_file_name, const string &in_file_info,
                                              ofstream &out_file, fs::path &mp3_path) {
    ostringstream ss;
    ostringstream status_line;
    out_file.close();  // just in case there is still a file associated to this stream
    fs::path mp3_path_base;
    if (case_insensitive_compare(in_file_name.extension().string(), ".wav")) {
        mp3_path_base = in_file_name.parent_path() / in_file_name.stem();
    } else {
        status_line << "WARNING: \"" << get_path_relative_to_top_level(in_file_name)
                    << "\" does not end with \".wav\" but is a valid WAV file." << endl
                    << "            ";
        mp3_path_base = in_file_name;
    }
    // Now generate a path for the output file which does not already exist
    // fs::path mp3_path;
    try {
        // try all filename from
        // "<mp3_path_base>.mp3", "<mp3_path_base> (1).mp3" up to
        // "<mp3_path_base> (655356).mp3"
        for (uint16_t i = 0; i < UINT16_MAX; ++i) {
            mp3_path = mp3_path_base;
            if (i > 0) {
                ostringstream number;
                number << " (" << i << ")";
                mp3_path += fs::path(number.str());
            }
            mp3_path += ".mp3";
            // if the "overwrite existing mp3" command line option was passed
            // then do not care if the file already exists
            // otherwise insist that the file does not already exist
            if (Configuration::overwrite_existing_mp3() || !fs::exists(mp3_path)) {
                // try to open it with the passed ofstream object
                out_file.open(mp3_path, ios::binary | ios::trunc | ios::out);
                // on failure abort. Most likely the process has no write permissions
                if (out_file.fail()) {
                    ss << "creating \"" << mp3_path.string() << "\" for writing";
                    ss << " failed. Check write permission of target directory";
                    out_file.clear();
                    return make_tuple(false, ss.str());
                }
                // opening worked, so break the name finding loop
                break;
            }
        }
    }
    // if an exception is thrown then something must have gone seriously wrong
    // e.g. there is no permission to access the parent directory of the path
    catch (const fs::filesystem_error &e) {
        ss << "finding a suitable output file name failed: " << e.code().message();
        return make_tuple(false, ss.str());
    }
    if (!out_file.is_open()) {
        ss << "finding output file name failed.";
        return make_tuple(false, ss.str());
    }
    // reaching that point means that the file was successfully created
    // and associated to the passed stream => write status message
    // first get the relative path of the converted WAV file relative to the top
    // level directory
    status_line << "\"" << get_path_relative_to_top_level(in_file_name) << "\"\t";
    status_line << " (" << in_file_info << ") \t-> ";
    status_line << "\"" << mp3_path.filename().string() << "\"\t";
    return make_tuple(true, status_line.str());
}

// helper function for printing an error of format
//       (<thread_number>) <message>: <error>
static void print_error(const string &message, const string &error) {
    ostringstream ss;
    ss << ERROR_PREFIX << message << "failed: " << endl;
    ss << SPACES_PREFIX << error << endl;
    tcerr << ss.str();
    set_return_code(RET_CODE_CONVERTING_SOME_FILES_FAILED);
};

// adds all id3 v2 tags for which corresponding info chunks are present in the passed meta_data
static void create_id3_v2_tags(LameInit &lame_guard, shared_ptr<ifstream> in, const ChunkDescriptorMap &meta_data) {
    // template lambda function (see auto keyword in front of (*setter) requires C++ 14)
    auto set_tag = [&lame_guard, in, &meta_data](auto (*setter)(lame_t, const char *), string list_info_fourcc) {
        if (!meta_data.count(list_info_fourcc)) {
            return;
        }
        // use this rather annoying syntax to access the element instead of
        // just meta_data.find(list_info_fourcc)
        auto start     = meta_data.find(list_info_fourcc)->second.start;
        auto data_size = meta_data.find(list_info_fourcc)->second.data_size;
        in->seekg(start);
        unique_ptr<char> tag_string_raw(new char[data_size]);
        in->read(tag_string_raw.get(), data_size);
        if (strlen(tag_string_raw.get()) >= (std::size_t)data_size) {
            // if the chunk data does not contain a null byte to mark a null terminated string
            // consider the tag invalid and just silently ignore it
            return;
        }
        // leave this debug code in for now
        // ostringstream ss;
        // ss << list_info_fourcc << ": " << tag_string_raw.get() << endl;
        // tcout << ss.str();
        setter(lame_guard, tag_string_raw.get());
    };
    id3tag_init(lame_guard);
    id3tag_v2_only(lame_guard);  // do not support ancient outdated id3 v1 tags by purpose

    set_tag(id3tag_set_title, "INAM");
    set_tag(id3tag_set_artist, "IART");
    set_tag(id3tag_set_album, "IMED");
    set_tag(id3tag_set_year, "ICRD");
    // several tags found which claim to mark comments
    // assume that only one will be present
    // if more are present the ICMT one takes precedence
    set_tag(id3tag_set_comment, "COMM");
    set_tag(id3tag_set_comment, "CMNT");
    set_tag(id3tag_set_comment, "ICMT");
    // also more than one ID for genre found
    set_tag(id3tag_set_track, "TRCK");
    set_tag(id3tag_set_track, "ITRK");
    // also more than one ID for genre found
    set_tag(id3tag_set_genre, "GENR");
    set_tag(id3tag_set_genre, "IGNR");
    id3tag_set_fieldvalue(lame_guard, "TCOM=Frank Kahl");
}

// calls the config functions of lame according to the content of the header
// and the encoding quality returned by Configuration::encoding_quality()
static bool config_lame(LameInit &lame_guard, shared_ptr<ifstream> &in, const string &message,
                        const FormatHeader &header, const ChunkDescriptorMap &list_info_chunk_meta_data) {
    if (!lame_guard.is_initialized()) {
        string error = "lame_init() failed";
        print_error(message, error);
        return false;
    }
    int res = 0;
    try {
        create_id3_v2_tags(lame_guard, in, list_info_chunk_meta_data);
        res = lame_set_num_channels(lame_guard, header.num_channels);
        LameInit::check_error(res, "lame_set_num_channels");
        res = lame_set_in_samplerate(lame_guard, header.samples_per_second);
        LameInit::check_error(res, "lame_set_in_samplerate");
        res = lame_set_mode(lame_guard, header.num_channels == 2 ? JOINT_STEREO : MONO);
        LameInit::check_error(res, "lame_set_mode");
        res = lame_set_quality(lame_guard, Configuration::encoding_quality()); /* 2=high  5 = medium  7=low */
        LameInit::check_error(res, "lame_set_quality");
        res = lame_init_params(lame_guard);
        LameInit::check_error(res, "lame_init_params");
    } catch (const lame_exception &e) {
        print_error(message, e.what());
        return false;
    }
    return true;
}

// helper function for convert_file_worker() for converting the next num_of_samples  audio samples of a WAV file in PCM
// format
void convert_pcm_int_chunk(LameInit &lame_guard, std::shared_ptr<std::ifstream> &in,
                           std::shared_ptr<std::ofstream> &out, const uint32_t number_of_samples,
                           const uint32_t bytes_per_sample, const FormatHeaderExtensible &header_extensible) {
    uint32_t    valid_bits_per_sample;
    auto const &header = header_extensible.header;
    if (header.audio_format == WAVE_FORMAT_EXTENSIBLE) {
        valid_bits_per_sample = header_extensible.samples.valid_bits_per_sample;
    } else {
        valid_bits_per_sample = header.bits_per_sample;
    }
    unique_ptr<int32_t> pcm_buffer(new int32_t[number_of_samples]);
    // formula found in the documentation of "lame_encode_buffer" in lame.h
    uint32_t                  mp3_buffer_size = (uint32_t)(1.25 * (double)number_of_samples + 7200.0);
    unique_ptr<unsigned char> mp3_buffer(new unsigned char[mp3_buffer_size]);
    for (uint32_t i = 0; i < number_of_samples; i++) {
        int32_t c = 0;
        in->read((char *)&c, bytes_per_sample);
        if (bytes_per_sample == 1) {
            c -= 128;
        }
        c <<= (sizeof(int32_t) * 8 - valid_bits_per_sample);  // expands values to the full range of int32_t
        pcm_buffer.get()[i] = c;
    }
    int bytes_converted = 0;
    if (header.num_channels == 2) {
        bytes_converted = lame_encode_buffer_interleaved_int(lame_guard, pcm_buffer.get(), number_of_samples / 2,
                                                             mp3_buffer.get(), mp3_buffer_size);
        LameInit::check_error(bytes_converted, "lame_encode_buffer_interleaved_int");
    } else {
        bytes_converted = lame_encode_buffer_int(lame_guard, pcm_buffer.get(), 0, number_of_samples, mp3_buffer.get(),
                                                 mp3_buffer_size);
        LameInit::check_error(bytes_converted, "lame_encode_buffer_int");
    }
    out->write((char *)mp3_buffer.get(), bytes_converted);
}

// helper function for convert_file_worker() for converting the next num_of_samples  audio samples of a WAV file in IEEE
// FLOAT format
void convert_ieee_float_chunk(LameInit &lame_guard, std::shared_ptr<std::ifstream> &in,
                              std::shared_ptr<std::ofstream> &out, const uint32_t number_of_samples,
                              const uint32_t bytes_per_sample, const FormatHeader &header) {
    unique_ptr<double> pcm_buffer(new double[number_of_samples]);
    // formula found in the documentation of "lame_encode_buffer" in lame.h
    uint32_t                  mp3_buffer_size = (uint32_t)(1.25 * (double)number_of_samples + 7200.0);
    unique_ptr<unsigned char> mp3_buffer(new unsigned char[mp3_buffer_size]);
    for (uint32_t i = 0; i < number_of_samples; i++) {
        double c = 0;
        switch (bytes_per_sample) {
            case 4:
                float f;
                in->read((char *)&f, sizeof(float));
                c = f;  // convert float to double
                break;
            case 8:
                in->read((char *)&c, sizeof(double));
                break;
            default:
                ostringstream err;
                err << "unexpected error: illegal bits per sample value " << header.bits_per_sample
                    << " for \"IEEE FLOAT\" format";
                throw runtime_error(err.str());
        }
        pcm_buffer.get()[i] = c;
    }
    int bytes_converted = 0;
    if (header.num_channels == 2) {
        bytes_converted = lame_encode_buffer_interleaved_ieee_double(
            lame_guard, pcm_buffer.get(), number_of_samples / 2, mp3_buffer.get(), mp3_buffer_size);
        LameInit::check_error(bytes_converted, "lame_encode_buffer_interleaved_ieee_double");
    } else {
        bytes_converted = lame_encode_buffer_ieee_double(lame_guard, pcm_buffer.get(), 0, number_of_samples,
                                                         mp3_buffer.get(), mp3_buffer_size);
        LameInit::check_error(bytes_converted, "lame_encode_buffer_ieee_double");
    }
    out->write((char *)mp3_buffer.get(), bytes_converted);
}

// this function does the actual conversion work and is being executed
// in one of the threads of the thread pool
// currently the argument thread_number is not used, but it can be useful to generate debug output
// containing the thread number, so I leave it in for now
static void convert_file_worker(shared_ptr<ifstream> in, shared_ptr<ofstream> out, const fs::path out_filename,
                                const FormatHeaderExtensible header_extensible, const ChunkDescriptor pcm_data_position,
                                string message, ChunkDescriptorMap list_info_chunk_meta_data, uint16_t thread_number) {
    // Define a lambda function for discard incomplete mp3 file in case of an error
    auto remove_mp3_file = [&out, &out_filename]() {
        out->close();
        std::error_code ec;
        fs::remove(out_filename, ec);  // try to delete the incomplete MP3 file, but do not make a fuzz about failing
    };
    // allocate input buffer for 8192 samples with maximum allowed bit size
    const uint32_t max_number_of_frames_in_a_chunk = 8192;
    try {
        const FormatHeader &header = header_extensible.header;
        LameInit            lame_guard;  // Initializes lame on construction and closes it on destruction
                              // can be used as first argument of type lame_global_flags for all lame functions
        // configure lame according to the info in header
        if (!config_lame(lame_guard, in, message, header, list_info_chunk_meta_data)) {
            return;
        }
        // move to stream position where the data starts
        in->seekg(pcm_data_position.start, ios_base::beg);

        uint32_t max_number_of_samples_in_a_chunk = max_number_of_frames_in_a_chunk * header.num_channels;
        uint32_t bytes_per_sample                 = (header.bits_per_sample + 7) / 8;
        uint32_t residual_number_of_samples       = (uint32_t)pcm_data_position.data_size / bytes_per_sample;

        uint32_t number_of_samples = 0;
        while (residual_number_of_samples > 0) {
            number_of_samples = residual_number_of_samples > max_number_of_samples_in_a_chunk
                                    ? max_number_of_samples_in_a_chunk
                                    : residual_number_of_samples;
            if (header.audio_format == WAVE_FORMAT_PCM
                || (header.audio_format == WAVE_FORMAT_EXTENSIBLE
                    && header_extensible.sub_format == KSDATAFORMAT_SUBTYPE_PCM)) {  // PCM
                convert_pcm_int_chunk(lame_guard, in, out, number_of_samples, bytes_per_sample, header_extensible);
            } else if (header.audio_format == WAVE_FORMAT_IEEE_FLOAT
                       || (header.audio_format == WAVE_FORMAT_EXTENSIBLE
                           && header_extensible.sub_format == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {  // IEEE_FLOAT
                convert_ieee_float_chunk(lame_guard, in, out, number_of_samples, bytes_per_sample, header);

            } else {
                set_return_code(RET_CODE_CONVERTING_SOME_FILES_FAILED);
                throw runtime_error("unexpected error: unsupported audio format. Should have been checked by "
                                    "check_sane_pcm_or_ieee_float_format_header()");
            }
            residual_number_of_samples -= number_of_samples;
            // check after each converted chunk if the user pressed Ctrl-C (SIGINT) or SIGTERM was sent
            if (SignalHandler::termination_requested()) {
                remove_mp3_file();
                return;
            }
        }
        // formula found in the documentation of "lame_encode_buffer" in lame.h
        uint32_t                  mp3_buffer_size = (uint32_t)(1.25 * (double)number_of_samples + 7200.0);
        unique_ptr<unsigned char> mp3_buffer(new unsigned char[mp3_buffer_size]);

        // now retrieve any lingering mp3 data into the mp3 buffer and write it
        int bytes_converted = lame_encode_flush(lame_guard, mp3_buffer.get(), mp3_buffer_size);
        out->write((char *)mp3_buffer.get(), bytes_converted);

        // then report successful completion
        ostringstream ss;
        ss << OK_PREFIX << message << " converted" << endl;
        tcout << ss.str();
    } catch (const exception &e) {
        print_error(message, e.what());
        remove_mp3_file();
    } catch (...) {
        print_error(message, "unknown exception caught");
        remove_mp3_file();
    }
}

/*!
 * Checks if:
 *    - the passed "filename" exists and is readable
 *    - "filename" is a valid WAV audio file of the supported formats
 *    - opens "filename" as an input stream
 *    - creates the target MP3 file as an output stream
 *    - enqueue a call to convert_file_worker to the thread pool for the actual conversion
 */

static void convert_file(const fs::path &filename, ThreadPool &thread_pool) {
    ostringstream ss;
    try {
        std::uintmax_t       file_size = fs::file_size(filename);
        shared_ptr<ifstream> file(new ifstream());

        // open input file
        file->open(filename, ios::binary);
        if (file->fail()) {
            // only print an error if the file name ends with a .wav extension
            if (case_insensitive_compare(filename.extension().string(), ".wav")) {
                ss.str("");
                ss << ERROR_PREFIX << "Opening \"" << get_path_relative_to_top_level(filename)
                   << "\" failed, check permissions." << endl;
                tcerr << ss.str();
            }
        }

        // check if a valid RIFF file
        // first read all top level chunks. A valid RIFF file contains at least one "RIFF" chunk
        auto [top_level_chunks, message] = read_all_chunk_descriptors(*file, 0, file_size);
        ChunkDescriptorMap riff_chunks;
        tie(riff_chunks, message) = get_chunk_descriptors_of_all_subschunks_of_chunk_of_passed_id_and_format_type(
            *file, top_level_chunks, "RIFF", "WAVE");
        if (riff_chunks.empty()) {
            // only print an error if the file name ends with a .wav extension
            if (case_insensitive_compare(filename.extension().string(), ".wav")) {
                ss.str("");
                ss << ERROR_PREFIX << "\"" << get_path_relative_to_top_level(filename)
                   << "\" is not a valid RIFF file: " << message << endl;
                tcerr << ss.str();
            }
            return;
        }

        // now aggregate the sub-chunks of all "LIST" chunks with format type "INFO" present in the top level chunks
        // and in the sub-chunks of the RIFF chunk. The info in the "LIST" chunk contained as "RIFF" sub-chunks takes
        // precedence
        ChunkDescriptorMap meta_data;
        aggregate_meta_data(*file, top_level_chunks, meta_data, "LIST", "INFO");
        aggregate_meta_data(*file, riff_chunks, meta_data, "LIST", "INFO");
        // then add a chunk with "id3" to the meta_data, if present
        // Again an "id3"-chunk in the riff_chunks takes precedence over an "id3"-chunk
        // on the top-level.
        aggregate_id3_tags(top_level_chunks, meta_data);
        aggregate_id3_tags(riff_chunks, meta_data);
        // check if RIFF file is a valid WAV file with supported content
        FormatHeaderExtensible format_header;
        ChunkDescriptor        pcm_data_position;
        bool                   was_successful;
        tie(was_successful, format_header, pcm_data_position, message) = is_valid_wav_file(*file, riff_chunks);
        if (!was_successful) {
            ss.str("");
            ss << ERROR_PREFIX << "\"" << get_path_relative_to_top_level(filename)
               << "\" is not a valid WAV file: " << message << endl;
            tcerr << ss.str();
            return;
        }

        // create an output file (name chosen such that no existing file is overwritten)
        shared_ptr<ofstream> out_file(new ofstream());
        fs::path             out_filename;
        tie(was_successful, message) = open_output_stream(filename, message, *out_file, out_filename);

        // convert into MP3 file
        if (was_successful) {
            string error;
            using std::placeholders::_1;
            function<void(const std::uint16_t)> fct = bind(convert_file_worker, file, out_file, out_filename,
                                                           format_header, pcm_data_position, message, meta_data, _1);
            // submit actual conversion function to thread pool
            thread_pool.enqueue(fct);
        } else {
            ss.str("");
            ss << ERROR_PREFIX << message << endl;
            tcerr << ss.str();
        }

    } catch (const exception &e) {
        ss.str("");
        ss << ERROR_PREFIX << "converting \"" << get_path_relative_to_top_level(filename) << "\" failed:" << e.what()
           << endl;
        tcerr << ss.str();
    }
}

/*!
 * Iterates over all regular files in the folder referenced by the argument dir_iter and if
 * Configuration::recurse_directories() returns true also all its sub-folders
 */
void convert_all_wav_files_in_directory(fs::recursive_directory_iterator &dir_iter) {
    ostringstream ss;
    ThreadPool    thread_pool(Configuration::number_of_threads());
    int           ret_code = 0;
    try {
        auto entry_dir_name = dir_iter->path().parent_path().string();
        ss.str("");
        ss << "Converting WAV files to MP3 using quality level " << Configuration::encoding_quality()
           << " (0 highest, 9 lowest) in directory \"" << entry_dir_name << "\" ";
        if (Configuration::recurse_directories()) {
            ss << "and all its subdirectories ";
        }
        ss << "using " << Configuration::number_of_threads() << " threads." << endl;
        tcout << ss.str();
        string current_dir_name;
        for (const fs::directory_entry &entry : dir_iter) {
            // if the user sends SITERM or presses Ctrl-C (sending SIGINT),
            // abort processing
            if (SignalHandler::termination_requested()) {
                break;
            }
            if (fs::is_directory(entry)) {
                // if the --recursive flag is NOT set
                // disable the recursion so that only the files in the top level
                // directory are processed
                if (!Configuration::recurse_directories()) {
                    dir_iter.disable_recursion_pending();
                }
                continue;
            }
            // convert the file if it ends with .wav or if the command line options
            // -a/--all (convert_all_files()==true) has been passed
            if (!(Configuration::convert_all_files()
                  || case_insensitive_compare(entry.path().extension().string(), ".wav"))) {
                continue;
            }
            try {
                convert_file(entry.path(), thread_pool);
            } catch (const exception &e) {
                ss.str("");
                ss << ERROR_PREFIX << "processing file " << entry.path().filename() << " failed: " << e.what() << endl;
                tcerr << ss.str();
                set_return_code(RET_CODE_CONVERTING_SOME_FILES_FAILED);
            }
        }
    } catch (const exception &e) {
        ss.str("");
        ss << ERROR_PREFIX << "iterating over files failed: " << e.what() << endl;
        tcerr << ss.str();
        set_return_code(RET_CODE_DIR_ITER_FAILED);
    }
}
