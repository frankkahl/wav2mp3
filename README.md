# wav2mp3
command line tool for converting all WAV audio files in a folder and all its sub-folders into MP3

- root folder is passed as first argument.
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
