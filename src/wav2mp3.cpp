// including project headers before system headers
// protects from forgetting necessary includes or erroneous forward declaration

#include "check_directory.h"
#include "configuration.h"
#include "convert_wav_files.h"
#include "riff_format.h"

#include "return_code.h"
#include "signal_handler.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;
namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    try {
        SignalHandler sig_handler;  // the construction of this instance installs the handler functions
                                    // for SIGINT and SIGTERM
                                    // the destruction restores the default handlers
        setlocale(LC_ALL, "");  // switch everything from the minimal "C" locale to the environments default locale to
                                // ensure that all diacritical letters like Umlaute are displayed properly under Windows
        if (!Configuration::parse_arguments(argc, argv)) {  // now parse the command line arguments
            return get_return_code();                       // and set the configuration parameters accordingly
        }
        auto dir_iter = check_directory(Configuration::directory_path());  // check if the passed directory exists,
                                                                           // is a directory and is accessible
        if (dir_iter == fs::end(dir_iter)) {
            return RET_CODE_DIR_ITER_FAILED;
        }
        convert_all_wav_files_in_directory(dir_iter);  // now convert all WAV files in the directory
    } catch (const std::exception& e) {
        cerr << "Aborting after exception: " << e.what();
        set_return_code(RET_CODE_EXCEPTION_CAUGHT);
    } catch (...) {
        cerr << "ERROR: Aborting after unknown exception." << endl;
        set_return_code(RET_CODE_EXCEPTION_CAUGHT);
    }
    return get_return_code();
}
