#ifndef __USB_DESCRIPTOR_H_
#define __USB_DESCRIPTOR_H_
#include "usbd_core.h"
#include "usbd_audio.h"


#define USBD_MAX_POWER     100
#define USBD_LANGID_STRING 1033

// Input endpoint
#define AUDIO_INP_EP    0x81
// Output endpoint
#define AUDIO_OUT_EP    0x02
// Output feedback
#define AUDIO_OFB_EP    0x83

/* AUDIO Class Config */
#if CONFIG_USBDEV_AUDIO_VERSION < 0x0200
#define AUDIO_OUT_FREQ0           48000U
#define AUDIO_OUT_FREQ1           96000U
#else
#define AUDIO_OUT_FREQ_MIN        48000U
#define AUDIO_OUT_FREQ_MAX        192000U
#define AUDIO_OUT_FREQ_STEP       48000U
#endif

#define AUDIO_OUT_FRAME_SIZE_BYTE 4u
#define AUDIO_OUT_RESOLUTION_BIT  24u
#define AUDIO_OUT_CHANNEL_NUM     2u

#if CONFIG_USBDEV_AUDIO_VERSION < 0x0200
#define AUDIO_INP_FREQ0                48000U
#define AUDIO_INP_FREQ1                96000U
#else
#define AUDIO_INP_FREQ_MIN             48000U
#define AUDIO_INP_FREQ_MAX             192000U
#define AUDIO_INP_FREQ_STEP            48000U
#endif

#define AUDIO_INP_FRAME_SIZE_BYTE     4u
#define AUDIO_INP_RESOLUTION_BIT      24u
#define AUDIO_INP_CHANNEL_NUM         2u

#ifdef CONFIG_USB_FS
#define EP_INTERVAL 0x01
#else
#define EP_INTERVAL 0x02
#endif


#if CONFIG_USBDEV_AUDIO_VERSION < 0x0200
#define AUDIO_OUT_PACKET_SZ ((uint32_t)((AUDIO_OUT_FREQ1 * AUDIO_OUT_FRAME_SIZE_BYTE * AUDIO_OUT_CHANNEL_NUM) / 1000))
#define AUDIO_INP_PACKET_SZ ((uint32_t)((AUDIO_INP_FREQ1 * AUDIO_INP_FRAME_SIZE_BYTE * AUDIO_INP_CHANNEL_NUM) / 1000))
#else
#define AUDIO_OUT_PACKET_SZ ((uint32_t)((AUDIO_OUT_FREQ_MAX * AUDIO_OUT_FRAME_SIZE_BYTE * AUDIO_OUT_CHANNEL_NUM) / 1000 / (8 / (1 << (EP_INTERVAL - 1)))))
#define AUDIO_INP_PACKET_SZ ((uint32_t)((AUDIO_INP_FREQ_MAX * AUDIO_INP_FRAME_SIZE_BYTE * AUDIO_INP_CHANNEL_NUM) / 1000 / (8 / (1 << (EP_INTERVAL - 1)))))
#endif

extern uint8_t audio_descriptor[];
extern uint32_t cur_out_freq;
extern uint32_t cur_inp_freq;

extern void set_feedback_value();
extern void audio_init();
extern void usbd_audio_out_callback(uint8_t ep, uint32_t nbytes);
extern void usbd_audio_in_callback(uint8_t ep, uint32_t nbytes);
extern void usbd_audio_in_fb_callback(uint8_t ep, uint32_t nbytes);

#endif // __USB_DESCRIPTOR_H_