#ifndef WAV_HEADER_H
#define WAV_HEADER_H

#include "guid.h"

#include <cstdint>
#include <map>
#include <regex>
#include <string>
// RIFF reference used:
// "https://docs.microsoft.com/en-us/windows/desktop/xaudio2/resource-interchange-file-format--riff-"
// PCM header reference used:
// "https://msdn.microsoft.com/en-us/library/windows/desktop/dd390970(v=vs.85).aspx"

// forward declaration of map with description strings for audio format codes
// defined in "riff_format.cpp"
extern std::map<uint16_t, std::string> audio_format_uint16_to_names;
// extern std::map<std::string, uint16_t> audio_format_names_to_uint16;
extern std::map<Guid, std::string> audio_format_guid_to_names;
// extern std::map<std::string, Guid> audio_format_names_to_guid;

// RIFF header

typedef struct RiffHeader {
    char     riff_id[4];       // RIFF FOURCC identifier, must contains "RIFF"
    uint32_t total_data_size;  // size of all valid data following this entry in file
                               // usually this is file size - 8, but that is not guaranteed
    char format[4];            // should contain "WAVE" in case of a WAV file
} RiffHeader;

typedef struct FormatHeader {
    std::uint16_t audio_format;  // must be 0x0001 for PCM
    std::uint16_t num_channels;  // should be 1 for mono and 2 for stereo, not sure if
                                 // larger numbers are allowed
    std::uint32_t samples_per_second;
    std::uint32_t bytes_per_second;  // Number of bytes per second.
                                     // For PCM must be
                                     // = block_align*samples_per_second
    std::uint16_t block_align;       // minimum atomic unit of data
                                     // for PCM this must be
                                     // = num_channels * bits_per_sample/8
    std::uint16_t bits_per_sample;   // number of bits per sample
                                     // must be an integer multiple of 8
                                     // for PCM only 8 or 16 are allowed
} FormatHeader;

typedef struct FormatHeaderExtensible {
    FormatHeader  header;
    std::uint16_t size;  // Size, in bytes, of extra format information appended to the end of
                         // this structure. This information can be used by non-PCM formats
                         // to store extra attributes for the audio_format field
                         // If no extra information is required by the wFormatTag,
                         // this member must be set to zero
    union {
        uint16_t valid_bits_per_sample;
        uint16_t samples_per_block;
        uint16_t reserved;
    } samples;
    uint32_t channel_mask = 0;
    Guid     sub_format;

} FormatHeaderExtensible;

// defines for audio formats in a FormatHeader structure (WAVEFORMATEX in the Windows SDK)
// taken (... OK, stolen ;-) ) from mmreg.h of Windows 10 SDK
#define WAVE_FORMAT_UNKNOWN 0x0000                    /* Microsoft Corporation */
#define WAVE_FORMAT_PCM 0x0001                        /* Microsoft Corporation */
#define WAVE_FORMAT_ADPCM 0x0002                      /* Microsoft Corporation */
#define WAVE_FORMAT_IEEE_FLOAT 0x0003                 /* Microsoft Corporation */
#define WAVE_FORMAT_VSELP 0x0004                      /* Compaq Computer Corp. */
#define WAVE_FORMAT_IBM_CVSD 0x0005                   /* IBM Corporation */
#define WAVE_FORMAT_ALAW 0x0006                       /* Microsoft Corporation */
#define WAVE_FORMAT_MULAW 0x0007                      /* Microsoft Corporation */
#define WAVE_FORMAT_DTS 0x0008                        /* Microsoft Corporation */
#define WAVE_FORMAT_DRM 0x0009                        /* Microsoft Corporation */
#define WAVE_FORMAT_WMAVOICE9 0x000A                  /* Microsoft Corporation */
#define WAVE_FORMAT_WMAVOICE10 0x000B                 /* Microsoft Corporation */
#define WAVE_FORMAT_OKI_ADPCM 0x0010                  /* OKI */
#define WAVE_FORMAT_DVI_ADPCM 0x0011                  /* Intel Corporation */
#define WAVE_FORMAT_IMA_ADPCM (WAVE_FORMAT_DVI_ADPCM) /*  Intel Corporation */
#define WAVE_FORMAT_MEDIASPACE_ADPCM 0x0012           /* Videologic */
#define WAVE_FORMAT_SIERRA_ADPCM 0x0013               /* Sierra Semiconductor Corp */
#define WAVE_FORMAT_G723_ADPCM 0x0014                 /* Antex Electronics Corporation */
#define WAVE_FORMAT_DIGISTD 0x0015                    /* DSP Solutions, Inc. */
#define WAVE_FORMAT_DIGIFIX 0x0016                    /* DSP Solutions, Inc. */
#define WAVE_FORMAT_DIALOGIC_OKI_ADPCM 0x0017         /* Dialogic Corporation */
#define WAVE_FORMAT_MEDIAVISION_ADPCM 0x0018          /* Media Vision, Inc. */
#define WAVE_FORMAT_CU_CODEC 0x0019                   /* Hewlett-Packard Company */
#define WAVE_FORMAT_HP_DYN_VOICE 0x001A               /* Hewlett-Packard Company */
#define WAVE_FORMAT_YAMAHA_ADPCM 0x0020               /* Yamaha Corporation of America */
#define WAVE_FORMAT_SONARC 0x0021                     /* Speech Compression */
#define WAVE_FORMAT_DSPGROUP_TRUESPEECH 0x0022        /* DSP Group, Inc */
#define WAVE_FORMAT_ECHOSC1 0x0023                    /* Echo Speech Corporation */
#define WAVE_FORMAT_AUDIOFILE_AF36 0x0024             /* Virtual Music, Inc. */
#define WAVE_FORMAT_APTX 0x0025                       /* Audio Processing Technology */
#define WAVE_FORMAT_AUDIOFILE_AF10 0x0026             /* Virtual Music, Inc. */
#define WAVE_FORMAT_PROSODY_1612 0x0027               /* Aculab plc */
#define WAVE_FORMAT_LRC 0x0028                        /* Merging Technologies S.A. */
#define WAVE_FORMAT_DOLBY_AC2 0x0030                  /* Dolby Laboratories */
#define WAVE_FORMAT_GSM610 0x0031                     /* Microsoft Corporation */
#define WAVE_FORMAT_MSNAUDIO 0x0032                   /* Microsoft Corporation */
#define WAVE_FORMAT_ANTEX_ADPCME 0x0033               /* Antex Electronics Corporation */
#define WAVE_FORMAT_CONTROL_RES_VQLPC 0x0034          /* Control Resources Limited */
#define WAVE_FORMAT_DIGIREAL 0x0035                   /* DSP Solutions, Inc. */
#define WAVE_FORMAT_DIGIADPCM 0x0036                  /* DSP Solutions, Inc. */
#define WAVE_FORMAT_CONTROL_RES_CR10 0x0037           /* Control Resources Limited */
#define WAVE_FORMAT_NMS_VBXADPCM 0x0038               /* Natural MicroSystems */
#define WAVE_FORMAT_CS_IMAADPCM 0x0039                /* Crystal Semiconductor IMA ADPCM */
#define WAVE_FORMAT_ECHOSC3 0x003A                    /* Echo Speech Corporation */
#define WAVE_FORMAT_ROCKWELL_ADPCM 0x003B             /* Rockwell International */
#define WAVE_FORMAT_ROCKWELL_DIGITALK 0x003C          /* Rockwell International */
#define WAVE_FORMAT_XEBEC 0x003D                      /* Xebec Multimedia Solutions Limited */
#define WAVE_FORMAT_G721_ADPCM 0x0040                 /* Antex Electronics Corporation */
#define WAVE_FORMAT_G728_CELP 0x0041                  /* Antex Electronics Corporation */
#define WAVE_FORMAT_MSG723 0x0042                     /* Microsoft Corporation */
#define WAVE_FORMAT_INTEL_G723_1 0x0043               /* Intel Corp. */
#define WAVE_FORMAT_INTEL_G729 0x0044                 /* Intel Corp. */
#define WAVE_FORMAT_SHARP_G726 0x0045                 /* Sharp */
#define WAVE_FORMAT_MPEG 0x0050                       /* Microsoft Corporation */
#define WAVE_FORMAT_RT24 0x0052                       /* InSoft, Inc. */
#define WAVE_FORMAT_PAC 0x0053                        /* InSoft, Inc. */
#define WAVE_FORMAT_MPEGLAYER3 0x0055                 /* ISO/MPEG Layer3 Format Tag */
#define WAVE_FORMAT_LUCENT_G723 0x0059                /* Lucent Technologies */
#define WAVE_FORMAT_CIRRUS 0x0060                     /* Cirrus Logic */
#define WAVE_FORMAT_ESPCM 0x0061                      /* ESS Technology */
#define WAVE_FORMAT_VOXWARE 0x0062                    /* Voxware Inc */
#define WAVE_FORMAT_CANOPUS_ATRAC 0x0063              /* Canopus, co., Ltd. */
#define WAVE_FORMAT_G726_ADPCM 0x0064                 /* APICOM */
#define WAVE_FORMAT_G722_ADPCM 0x0065                 /* APICOM */
#define WAVE_FORMAT_DSAT 0x0066                       /* Microsoft Corporation */
#define WAVE_FORMAT_DSAT_DISPLAY 0x0067               /* Microsoft Corporation */
#define WAVE_FORMAT_VOXWARE_BYTE_ALIGNED 0x0069       /* Voxware Inc */
#define WAVE_FORMAT_VOXWARE_AC8 0x0070                /* Voxware Inc */
#define WAVE_FORMAT_VOXWARE_AC10 0x0071               /* Voxware Inc */
#define WAVE_FORMAT_VOXWARE_AC16 0x0072               /* Voxware Inc */
#define WAVE_FORMAT_VOXWARE_AC20 0x0073               /* Voxware Inc */
#define WAVE_FORMAT_VOXWARE_RT24 0x0074               /* Voxware Inc */
#define WAVE_FORMAT_VOXWARE_RT29 0x0075               /* Voxware Inc */
#define WAVE_FORMAT_VOXWARE_RT29HW 0x0076             /* Voxware Inc */
#define WAVE_FORMAT_VOXWARE_VR12 0x0077               /* Voxware Inc */
#define WAVE_FORMAT_VOXWARE_VR18 0x0078               /* Voxware Inc */
#define WAVE_FORMAT_VOXWARE_TQ40 0x0079               /* Voxware Inc */
#define WAVE_FORMAT_VOXWARE_SC3 0x007A                /* Voxware Inc */
#define WAVE_FORMAT_VOXWARE_SC3_1 0x007B              /* Voxware Inc */
#define WAVE_FORMAT_SOFTSOUND 0x0080                  /* Softsound, Ltd. */
#define WAVE_FORMAT_VOXWARE_TQ60 0x0081               /* Voxware Inc */
#define WAVE_FORMAT_MSRT24 0x0082                     /* Microsoft Corporation */
#define WAVE_FORMAT_G729A 0x0083                      /* AT&T Labs, Inc. */
#define WAVE_FORMAT_MVI_MVI2 0x0084                   /* Motion Pixels */
#define WAVE_FORMAT_DF_G726 0x0085                    /* DataFusion Systems (Pty) (Ltd) */
#define WAVE_FORMAT_DF_GSM610 0x0086                  /* DataFusion Systems (Pty) (Ltd) */
#define WAVE_FORMAT_ISIAUDIO 0x0088                   /* Iterated Systems, Inc. */
#define WAVE_FORMAT_ONLIVE 0x0089                     /* OnLive! Technologies, Inc. */
#define WAVE_FORMAT_MULTITUDE_FT_SX20 0x008A          /* Multitude Inc. */
#define WAVE_FORMAT_INFOCOM_ITS_G721_ADPCM 0x008B     /* Infocom */
#define WAVE_FORMAT_CONVEDIA_G729 0x008C              /* Convedia Corp. */
#define WAVE_FORMAT_CONGRUENCY 0x008D                 /* Congruency Inc. */
#define WAVE_FORMAT_SBC24 0x0091                      /* Siemens Business Communications Sys */
#define WAVE_FORMAT_DOLBY_AC3_SPDIF 0x0092            /* Sonic Foundry */
#define WAVE_FORMAT_MEDIASONIC_G723 0x0093            /* MediaSonic */
#define WAVE_FORMAT_PROSODY_8KBPS 0x0094              /* Aculab plc */
#define WAVE_FORMAT_ZYXEL_ADPCM 0x0097                /* ZyXEL Communications, Inc. */
#define WAVE_FORMAT_PHILIPS_LPCBB 0x0098              /* Philips Speech Processing */
#define WAVE_FORMAT_PACKED 0x0099                     /* Studer Professional Audio AG */
#define WAVE_FORMAT_MALDEN_PHONYTALK 0x00A0           /* Malden Electronics Ltd. */
#define WAVE_FORMAT_RACAL_RECORDER_GSM 0x00A1         /* Racal recorders */
#define WAVE_FORMAT_RACAL_RECORDER_G720_A 0x00A2      /* Racal recorders */
#define WAVE_FORMAT_RACAL_RECORDER_G723_1 0x00A3      /* Racal recorders */
#define WAVE_FORMAT_RACAL_RECORDER_TETRA_ACELP 0x00A4 /* Racal recorders */
#define WAVE_FORMAT_NEC_AAC 0x00B0                    /* NEC Corp. */
#define WAVE_FORMAT_RAW_AAC1                                                                                           \
    0x00FF /* For Raw AAC, with format block AudioSpecificConfig() (as defined by MPEG-4), that follows WAVEFORMATEX \ \
            * \                                                                                                        \
            * \ \ \ \ \ \ \ \                                                                                                                     \
            */
#define WAVE_FORMAT_RHETOREX_ADPCM 0x0100            /* Rhetorex Inc. */
#define WAVE_FORMAT_IRAT 0x0101                      /* BeCubed Software Inc. */
#define WAVE_FORMAT_VIVO_G723 0x0111                 /* Vivo Software */
#define WAVE_FORMAT_VIVO_SIREN 0x0112                /* Vivo Software */
#define WAVE_FORMAT_PHILIPS_CELP 0x0120              /* Philips Speech Processing */
#define WAVE_FORMAT_PHILIPS_GRUNDIG 0x0121           /* Philips Speech Processing */
#define WAVE_FORMAT_DIGITAL_G723 0x0123              /* Digital Equipment Corporation */
#define WAVE_FORMAT_SANYO_LD_ADPCM 0x0125            /* Sanyo Electric Co., Ltd. */
#define WAVE_FORMAT_SIPROLAB_ACEPLNET 0x0130         /* Sipro Lab Telecom Inc. */
#define WAVE_FORMAT_SIPROLAB_ACELP4800 0x0131        /* Sipro Lab Telecom Inc. */
#define WAVE_FORMAT_SIPROLAB_ACELP8V3 0x0132         /* Sipro Lab Telecom Inc. */
#define WAVE_FORMAT_SIPROLAB_G729 0x0133             /* Sipro Lab Telecom Inc. */
#define WAVE_FORMAT_SIPROLAB_G729A 0x0134            /* Sipro Lab Telecom Inc. */
#define WAVE_FORMAT_SIPROLAB_KELVIN 0x0135           /* Sipro Lab Telecom Inc. */
#define WAVE_FORMAT_VOICEAGE_AMR 0x0136              /* VoiceAge Corp. */
#define WAVE_FORMAT_G726ADPCM 0x0140                 /* Dictaphone Corporation */
#define WAVE_FORMAT_DICTAPHONE_CELP68 0x0141         /* Dictaphone Corporation */
#define WAVE_FORMAT_DICTAPHONE_CELP54 0x0142         /* Dictaphone Corporation */
#define WAVE_FORMAT_QUALCOMM_PUREVOICE 0x0150        /* Qualcomm, Inc. */
#define WAVE_FORMAT_QUALCOMM_HALFRATE 0x0151         /* Qualcomm, Inc. */
#define WAVE_FORMAT_TUBGSM 0x0155                    /* Ring Zero Systems, Inc. */
#define WAVE_FORMAT_MSAUDIO1 0x0160                  /* Microsoft Corporation */
#define WAVE_FORMAT_WMAUDIO2 0x0161                  /* Microsoft Corporation */
#define WAVE_FORMAT_WMAUDIO3 0x0162                  /* Microsoft Corporation */
#define WAVE_FORMAT_WMAUDIO_LOSSLESS 0x0163          /* Microsoft Corporation */
#define WAVE_FORMAT_WMASPDIF 0x0164                  /* Microsoft Corporation */
#define WAVE_FORMAT_UNISYS_NAP_ADPCM 0x0170          /* Unisys Corp. */
#define WAVE_FORMAT_UNISYS_NAP_ULAW 0x0171           /* Unisys Corp. */
#define WAVE_FORMAT_UNISYS_NAP_ALAW 0x0172           /* Unisys Corp. */
#define WAVE_FORMAT_UNISYS_NAP_16K 0x0173            /* Unisys Corp. */
#define WAVE_FORMAT_SYCOM_ACM_SYC008 0x0174          /* SyCom Technologies */
#define WAVE_FORMAT_SYCOM_ACM_SYC701_G726L 0x0175    /* SyCom Technologies */
#define WAVE_FORMAT_SYCOM_ACM_SYC701_CELP54 0x0176   /* SyCom Technologies */
#define WAVE_FORMAT_SYCOM_ACM_SYC701_CELP68 0x0177   /* SyCom Technologies */
#define WAVE_FORMAT_KNOWLEDGE_ADVENTURE_ADPCM 0x0178 /* Knowledge Adventure, Inc. */
#define WAVE_FORMAT_FRAUNHOFER_IIS_MPEG2_AAC 0x0180  /* Fraunhofer IIS */
#define WAVE_FORMAT_DTS_DS 0x0190                    /* Digital Theatre Systems, Inc. */
#define WAVE_FORMAT_CREATIVE_ADPCM 0x0200            /* Creative Labs, Inc */
#define WAVE_FORMAT_CREATIVE_FASTSPEECH8 0x0202      /* Creative Labs, Inc */
#define WAVE_FORMAT_CREATIVE_FASTSPEECH10 0x0203     /* Creative Labs, Inc */
#define WAVE_FORMAT_UHER_ADPCM 0x0210                /* UHER informatic GmbH */
#define WAVE_FORMAT_ULEAD_DV_AUDIO 0x0215            /* Ulead Systems, Inc. */
#define WAVE_FORMAT_ULEAD_DV_AUDIO_1 0x0216          /* Ulead Systems, Inc. */
#define WAVE_FORMAT_QUARTERDECK 0x0220               /* Quarterdeck Corporation */
#define WAVE_FORMAT_ILINK_VC 0x0230                  /* I-link Worldwide */
#define WAVE_FORMAT_RAW_SPORT 0x0240                 /* Aureal Semiconductor */
#define WAVE_FORMAT_ESST_AC3 0x0241                  /* ESS Technology, Inc. */
#define WAVE_FORMAT_GENERIC_PASSTHRU 0x0249
#define WAVE_FORMAT_IPI_HSX 0x0250                    /* Interactive Products, Inc. */
#define WAVE_FORMAT_IPI_RPELP 0x0251                  /* Interactive Products, Inc. */
#define WAVE_FORMAT_CS2 0x0260                        /* Consistent Software */
#define WAVE_FORMAT_SONY_SCX 0x0270                   /* Sony Corp. */
#define WAVE_FORMAT_SONY_SCY 0x0271                   /* Sony Corp. */
#define WAVE_FORMAT_SONY_ATRAC3 0x0272                /* Sony Corp. */
#define WAVE_FORMAT_SONY_SPC 0x0273                   /* Sony Corp. */
#define WAVE_FORMAT_TELUM_AUDIO 0x0280                /* Telum Inc. */
#define WAVE_FORMAT_TELUM_IA_AUDIO 0x0281             /* Telum Inc. */
#define WAVE_FORMAT_NORCOM_VOICE_SYSTEMS_ADPCM 0x0285 /* Norcom Electronics Corp. */
#define WAVE_FORMAT_FM_TOWNS_SND 0x0300               /* Fujitsu Corp. */
#define WAVE_FORMAT_MICRONAS 0x0350                   /* Micronas Semiconductors, Inc. */
#define WAVE_FORMAT_MICRONAS_CELP833 0x0351           /* Micronas Semiconductors, Inc. */
#define WAVE_FORMAT_BTV_DIGITAL 0x0400                /* Brooktree Corporation */
#define WAVE_FORMAT_INTEL_MUSIC_CODER 0x0401          /* Intel Corp. */
#define WAVE_FORMAT_INDEO_AUDIO 0x0402                /* Ligos */
#define WAVE_FORMAT_QDESIGN_MUSIC 0x0450              /* QDesign Corporation */
#define WAVE_FORMAT_ON2_VP7_AUDIO 0x0500              /* On2 Technologies */
#define WAVE_FORMAT_ON2_VP6_AUDIO 0x0501              /* On2 Technologies */
#define WAVE_FORMAT_VME_VMPCM 0x0680                  /* AT&T Labs, Inc. */
#define WAVE_FORMAT_TPC 0x0681                        /* AT&T Labs, Inc. */
#define WAVE_FORMAT_LIGHTWAVE_LOSSLESS 0x08AE         /* Clearjump */
#define WAVE_FORMAT_OLIGSM 0x1000                     /* Ing C. Olivetti & C., S.p.A. */
#define WAVE_FORMAT_OLIADPCM 0x1001                   /* Ing C. Olivetti & C., S.p.A. */
#define WAVE_FORMAT_OLICELP 0x1002                    /* Ing C. Olivetti & C., S.p.A. */
#define WAVE_FORMAT_OLISBC 0x1003                     /* Ing C. Olivetti & C., S.p.A. */
#define WAVE_FORMAT_OLIOPR 0x1004                     /* Ing C. Olivetti & C., S.p.A. */
#define WAVE_FORMAT_LH_CODEC 0x1100                   /* Lernout & Hauspie */
#define WAVE_FORMAT_LH_CODEC_CELP 0x1101              /* Lernout & Hauspie */
#define WAVE_FORMAT_LH_CODEC_SBC8 0x1102              /* Lernout & Hauspie */
#define WAVE_FORMAT_LH_CODEC_SBC12 0x1103             /* Lernout & Hauspie */
#define WAVE_FORMAT_LH_CODEC_SBC16 0x1104             /* Lernout & Hauspie */
#define WAVE_FORMAT_NORRIS 0x1400                     /* Norris Communications, Inc. */
#define WAVE_FORMAT_ISIAUDIO_2 0x1401                 /* ISIAudio */
#define WAVE_FORMAT_SOUNDSPACE_MUSICOMPRESS 0x1500    /* AT&T Labs, Inc. */
#define WAVE_FORMAT_MPEG_ADTS_AAC 0x1600              /* Microsoft Corporation */
#define WAVE_FORMAT_MPEG_RAW_AAC 0x1601               /* Microsoft Corporation */
#define WAVE_FORMAT_MPEG_LOAS 0x1602           /* Microsoft Corporation (MPEG-4 Audio Transport Streams (LOAS/LATM) */
#define WAVE_FORMAT_NOKIA_MPEG_ADTS_AAC 0x1608 /* Microsoft Corporation */
#define WAVE_FORMAT_NOKIA_MPEG_RAW_AAC 0x1609  /* Microsoft Corporation */
#define WAVE_FORMAT_VODAFONE_MPEG_ADTS_AAC 0x160A /* Microsoft Corporation */
#define WAVE_FORMAT_VODAFONE_MPEG_RAW_AAC 0x160B  /* Microsoft Corporation */
#define WAVE_FORMAT_MPEG_HEAAC 0x1610             /* Microsoft Corporation (MPEG-2 AAC or MPEG-4 HE-AAC v1/v2 */
                                                  /* streams with any payload (ADTS, ADIF, LOAS/LATM, RAW). */
/* Format block includes MP4 AudioSpecificConfig() -- see HEAACWAVEFORMAT below */
#define WAVE_FORMAT_VOXWARE_RT24_SPEECH 0x181C   /* Voxware Inc. */
#define WAVE_FORMAT_SONICFOUNDRY_LOSSLESS 0x1971 /* Sonic Foundry */
#define WAVE_FORMAT_INNINGS_TELECOM_ADPCM 0x1979 /* Innings Telecom Inc. */
#define WAVE_FORMAT_LUCENT_SX8300P 0x1C07        /* Lucent Technologies */
#define WAVE_FORMAT_LUCENT_SX5363S 0x1C0C        /* Lucent Technologies */
#define WAVE_FORMAT_CUSEEME 0x1F03               /* CUSeeMe */
#define WAVE_FORMAT_NTCSOFT_ALF2CM_ACM 0x1FC4    /* NTCSoft */
#define WAVE_FORMAT_DVM 0x2000                   /* FAST Multimedia AG */
#define WAVE_FORMAT_DTS2 0x2001
#define WAVE_FORMAT_MAKEAVIS 0x3313
#define WAVE_FORMAT_DIVIO_MPEG4_AAC 0x4143          /* Divio, Inc. */
#define WAVE_FORMAT_NOKIA_ADAPTIVE_MULTIRATE 0x4201 /* Nokia */
#define WAVE_FORMAT_DIVIO_G726 0x4243               /* Divio, Inc. */
#define WAVE_FORMAT_LEAD_SPEECH 0x434C              /* LEAD Technologies */
#define WAVE_FORMAT_LEAD_VORBIS 0x564C              /* LEAD Technologies */
#define WAVE_FORMAT_WAVPACK_AUDIO 0x5756            /* xiph.org */
#define WAVE_FORMAT_ALAC 0x6C61                     /* Apple Lossless */
#define WAVE_FORMAT_OGG_VORBIS_MODE_1 0x674F        /* Ogg Vorbis */
#define WAVE_FORMAT_OGG_VORBIS_MODE_2 0x6750        /* Ogg Vorbis */
#define WAVE_FORMAT_OGG_VORBIS_MODE_3 0x6751        /* Ogg Vorbis */
#define WAVE_FORMAT_OGG_VORBIS_MODE_1_PLUS 0x676F   /* Ogg Vorbis */
#define WAVE_FORMAT_OGG_VORBIS_MODE_2_PLUS 0x6770   /* Ogg Vorbis */
#define WAVE_FORMAT_OGG_VORBIS_MODE_3_PLUS 0x6771   /* Ogg Vorbis */
#define WAVE_FORMAT_3COM_NBX 0x7000                 /* 3COM Corp. */
#define WAVE_FORMAT_OPUS 0x704F                     /* Opus */
#define WAVE_FORMAT_FAAD_AAC 0x706D
#define WAVE_FORMAT_AMR_NB 0x7361                  /* AMR Narrowband */
#define WAVE_FORMAT_AMR_WB 0x7362                  /* AMR Wideband */
#define WAVE_FORMAT_AMR_WP 0x7363                  /* AMR Wideband Plus */
#define WAVE_FORMAT_GSM_AMR_CBR 0x7A21             /* GSMA/3GPP */
#define WAVE_FORMAT_GSM_AMR_VBR_SID 0x7A22         /* GSMA/3GPP */
#define WAVE_FORMAT_COMVERSE_INFOSYS_G723_1 0xA100 /* Comverse Infosys */
#define WAVE_FORMAT_COMVERSE_INFOSYS_AVQSBC 0xA101 /* Comverse Infosys */
#define WAVE_FORMAT_COMVERSE_INFOSYS_SBC 0xA102    /* Comverse Infosys */
#define WAVE_FORMAT_SYMBOL_G729_A 0xA103           /* Symbol Technologies */
#define WAVE_FORMAT_VOICEAGE_AMR_WB 0xA104         /* VoiceAge Corp. */
#define WAVE_FORMAT_INGENIENT_G726 0xA105          /* Ingenient Technologies, Inc. */
#define WAVE_FORMAT_MPEG4_AAC 0xA106               /* ISO/MPEG-4 */
#define WAVE_FORMAT_ENCORE_G726 0xA107             /* Encore Software */
#define WAVE_FORMAT_ZOLL_ASAO 0xA108               /* ZOLL Medical Corp. */
#define WAVE_FORMAT_SPEEX_VOICE 0xA109             /* xiph.org */
#define WAVE_FORMAT_VIANIX_MASC 0xA10A             /* Vianix LLC */
#define WAVE_FORMAT_WM9_SPECTRUM_ANALYZER 0xA10B   /* Microsoft */
#define WAVE_FORMAT_WMF_SPECTRUM_ANAYZER 0xA10C    /* Microsoft */
#define WAVE_FORMAT_GSM_610 0xA10D
#define WAVE_FORMAT_GSM_620 0xA10E
#define WAVE_FORMAT_GSM_660 0xA10F
#define WAVE_FORMAT_GSM_690 0xA110
#define WAVE_FORMAT_GSM_ADAPTIVE_MULTIRATE_WB 0xA111
#define WAVE_FORMAT_POLYCOM_G722 0xA112               /* Polycom */
#define WAVE_FORMAT_POLYCOM_G728 0xA113               /* Polycom */
#define WAVE_FORMAT_POLYCOM_G729_A 0xA114             /* Polycom */
#define WAVE_FORMAT_POLYCOM_SIREN 0xA115              /* Polycom */
#define WAVE_FORMAT_GLOBAL_IP_ILBC 0xA116             /* Global IP */
#define WAVE_FORMAT_RADIOTIME_TIME_SHIFT_RADIO 0xA117 /* RadioTime */
#define WAVE_FORMAT_NICE_ACA 0xA118                   /* Nice Systems */
#define WAVE_FORMAT_NICE_ADPCM 0xA119                 /* Nice Systems */
#define WAVE_FORMAT_VOCORD_G721 0xA11A                /* Vocord Telecom */
#define WAVE_FORMAT_VOCORD_G726 0xA11B                /* Vocord Telecom */
#define WAVE_FORMAT_VOCORD_G722_1 0xA11C              /* Vocord Telecom */
#define WAVE_FORMAT_VOCORD_G728 0xA11D                /* Vocord Telecom */
#define WAVE_FORMAT_VOCORD_G729 0xA11E                /* Vocord Telecom */
#define WAVE_FORMAT_VOCORD_G729_A 0xA11F              /* Vocord Telecom */
#define WAVE_FORMAT_VOCORD_G723_1 0xA120              /* Vocord Telecom */
#define WAVE_FORMAT_VOCORD_LBC 0xA121                 /* Vocord Telecom */
#define WAVE_FORMAT_NICE_G728 0xA122                  /* Nice Systems */
#define WAVE_FORMAT_FRACE_TELECOM_G729 0xA123         /* France Telecom */
#define WAVE_FORMAT_CODIAN 0xA124                     /* CODIAN */
#define WAVE_FORMAT_FLAC 0xF1AC                       /* flac.sourceforge.net */
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE                 /* Microsoft */

#define KSDATAFORMAT_SUBTYPE_ANALOG Guid("6dba3190-67bd-11cf-a0f7-0020afd156e4")
#define KSDATAFORMAT_SUBTYPE_PCM Guid("00000001-0000-0010-8000-00aa00389b71")
#define KSDATAFORMAT_SUBTYPE_IEEE_FLOAT Guid("00000003-0000-0010-8000-00aa00389b71")
#define KSDATAFORMAT_SUBTYPE_ALAW Guid("00000006-0000-0010-8000-00aa00389b71")
#define KSDATAFORMAT_SUBTYPE_MULAW Guid("00000007-0000-0010-8000-00aa00389b71")
#define KSDATAFORMAT_SUBTYPE_ADPCM Guid("00000002-0000-0010-8000-00aa00389b71")
#define KSDATAFORMAT_SUBTYPE_MPEG Guid("00000050-0000-0010-8000-00aa00389b71")
#endif  // WAV_HEADER_H