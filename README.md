# wav2mp3

command line tool for converting all WAV audio files in a folder
and all its sub-folders into MP3. All files of a supported format are converted,
regardless of their file extension. Currently a fixed lame quality setting of 5 is used

1. Description
   - command line tool
   - tested under Linux (Ubuntu 18.04)
     and Windows (Windows 7)
   - root folder is passed as first argument.
   - a WAV file of name "<name>.wav" is converted into an MP3 with the file name
     "<name>.mp3". If such a file already exists the resulting MP3 has the name
     "<name> (n).mp3" where n is the first index for which not already an MP3 file exists.
     An existing MP3 file is never overwritten.
   - uses the lame 3.100 library for MP3 encoding
   - uses as many threads as cores are available
   - can be interrupted by pressing Ctrl-C
   - supported formats are:
     - PCM:
       - 8, 16, 24, 32 bit integer, mono or stereo
     - IEEE_FLOAT:
       - 32, 64 bit float, mono or stereo
     - WAVE_FORMAT_EXTENSIBLE (Microsoft):
       - PCM:
       	 - 8...32 bit (all bit widths in between are supported),
           mono or stereo
       - IEEE_FLOAT:
       	 - 32, 64 bit float, mono or stereo

2. Compiling from sources
   - uses C++ 17 Standard
   - requires VS 2017 under Windows and
     GCC 9.1 under Linux
   - requires CMake 3.14 for building (must be in search path)
   - can be configured to use either
     - pthreads (POSIX 1003.1-2001) or
     - C++ native threading library (C++ 11 or newer)
   - Windows:
     - links statically against LAME 3.10 (see http://lame.sourceforge.net/)
       and - if used - pthreads (POSIX 1003.1-2001)
     - precompiled static libraries of LAME and pthreads
       are included
     - compiling:
       - build_wav2mp3.bat (pthreads version) or
       - build_wav2mp3_using_C++_threads.bat (C++ native threads)
          
   - Linux:
     - tested under Linux Ubuntu 18.04
     - requires pthreads - if used - and lame development packages
       (see more below)
     - compiling:
       - build_wav2mp3.sh (pthreads version) or
       - build_wav2mp3_using_C++_threads.sh (C++ native threads)

3. Precompiled binaries:
   - Windows: bin/windows/release/wav2mp3.exe
   - Linux:   bin/linux/release/wav2mp3

4. To Do:
   - Adding command line switches for:
     - recursive/non-recursive directory processing
     - overwriting already existing MP3 files
     - setting lame encoding quality
   - Transferring possible meta information (e.g. Author, Album etc.)
     present in the WAV file to ID3 Tags

5. Licenses:
   - see file "LICENSE"


