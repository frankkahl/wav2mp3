cmake_minimum_required(VERSION 3.14)
project ("wav2mp3")
add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(SOURCES "src")
set(HEADERS "src")

#add_compile_definitions(USE_PTHREAD)

set(CPPFILES
  "${SOURCES}/wav2mp3.cpp"
  "${SOURCES}/argument_processing.cpp"
  "${SOURCES}/convert_wav_files.cpp"
  "${SOURCES}/riff_format.cpp"
  "${SOURCES}/lame_wrapper.cpp"
  "${SOURCES}/guid.cpp"
  "${SOURCES}/thread_pool.cpp"
  "${SOURCES}/tiostream.cpp"
  "${SOURCES}/signal_handler.cpp"
  "${SOURCES}/return_code.cpp"
  )

set(HFILES
  "${SOURCES}/argument_processing.h"
  "${SOURCES}/convert_wav_files.h"
  "${SOURCES}/riff_format.h"
  "${SOURCES}/lame_wrapper.h"
  "${SOURCES}/guid.h"
  "${SOURCES}/thread_pool.h"
  "${SOURCES}/thread_queue_impl.h"
  "${SOURCES}/thread_queue.h"
  "${SOURCES}/tiostream.h"
  "${SOURCES}/signal_handler.h"
  "${SOURCES}/thread_includes.h"
  "${SOURCES}/return_code.h"
 )

if(DEFINED USE_PTHREAD)
    message(STATUS "Using pthreads")
    add_compile_definitions(USE_PTHREAD)
    set(CPPFILES
      ${CPPFILES}
      "${SOURCES}/mutex.cpp"
      "${SOURCES}/condition_variable.cpp"
      "${SOURCES}/thread.cpp"
      #"${SOURCES}/hardware_concurrency.cpp"
      )

    set(HFILES
      ${HFILES}
      "${SOURCES}/mutex.h"
      "${SOURCES}/condition_variable.h"
      "${SOURCES}/thread.h"
      #"${SOURCES}/hardware_concurrency.h"
     )
endif(DEFINED USE_PTHREAD)

include_directories("${HEADERS}")


add_executable (wav2mp3 ${CPPFILES} ${HFILES})
if (CMAKE_HOST_WIN32)
   set(LAME_DIR "lame-3.100")
   set(PTHREADS_DIR "pthreads")
   include_directories("${LAME_DIR}/include")
   include_directories("${PTHREADS_DIR}/include")
   target_link_libraries(wav2mp3 debug "${PTHREADS_DIR}/lib/libpthreadVC3d")
   target_link_libraries(wav2mp3 optimized "${PTHREADS_DIR}/lib/libpthreadVC3d")
   target_link_libraries(wav2mp3 debug "${LAME_DIR}/output/Debug/libmp3lame-static")
   target_link_libraries(wav2mp3 debug "${LAME_DIR}/output/Debug/libmpghip-static")
   target_link_libraries(wav2mp3 optimized "${LAME_DIR}/output/Release/libmp3lame-static")
   target_link_libraries(wav2mp3 optimized "${LAME_DIR}/output/Release/libmpghip-static")
endif (CMAKE_HOST_WIN32)
