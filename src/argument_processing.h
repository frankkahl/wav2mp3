//
// imports function "check_arguments" for checking validity of command line arguments
//

#ifndef ARGUMENT_PROCESSING_H
#define ARGUMENT_PROCESSING_H

// forward declaration instead of including <filesystem>
// to avoid unnecessary namespace pollution of all cpp files which include this header
namespace std {
	namespace filesystem {
		class recursive_directory_iterator;
	}
}

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
std::filesystem::recursive_directory_iterator check_arguments(int argc, const char* argv[]);

#endif // ARGUMENT_PROCESSING_H