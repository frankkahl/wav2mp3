#ifndef WAV_HEADER_H
#define WAV_HEADER_H

#include <string>
#include <map>
#include <cstdint>
// RIFF reference used:
// "https://docs.microsoft.com/en-us/windows/desktop/xaudio2/resource-interchange-file-format--riff-"
// PCM header reference used:
// "https://msdn.microsoft.com/en-us/library/windows/desktop/dd390970(v=vs.85).aspx"

// forward declaration of map with description strings for audio format codes 
// defined in "riff_format.cpp"
extern std::map<uint16_t, std::string> audio_format_uint16_to_names;
extern std::map<std::string, uint16_t> audio_format_names_to_uint16;

// RIFF header

typedef struct RiffHeader {
	char riff_id[4];                // RIFF FOURCC identifier, must contains "RIFF"
	uint32_t total_data_size;       // size of all valid data following this entry in file
	                                // usually this is file size - 8, but that is not guaranteed
	char format[4];                 // should contain "WAVE" in case of a WAV file
} RiffHeader;


typedef struct FormatHeader {
	std::uint16_t audio_format;		// must be 0x0001 for PCM
	std::uint16_t num_channels;		// should be 1 for mono and 2 for stereo, not sure if
	                                // larger numbers are allowed
	std::uint32_t samples_per_second;
	std::uint32_t bytes_per_second; // Number of bytes per second.
	                                // For PCM must be
	                                // = block_align*samples_per_second
	std::uint16_t block_align;      // minimum atomic unit of data
	                                // for PCM this must be
	                                // = num_channels * bits_per_sample/8
	std::uint16_t bits_per_sample;  // number of bits per sample
									// must be an integer multiple of 8
	                                // for PCM only 8 or 16 are allowed
} FormatHeader;

#endif // WAV_HEADER_H