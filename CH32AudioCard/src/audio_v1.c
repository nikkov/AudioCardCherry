#include "usb_descr.h"

#ifdef CONFIG_USB_FS

#define USBD_VID           0xFFFF
#define USBD_PID           0xFFFF

#ifdef AUDIO_OFB_EP
#define AUDIO_AS_DESCRIPTOR_VAR_INIT_LEN(fs_num) AUDIO_AS_DESCRIPTOR_FB_INIT_LEN(fs_num)
#else
#define AUDIO_AS_DESCRIPTOR_VAR_INIT_LEN(fs_num) AUDIO_AS_DESCRIPTOR_INIT_LEN(fs_num)
#endif                             


#define USB_AUDIO_CONFIG_DESC_SIZ (unsigned long)(9 +                                       \
                                                  AUDIO_AC_DESCRIPTOR_INIT_LEN(2) +         \
                                                  AUDIO_SIZEOF_AC_INPUT_TERMINAL_DESC +     \
                                                  AUDIO_SIZEOF_AC_FEATURE_UNIT_DESC(2, 1) + \
                                                  AUDIO_SIZEOF_AC_OUTPUT_TERMINAL_DESC +    \
                                                  AUDIO_SIZEOF_AC_INPUT_TERMINAL_DESC +     \
                                                  AUDIO_SIZEOF_AC_FEATURE_UNIT_DESC(2, 1) + \
                                                  AUDIO_SIZEOF_AC_OUTPUT_TERMINAL_DESC +    \
                                                  AUDIO_AS_DESCRIPTOR_VAR_INIT_LEN(2) +     \
                                                  AUDIO_AS_DESCRIPTOR_INIT_LEN(2))

#define AUDIO_AC_SIZ (AUDIO_SIZEOF_AC_HEADER_DESC(2) +          \
                      AUDIO_SIZEOF_AC_INPUT_TERMINAL_DESC +     \
                      AUDIO_SIZEOF_AC_FEATURE_UNIT_DESC(2, 1) + \
                      AUDIO_SIZEOF_AC_OUTPUT_TERMINAL_DESC +    \
                      AUDIO_SIZEOF_AC_INPUT_TERMINAL_DESC +     \
                      AUDIO_SIZEOF_AC_FEATURE_UNIT_DESC(2, 1) + \
                      AUDIO_SIZEOF_AC_OUTPUT_TERMINAL_DESC)

const uint8_t audio_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0xef, 0x02, 0x01, USBD_VID, USBD_PID, 0x0001, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_AUDIO_CONFIG_DESC_SIZ, 0x03, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    AUDIO_AC_DESCRIPTOR_INIT(0x00, 0x03, AUDIO_AC_SIZ, 0x00, 0x01, 0x02),

    AUDIO_AC_INPUT_TERMINAL_DESCRIPTOR_INIT(AC_INP_TERMINAL_INP_ID, AUDIO_INTERM_MIC, 0x02, 0x0003),
    AUDIO_AC_FEATURE_UNIT_DESCRIPTOR_INIT(AC_FEATURE_INP_UNIT_ID, AC_INP_TERMINAL_INP_ID, 0x01, 0x03, 0x00, 0x00),
    AUDIO_AC_OUTPUT_TERMINAL_DESCRIPTOR_INIT(AC_OUT_TERMINAL_INP_ID, AUDIO_TERMINAL_STREAMING, AC_FEATURE_INP_UNIT_ID),

    AUDIO_AC_INPUT_TERMINAL_DESCRIPTOR_INIT(AC_INP_TERMINAL_OUT_ID, AUDIO_TERMINAL_STREAMING, 0x02, 0x0003),
    AUDIO_AC_FEATURE_UNIT_DESCRIPTOR_INIT(AC_FEATURE_OUT_UNIT_ID, AC_INP_TERMINAL_OUT_ID, 0x01, 0x03, 0x00, 0x00),
    AUDIO_AC_OUTPUT_TERMINAL_DESCRIPTOR_INIT(AC_OUT_TERMINAL_OUT_ID, AUDIO_OUTTERM_SPEAKER, AC_FEATURE_OUT_UNIT_ID),
    
#ifdef AUDIO_OFB_EP
    AUDIO_AS_DESCRIPTOR_FB_INIT(0x01, AC_INP_TERMINAL_OUT_ID, 0x02, AUDIO_OUT_FRAME_SIZE_BYTE, AUDIO_OUT_RESOLUTION_BIT, AUDIO_OUT_EP, AUDIO_OUT_PACKET_SZ,\
                             EP_INTERVAL, AUDIO_OFB_EP, 3, AUDIO_SAMPLE_FREQ_3B(AUDIO_OUT_FREQ0), AUDIO_SAMPLE_FREQ_3B(AUDIO_OUT_FREQ1)),
#else
    AUDIO_AS_DESCRIPTOR_INIT(0x01, AC_INP_TERMINAL_OUT_ID, 0x02, AUDIO_OUT_FRAME_SIZE_BYTE, AUDIO_OUT_RESOLUTION_BIT, AUDIO_OUT_EP, AUDIO_OUT_PACKET_SZ,\
                             EP_INTERVAL, AUDIO_SAMPLE_FREQ_3B(AUDIO_OUT_FREQ0), AUDIO_SAMPLE_FREQ_3B(AUDIO_OUT_FREQ1)),
#endif                             
    AUDIO_AS_DESCRIPTOR_INIT(0x02, AC_OUT_TERMINAL_INP_ID, 0x02, AUDIO_INP_FRAME_SIZE_BYTE, AUDIO_INP_RESOLUTION_BIT, AUDIO_INP_EP, AUDIO_INP_PACKET_SZ,\
                             EP_INTERVAL, AUDIO_SAMPLE_FREQ_3B(AUDIO_INP_FREQ0), AUDIO_SAMPLE_FREQ_3B(AUDIO_INP_FREQ1)),
    ///////////////////////////////////////
    /// string0 descriptor
    ///////////////////////////////////////
    USB_LANGID_INIT(USBD_LANGID_STRING),
    ///////////////////////////////////////
    /// string1 descriptor
    ///////////////////////////////////////
    0x14,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ///////////////////////////////////////
    /// string2 descriptor
    ///////////////////////////////////////
    0x28,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ' ', 0x00,                  /* wcChar9 */
    'U', 0x00,                  /* wcChar10 */
    'A', 0x00,                  /* wcChar11 */
    'C', 0x00,                  /* wcChar12 */
    '1', 0x00,                  /* wcChar13 */
    ' ', 0x00,                  /* wcChar14 */
    'D', 0x00,                  /* wcChar15 */
    'E', 0x00,                  /* wcChar16 */
    'M', 0x00,                  /* wcChar17 */
    'O', 0x00,                  /* wcChar18 */
    ///////////////////////////////////////
    /// string3 descriptor
    ///////////////////////////////////////
    0x16,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    '2', 0x00,                  /* wcChar0 */
    '0', 0x00,                  /* wcChar1 */
    '2', 0x00,                  /* wcChar2 */
    '1', 0x00,                  /* wcChar3 */
    '0', 0x00,                  /* wcChar4 */
    '3', 0x00,                  /* wcChar5 */
    '1', 0x00,                  /* wcChar6 */
    '0', 0x00,                  /* wcChar7 */
    '0', 0x00,                  /* wcChar8 */
    '1', 0x00,                  /* wcChar9 */
#ifdef CONFIG_USB_HS
    ///////////////////////////////////////
    /// device qualifier descriptor
    ///////////////////////////////////////
    0x0a,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x00,
    0x00,
    0x00,
    0x40,
    0x01,
    0x00,
#endif
    0x00
};


void usbd_audio_set_sampling_freq(uint8_t entity_id, uint8_t ep_ch, uint32_t sampling_freq) {
    switch(ep_ch) {
        case AUDIO_OUT_EP:
            cur_out_freq = sampling_freq;
            set_feedback_value();
            USB_LOG_RAW("spk set freq: 0x%X,0x%X,%lu\r\n", entity_id, ep_ch, sampling_freq);
            break;
        case AUDIO_INP_EP:
            cur_inp_freq = sampling_freq;
            USB_LOG_RAW("mic set freq: 0x%X,0x%X,%lu\r\n", entity_id, ep_ch, sampling_freq);
            break;
        default:
            USB_LOG_RAW("unknown ep_ch: 0x%X,0x%X,%lu\r\n", entity_id, ep_ch, sampling_freq);
    }
}

static struct usbd_endpoint audio_in_ep = {
    .ep_cb = usbd_audio_in_callback,
    .ep_addr = AUDIO_INP_EP
};

static struct usbd_endpoint audio_out_ep = {
    .ep_cb = usbd_audio_out_callback,
    .ep_addr = AUDIO_OUT_EP
};

#ifdef AUDIO_OFB_EP
static struct usbd_endpoint audio_out_fb_ep = {
    .ep_cb = usbd_audio_in_fb_callback,
    .ep_addr = AUDIO_OFB_EP
};
#endif

struct usbd_interface intf0;
struct usbd_interface intf1;
struct usbd_interface intf2;

// call from main programm
void audio_init()
{
    usbd_desc_register(audio_descriptor);
    usbd_add_interface(usbd_audio_init_intf(&intf0));
    usbd_add_interface(usbd_audio_init_intf(&intf1));
    usbd_add_interface(usbd_audio_init_intf(&intf2));
    usbd_add_endpoint(&audio_in_ep);
    usbd_add_endpoint(&audio_out_ep);
#ifdef AUDIO_OFB_EP
    usbd_add_endpoint(&audio_out_fb_ep);
#endif    

    usbd_audio_add_entity(AC_FEATURE_OUT_UNIT_ID, AUDIO_CONTROL_FEATURE_UNIT);
    usbd_audio_add_entity(AC_FEATURE_INP_UNIT_ID, AUDIO_CONTROL_FEATURE_UNIT);
}


#endif // CONFIG_USB_FS
