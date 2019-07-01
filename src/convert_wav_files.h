//
// contains function convert_all_wav_files_in_directory for converting all WAV files in a directory to MP3 files
//

#ifndef CONVERT_WAV_FILES_H
#define CONVERT_WAV_FILES_H

#include <filesystem>   // forward declarations could be used here but they can be very error prone, see:
                        // https://google.github.io/styleguide/cppguide.html#Forward_Declarations


/*!
 * convert all WAV files in the directory the passed iterator points to into MP3 files
 * return: true if successful, false otherwise
 */
void convert_all_wav_files_in_directory(const std::filesystem::recursive_directory_iterator &dir_iter);

#endif // CONVERT_WAV_FILES_H
