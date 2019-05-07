#include "wav2mp3.h"

// including project headers before system headers
// protects from forgetting necessary includes or erroneous forward declaration
#include "convert_wav_files.h"
#include "argument_processing.h"
#include "pthread.h"

#include <string>
#include <filesystem>
#include <fstream>
#include <cstdint>

using namespace std;
namespace fs = std::filesystem;


int main(int argc, const char* argv[])
{
	setlocale(LC_ALL, "");	// switch everything from the minimal "C" locale to the environments default locale to ensure that
	                        // all diacritical letters like Umlaute are displayed properly under Windows

	auto &dir_iter = check_arguments(argc, argv);
	if (dir_iter == fs::end(dir_iter)) {
		return -1;
	}
	bool res = convert_all_wav_files_in_directory(dir_iter);
	return 0;
}
