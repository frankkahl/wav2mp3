#include "convert_wav_files.h"
#include "wav_header.h"

#include "lame.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <exception>

namespace fs = std::filesystem;
using namespace std;


bool load_wav_file(const fs::path &wav_filename) {
	try {
		ifstream wav_file;
		wav_file.open(wav_filename, ios::binary);
		if (wav_file.fail()) {
			cerr << "ERROR: Opening \"" << wav_filename.string() << "\" failed, check permissions." << endl;
			return false;
		}
		wav_header header;
		wav_file.read((char*)&header, sizeof(header));
		if (wav_file.eof() || wav_file.fail()) {
			cerr << "ERROR: Reading WAV header from \"" << wav_filename.string() << "\" failed." << endl;
			return false;
		}
		if (string(header.riff_header, sizeof(header.riff_header)) != "RIFF") {
			cerr << "\"" << wav_filename.string() << "\" is not a WAV file." << endl;
			return false;
		}
		return true;
	}
	catch (const exception &e) {
		cerr << "ERROR: Reading WAV header failed:" << e.what() << endl;
		return false;
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
