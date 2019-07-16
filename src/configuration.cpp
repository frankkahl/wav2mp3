// defines class Configuration
// This class consists only of static members
// So that the configuration values are readily accessible from
// all parts of the program without needing to passing around instaces
// The Configuration::parse_arguments(...) static function
// allows to modify the default configuration parameters via command line options
// For command line parsing the header-only library cxxopts hosted onl GitHub:
//    https://github.com/jarro2783/cxxopts
// is used

#include "configuration.h"
#include <lame/lame.h>
#include <cstdint>
#include <cxxopts.hpp>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include "return_code.h"
#include "thread_includes.h"

using namespace std;
namespace fs = std::filesystem;

string Configuration::_version = WAV2MP3_VERSION;  // passed via -D compiler option
                                                   // by CMake-generated Makefile
string   Configuration::_name                   = fs::path(WAV2MP3_NAME).filename().string();
string   Configuration::_directory_path         = ".";
bool     Configuration::_recurse_directories    = RECURSE_DIRECTORIES;
int      Configuration::_encoding_quality       = ENCODING_QUALITY;
bool     Configuration::_overwrite_existing_mp3 = OVERWRITE_EXISTING_MP3;
bool     Configuration::_convert_all_files      = CONVERT_ALL_FILES;
uint16_t Configuration::_number_of_threads      = pthread::thread::hardware_concurrency();

// handles processing of command line arguments and setting the configuration parameters accordingly
// uses cxxopts to do the job
bool Configuration::parse_arguments(int argc, char *argv[]) {
    _name = fs::path(argv[0]).filename().string();
    cxxopts::Options options(_name, version() + ": converts all WAV files in passed directory to MP3");
    options.positional_help("directory");
    // clang-format off
    vector<string> superfluous_arguments;
    options.add_options()
        ("h,help", "print help")
        ("v,version", "print version")
        ("r,recursive", "recurse through all sub-directories",
          cxxopts::value<bool>(_recurse_directories))
        ("o,overwrite", "overwrite existing MP3 files instead of creating one with an alternative name not used yet",
          cxxopts::value<bool>(_overwrite_existing_mp3))
        ("q,quality", "set quality level of MP3 compression (integer between 0 (highest) and 9 (lowest))",
         cxxopts::value<int>(_encoding_quality)->default_value(std::to_string(_encoding_quality)))
        ("a,all", "try to convert all files, not only those with the extension .wav.", cxxopts::value<bool>(_convert_all_files))
        ("t,threads", "number of threads (maximum " + to_string(_number_of_threads) + ")",
         cxxopts::value<uint16_t>(_number_of_threads)->default_value(to_string(_number_of_threads)))
        ("directory", "root directory to search for WAV files", cxxopts::value<string>(_directory_path))
        ("superfluous", "", cxxopts::value<vector<string> >(superfluous_arguments));
    // clang-format on
    try {
        set_return_code(RET_CODE_INVALID_ARGUMENTS);
        options.parse_positional({"directory", "superfluous"});
        auto result = options.parse(argc, argv);
        if (result.count("help")) {
            cout << options.help({""}) << endl;
            set_return_code(RET_CODE_OK);
            return false;
        }
        if (result.count("version")) {
            cout << version() << endl;
            set_return_code(RET_CODE_OK);
            return false;
        }
        if (!result.count("directory")) {
            cerr << "ERROR: no directory passed" << endl;
            cerr << options.help({""}) << endl;
            return false;
        }
        if (superfluous_arguments.size()) {
            cerr << "ERROR: only one positional argument allowed." << endl;
            cerr << options.help({""}) << endl;
            return false;
        }
        if (_encoding_quality < 0 || _encoding_quality > 9) {
            cerr << "ERROR: encoding quality must be an integer between 0 and 9" << endl;
            cerr << options.help({""}) << endl;
            return false;
        }
        _number_of_threads = _number_of_threads < 1 ? 1 : _number_of_threads;  // at least one thread is necessary
        if (_number_of_threads > pthread::thread::hardware_concurrency()) {
            auto hardware_concurrency = pthread::thread::hardware_concurrency();
            cerr << "WARNING: Number of threads reduced from " << _number_of_threads << " to " << hardware_concurrency
                 << " (maximum number of concurrently running threads supported by hardware)" << endl;
            _number_of_threads = hardware_concurrency;
        }
    } catch (const cxxopts::OptionParseException &e) {
        cerr << "ERROR: " << e.what() << endl;
        cerr << options.help({""}) << endl;
        return false;
    }

    return true;
}

string Configuration::directory_path() {
    return _directory_path;
}

bool Configuration::recurse_directories() {
    return Configuration::_recurse_directories;
}

int Configuration::encoding_quality() {
    return Configuration::_encoding_quality;
}

bool Configuration::overwrite_existing_mp3() {
    return Configuration::_overwrite_existing_mp3;
}

bool Configuration::convert_all_files() {
    return Configuration::_convert_all_files;
}

uint16_t Configuration::number_of_threads() {
    return Configuration::_number_of_threads;
}

string Configuration::version() {
    ostringstream ss;
    ss << _name << " " << _version << " using lame " << get_lame_version() << ", ";
    ss << "cxxopts " << CXXOPTS__VERSION_MAJOR << "." << CXXOPTS__VERSION_MINOR << "." << CXXOPTS__VERSION_PATCH;
#ifdef USE_CPP11_THREADS
    ss << " and C++ native threading library";
#else
    ss << " and pthreads";
#endif
    return ss.str();
}