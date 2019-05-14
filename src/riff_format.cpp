#include "riff_format.h"

#include <algorithm>
#include <cstdint>
#include <map>
#include <regex>
#include <string>

using namespace std;
// maps uint16_t codes for audio_formats to logical names
map<uint16_t, string> audio_format_uint16_to_names = {
    {WAVE_FORMAT_UNKNOWN, "UNKNOWN"},                                       /* Microsoft Corporation */
    {WAVE_FORMAT_PCM, "PCM"},                                               /* Microsoft Corporation */
    {WAVE_FORMAT_ADPCM, "ADPCM"},                                           /* Microsoft Corporation */
    {WAVE_FORMAT_IEEE_FLOAT, "IEEE_FLOAT"},                                 /* Microsoft Corporation */
    {WAVE_FORMAT_VSELP, "VSELP"},                                           /* Compaq Computer Corp. */
    {WAVE_FORMAT_IBM_CVSD, "IBM_CVSD"},                                     /* IBM Corporation */
    {WAVE_FORMAT_ALAW, "ALAW"},                                             /* Microsoft Corporation */
    {WAVE_FORMAT_MULAW, "MULAW"},                                           /* Microsoft Corporation */
    {WAVE_FORMAT_DTS, "DTS"},                                               /* Microsoft Corporation */
    {WAVE_FORMAT_DRM, "DRM"},                                               /* Microsoft Corporation */
    {WAVE_FORMAT_WMAVOICE9, "WMAVOICE9"},                                   /* Microsoft Corporation */
    {WAVE_FORMAT_WMAVOICE10, "WMAVOICE10"},                                 /* Microsoft Corporation */
    {WAVE_FORMAT_OKI_ADPCM, "OKI_ADPCM"},                                   /* OKI */
    {WAVE_FORMAT_DVI_ADPCM, "DVI_ADPCM"},                                   /* Intel Corporation */
    {WAVE_FORMAT_MEDIASPACE_ADPCM, "MEDIASPACE_ADPCM"},                     /* Videologic */
    {WAVE_FORMAT_SIERRA_ADPCM, "SIERRA_ADPCM"},                             /* Sierra Semiconductor Corp */
    {WAVE_FORMAT_G723_ADPCM, "G723_ADPCM"},                                 /* Antex Electronics Corporation */
    {WAVE_FORMAT_DIGISTD, "DIGISTD"},                                       /* DSP Solutions, Inc. */
    {WAVE_FORMAT_DIGIFIX, "DIGIFIX"},                                       /* DSP Solutions, Inc. */
    {WAVE_FORMAT_DIALOGIC_OKI_ADPCM, "DIALOGIC_OKI_ADPCM"},                 /* Dialogic Corporation */
    {WAVE_FORMAT_MEDIAVISION_ADPCM, "MEDIAVISION_ADPCM"},                   /* Media Vision, Inc. */
    {WAVE_FORMAT_CU_CODEC, "CU_CODEC"},                                     /* Hewlett-Packard Company */
    {WAVE_FORMAT_HP_DYN_VOICE, "HP_DYN_VOICE"},                             /* Hewlett-Packard Company */
    {WAVE_FORMAT_YAMAHA_ADPCM, "YAMAHA_ADPCM"},                             /* Yamaha Corporation of America */
    {WAVE_FORMAT_SONARC, "SONARC"},                                         /* Speech Compression */
    {WAVE_FORMAT_DSPGROUP_TRUESPEECH, "DSPGROUP_TRUESPEECH"},               /* DSP Group, Inc */
    {WAVE_FORMAT_ECHOSC1, "ECHOSC1"},                                       /* Echo Speech Corporation */
    {WAVE_FORMAT_AUDIOFILE_AF36, "AUDIOFILE_AF36"},                         /* Virtual Music, Inc. */
    {WAVE_FORMAT_APTX, "APTX"},                                             /* Audio Processing Technology */
    {WAVE_FORMAT_AUDIOFILE_AF10, "AUDIOFILE_AF10"},                         /* Virtual Music, Inc. */
    {WAVE_FORMAT_PROSODY_1612, "PROSODY_1612"},                             /* Aculab plc */
    {WAVE_FORMAT_LRC, "LRC"},                                               /* Merging Technologies S.A. */
    {WAVE_FORMAT_DOLBY_AC2, "DOLBY_AC2"},                                   /* Dolby Laboratories */
    {WAVE_FORMAT_GSM610, "GSM610"},                                         /* Microsoft Corporation */
    {WAVE_FORMAT_MSNAUDIO, "MSNAUDIO"},                                     /* Microsoft Corporation */
    {WAVE_FORMAT_ANTEX_ADPCME, "ANTEX_ADPCME"},                             /* Antex Electronics Corporation */
    {WAVE_FORMAT_CONTROL_RES_VQLPC, "CONTROL_RES_VQLPC"},                   /* Control Resources Limited */
    {WAVE_FORMAT_DIGIREAL, "DIGIREAL"},                                     /* DSP Solutions, Inc. */
    {WAVE_FORMAT_DIGIADPCM, "DIGIADPCM"},                                   /* DSP Solutions, Inc. */
    {WAVE_FORMAT_CONTROL_RES_CR10, "CONTROL_RES_CR10"},                     /* Control Resources Limited */
    {WAVE_FORMAT_NMS_VBXADPCM, "NMS_VBXADPCM"},                             /* Natural MicroSystems */
    {WAVE_FORMAT_CS_IMAADPCM, "CS_IMAADPCM"},                               /* Crystal Semiconductor IMA ADPCM */
    {WAVE_FORMAT_ECHOSC3, "ECHOSC3"},                                       /* Echo Speech Corporation */
    {WAVE_FORMAT_ROCKWELL_ADPCM, "ROCKWELL_ADPCM"},                         /* Rockwell International */
    {WAVE_FORMAT_ROCKWELL_DIGITALK, "ROCKWELL_DIGITALK"},                   /* Rockwell International */
    {WAVE_FORMAT_XEBEC, "XEBEC"},                                           /* Xebec Multimedia Solutions Limited */
    {WAVE_FORMAT_G721_ADPCM, "G721_ADPCM"},                                 /* Antex Electronics Corporation */
    {WAVE_FORMAT_G728_CELP, "G728_CELP"},                                   /* Antex Electronics Corporation */
    {WAVE_FORMAT_MSG723, "MSG723"},                                         /* Microsoft Corporation */
    {WAVE_FORMAT_INTEL_G723_1, "INTEL_G723_1"},                             /* Intel Corp. */
    {WAVE_FORMAT_INTEL_G729, "INTEL_G729"},                                 /* Intel Corp. */
    {WAVE_FORMAT_SHARP_G726, "SHARP_G726"},                                 /* Sharp */
    {WAVE_FORMAT_MPEG, "MPEG"},                                             /* Microsoft Corporation */
    {WAVE_FORMAT_RT24, "RT24"},                                             /* InSoft, Inc. */
    {WAVE_FORMAT_PAC, "PAC"},                                               /* InSoft, Inc. */
    {WAVE_FORMAT_MPEGLAYER3, "MPEGLAYER3"},                                 /* ISO/MPEG Layer3 Format Tag */
    {WAVE_FORMAT_LUCENT_G723, "LUCENT_G723"},                               /* Lucent Technologies */
    {WAVE_FORMAT_CIRRUS, "CIRRUS"},                                         /* Cirrus Logic */
    {WAVE_FORMAT_ESPCM, "ESPCM"},                                           /* ESS Technology */
    {WAVE_FORMAT_VOXWARE, "VOXWARE"},                                       /* Voxware Inc */
    {WAVE_FORMAT_CANOPUS_ATRAC, "CANOPUS_ATRAC"},                           /* Canopus, co., Ltd. */
    {WAVE_FORMAT_G726_ADPCM, "G726_ADPCM"},                                 /* APICOM */
    {WAVE_FORMAT_G722_ADPCM, "G722_ADPCM"},                                 /* APICOM */
    {WAVE_FORMAT_DSAT, "DSAT"},                                             /* Microsoft Corporation */
    {WAVE_FORMAT_DSAT_DISPLAY, "DSAT_DISPLAY"},                             /* Microsoft Corporation */
    {WAVE_FORMAT_VOXWARE_BYTE_ALIGNED, "VOXWARE_BYTE_ALIGNED"},             /* Voxware Inc */
    {WAVE_FORMAT_VOXWARE_AC8, "VOXWARE_AC8"},                               /* Voxware Inc */
    {WAVE_FORMAT_VOXWARE_AC10, "VOXWARE_AC10"},                             /* Voxware Inc */
    {WAVE_FORMAT_VOXWARE_AC16, "VOXWARE_AC16"},                             /* Voxware Inc */
    {WAVE_FORMAT_VOXWARE_AC20, "VOXWARE_AC20"},                             /* Voxware Inc */
    {WAVE_FORMAT_VOXWARE_RT24, "VOXWARE_RT24"},                             /* Voxware Inc */
    {WAVE_FORMAT_VOXWARE_RT29, "VOXWARE_RT29"},                             /* Voxware Inc */
    {WAVE_FORMAT_VOXWARE_RT29HW, "VOXWARE_RT29HW"},                         /* Voxware Inc */
    {WAVE_FORMAT_VOXWARE_VR12, "VOXWARE_VR12"},                             /* Voxware Inc */
    {WAVE_FORMAT_VOXWARE_VR18, "VOXWARE_VR18"},                             /* Voxware Inc */
    {WAVE_FORMAT_VOXWARE_TQ40, "VOXWARE_TQ40"},                             /* Voxware Inc */
    {WAVE_FORMAT_VOXWARE_SC3, "VOXWARE_SC3"},                               /* Voxware Inc */
    {WAVE_FORMAT_VOXWARE_SC3_1, "VOXWARE_SC3_1"},                           /* Voxware Inc */
    {WAVE_FORMAT_SOFTSOUND, "SOFTSOUND"},                                   /* Softsound, Ltd. */
    {WAVE_FORMAT_VOXWARE_TQ60, "VOXWARE_TQ60"},                             /* Voxware Inc */
    {WAVE_FORMAT_MSRT24, "MSRT24"},                                         /* Microsoft Corporation */
    {WAVE_FORMAT_G729A, "G729A"},                                           /* AT&T Labs, Inc. */
    {WAVE_FORMAT_MVI_MVI2, "MVI_MVI2"},                                     /* Motion Pixels */
    {WAVE_FORMAT_DF_G726, "DF_G726"},                                       /* DataFusion Systems (Pty) (Ltd) */
    {WAVE_FORMAT_DF_GSM610, "DF_GSM610"},                                   /* DataFusion Systems (Pty) (Ltd) */
    {WAVE_FORMAT_ISIAUDIO, "ISIAUDIO"},                                     /* Iterated Systems, Inc. */
    {WAVE_FORMAT_ONLIVE, "ONLIVE"},                                         /* OnLive! Technologies, Inc. */
    {WAVE_FORMAT_MULTITUDE_FT_SX20, "MULTITUDE_FT_SX20"},                   /* Multitude Inc. */
    {WAVE_FORMAT_INFOCOM_ITS_G721_ADPCM, "INFOCOM_ITS_G721_ADPCM"},         /* Infocom */
    {WAVE_FORMAT_CONVEDIA_G729, "CONVEDIA_G729"},                           /* Convedia Corp. */
    {WAVE_FORMAT_CONGRUENCY, "CONGRUENCY"},                                 /* Congruency Inc. */
    {WAVE_FORMAT_SBC24, "SBC24"},                                           /* Siemens Business Communications Sys */
    {WAVE_FORMAT_DOLBY_AC3_SPDIF, "DOLBY_AC3_SPDIF"},                       /* Sonic Foundry */
    {WAVE_FORMAT_MEDIASONIC_G723, "MEDIASONIC_G723"},                       /* MediaSonic */
    {WAVE_FORMAT_PROSODY_8KBPS, "PROSODY_8KBPS"},                           /* Aculab plc */
    {WAVE_FORMAT_ZYXEL_ADPCM, "ZYXEL_ADPCM"},                               /* ZyXEL Communications, Inc. */
    {WAVE_FORMAT_PHILIPS_LPCBB, "PHILIPS_LPCBB"},                           /* Philips Speech Processing */
    {WAVE_FORMAT_PACKED, "PACKED"},                                         /* Studer Professional Audio AG */
    {WAVE_FORMAT_MALDEN_PHONYTALK, "MALDEN_PHONYTALK"},                     /* Malden Electronics Ltd. */
    {WAVE_FORMAT_RACAL_RECORDER_GSM, "RACAL_RECORDER_GSM"},                 /* Racal recorders */
    {WAVE_FORMAT_RACAL_RECORDER_G720_A, "RACAL_RECORDER_G720_A"},           /* Racal recorders */
    {WAVE_FORMAT_RACAL_RECORDER_G723_1, "RACAL_RECORDER_G723_1"},           /* Racal recorders */
    {WAVE_FORMAT_RACAL_RECORDER_TETRA_ACELP, "RACAL_RECORDER_TETRA_ACELP"}, /* Racal recorders */
    {WAVE_FORMAT_NEC_AAC, "NEC_AAC"},                                       /* NEC Corp. */
    {WAVE_FORMAT_RAW_AAC1, "RAW_AAC1"}, /* For Raw AAC, with format block AudioSpecificConfig() (as defined by MPEG-4),
                                           that follows WAVEFORMATEX */
    {WAVE_FORMAT_RHETOREX_ADPCM, "RHETOREX_ADPCM"},                       /* Rhetorex Inc. */
    {WAVE_FORMAT_IRAT, "IRAT"},                                           /* BeCubed Software Inc. */
    {WAVE_FORMAT_VIVO_G723, "VIVO_G723"},                                 /* Vivo Software */
    {WAVE_FORMAT_VIVO_SIREN, "VIVO_SIREN"},                               /* Vivo Software */
    {WAVE_FORMAT_PHILIPS_CELP, "PHILIPS_CELP"},                           /* Philips Speech Processing */
    {WAVE_FORMAT_PHILIPS_GRUNDIG, "PHILIPS_GRUNDIG"},                     /* Philips Speech Processing */
    {WAVE_FORMAT_DIGITAL_G723, "DIGITAL_G723"},                           /* Digital Equipment Corporation */
    {WAVE_FORMAT_SANYO_LD_ADPCM, "SANYO_LD_ADPCM"},                       /* Sanyo Electric Co., Ltd. */
    {WAVE_FORMAT_SIPROLAB_ACEPLNET, "SIPROLAB_ACEPLNET"},                 /* Sipro Lab Telecom Inc. */
    {WAVE_FORMAT_SIPROLAB_ACELP4800, "SIPROLAB_ACELP4800"},               /* Sipro Lab Telecom Inc. */
    {WAVE_FORMAT_SIPROLAB_ACELP8V3, "SIPROLAB_ACELP8V3"},                 /* Sipro Lab Telecom Inc. */
    {WAVE_FORMAT_SIPROLAB_G729, "SIPROLAB_G729"},                         /* Sipro Lab Telecom Inc. */
    {WAVE_FORMAT_SIPROLAB_G729A, "SIPROLAB_G729A"},                       /* Sipro Lab Telecom Inc. */
    {WAVE_FORMAT_SIPROLAB_KELVIN, "SIPROLAB_KELVIN"},                     /* Sipro Lab Telecom Inc. */
    {WAVE_FORMAT_VOICEAGE_AMR, "VOICEAGE_AMR"},                           /* VoiceAge Corp. */
    {WAVE_FORMAT_G726ADPCM, "G726ADPCM"},                                 /* Dictaphone Corporation */
    {WAVE_FORMAT_DICTAPHONE_CELP68, "DICTAPHONE_CELP68"},                 /* Dictaphone Corporation */
    {WAVE_FORMAT_DICTAPHONE_CELP54, "DICTAPHONE_CELP54"},                 /* Dictaphone Corporation */
    {WAVE_FORMAT_QUALCOMM_PUREVOICE, "QUALCOMM_PUREVOICE"},               /* Qualcomm, Inc. */
    {WAVE_FORMAT_QUALCOMM_HALFRATE, "QUALCOMM_HALFRATE"},                 /* Qualcomm, Inc. */
    {WAVE_FORMAT_TUBGSM, "TUBGSM"},                                       /* Ring Zero Systems, Inc. */
    {WAVE_FORMAT_MSAUDIO1, "MSAUDIO1"},                                   /* Microsoft Corporation */
    {WAVE_FORMAT_WMAUDIO2, "WMAUDIO2"},                                   /* Microsoft Corporation */
    {WAVE_FORMAT_WMAUDIO3, "WMAUDIO3"},                                   /* Microsoft Corporation */
    {WAVE_FORMAT_WMAUDIO_LOSSLESS, "WMAUDIO_LOSSLESS"},                   /* Microsoft Corporation */
    {WAVE_FORMAT_WMASPDIF, "WMASPDIF"},                                   /* Microsoft Corporation */
    {WAVE_FORMAT_UNISYS_NAP_ADPCM, "UNISYS_NAP_ADPCM"},                   /* Unisys Corp. */
    {WAVE_FORMAT_UNISYS_NAP_ULAW, "UNISYS_NAP_ULAW"},                     /* Unisys Corp. */
    {WAVE_FORMAT_UNISYS_NAP_ALAW, "UNISYS_NAP_ALAW"},                     /* Unisys Corp. */
    {WAVE_FORMAT_UNISYS_NAP_16K, "UNISYS_NAP_16K"},                       /* Unisys Corp. */
    {WAVE_FORMAT_SYCOM_ACM_SYC008, "SYCOM_ACM_SYC008"},                   /* SyCom Technologies */
    {WAVE_FORMAT_SYCOM_ACM_SYC701_G726L, "SYCOM_ACM_SYC701_G726L"},       /* SyCom Technologies */
    {WAVE_FORMAT_SYCOM_ACM_SYC701_CELP54, "SYCOM_ACM_SYC701_CELP54"},     /* SyCom Technologies */
    {WAVE_FORMAT_SYCOM_ACM_SYC701_CELP68, "SYCOM_ACM_SYC701_CELP68"},     /* SyCom Technologies */
    {WAVE_FORMAT_KNOWLEDGE_ADVENTURE_ADPCM, "KNOWLEDGE_ADVENTURE_ADPCM"}, /* Knowledge Adventure, Inc. */
    {WAVE_FORMAT_FRAUNHOFER_IIS_MPEG2_AAC, "FRAUNHOFER_IIS_MPEG2_AAC"},   /* Fraunhofer IIS */
    {WAVE_FORMAT_DTS_DS, "DTS_DS"},                                       /* Digital Theatre Systems, Inc. */
    {WAVE_FORMAT_CREATIVE_ADPCM, "CREATIVE_ADPCM"},                       /* Creative Labs, Inc */
    {WAVE_FORMAT_CREATIVE_FASTSPEECH8, "CREATIVE_FASTSPEECH8"},           /* Creative Labs, Inc */
    {WAVE_FORMAT_CREATIVE_FASTSPEECH10, "CREATIVE_FASTSPEECH10"},         /* Creative Labs, Inc */
    {WAVE_FORMAT_UHER_ADPCM, "UHER_ADPCM"},                               /* UHER informatic GmbH */
    {WAVE_FORMAT_ULEAD_DV_AUDIO, "ULEAD_DV_AUDIO"},                       /* Ulead Systems, Inc. */
    {WAVE_FORMAT_ULEAD_DV_AUDIO_1, "ULEAD_DV_AUDIO_1"},                   /* Ulead Systems, Inc. */
    {WAVE_FORMAT_QUARTERDECK, "QUARTERDECK"},                             /* Quarterdeck Corporation */
    {WAVE_FORMAT_ILINK_VC, "ILINK_VC"},                                   /* I-link Worldwide */
    {WAVE_FORMAT_RAW_SPORT, "RAW_SPORT"},                                 /* Aureal Semiconductor */
    {WAVE_FORMAT_ESST_AC3, "ESST_AC3"},                                   /* ESS Technology, Inc. */
    {WAVE_FORMAT_GENERIC_PASSTHRU, "GENERIC_PASSTHRU"},
    {WAVE_FORMAT_IPI_HSX, "IPI_HSX"},                                       /* Interactive Products, Inc. */
    {WAVE_FORMAT_IPI_RPELP, "IPI_RPELP"},                                   /* Interactive Products, Inc. */
    {WAVE_FORMAT_CS2, "CS2"},                                               /* Consistent Software */
    {WAVE_FORMAT_SONY_SCX, "SONY_SCX"},                                     /* Sony Corp. */
    {WAVE_FORMAT_SONY_SCY, "SONY_SCY"},                                     /* Sony Corp. */
    {WAVE_FORMAT_SONY_ATRAC3, "SONY_ATRAC3"},                               /* Sony Corp. */
    {WAVE_FORMAT_SONY_SPC, "SONY_SPC"},                                     /* Sony Corp. */
    {WAVE_FORMAT_TELUM_AUDIO, "TELUM_AUDIO"},                               /* Telum Inc. */
    {WAVE_FORMAT_TELUM_IA_AUDIO, "TELUM_IA_AUDIO"},                         /* Telum Inc. */
    {WAVE_FORMAT_NORCOM_VOICE_SYSTEMS_ADPCM, "NORCOM_VOICE_SYSTEMS_ADPCM"}, /* Norcom Electronics Corp. */
    {WAVE_FORMAT_FM_TOWNS_SND, "FM_TOWNS_SND"},                             /* Fujitsu Corp. */
    {WAVE_FORMAT_MICRONAS, "MICRONAS"},                                     /* Micronas Semiconductors, Inc. */
    {WAVE_FORMAT_MICRONAS_CELP833, "MICRONAS_CELP833"},                     /* Micronas Semiconductors, Inc. */
    {WAVE_FORMAT_BTV_DIGITAL, "BTV_DIGITAL"},                               /* Brooktree Corporation */
    {WAVE_FORMAT_INTEL_MUSIC_CODER, "INTEL_MUSIC_CODER"},                   /* Intel Corp. */
    {WAVE_FORMAT_INDEO_AUDIO, "INDEO_AUDIO"},                               /* Ligos */
    {WAVE_FORMAT_QDESIGN_MUSIC, "QDESIGN_MUSIC"},                           /* QDesign Corporation */
    {WAVE_FORMAT_ON2_VP7_AUDIO, "ON2_VP7_AUDIO"},                           /* On2 Technologies */
    {WAVE_FORMAT_ON2_VP6_AUDIO, "ON2_VP6_AUDIO"},                           /* On2 Technologies */
    {WAVE_FORMAT_VME_VMPCM, "VME_VMPCM"},                                   /* AT&T Labs, Inc. */
    {WAVE_FORMAT_TPC, "TPC"},                                               /* AT&T Labs, Inc. */
    {WAVE_FORMAT_LIGHTWAVE_LOSSLESS, "LIGHTWAVE_LOSSLESS"},                 /* Clearjump */
    {WAVE_FORMAT_OLIGSM, "OLIGSM"},                                         /* Ing C. Olivetti & C., S.p.A. */
    {WAVE_FORMAT_OLIADPCM, "OLIADPCM"},                                     /* Ing C. Olivetti & C., S.p.A. */
    {WAVE_FORMAT_OLICELP, "OLICELP"},                                       /* Ing C. Olivetti & C., S.p.A. */
    {WAVE_FORMAT_OLISBC, "OLISBC"},                                         /* Ing C. Olivetti & C., S.p.A. */
    {WAVE_FORMAT_OLIOPR, "OLIOPR"},                                         /* Ing C. Olivetti & C., S.p.A. */
    {WAVE_FORMAT_LH_CODEC, "LH_CODEC"},                                     /* Lernout & Hauspie */
    {WAVE_FORMAT_LH_CODEC_CELP, "LH_CODEC_CELP"},                           /* Lernout & Hauspie */
    {WAVE_FORMAT_LH_CODEC_SBC8, "LH_CODEC_SBC8"},                           /* Lernout & Hauspie */
    {WAVE_FORMAT_LH_CODEC_SBC12, "LH_CODEC_SBC12"},                         /* Lernout & Hauspie */
    {WAVE_FORMAT_LH_CODEC_SBC16, "LH_CODEC_SBC16"},                         /* Lernout & Hauspie */
    {WAVE_FORMAT_NORRIS, "NORRIS"},                                         /* Norris Communications, Inc. */
    {WAVE_FORMAT_ISIAUDIO_2, "ISIAUDIO_2"},                                 /* ISIAudio */
    {WAVE_FORMAT_SOUNDSPACE_MUSICOMPRESS, "SOUNDSPACE_MUSICOMPRESS"},       /* AT&T Labs, Inc. */
    {WAVE_FORMAT_MPEG_ADTS_AAC, "MPEG_ADTS_AAC"},                           /* Microsoft Corporation */
    {WAVE_FORMAT_MPEG_RAW_AAC, "MPEG_RAW_AAC"},                             /* Microsoft Corporation */
    {WAVE_FORMAT_MPEG_LOAS, "MPEG_LOAS"}, /* Microsoft Corporation (MPEG-4 Audio Transport Streams (LOAS/LATM) */
    {WAVE_FORMAT_NOKIA_MPEG_ADTS_AAC, "NOKIA_MPEG_ADTS_AAC"},       /* Microsoft Corporation */
    {WAVE_FORMAT_NOKIA_MPEG_RAW_AAC, "NOKIA_MPEG_RAW_AAC"},         /* Microsoft Corporation */
    {WAVE_FORMAT_VODAFONE_MPEG_ADTS_AAC, "VODAFONE_MPEG_ADTS_AAC"}, /* Microsoft Corporation */
    {WAVE_FORMAT_VODAFONE_MPEG_RAW_AAC, "VODAFONE_MPEG_RAW_AAC"},   /* Microsoft Corporation */
    {WAVE_FORMAT_MPEG_HEAAC,
     "MPEG_HEAAC"}, /* Microsoft Corporation (MPEG-2 AAC or MPEG-4 HE-AAC v1/v2 streams with any payload (ADTS, ADIF,
                       LOAS/LATM, RAW). Format block includes MP4 AudioSpecificConfig() -- see HEAACWAVEFORMAT below */
    {WAVE_FORMAT_VOXWARE_RT24_SPEECH, "VOXWARE_RT24_SPEECH"},     /* Voxware Inc. */
    {WAVE_FORMAT_SONICFOUNDRY_LOSSLESS, "SONICFOUNDRY_LOSSLESS"}, /* Sonic Foundry */
    {WAVE_FORMAT_INNINGS_TELECOM_ADPCM, "INNINGS_TELECOM_ADPCM"}, /* Innings Telecom Inc. */
    {WAVE_FORMAT_LUCENT_SX8300P, "LUCENT_SX8300P"},               /* Lucent Technologies */
    {WAVE_FORMAT_LUCENT_SX5363S, "LUCENT_SX5363S"},               /* Lucent Technologies */
    {WAVE_FORMAT_CUSEEME, "CUSEEME"},                             /* CUSeeMe */
    {WAVE_FORMAT_NTCSOFT_ALF2CM_ACM, "NTCSOFT_ALF2CM_ACM"},       /* NTCSoft */
    {WAVE_FORMAT_DVM, "DVM"},                                     /* FAST Multimedia AG */
    {WAVE_FORMAT_DTS2, "DTS2"},
    {WAVE_FORMAT_MAKEAVIS, "MAKEAVIS"},
    {WAVE_FORMAT_DIVIO_MPEG4_AAC, "DIVIO_MPEG4_AAC"},                   /* Divio, Inc. */
    {WAVE_FORMAT_NOKIA_ADAPTIVE_MULTIRATE, "NOKIA_ADAPTIVE_MULTIRATE"}, /* Nokia */
    {WAVE_FORMAT_DIVIO_G726, "DIVIO_G726"},                             /* Divio, Inc. */
    {WAVE_FORMAT_LEAD_SPEECH, "LEAD_SPEECH"},                           /* LEAD Technologies */
    {WAVE_FORMAT_LEAD_VORBIS, "LEAD_VORBIS"},                           /* LEAD Technologies */
    {WAVE_FORMAT_WAVPACK_AUDIO, "WAVPACK_AUDIO"},                       /* xiph.org */
    {WAVE_FORMAT_ALAC, "ALAC"},                                         /* Apple Lossless */
    {WAVE_FORMAT_OGG_VORBIS_MODE_1, "OGG_VORBIS_MODE_1"},               /* Ogg Vorbis */
    {WAVE_FORMAT_OGG_VORBIS_MODE_2, "OGG_VORBIS_MODE_2"},               /* Ogg Vorbis */
    {WAVE_FORMAT_OGG_VORBIS_MODE_3, "OGG_VORBIS_MODE_3"},               /* Ogg Vorbis */
    {WAVE_FORMAT_OGG_VORBIS_MODE_1_PLUS, "OGG_VORBIS_MODE_1_PLUS"},     /* Ogg Vorbis */
    {WAVE_FORMAT_OGG_VORBIS_MODE_2_PLUS, "OGG_VORBIS_MODE_2_PLUS"},     /* Ogg Vorbis */
    {WAVE_FORMAT_OGG_VORBIS_MODE_3_PLUS, "OGG_VORBIS_MODE_3_PLUS"},     /* Ogg Vorbis */
    {WAVE_FORMAT_3COM_NBX, "3COM_NBX"},                                 /* 3COM Corp. */
    {WAVE_FORMAT_OPUS, "OPUS"},                                         /* Opus */
    {WAVE_FORMAT_FAAD_AAC, "FAAD_AAC"},
    {WAVE_FORMAT_AMR_NB, "AMR_NB"},                                   /* AMR Narrowband */
    {WAVE_FORMAT_AMR_WB, "AMR_WB"},                                   /* AMR Wideband */
    {WAVE_FORMAT_AMR_WP, "AMR_WP"},                                   /* AMR Wideband Plus */
    {WAVE_FORMAT_GSM_AMR_CBR, "GSM_AMR_CBR"},                         /* GSMA/3GPP */
    {WAVE_FORMAT_GSM_AMR_VBR_SID, "GSM_AMR_VBR_SID"},                 /* GSMA/3GPP */
    {WAVE_FORMAT_COMVERSE_INFOSYS_G723_1, "COMVERSE_INFOSYS_G723_1"}, /* Comverse Infosys */
    {WAVE_FORMAT_COMVERSE_INFOSYS_AVQSBC, "COMVERSE_INFOSYS_AVQSBC"}, /* Comverse Infosys */
    {WAVE_FORMAT_COMVERSE_INFOSYS_SBC, "COMVERSE_INFOSYS_SBC"},       /* Comverse Infosys */
    {WAVE_FORMAT_SYMBOL_G729_A, "SYMBOL_G729_A"},                     /* Symbol Technologies */
    {WAVE_FORMAT_VOICEAGE_AMR_WB, "VOICEAGE_AMR_WB"},                 /* VoiceAge Corp. */
    {WAVE_FORMAT_INGENIENT_G726, "INGENIENT_G726"},                   /* Ingenient Technologies, Inc. */
    {WAVE_FORMAT_MPEG4_AAC, "MPEG4_AAC"},                             /* ISO/MPEG-4 */
    {WAVE_FORMAT_ENCORE_G726, "ENCORE_G726"},                         /* Encore Software */
    {WAVE_FORMAT_ZOLL_ASAO, "ZOLL_ASAO"},                             /* ZOLL Medical Corp. */
    {WAVE_FORMAT_SPEEX_VOICE, "SPEEX_VOICE"},                         /* xiph.org */
    {WAVE_FORMAT_VIANIX_MASC, "VIANIX_MASC"},                         /* Vianix LLC */
    {WAVE_FORMAT_WM9_SPECTRUM_ANALYZER, "WM9_SPECTRUM_ANALYZER"},     /* Microsoft */
    {WAVE_FORMAT_WMF_SPECTRUM_ANAYZER, "WMF_SPECTRUM_ANAYZER"},       /* Microsoft */
    {WAVE_FORMAT_GSM_610, "GSM_610"},
    {WAVE_FORMAT_GSM_620, "GSM_620"},
    {WAVE_FORMAT_GSM_660, "GSM_660"},
    {WAVE_FORMAT_GSM_690, "GSM_690"},
    {WAVE_FORMAT_GSM_ADAPTIVE_MULTIRATE_WB, "GSM_ADAPTIVE_MULTIRATE_WB"},
    {WAVE_FORMAT_POLYCOM_G722, "POLYCOM_G722"},                             /* Polycom */
    {WAVE_FORMAT_POLYCOM_G728, "POLYCOM_G728"},                             /* Polycom */
    {WAVE_FORMAT_POLYCOM_G729_A, "POLYCOM_G729_A"},                         /* Polycom */
    {WAVE_FORMAT_POLYCOM_SIREN, "POLYCOM_SIREN"},                           /* Polycom */
    {WAVE_FORMAT_GLOBAL_IP_ILBC, "GLOBAL_IP_ILBC"},                         /* Global IP */
    {WAVE_FORMAT_RADIOTIME_TIME_SHIFT_RADIO, "RADIOTIME_TIME_SHIFT_RADIO"}, /* RadioTime */
    {WAVE_FORMAT_NICE_ACA, "NICE_ACA"},                                     /* Nice Systems */
    {WAVE_FORMAT_NICE_ADPCM, "NICE_ADPCM"},                                 /* Nice Systems */
    {WAVE_FORMAT_VOCORD_G721, "VOCORD_G721"},                               /* Vocord Telecom */
    {WAVE_FORMAT_VOCORD_G726, "VOCORD_G726"},                               /* Vocord Telecom */
    {WAVE_FORMAT_VOCORD_G722_1, "VOCORD_G722_1"},                           /* Vocord Telecom */
    {WAVE_FORMAT_VOCORD_G728, "VOCORD_G728"},                               /* Vocord Telecom */
    {WAVE_FORMAT_VOCORD_G729, "VOCORD_G729"},                               /* Vocord Telecom */
    {WAVE_FORMAT_VOCORD_G729_A, "VOCORD_G729_A"},                           /* Vocord Telecom */
    {WAVE_FORMAT_VOCORD_G723_1, "VOCORD_G723_1"},                           /* Vocord Telecom */
    {WAVE_FORMAT_VOCORD_LBC, "VOCORD_LBC"},                                 /* Vocord Telecom */
    {WAVE_FORMAT_NICE_G728, "NICE_G728"},                                   /* Nice Systems */
    {WAVE_FORMAT_FRACE_TELECOM_G729, "FRACE_TELECOM_G729"},                 /* France Telecom */
    {WAVE_FORMAT_CODIAN, "CODIAN"},                                         /* CODIAN */
    {WAVE_FORMAT_FLAC, "FLAC"},                                             /* flac.sourceforge.net */
    {WAVE_FORMAT_EXTENSIBLE, "WAVE_FORMAT_EXTENSIBLE"}                                  /* Microsoft */
};

map<Guid, string> audio_format_guid_to_names = {
    {KSDATAFORMAT_SUBTYPE_ANALOG, "KSDATAFORMAT_SUBTYPE_ANALOG"},
    {KSDATAFORMAT_SUBTYPE_PCM, "KSDATAFORMAT_SUBTYPE_PCM"},
    {KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, "KSDATAFORMAT_SUBTYPE_IEEE_FLOAT"},
    {KSDATAFORMAT_SUBTYPE_ALAW, "KSDATAFORMAT_SUBTYPE_ALAW"},
    {KSDATAFORMAT_SUBTYPE_MULAW, "KSDATAFORMAT_SUBTYPE_MULAW"},
    {KSDATAFORMAT_SUBTYPE_ADPCM, "KSDATAFORMAT_SUBTYPE_ADPCM"},
    {KSDATAFORMAT_SUBTYPE_MPEG, "KSDATAFORMAT_SUBTYPE_MPEG"},

};

//// helper for swapping key, value of a map
//// This function assumes that all values of the passed map are unique
//template <class T1, class T2>
//static map<T2, T1> swap_keys_values(const map<T1, T2>& m) {
//    map<T2, T1> m1;
//
//    for (auto&& item : m) {
//        m1.emplace(item.second, item.first);
//    }
//
//    return m1;
//};
//
//extern map<string, uint16_t> audio_format_names_to_uint16 = swap_keys_values(audio_format_uint16_to_names);
//extern map<string, Guid> audio_format_names_to_guid = swap_keys_values(audio_format_guid_to_names);