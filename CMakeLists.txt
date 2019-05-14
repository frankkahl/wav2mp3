cmake_minimum_required(VERSION 3.14)
project ("wav2mp3")

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(LAME_DIR "lame-3.100")
set(PTHREADS_DIR "pthreads")

set(SOURCES "src")
set(HEADERS "src")

set(CPPFILES
  "${SOURCES}/wav2mp3.cpp"
  "${SOURCES}/argument_processing.cpp"
  "${SOURCES}/convert_wav_files.cpp"
  "${SOURCES}/riff_format.cpp"
  "${SOURCES}/lame_wrapper.cpp"
  "${SOURCES}/guid.cpp"
  )

set(HFILES
  "${HEADERS}/wav2mp3.h"
  "${SOURCES}/argument_processing.h"
  "${SOURCES}/convert_wav_files.h"
  "${SOURCES}/riff_format.h"
  "${SOURCES}/lame_wrapper.h"
  "${SOURCES}/guid.h"
 )

include_directories("${HEADERS}")
include_directories("${LAME_DIR}/include")
include_directories("${PTHREADS_DIR}/include")
link_directories("${LAME_DIR}/output/debug")
link_directories("${PTHREADS_DIR}/lib")

add_executable (wav2mp3 ${CPPFILES} ${HFILES})
target_link_libraries(wav2mp3 debug libpthreadVC3d.lib)
target_link_libraries(wav2mp3 optimized libpthreadVC3.lib)
target_link_libraries(wav2mp3 debug "${LAME_DIR}/output/Debug/libmp3lame-static")
target_link_libraries(wav2mp3 debug "${LAME_DIR}/output/Debug/libmpghip-static")
target_link_libraries(wav2mp3 optimized "${LAME_DIR}/output/Release/libmp3lame-static")
target_link_libraries(wav2mp3 optimized "${LAME_DIR}/output/Release/libmpghip-static")
