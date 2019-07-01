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

#include "lame_wrapper.h"
#include "riff_format.h"
#include "signal_handler.h"
#include "thread_pool.h"
#include "tiostream.h"
#include "return_code.h"

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

namespace fs = std::filesystem;
using namespace std;

// helper type storing both the start offset and the size of the data payload
// of a chunk
typedef struct ChunkPosition {
    std::streampos start;           // position to pass to ifstream::seekg(...) to move file pointer to beggining of chunk data
    std::streamsize data_size = 0;  // size of chunk data.
} ChunkPosition;

// helper type mapping the chunk id to the position information of its data
typedef std::map<std::string, ChunkPosition> ChunkPositionMap;

/*!
 *   Tries to read the the RIFF master header (see RiffHeader in riff_format.h)
 *   returns: if successful: tuple(true, <empty string>)
 *            on failure: tuple(false, <error message>)
 */
static tuple<bool, string> read_and_check_riff_section(ifstream &file, const uintmax_t file_size) {
    RiffHeader header;
    stringstream ss;  // for composing error message to return in case of failure
    if (file_size < sizeof(header)) {
        ss << "file too small for even the RIFF header";
        return make_tuple(false, ss.str());
    }
    file.read((char *)&header, sizeof(header));
    if (file.fail()) {
        ss << "Reading the RIFF header failed: this error should never happen from the humble perspective of the poor programmmer." << endl;
        return make_tuple(false, ss.str());
    }
    // check sanity of RIFF chunk first
    if (string(header.riff_id, sizeof(header.riff_id)) != "RIFF") {
        ss << "FOURCC id of first chunk is not \"RIFF\"";
        return make_tuple(false, ss.str());
    }
    if (header.total_data_size > file_size - sizeof(header.riff_id) - sizeof(header.total_data_size)) {
        ss << "total file size is smaller than total data size the RIFF header claims to be present";
        return make_tuple(false, ss.str());
    }
    auto format = string(header.format, sizeof(header.format));
    if (format != "WAVE") {
        ss << "unsupported format \"" << format << "\" instead of \"WAVE\"";
        return make_tuple(false, ss.str());
    }
    return make_tuple(true, string());
}

/*!
 * Checks if the passed filestream points to a valid RIFF file.
 * Returns a tuple of:
 *     - ChunkPositionMap: maps the FOURCC chunk ids of all valid chunks found to
 *       the positions and sizes of their data blocks.
 *       If empty then either the file is not a RIFF file or it is corrupt.
 *     - string: is empty if everything went fine, otherwise it contains a warning or error message.
 */
static tuple<ChunkPositionMap, string> is_valid_riff_file(ifstream &file, const uintmax_t file_size) {
    stringstream ss;
    ChunkPositionMap chunk_positions;
    auto [was_successful, message] = read_and_check_riff_section(file, file_size);  // better to unpack and the repack for return instead of
                                                                                    // checking for the success code by the cryptic statement
                                                                                    // std::get<0>(tuple)
    if (!was_successful) {
        return make_tuple(chunk_positions, message);
    }

    // now inspect all chunks and store their positions and sizes
    while (!(file.eof() || file.fail())) {
        char chunk_id_raw[4];
        uint32_t valid_data_size;

        file.read(chunk_id_raw, sizeof(chunk_id_raw));                 // first read FOURCC id of chunk
        file.read((char *)&valid_data_size, sizeof(valid_data_size));  // then the size of valid data in the data block
        if (file.eof() || file.fail()) {
            ss << "Reading chunk id and size failed.";
            break;
        }
        auto chunk_id = string(chunk_id_raw, sizeof(chunk_id_raw));
        // store start of data position in ChunkPosition struct first
        // but do NOT store it yet...
        ChunkPosition chunk_pos = {file.tellg(), valid_data_size};

        // ... first advance pointer to the position of the last byte of the valid data block according to valid_data_size
        file.seekg(valid_data_size - 1, ios_base::cur);
        // then peek for the byte to enforce the eofbit or failbit to be set if not enough data is available
        file.peek();
        if (file.eof() || file.fail()) {
            ss << "less data available as claimed in chunk \"" << chunk_id << "\"";
            break;
        }
        // only if the claimed data is really there consider the chunk valid
        // an add it to the chunk_positions
        if (chunk_positions.count(chunk_id)) {
            ss << "multiple \"" << chunk_id << "\" chunks found, using the latest one. ";
        }
        chunk_positions[chunk_id] = chunk_pos;

        // the size of a data block is always padded to the nearest word boundary
        // => calculate this padded size from the size of the valid data frame
        uint32_t padded_data_size = valid_data_size / sizeof(uint16_t) * sizeof(uint16_t);
        if (valid_data_size % sizeof(uint16_t)) {
            padded_data_size += sizeof(uint16_t);
        }

        // now forward by the number of padding bytes plus 1 to set the pointer to the beginning of the next chunk
        // adding 1 is needed since the pointer currently points to the last byte of the current data block,
        // not one position behind it
        file.seekg(padded_data_size - valid_data_size + 1, ios_base::cur);
        file.peek();  // if peeking the next byte just sets the eofbit without setting the failbit or badbit
                      // then the RIFF file is well formed, so no warning or error message is set
        if (file.eof() && !file.fail()) {
            break;
        }
    }
    // cout << "--- found chunks: ";
    // for (auto const &[key, value] : chunk_positions) {
    //    cout << '"' << key << '"' << "  ";
    //}
    // cout << endl;
    return make_tuple(chunk_positions, ss.str());
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
static tuple<bool, string> check_sane_pcm_or_ieee_float_format_header(const FormatHeaderExtensible &header_extensible) {
    stringstream ss;
    const FormatHeader &header = header_extensible.header;

    ss << right << setfill('0') << setw(4) << hex;  // switch to an output style suited for FOURCC values
    if (!(header.audio_format == WAVE_FORMAT_PCM || header.audio_format == WAVE_FORMAT_IEEE_FLOAT || header.audio_format == WAVE_FORMAT_EXTENSIBLE)) {
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
    if (header.audio_format == WAVE_FORMAT_EXTENSIBLE &&
        !(header_extensible.sub_format == KSDATAFORMAT_SUBTYPE_PCM || header_extensible.sub_format == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {
        ss << "unsupported sub-format in WAVE_FORMAT_EXTENSIBLE header ";
        if (audio_format_guid_to_names.count(header_extensible.sub_format)) {
            ss << "\"" << audio_format_guid_to_names[header_extensible.sub_format] << "\" ({" << header_extensible.sub_format.string() << "})";
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
    if (header.audio_format == WAVE_FORMAT_PCM || (header.audio_format == WAVE_FORMAT_EXTENSIBLE && header_extensible.sub_format == KSDATAFORMAT_SUBTYPE_PCM)) {
        if (bps % 8) {
            ss << setfill(' ') << setw(0) << dec;
            ss << "bits per sample of " << bps << " illegal for \"PCM\"; should always be a multiple of 8 with a maximum of " << 8 * sizeof(int32_t);
            bps = (bps + 7) / 8 * 8;  // correct container size to the next larger multiple of 8
            ss << ", adjusting to " << bps << "bits per sample";
        }
        if (bps > 8 * sizeof(int32_t)) {
            ss << setfill(' ') << setw(0) << dec;
            ss << "bits per sample of " << bps << " illegal for \"PCM\"; must be <= 32";
        }
    }
    if (header.audio_format == WAVE_FORMAT_IEEE_FLOAT ||
        (header.audio_format == WAVE_FORMAT_EXTENSIBLE && header_extensible.sub_format == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {
        if (!(bps == 8 * sizeof(float) || bps == 8 * sizeof(double))) {
            ss << "bits per sample of " << dec << bps << " illegal for \"IEEE FLOAT\"; must be 32 or 64";
            return make_tuple(false, ss.str());
        }
    }
    if (header.audio_format == WAVE_FORMAT_EXTENSIBLE && header_extensible.sub_format == KSDATAFORMAT_SUBTYPE_PCM) {
        if (header_extensible.samples.valid_bits_per_sample > bps) {
            ss << "the valid bits per sample (" << header_extensible.samples.valid_bits_per_sample << ") in the extensible part of WAVE_FORMAT_EXTENSIBLE "
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
 *  Checks if the chunks in chunk_positions form a valid WAV file.
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
 *                                <position and length of PCM data as ChunkPosition>,
 *								  <info string>)
 *				<info_string>: describes found audio data, e.g. "41.0 kHz, 16 bit, stereo"
 *                             should be used for info output to the user.
 *           on failure:    tuple(false, <undefined format_header>,
 *                                <undefined ChunkPosition object>, <error_message>)
 */
static tuple<bool, FormatHeaderExtensible, ChunkPosition, string> is_valid_wav_file(ifstream &file, ChunkPositionMap &chunk_positions) {
    stringstream ss;
    FormatHeaderExtensible format_header;
    ChunkPosition data_chunk_payload;
    // check first if there is a "fmt " chunk
    if (!chunk_positions.count("fmt ")) {
        ss << "no \"fmt \" chunk found";
        return make_tuple(false, format_header, data_chunk_payload, ss.str());
    }
    // then make sure there is a data chunk
    if (!chunk_positions.count("data")) {
        ss << "no \"data\" chunk found";
        return make_tuple(false, format_header, data_chunk_payload, ss.str());
    }
    auto size = sizeof(format_header);
    auto data_size = chunk_positions["fmt "].data_size;
    // and if it contains enough data to be the format header we expect
    if (chunk_positions["fmt "].data_size < sizeof(FormatHeader)) {
        ss << "not enough bytes to read the base format header";
        return make_tuple(false, format_header, data_chunk_payload, ss.str());
    }
    // then move pointer to start of data
    // and read it into the FormatHeader struct
    file.seekg(chunk_positions["fmt "].start);
    file.read((char *)&format_header.header, sizeof(format_header.header));
    if (format_header.header.audio_format == WAVE_FORMAT_EXTENSIBLE) {
        if (chunk_positions["fmt "].data_size < sizeof(FormatHeaderExtensible)) {
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
    // with PCM content
    // so return the validated FormatHeader structure and the position and length
    // of the PCM data in the stream in a ChunkPosition structure
    return make_tuple(true, format_header, chunk_positions["data"], info_string);
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
 *  Creates the MP3 file and associates it with the passed stream "out_file".
 *  The path of the MP3 file is generated using the following rules:
 *     - if in_file_name has a ".wav" file ending replace it by ".mp3",
 *       if not then just add ".mp3"
 *       The resulting path is referred to as "<mp3_pathname_base>.mp3"
 *     - if the path "<mp3_pathname_base>.mp3" already exists,
 *       then use "<mp3_pathname_base> (1).mp3",
 *       if that file already exists
 *       then use "<mp3_pathname_base> (2).mp3" and so on
 *  if successful:
 *       - "out_file" is a valid open stream
 *       - return: tuple(true, <conversion info string>)
 *         where <conversion info string> has the form:
 *           line 1 (if the case) :     "WARNING: <in_file_name> does not end with .wav.\n"
 *           line 2               :     "<basename of in_file_name> (<in_file_info>) -> <basename of <mp3_pathname_base> (<n>).mp3
 *  on failure:
 *       - "out_file" is closed and invalid
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
static tuple<bool, string> open_output_stream(const fs::path &in_file_name, const string &in_file_info, ofstream &out_file, fs::path &mp3_path) {
    ostringstream ss;
    ostringstream status_line;
    out_file.close();  // just in case there is still a file associated to this stream
    fs::path mp3_path_base;
    if (case_insensitive_compare(in_file_name.extension().string(), ".wav")) {
        mp3_path_base = in_file_name.parent_path() / in_file_name.stem();
    } else {
        status_line << "WARNING: \"" << in_file_name.filename().string() << "\" does not end with \".wav\" but is a valid WAV file." << endl << "\t";
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
            // as soon as a filename does not exist yet
            if (!fs::exists(mp3_path)) {
                // try to open it with the passed ofstream object
                out_file.open(mp3_path, ios::binary | ios::trunc | ios::out);
                // on failure abort. Most likely the proces has no write permissions
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
    status_line << "\"" << in_file_name.filename().string() << "\"\t";
    status_line << " (" << in_file_info << ") \t-> ";
    status_line << "\"" << mp3_path.filename().string() << "\"\t";
    return make_tuple(true, status_line.str());
}

// this function does the actual conversion work and is being executed in one of the threads of the thread pool
static tuple<bool, string> convert_file_worker(shared_ptr<ifstream> in, shared_ptr<ofstream> out, const fs::path out_filename,
                                               const FormatHeaderExtensible header_extensible, const ChunkPosition pcm_data_position, string message,
                                               uint16_t thread_number) {
    string error;
    const FormatHeader &header = header_extensible.header;
    in->seekg(pcm_data_position.start, ios_base::beg);
    // define lambda expression to avoid repetetive code for reporting
    // errors
    auto print_error = [&message, thread_number](const string &error) {
        ostringstream ss;
        ss << "\t (" << thread_number << ") " << message << "failed: " << endl;
        ss << "\t\t " << error << endl;
        tcerr << ss.str();
        set_return_code(RET_CODE_CONVERTING_SOME_FILES_FAILED);
    };
    LameInit lame_guard;  // Initializes lame on construction and closes it on destruction
                          // can be used as first argument of type lame_global_flags for all lame functions
    if (!lame_guard.is_initialized()) {
        error = "lame_init() failed";
        print_error(error);
        return make_tuple(false, error);
    }
    lame_set_num_channels(lame_guard, header.num_channels);
    lame_set_in_samplerate(lame_guard, header.samples_per_second);
    lame_set_mode(lame_guard, header.num_channels == 2 ? JOINT_STEREO : MONO);
    lame_set_quality(lame_guard, 5); /* 2=high  5 = medium  7=low */
    int ret_code = lame_init_params(lame_guard);
    if (ret_code == -1) {
        error = "lame_init_params() failed";
        print_error(error);
        set_return_code(RET_CODE_LAME_ERROR);
        return make_tuple(false, error);
    }
    // allocate input buffer for 8192 samples with maximum allowed bit size
    const uint32_t number_of_samples = 8192;
    uint32_t pcm_buffer_size = number_of_samples * header.num_channels;
    // then allocate the output buffer
    // formula for the worst case size I found on the internet
    uint32_t mp3_buffer_size = (uint32_t)(1.25 * (double)number_of_samples + 7200.0);
    unique_ptr<unsigned char> mp3_buffer(new unsigned char[mp3_buffer_size]);

    uint32_t valid_bits_per_sample;
    if (header.audio_format == WAVE_FORMAT_EXTENSIBLE) {
        valid_bits_per_sample = header_extensible.samples.valid_bits_per_sample;
    } else {
        valid_bits_per_sample = header.bits_per_sample;
    }

    uint32_t bytes_per_sample = (header.bits_per_sample + 7) / 8;
    uint32_t residual_number_of_samples = (uint32_t)pcm_data_position.data_size / bytes_per_sample;
    int bytes_converted;
    while (residual_number_of_samples > 0) {
        uint32_t read_data = residual_number_of_samples > pcm_buffer_size ? pcm_buffer_size : residual_number_of_samples;
        if (header.audio_format == WAVE_FORMAT_PCM ||
            (header.audio_format == WAVE_FORMAT_EXTENSIBLE && header_extensible.sub_format == KSDATAFORMAT_SUBTYPE_PCM)) {  // PCM
            unique_ptr<int32_t> pcm_buffer(new int32_t[pcm_buffer_size]);
            for (uint32_t i = 0; i < read_data; i++) {
                int32_t c = 0;
                in->read((char *)&c, bytes_per_sample);
                if (bytes_per_sample == 1) {
                    c -= 128;
                }
                c <<= (sizeof(int32_t) * 8 - valid_bits_per_sample);  // expands values to the full range of int32_t
                pcm_buffer.get()[i] = c;
            }
            if (header.num_channels == 2) {
                bytes_converted = lame_encode_buffer_interleaved_int(lame_guard, pcm_buffer.get(), read_data / 2, mp3_buffer.get(), mp3_buffer_size);
            } else {
                bytes_converted = lame_encode_buffer_int(lame_guard, pcm_buffer.get(), 0, read_data, mp3_buffer.get(), mp3_buffer_size);
            }

        } else if (header.audio_format == WAVE_FORMAT_IEEE_FLOAT ||
                   (header.audio_format == WAVE_FORMAT_EXTENSIBLE && header_extensible.sub_format == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)) {  // IEEE_FLOAT
            unique_ptr<double> pcm_buffer(new double[pcm_buffer_size]);
            for (uint32_t i = 0; i < read_data; i++) {
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
                        err << "unexpected error: illegal bits per sample value " << header.bits_per_sample << " for \"IEEE FLOAT\" format";
                        print_error(err.str());
                        return make_tuple(false, err.str());
                }
                pcm_buffer.get()[i] = c;
            }
            if (header.num_channels == 2) {
                bytes_converted = lame_encode_buffer_interleaved_ieee_double(lame_guard, pcm_buffer.get(), read_data / 2, mp3_buffer.get(), mp3_buffer_size);
            } else {
                bytes_converted = lame_encode_buffer_ieee_double(lame_guard, pcm_buffer.get(), 0, read_data, mp3_buffer.get(), mp3_buffer_size);
            }

        } else {
            error = "unexpected error: unsupported audio format. Should have been checked by check_sane_pcm_or_ieee_float_format_header()";
            print_error(error);
            return make_tuple(false, error);
        }
        residual_number_of_samples -= read_data;
        out->write((char *)mp3_buffer.get(), bytes_converted);
        // check after each converted chunk if the user pressed Ctrl-C (SIGINT) or SIGTERM was sent
        if (SignalHandler::termination_requested()) {
            out->close();
            std::error_code ec;
            fs::remove(out_filename, ec);  // try to delete the incomplete MP3 file, but do not make a fuzz about failing
            return make_tuple(false, error);
        }
    }
    bytes_converted = lame_encode_flush(lame_guard, mp3_buffer.get(), mp3_buffer_size);
    out->write((char *)mp3_buffer.get(), bytes_converted);

    ostringstream ss;
    ss << "\t (" << thread_number << ") " << message << " converted" << endl;
    tcout << ss.str();
    return make_tuple(true, string());
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
        std::uintmax_t file_size = fs::file_size(filename);
        shared_ptr<ifstream> file(new ifstream());

        // open input file
        file->open(filename, ios::binary);
        if (file->fail()) {
            // only print an error if the file name ends with a .wav extension
            if (case_insensitive_compare(filename.extension().string(), ".wav")) {
                ss.str("");
                ss << "\t (main) ERROR: Opening \"" << filename.filename().string() << "\" failed, check permissions." << endl;
                tcerr << ss.str();
            }
        }

        // check if a valid RIFF file
        auto [chunk_positions, message] = is_valid_riff_file(*file, file_size);

        if (chunk_positions.empty()) {
            // only print an error if the file name ends with a .wav extension
            if (case_insensitive_compare(filename.extension().string(), ".wav")) {
                ss.str("");
                ss << "\t (main) ERROR: \"" << filename.filename().string() << "\" is not a valid RIFF file: " << message << endl;
                tcerr << ss.str();
            }
        }

        // check if RIFF file is a valid WAV file with supported content
        FormatHeaderExtensible format_header;
        ChunkPosition pcm_data_position;
        bool was_successful;
        tie(was_successful, format_header, pcm_data_position, message) = is_valid_wav_file(*file, chunk_positions);
        if (!was_successful) {
            ss.str("");
            ss << "\t (main) \"" << filename.filename().string() << "\" is not a valid WAV file: " << message << endl;
            tcerr << ss.str();
        }

        // create an output file (name chosen such that no existing file is overwritten)
        shared_ptr<ofstream> out_file(new ofstream());
        fs::path out_filename;
        tie(was_successful, message) = open_output_stream(filename, message, *out_file, out_filename);

        // convert into MP3 file
        if (was_successful) {
            string error;
            using std::placeholders::_1;
            function<tuple<bool, string>(const std::uint16_t)> fct =
                bind(convert_file_worker, file, out_file, out_filename, format_header, pcm_data_position, message, _1);
            // submit actual conversion function to thread pool
            thread_pool.enqueue(fct);
        } else {
            ss.str("");
            ss << "\t (main) " << message << endl;
            tcerr << ss.str();
        }

    } catch (const exception &e) {
        ss.str("");
        ss << "\t (main) ERROR: converting \"" << filename.filename().string() << "\" failed:" << e.what() << endl;
        tcerr << ss.str();
    }
}

/*!
 * Iterates over all regular files in the folder and all its sub-folders referenced by
 * the argument dir_iter
 */
void convert_all_wav_files_in_directory(const fs::recursive_directory_iterator &dir_iter) {
    ostringstream ss;
    ThreadPool thread_pool;
    int ret_code = 0;
    try {
        string current_dir_name;
        for (const fs::directory_entry &entry : dir_iter) {
            // if the user sends SITERM or presses Ctrl-C, sending SIGINT,
            // abort processing
            if (SignalHandler::termination_requested()) {
                break;
            }
            if (fs::is_directory(entry)) {
                continue;
            }
            auto entry_dir_name = entry.path().parent_path().string();
            if (current_dir_name != entry_dir_name) {
                ss.str("");
                ss << "Processing directory \"" << entry_dir_name << "\"" << endl;
                tcout << ss.str();
                current_dir_name = entry_dir_name;
            }
            try {
                convert_file(entry.path(), thread_pool);
            } catch (const exception &e) {
                ss.str("");
                ss << "ERROR: Processing file " << entry.path().filename() << " failed: " << e.what() << endl;
                tcerr << ss.str();
                set_return_code(RET_CODE_CONVERTING_SOME_FILES_FAILED);
            }
        }
    } catch (const exception &e) {
        ss.str("");
        ss << "ERROR: Iterating over files failed: " << e.what() << endl;
        tcerr << ss.str();
        set_return_code(RET_CODE_DIR_ITER_FAILED);
    }
}
