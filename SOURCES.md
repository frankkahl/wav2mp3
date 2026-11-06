# Sources of the test files

All audio test files in this repository are either in the public domain, dedicated to the
public domain under CC0 1.0, or were created for this project. They may be redistributed
together with wav2mp3 under its MIT license. Some files were converted, trimmed or tagged
with ffmpeg; these modifications are released under CC0 1.0 as well.

## wav_testfiles

| File | Format | Source | License |
|---|---|---|---|
| `beethoven_op110_adagio.wav` | PCM 16 bit, 22050 Hz, mono, first 5 minutes, with LIST/INFO metadata | Ludwig van Beethoven, Piano Sonata No. 31, Op. 110, III. Adagio, ma non troppo; Musopen recording on [Wikimedia Commons](https://commons.wikimedia.org/wiki/File:Ludwig_van_Beethoven_-_sonata_no._31_in_a_flat_major,_op._110_-_iii._adagio,_ma_non_troppo.ogg) | Public domain |
| `mussorgsky_night_on_bald_mountain.wav` | PCM 16 bit, 22050 Hz, mono, first 4 minutes | Modest Mussorgsky, Night on Bald Mountain; Musopen recording on [Wikimedia Commons](https://commons.wikimedia.org/wiki/File:Modest_Mussorgsky_-_night_on_bald_mountain.ogg) | Public domain |
| `mussorgsky_night_on_bald_mountain_stereo.wav` | PCM 16 bit, 44100 Hz, stereo, 30 seconds starting at 4:00 | same as above | Public domain |
| `apollo11_eagle_has_landed.wav` | PCM 8 bit, 11025 Hz, mono, with LIST/INFO metadata | NASA, Apollo 11, "The Eagle has landed", on [Wikimedia Commons](https://commons.wikimedia.org/wiki/File:Apollo_11_Eagle_Has_Landed.mp3) (originally from nasa.gov) | Public domain (work of the US government) |
| `apollo8_merry_christmas.wav` | PCM 8 bit, 11025 Hz, mono | NASA, Apollo 8, "Merry Christmas", on [Wikimedia Commons](https://commons.wikimedia.org/wiki/File:Apollo_8_-_Merry_Christmas.mp3) | Public domain (work of the US government) |
| `apollo11_launch_day.wav` | PCM 16 bit, 22050 Hz, mono, first 30 seconds | NASA, Apollo 11 launch day communication relayed through Canary Station, on [Wikimedia Commons](https://commons.wikimedia.org/wiki/File:Apollo_11_launch_day_communication_relayed_through_Canary_Station.wav) | Public domain (work of the US government) |
| `adafruit_bell.wav` | PCM 16 bit, 22050 Hz, stereo, with LIST/INFO and "id3 " chunks, unmodified | `sounds of adafruit/bell.wav` from [adafruit/Adafruit-Sound-Samples](https://github.com/adafruit/Adafruit-Sound-Samples) | CC0 1.0 |
| `adafruit_elevator.wav` | PCM 16 bit, 22050 Hz, mono, unmodified | `sounds of adafruit/elevator.wav` from [adafruit/Adafruit-Sound-Samples](https://github.com/adafruit/Adafruit-Sound-Samples) | CC0 1.0 |
| `testcase.wav`, `testcase ö.wav`, `testcase._wav` | PCM 16 bit, 44100 Hz, stereo | created by Frank Kahl for this project | MIT, like wav2mp3 |
| `not_a_wav_file.wav`, `not_a_wav_file._wav` | plain text, no WAV file | created by Frank Kahl for this project | MIT, like wav2mp3 |

## wav_format_testfiles

All files in this folder were generated with ffmpeg. The audio is taken from `wav_testfiles/apollo11_launch_day.wav` (mono files)
and `wav_testfiles/mussorgsky_night_on_bald_mountain_stereo.wav` (stereo files), both in the
public domain (see above). The files reproduce the WAV formats of commonly used sample
collections, without containing any of their audio.

| Files | Format |
|---|---|
| `8k8bitpcm.wav`, `11k8bitpcm.wav` | PCM 8 bit, 8000 / 11025 Hz, mono |
| `8k16bitpcm.wav`, `11k16bitpcm.wav` | PCM 16 bit, 8000 / 11025 Hz, mono |
| `8kulaw.wav`, `11kulaw.wav` | µ-law, 8000 / 11025 Hz, mono |
| `8kadpcm.wav`, `11kadpcm.wav` | IMA ADPCM (0x0011), 8000 / 11025 Hz, mono |
| `8kgsm.wav`, `11kgsm.wav` | GSM 6.10 (0x0031), 8000 / 11025 Hz, mono |
| `8kmp38.wav`, `8kmp316.wav` | MPEG Layer 3 (0x0055), 8 / 16 kbit/s, 8000 Hz, mono |
| `8kcelp.wav`, `8ksbc12.wav`, `8ktruespeech.wav` | header of the formats 0x0070, 0x0072 and 0x0022 (not supported by ffmpeg) with silent data |
| `stereo-<format>.wav` | stereo, 8000 Hz, as uint8, int12, int16, int24, int32, float32, float64, A-law and µ-law, with an unknown `test` chunk and a LIST/INFO chunk |
| `stereo-<format>WE.wav` | the same formats as WAVE_FORMAT_EXTENSIBLE |
