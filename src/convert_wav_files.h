//
// contains function convert_all_wav_files_in_directory for converting all WAV files in a directory to MP3 files
//

#ifndef CONVERT_WAV_FILES_H
#define CONVERT_WAV_FILES_H

// forward declaration instead of including <filesystem>
// to avoid unnecessary namespace pollution of cpp files including this header
namespace std {
	namespace filesystem {
		class recursive_directory_iterator;
	}
}

/*!
 * convert all WAV files in the directory the passed iterator points to into MP3 files
 * return: true if successful, false otherwise
 */
bool convert_all_wav_files_in_directory(const std::filesystem::recursive_directory_iterator &dir_iter);

#endif // CONVERT_WAV_FILES_H
