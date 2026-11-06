# wav2mp3

> [!WARNING]
> The Windows version was last tested about 6 years ago (2020). Currently I have no
> access to Windows and Visual Studio to test whether the build instructions and the
> app itself still work. If you run into problems and have a fix, please submit a
> pull request.

command line tool for converting all WAV audio files in a directory
and optionally in all its sub-folders to MP3.
Under Windows all external libraries have been statically linked to reduce
dependency problems.

current version: 1.0.1

1. wav2mp3 --help output:

        wav2mp3 1.0.1 using lame 3.100, cxxopts 2.2.0 and pthreads (NPTL 2.39): converts all WAV files in passed directory to MP3
        Usage:
          wav2mp3 [OPTION...] directory

          -h, --help         print help
          -v, --version      print version
          -r, --recursive    recurse through all sub-directories
          -o, --overwrite    overwrite existing MP3 files instead of creating one
                             with an alternative name not used yet
          -q, --quality arg  set quality level of MP3 compression (integer between 0
                             (highest) and 9 (lowest)) (default: 5)
          -a, --all          try to convert all files, not only those with the
                             extension .wav.
          -t, --threads arg  number of threads (maximum 32) (default: 32)

        (the maximum and default number of threads equal the number of CPU cores
         of the machine; the output above was taken on a machine with 32 cores)

2. Description
   - tested under Linux (TUXEDO OS, based on Ubuntu 24.04)
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
   - compression quality can be set via command line (default is 5, 0-9 are allowed)
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

3. Compiling from sources
   - uses C++ 17 Standard
   - requires VS 2019 under Windows and
     GCC 9.1 or newer under Linux
   - requires CMake 3.14 for building under Windows and CMake 3.24 under Linux
     (the Linux build scripts use "cmake --fresh"); CMake must be in the search path
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
       - build_wav2mp3_using_C++_threads.bat (C++ native thread library)

   - Linux:
     - tested under Linux TUXEDO OS (based on Ubuntu 24.04)
     - requires pthreads, lame development packages,
       Ninja and pkg-config (see more below)
     - compiling:
       - install required packages and checkout repository
         following the instructions in the
         "Addendum: preparing a Linux build and test environment" below
       - build by executing
          * build_wav2mp3.sh (pthreads version) or
          * build_wav2mp3_using_C++_threads.sh (C++ native thread library)

4. Precompiled binaries:
   - Windows: bin/windows/release/wav2mp3.exe
     (still version 1.0.0, does not contain the fixes of version 1.0.1)
   - Linux:   bin/linux/release/wav2mp3 (version 1.0.1)

5. To Do:
   - Transferring all meta data (e.g. Author, Album etc.)
     in a potentially present "id3 " chunk to the MP3 file

6. Licenses:
   - see file "LICENSE"

------------------------------------------------------------------------

# Addendum: preparing a Linux build and test environment

   - last tested on 2026-11-06 on TUXEDO OS 24.04.5 LTS,
     which is based on Ubuntu 24.04 LTS ("noble"), using:
     - GCC 13.3.0
     - pthreads: NPTL 2.39 (part of glibc 2.39)
     - lame 3.100
     - CMake 4.2.3
     - Ninja 1.11.1
     - pkgconf 1.8.1
   - the same commands apply to Ubuntu 24.04 and other distributions based on it
   - installing the required packages:

         sudo apt-get update
         sudo apt-get install build-essential
         # gcc should already be on the system
         # GCC 9 or newer required for full C++17 support
         sudo apt-get install gcc g++
         sudo apt-get install cmake
         sudo apt-get install ninja-build
         sudo apt-get install pkgconf
         sudo apt-get install libmp3lame-dev
         sudo apt-get install git

   - checking out the repository and compiling:

         git clone "https://github.com/frankkahl/wav2mp3.git" wav2mp3
         cd wav2mp3
         ./build_wav2mp3.sh
