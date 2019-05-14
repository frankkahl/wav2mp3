#include "argument_processing.h"  // including its own header file is a good practice to reveal
                                  // signature imcompatibilies between cpp and h file right away

#include <cstring>
#include <exception>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <tuple>

using namespace std;
namespace fs = filesystem;

/*!
 * Checks if the passed string is a valid. A path is valid if it meets the syntactical requirements for a path
 * and the file system object it points to
 *    - exists
 *	  - is a directory
 *    - its permissions allow this process to open it
 * return: empty directory_iterator on failure, valid directory_iterator otherwise
 * remark: this function is declared static since it is used only inside this file
 */
static fs::recursive_directory_iterator is_directory_valid(const string &passed_path) {
  auto failed = fs::recursive_directory_iterator();  // if function fails return this empty directory_iterator
  try {
    auto path = fs::path(passed_path);
    // if the path is relative and does not begin with a path separator
    // then convert it to a normalized relative path (e.g "test\foo\bar\..\.." is normalized to "test"
    // Being relative and begin with a path separator can happen only under Windows when the dreaded drive letter is
    // omitted In this case convert it to a normalized absolute path
    if (path.is_relative() && (path.lexically_normal().wstring()[0] != fs::path::preferred_separator)) {
      path = fs::proximate(
          path);  // relative() would convert a path without any separators to an empty string, so proximate() is used
    } else {
      path = fs::absolute(path);
    }
    // now check existance
    if (!fs::exists(path)) {
      cerr << "ERROR: \"" << path.string() << "\" does not exist" << endl;
      return failed;
    }
    // and if it is a directory
    if (!fs::is_directory(path)) {
      cerr << "ERROR: \"" << path.string() << "\" is not a directory" << endl;
      return failed;
    }
    // then construct a directory iterator
    // This construction can still fail with an exception of filesystem_error thrown
    // if we have no permission to open the directory
    auto dir_iter = fs::recursive_directory_iterator(path);
    // just print a warning if a valid but empty directory has been passed
    if (dir_iter == fs::end(dir_iter)) {
      cout << "WARNING: \"" << path.string() << "\" is empty, nothing to convert." << endl;
    }
    return dir_iter;
  } catch (const fs::filesystem_error &e) {
    cerr << "ERROR: Validating \"" << passed_path << "\" failed: " << e.code().message() << endl;
    return failed;
  }
  // just in case another exception is thrown
  catch (const exception &e) {
    cerr << "ERROR: Validating \"" << passed_path << "\" failed: " << e.what() << endl;
    return failed;
  }
}

// see header file for documentation
fs::recursive_directory_iterator check_arguments(int argc, const char *argv[]) {
  // create help message to be printed if an illegal number or type of arguments have been passed
  ostringstream oss;
  oss << "usage: " << fs::path(argv[0]).filename() << " <directory path>" << endl;
  oss << "\tconverts all WAV files in <directory path> to MP3" << endl;
  auto help_message = oss.str();

  // now check argument count
  switch (argc) {
    case 1:  // no additional argument besides the executable path passed => print help
      cout << help_message;
      break;
    case 2:  // exactly one argument passed => return the result of the validity check
      return is_directory_valid(string(argv[1]));
      break;
    default:  // too many arguments passed => print error and help messages
      cerr << "ERROR: pass a valid directory as only argument." << endl;
      cout << help_message;
  }
  return fs::recursive_directory_iterator();
}
