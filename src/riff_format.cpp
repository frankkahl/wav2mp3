#include "riff_format.h"

#include <map>
#include <string>
#include <cstdint>
using namespace std;

// maps uint16_t codes for audio_formats to logical names
extern map<uint16_t, string> audio_format_uint16_to_names = { { 0x0001,  	"PCM"},
											{ 0x0002,  	"MS ADPCM"},
											{ 0x0003,   "IEEE FLOAT"},
											{ 0x0005,   "IBM CVSD"},
											{ 0x0006,   "ALAW"},
											{ 0x0007,   "MULAW"},
											{ 0x0010,   "OKI ADPCM"},
											{ 0x0011,   "DVI / IMA ADPCM"},
											{ 0x0012,   "MEDIASPACE ADPCM"},
											{ 0x0013,   "SIERRA ADPCM"},
											{ 0x0014,   "G723 ADPCM"},
											{ 0x0015,   "DIGISTD"},
											{ 0x0016,   "DIGIFIX"},
											{ 0x0017,   "DIALOGIC OKI ADPCM"},
											{ 0x0020,   "YAMAHA ADPCM"},
											{ 0x0021, 	"SONARC"},
											{ 0x0022, 	"DSPGROUP TRUESPEECH"},
											{ 0x0023, 	"ECHOSC1"},
											{ 0x0024, 	"AUDIOFILE AF36"},
											{ 0x0025, 	"APTX"},
											{ 0x0026, 	"AUDIOFILE AF10"},
											{ 0x0030, 	"DOLBY AC2"},
											{ 0x0031, 	"GSM610" },
											{ 0x0033, 	"ANTEX ADPCME"},
											{ 0x0034, 	"CONTROL RES VQLPC"},
											{ 0x0035, 	"CONTROL RES VQLPC"},
											{ 0x0036, 	"DIGIADPCM"},
											{ 0x0037, 	"CONTROL RES CR10"},
											{ 0x0038, 	"NMS VBXADPCM"},
											{ 0x0039, 	"CS IMAADPCM(Roland RDAC)"},
											{ 0x0040, 	"G721 ADPCM"},
											{ 0x0050, 	"MPEG - 1 Layer I, II"},
											{ 0x0055, 	"MPEG - 1 Layer III(MP3)"},
											{ 0x0069, 	"Xbox ADPCM"},
											{ 0x0200, 	"CREATIVE ADPCM"},
											{ 0x0202, 	"CREATIVE FASTSPEECH8"},
											{ 0x0203, 	"CREATIVE FASTSPEECH10"},
											{ 0x0300, 	"FM TOWNS SND"},
											{ 0x1000, 	"OLIGSM"},
											{ 0x1001, 	"OLIADPCM"},
											{ 0x1002, 	"OLICELP"},
											{ 0x1003, 	"OLISBC"},
											{ 0x1004, 	"OLIOPR"}
};

// helper for swapping key, value of a map
// This function assumes that all values of the passed map are unique
template <class T1, class T2>
static map<T2, T1> swap_keys_values(const map<T1, T2> &m) {
	map<T2, T1> m1;

	for (auto&& item : m) {
		m1.emplace(item.second, item.first);
	}

	return m1;
};

extern map<string, uint16_t> audio_format_names_to_uint16 = swap_keys_values(audio_format_uint16_to_names);