#include "wav2mp3.h"

// including project headers before system headers
// protects from forgetting necessary includes or erroneous forward declaration
#include "argument_processing.h"
#include "convert_wav_files.h"
#include "pthread.h"
#include "riff_format.h"
//#include "thread_pool.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

//int task(const std::uint16_t thread_number) {
//    std::ostringstream ss;
//    ss << "(" << thread_number << ") Start" << std::endl;
//    tcout << ss.str();
//    std::this_thread::sleep_for(2s);
//    ss.str("");
//    ss << "(" << thread_number << ") Stop after 2s " << std::endl;
//    tcout << ss.str();
//    if (thread_number == 3) {
//        throw std::exception("TestException");
//    }
//    return thread_number * 100;
//}

using namespace std;
namespace fs = std::filesystem;

int main(int argc, const char* argv[]) {
    // Guid_ a;
    // cout << sizeof(a) << endl;
    // cout <<
    // sizeof(a.time_low)+sizeof(a.time_mid)+sizeof(a.time_hi_and_version)+sizeof(a.clock_seq)+sizeof(a.node_high)+sizeof(a.node_low)
    // << endl; Guid default; Guid guid1("abc023F9-ff0e-12e0-fffe-0134fa0b1123"); Guid
    // guid2("abc023F9-ff0e-12e0-fffe-0134fa0b1123"); cout << guid1.string() << endl; cout << guid2 << endl; cout <<
    // (guid1 == guid2) << endl; cout << guid1.is_empty() << endl; cout << default.is_empty() << endl; cout <<
    // sizeof(Guid) << endl; return 0;
    setlocale(LC_ALL, "");  // switch everything from the minimal "C" locale to the environments default locale to
                            // ensure that all diacritical letters like Umlaute are displayed properly under Windows
    //ThreadPool<int> tp;
    //std::vector<std::future<int> > futures;
    //for (int i = 0; i < 12; i++) {
    //    futures.push_back(tp.enqueue(&task));
    //}
    //for (std::size_t i = 0; i < futures.size(); i++) {
    //    try {
    //        std::ostringstream ss;
    //        ss << "(main) future # " << i << ": " << futures[i].get() << std::endl;
    //        tcout << ss.str();
    //    } catch (const std::exception& exc) {
    //        std::ostringstream ss;
    //        ss << "(main) future # " << i << ": exception thrown: " << exc.what() << std::endl;
    //        tcerr << ss.str();
    //    } catch (...) {
    //        std::ostringstream ss;
    //        ss << "(main) future # " << i << ": exception thrown" << std::endl;
    //        tcerr << ss.str();
    //    }
    //}
    //return 0;
    auto& dir_iter = check_arguments(argc, argv);
    if (dir_iter == fs::end(dir_iter)) {
        return -1;
    }
    bool res = convert_all_wav_files_in_directory(dir_iter);
    return 0;
}
