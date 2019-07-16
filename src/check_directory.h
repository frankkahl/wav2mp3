//
// imports function "check_directory" for checking if a given path does exist, is a directory and is accessible
//

#ifndef CHECK_DIRECTORY_H
#define CHECK_DIRECTORY_H

#include <filesystem>  // forward declarations could be used here but they can be very error prone, see:
                       // https://google.github.io/styleguide/cppguide.html#Forward_Declarations

/*!
 * Used for checking the command line arguments passed to main()
 * If the criteria
 *     - exactly one argument is passed
 *     - the argument has a valid path syntax and points to a file system object which
 *         - exist
 *	       - is a directory
 *         - its permissions allow this process to open it
 * are met, a valid directory_iterator is returned.
 * in case of failure an empty directory_iterator is returned.
 * Status and error messages are written to cout and cerr, respectively.
 */
std::filesystem::recursive_directory_iterator check_directory(const std::string &passed_path);

#endif  // CHECK_DIRECTORY_H