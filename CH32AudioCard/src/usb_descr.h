#ifndef __USB_DESCRIPTOR_H_
#define __USB_DESCRIPTOR_H_
#include "usbd_core.h"
#include "usbd_audio.h"


#define AUDIO_IN_EP  0x81
#define AUDIO_OUT_EP 0x02
#define AUDIO_OUT_FB_EP 0x83

/* AUDIO Class Config */
#define AUDIO_SPEAKER_FREQ0           16000U
#define AUDIO_SPEAKER_FREQ1           44100U
#define AUDIO_SPEAKER_FREQ2           48000U
#define AUDIO_SPEAKER_FREQ3           96000U
#define AUDIO_SPEAKER_FRAME_SIZE_BYTE 2u
#define AUDIO_SPEAKER_RESOLUTION_BIT  16u
#define AUDIO_SPEAKER_CHANNEL_NUM     2u

#define AUDIO_MIC_FREQ0                16000U
#define AUDIO_MIC_FREQ1                48000U
#define AUDIO_MIC_FREQ2                96000U
#define AUDIO_MIC_FRAME_SIZE_BYTE     2u
#define AUDIO_MIC_RESOLUTION_BIT      16u
#define AUDIO_MIC_CHANNEL_NUM         2u

//#define AUDIO_SAMPLE_FREQ(frq) (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))

/* AudioFreq * DataSize (2 bytes) * NumChannels (Stereo: 2) */
#define AUDIO_OUT_PACKET_SZ ((uint32_t)((AUDIO_SPEAKER_FREQ3 * AUDIO_SPEAKER_FRAME_SIZE_BYTE * AUDIO_SPEAKER_CHANNEL_NUM) / 1000))
/* 16bit(2 Bytes) (Mono:2) */
#define AUDIO_IN_PACKET_SZ ((uint32_t)((AUDIO_MIC_FREQ2 * AUDIO_MIC_FRAME_SIZE_BYTE * AUDIO_MIC_CHANNEL_NUM) / 1000))

extern const uint8_t audio_descriptor[];

#endif // __USB_DESCRIPTOR_H_