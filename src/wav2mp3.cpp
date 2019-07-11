// including project headers before system headers
// protects from forgetting necessary includes or erroneous forward declaration
#include "argument_processing.h"
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

int main(int argc, const char* argv[]) {
    try {
        SignalHandler sig_handler;  // the construction of this instance installs the handler functions
                                    // for SIGINT and SIGTERM
                                    // the destruction restores the default handlers
        setlocale(LC_ALL, "");      // switch everything from the minimal "C" locale to the environments default locale to
                                    // ensure that all diacritical letters like Umlaute are displayed properly under Windows
        auto dir_iter = check_arguments(argc, argv);
        if (dir_iter == fs::end(dir_iter)) {
            return RET_CODE_DIR_ITER_FAILED;
        }
        convert_all_wav_files_in_directory(dir_iter);
    } catch (const std::exception& e) {
        cerr << "Aborting after exception: " << e.what();
        set_return_code(RET_CODE_EXCEPTION_CAUGHT);
    } catch (...) {
        cerr << "ERROR: Aborting after unknown exception." << endl;
        set_return_code(RET_CODE_EXCEPTION_CAUGHT);
    }
    return get_return_code();
}
