# wav2mp3
command line tool for converting all WAV audio files in a directory
and and optionally in all its sub-folders to MP3.
Under Windows all external libraries have been statically linked to reduce
dependency problems.

current version: 1.0.0

1. wav2mp3 --help output:

        wav2mp3.exe 0.9.0 using lame 3.100 and pthreads: converts all WAV files in passed directory to MP3
        Usage:
          wav2mp3.exe [OPTION...] directory

          -h, --help         print help
          -v, --version      print version
          -r, --recursive    recurse through all sub-directories
          -o, --overwrite    overwrite existing MP3 files instead of creating one
                             with an alternative name not used yet
          -q, --quality arg  set quality level of MP3 compression (integer between 0
                             (highest) and 9 (lowest)) (default: 5)
          -a, --all          try to convert all files, not only those with the
                             extension .wav.
          -t, --threads arg  number of threads (maximum 4) (default: 4)

2. Description
   - tested under Linux (Ubuntu 18.04)
     and Windows (Windows 7)
   - a WAV file of name "<name>.wav" is converted into an MP3 with the file name
     "<name>.mp3". If such a file already exists it will be
     overwritten if the command line option -o/--overwrite is passed.
     Otherwise the resulting MP3 has the name
     "<name> (n).mp3" where n is the first index for which not already
     an MP3 file exists.
   - uses the lame 3.100 library for MP3 encoding (http://lame.sourceforge.net/)
   - uses cxxopts 2.2.0 for command line processing (https://github.com/jarro2783/cxxopts)
   - by default uses as many threads as cores are available.
     This can be changed using the command line parameter -t/--threads
   - can be interrupted by pressing Ctrl-C or sending SIGTERM
   - compression quality can be set via command line (default is 5, 0-9 are allowd)
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
   - Meta data in a potentially present LIST chunk with format-type INFO are transferred to
     ID3 tags
     REMARK: Storing meta data in a LIST-chunk of format INFO is the standard but poorly used.
             Instead storing meta data in an "id3 "-chunk seems to be the unofficial but de-facto
	     standard supported by the majority of the software, including the Windows Explorer!
	     => planned to support transfer of "id3 "-chunks in version 1.1.0
2. Compiling from sources
   - uses C++ 17 Standard
   - requires VS 2017 under Windows (tested with MSVC 19.16.27030.1) and
     GCC 9.1 under Linux
   - requires CMake 3.14 for building (must be in search path)
   - can be configured to use either
     - pthreads (POSIX 1003.1-2001) or
     - C++ native threading library (since C++ 11)
   - Windows:
     - links statically against:
         * LAME 3.100 (see http://lame.sourceforge.net/)
         * pthreads (POSIX 1003.1-2001)
           (see https://sourceforge.net/projects/pthreads4w/)
           (not if the native C++ threads are used)
         * Visual Studio standard libs (/MT and /MTd compiler switches)
     - precompiled static libraries of LAME and pthreads are included
     - build by executing:
       - build_wav2mp3.bat (pthreads version) or
       - build_wav2mp3_using_C++_threads.bat (C++ native threads)
          
   - Linux:
     - tested under Linux Ubuntu 18.04
     - requires pthreads - if used - and lame development packages
       (see more below)
     - compiling:
       - install required packages and checkout repository
         following instructions in file:
            preparing_ubuntu_18.04_test_environment.txt
       - build by executing
          * build_wav2mp3.sh (pthreads version) or
          * build_wav2mp3_using_C++_threads.sh (C++ native threads)

3. Precompiled binaries:
   - Windows: bin/windows/release/wav2mp3.exe
   - Linux:   bin/linux/release/wav2mp3

4. To Do:
   - Transferring all meta data (e.g. Author, Album etc.)
     in a potentially present "id3 " chunk to the MP3 file

5. Licenses:
   - see file "LICENSE"
