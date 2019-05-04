#include "convert_wav_files.h"

#include "wav_header.h"
#include "lame.h"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <exception>
#include <tuple>
#include <sstream>
#include <map>

namespace fs = std::filesystem;
using namespace std;


static tuple<bool, string> read_and_check_riff_section(ifstream &wav_file, const uintmax_t file_size) {
	riff_header header;
	stringstream ss; // for composing error message to return in case of failure
	if (file_size < sizeof(header)) {
		ss << "file too small for even the RIFF header";
		return make_tuple(false, ss.str());
	}
	wav_file.read((char *)&header, sizeof(header));
	if (wav_file.fail()) {
		ss << "Reading the RIFF header failed: this error should never happen from the humble perspective of the poor programmmer." << endl;
		return make_tuple(false, ss.str());
	}
	// check sanity of RIFF chunk first
	if (string(header.riff_id, sizeof(header.riff_id)) != "RIFF") {
		ss << "id of RIFF chunk does not contain \"RIFF\"";
		return make_tuple(false, ss.str());
	}
	if (header.total_data_size > file_size - sizeof(header.riff_id) - sizeof(header.total_data_size)) {
		ss << "SERIOUS ERROR: total file size is smaller than total data size the RIFF header claims to be present";
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
 * Returns a tuple with first component true if the passed wav_header structure plus the passed file size describes a sane WAV file.
 * If not returns a tuple of false and a string describing the error in format
 */
static tuple<bool, map<string, chunk_position>, string> is_valid_riff_file(ifstream &wav_file, const uintmax_t file_size) {
	stringstream ss;
	map<string, chunk_position> chunk_positions;
	auto[was_successful, message] = read_and_check_riff_section(wav_file, file_size); // better to unpack and the repack for return instead of
																					   // checking for the success code by the cryptic statement
																					   // std::get<0>(tuple)
	if (!was_successful) {
		return make_tuple(was_successful, chunk_positions, message);
	}


	// now inspect all chunks and store their positions and sizes
	was_successful = false;
	while (!wav_file.eof() || !wav_file.fail()) {
		char chunk_id_raw[4];
		uint32_t valid_data_size;
		wav_file.read(chunk_id_raw, sizeof(chunk_id_raw));					// first read FOURCC id of chunk
		wav_file.read((char *)&valid_data_size, sizeof(valid_data_size));  // then the size of valid data in the data block
		if (wav_file.fail()) {
			ss << "Reading chunk id and size failed.";
			break;
		}
		// the size of a data block is always padded to the nearest word boundary
		// => calculate this padded size from the size of the valid data frame
		uint32_t padded_data_size = valid_data_size / sizeof(uint16_t) * sizeof(uint16_t);
		if (valid_data_size % sizeof(uint16_t)) {
			padded_data_size += sizeof(uint16_t);
		}
		// only if advancing beyond the data is successful the chunk is considered to be valid
		wav_file.seekg(valid_data_size, ios_base::cur);
		if (wav_file.fail()) {
			ss << "Moving file pointer behind data of chunk failed.";
			break;
		}

		auto chunk_id = string(chunk_id_raw, sizeof(chunk_id_raw));
		struct chunk_position chunk_pos = { wav_file.tellg(), valid_data_size };
		if (chunk_positions.count(chunk_id)) {
			ss << "Multiple \"" << chunk_id << "\" chunks found, using the latest one. ";
		}
		chunk_positions[chunk_id] = chunk_pos;
		// now we advance by the number of padding bytes to the beginning of the next chunk
		wav_file.seekg(padded_data_size - valid_data_size, ios_base::cur);
	}

	if (wav_file.eof()) {
		was_successful = true;
	}
	return make_tuple(was_successful, chunk_positions, ss.str());
}

static tuple<bool, fmt_header, string> is_valid_wav_file()
	if (!chunk_positions.count("fmt ")) {
		ss << "no \"fmt \" chunks found";
		return make_tuple(false, ss.str());
	}
	if (chunk_positions["fmt "].data_size < sizeof(fmt_header)) {
		ss << "not enough bytes to read the format header";
		return make_tuple(false, ss.str());
	}
	wav_file.seekg(chunk_positions["fmt "].start);
	struct fmt_header fmt;
	wav_file.read((char *)&fmt, sizeof(fmt));

	// now check the format chunk
	// first for correct header string "fmt "
	auto fmt_header = string(header.format, sizeof(header.format));
	if (fmt_header != "fmt ") {
		ss << "id of format header contains \"" << fmt_header << "\" instead of \"fmt \"";
		return make_tuple(false, ss.str());
	}
	// only the audio format PCM is supported by lame, so check for it
	if (header.audio_format != 0x0001) {
		ss << "unsupported audio format \"";
		if (audio_format_names.count(header.audio_format)) {
			ss << audio_format_names[header.audio_format];
		}
		else {
			ss << hex << header.audio_format;
		}
		ss << "\" instead of PCM (0x0001)";
		return make_tuple(false, ss.str());
	}
	auto fmt_header = string(header.format, sizeof(header.format));
	if (fmt_header != "fmt ") {
		ss << "id of format header contains \"" << fmt_header << "\" instead of \"fmt \"";
		return make_tuple(false, ss.str());
	}

}

static tuple<bool, wav_header, const ifstream &> load_wav_file(const fs::path &wav_filename) {
	auto failed = make_tuple(false, wav_header(), ifstream());
	try {
		ifstream wav_file;
		wav_file.open(wav_filename, ios::binary);
		if (wav_file.fail()) {
			cerr << "ERROR: Opening \"" << wav_filename.string() << "\" failed, check permissions." << endl;
			return failed;
		}
		wav_header header;
		wav_file.read((char*)&header, sizeof(header));
		if (wav_file.eof() || wav_file.fail()) {
			cerr << "ERROR: Reading WAV header from \"" << wav_filename.string() << "\" failed." << endl;
			return failed;
		}
		if (string(header.riff_header, sizeof(header.riff_header)) != "RIFF") {
			cerr << "\"" << wav_filename.string() << "\" is not a WAV file." << endl;
			return failed;
		}
		return true;
	}
	catch (const exception &e) {
		cerr << "ERROR: Reading WAV header failed:" << e.what() << endl;
		return failed;
	}
}

bool convert_wav_file(const fs::path &wav_filename) {
	bool retval = true;
	return retval;
}

bool convert_all_wav_files_in_directory(const fs::recursive_directory_iterator &dir_iter) {
	try {
		string current_dir_name;
		for (const fs::directory_entry &entry : dir_iter) {
			if (fs::is_directory(entry)) {
				continue;
			}
			auto entry_dir_name = entry.path().parent_path().string();
			if (current_dir_name != entry_dir_name) {
				cout << "Processing directory \"" << entry_dir_name << "\"" << endl;
				current_dir_name = entry_dir_name;
			}
			try {
				cout << "Processing file: " << entry.path().filename() << endl;
				load_wav_file(entry.path());
			}
			catch (const exception &e) {
				cerr << "ERROR: Processing file " << entry.path().filename() << " failed: " << e.what() << endl;
			}
		}
	}
	catch (const exception &e) {
		cerr << "ERROR: Iterating over files in " << "" << " failed: " << e.what() << endl;
		return false;
	}
	return true;
}
