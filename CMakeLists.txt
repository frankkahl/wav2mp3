cmake_minimum_required(VERSION 3.14)
project ("wav2mp3")

## C++ 17 is required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


## both headers and cpp files are in the "src" folder
set(SOURCES "src")
set(HEADERS "src")


## first add all header and source files which are required
## by both flavors (using pthread or the native C++ threads)
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

## if pthreads are used, add the headers and source files encapsulating
## the pthreads functions in a objects mimicing the interface of the native C++ 11 threading library
if(NOT DEFINED USE_CPP11_THREADS)
    message(STATUS "Using pthreads")
    set(CPPFILES
      ${CPPFILES}
      "${SOURCES}/mutex.cpp"
      "${SOURCES}/condition_variable.cpp"
      "${SOURCES}/thread.cpp"
      "${SOURCES}/check_pthread_error.cpp"
      )

    set(HFILES
      ${HFILES}
      "${SOURCES}/mutex.h"
      "${SOURCES}/condition_variable.h"
      "${SOURCES}/thread.h"
      "${SOURCES}/check_pthread_error.h"
     )
else(NOT DEFINED USE_CPP11_THREADS)
   add_compile_definitions(USE_CPP11_THREADS)
   message(STATUS "Using native C++ threads introduced with C++ 11")
endif(NOT DEFINED USE_CPP11_THREADS)

## add "src" directory to the search path for the headers
include_directories("${HEADERS}")

## now define the target executable
add_executable (wav2mp3 ${CPPFILES} ${HFILES})

## now do the platform specific configurations
## first configure for Windows
if (CMAKE_HOST_WIN32)
   ## for Windows both the pthreads and the lame libraries
   ## are contained as already precompiled static libraries

   ## configure the locations of the static lame libraries and the header file,
   ## both for Release and Debug compiles
   set(LAME_DIR "lame-3.100")
   include_directories("${LAME_DIR}/include")
   target_link_libraries(wav2mp3 debug "${LAME_DIR}/output/Debug/libmp3lame-static")
   target_link_libraries(wav2mp3 debug "${LAME_DIR}/output/Debug/libmpghip-static")
   target_link_libraries(wav2mp3 optimized "${LAME_DIR}/output/Release/libmp3lame-static")
   target_link_libraries(wav2mp3 optimized "${LAME_DIR}/output/Release/libmpghip-static")

   ## only if pthreads are used
   ## configure the location of the pthread static libraries and the header file
   if(NOT DEFINED USE_CPP11_THREADS)
      set(PTHREADS_DIR "pthreads")
      include_directories("${PTHREADS_DIR}/include")
      target_link_libraries(wav2mp3 debug "${PTHREADS_DIR}/lib/libpthreadVC3d")
      target_link_libraries(wav2mp3 optimized "${PTHREADS_DIR}/lib/libpthreadVC3d")
   endif(NOT DEFINED USE_CPP11_THREADS)

## then check if compiling on a UNIX like system, e.g. linux
elseif (CMAKE_HOST_UNIX)
   ## set different target folder for the Release and the Debug target type so that
   ## both can created simultaneously
   set_target_properties(wav2mp3 PROPERTIES
                         RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build/bin/${CMAKE_BUILD_TYPE}")

			 
   ## on linux the C++ threading library oviously uses pthreads
   ## so search and add it for both flavors
   include (FindThreads)
   if ("${CMAKE_USE_PTHREADS_INIT}" STREQUAL "")
      message (FATAL_ERROR "!! pthread library not found!")
   endif ("${CMAKE_USE_PTHREADS_INIT}" STREQUAL "")
   message (STATUS "-- pthread library used: ${CMAKE_THREAD_LIBS_INIT}")

   target_link_libraries(wav2mp3 ${CMAKE_THREAD_LIBS_INIT})

   ## try to find the lame library
   find_library(LIBLAME mp3lame)
   if ("${LIBLAME}" STREQUAL "LIBLAME-NOTFOUND")
      message(FATAL_ERROR "!! lame library not found!")
   endif ("${LIBLAME}" STREQUAL "LIBLAME-NOTFOUND" )
   
   message(STATUS "-- lame library used: ${LIBLAME}")

   target_link_libraries(wav2mp3 ${LIBLAME})
   
else (CMAKE_HOST_WIN32)
   message( FATAL_ERROR "Target platform ${CMAKE_HOST_SYSTEM} not supported yet." )
endif (CMAKE_HOST_WIN32)
