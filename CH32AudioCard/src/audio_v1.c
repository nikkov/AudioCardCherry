#include "debug.h"
#include "usbd_core.h"
#include "usbd_audio.h"
#include "ring_buf.h"

#ifdef CONFIG_USB_FS

#define USBD_VID           0xFFFF
#define USBD_PID           0xFFFF
#define USBD_MAX_POWER     100
#define USBD_LANGID_STRING 1033

#ifdef CONFIG_USB_HS
#define EP_INTERVAL 0x04
#else
#define EP_INTERVAL 0x01
#endif

#define AUDIO_IN_EP  0x81
#define AUDIO_OUT_EP 0x02

/* AUDIO Class Config */
#define AUDIO_SPEAKER_FREQ            48000U
#define AUDIO_SPEAKER_FRAME_SIZE_BYTE 2u
#define AUDIO_SPEAKER_RESOLUTION_BIT  16u
#define AUDIO_MIC_FREQ                48000U
#define AUDIO_MIC_FRAME_SIZE_BYTE     2u
#define AUDIO_MIC_RESOLUTION_BIT      16u

#define AUDIO_SAMPLE_FREQ(frq) (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))

/* AudioFreq * DataSize (2 bytes) * NumChannels (Stereo: 2) */
#define AUDIO_OUT_PACKET ((uint32_t)((AUDIO_SPEAKER_FREQ * AUDIO_SPEAKER_FRAME_SIZE_BYTE * 2) / 1000))
/* 16bit(2 Bytes) (Mono:2) */
#define AUDIO_IN_PACKET ((uint32_t)((AUDIO_MIC_FREQ * AUDIO_MIC_FRAME_SIZE_BYTE * 2) / 1000))

#define USB_AUDIO_CONFIG_DESC_SIZ (unsigned long)(9 +                                       \
                                                  AUDIO_AC_DESCRIPTOR_INIT_LEN(2) +         \
                                                  AUDIO_SIZEOF_AC_INPUT_TERMINAL_DESC +     \
                                                  AUDIO_SIZEOF_AC_FEATURE_UNIT_DESC(2, 1) + \
                                                  AUDIO_SIZEOF_AC_OUTPUT_TERMINAL_DESC +    \
                                                  AUDIO_SIZEOF_AC_INPUT_TERMINAL_DESC +     \
                                                  AUDIO_SIZEOF_AC_FEATURE_UNIT_DESC(2, 1) + \
                                                  AUDIO_SIZEOF_AC_OUTPUT_TERMINAL_DESC +    \
                                                  AUDIO_AS_DESCRIPTOR_INIT_LEN(1) +         \
                                                  AUDIO_AS_DESCRIPTOR_INIT_LEN(1))

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
    AUDIO_AC_INPUT_TERMINAL_DESCRIPTOR_INIT(0x01, AUDIO_INTERM_MIC, 0x02, 0x0003),
    AUDIO_AC_FEATURE_UNIT_DESCRIPTOR_INIT(0x02, 0x01, 0x01, 0x03, 0x00, 0x00),
    AUDIO_AC_OUTPUT_TERMINAL_DESCRIPTOR_INIT(0x03, AUDIO_TERMINAL_STREAMING, 0x02),
    AUDIO_AC_INPUT_TERMINAL_DESCRIPTOR_INIT(0x04, AUDIO_TERMINAL_STREAMING, 0x02, 0x0003),
    AUDIO_AC_FEATURE_UNIT_DESCRIPTOR_INIT(0x05, 0x04, 0x01, 0x03, 0x00, 0x00),
    AUDIO_AC_OUTPUT_TERMINAL_DESCRIPTOR_INIT(0x06, AUDIO_OUTTERM_SPEAKER, 0x05),
    AUDIO_AS_DESCRIPTOR_INIT(0x01, 0x04, 0x02, AUDIO_SPEAKER_FRAME_SIZE_BYTE, AUDIO_SPEAKER_RESOLUTION_BIT, AUDIO_OUT_EP, AUDIO_OUT_PACKET,\
                             EP_INTERVAL, AUDIO_SAMPLE_FREQ_3B(AUDIO_SPEAKER_FREQ)),
    AUDIO_AS_DESCRIPTOR_INIT(0x02, 0x03, 0x02, AUDIO_MIC_FRAME_SIZE_BYTE, AUDIO_MIC_RESOLUTION_BIT, AUDIO_IN_EP, AUDIO_IN_PACKET,\
                             EP_INTERVAL, AUDIO_SAMPLE_FREQ_3B(AUDIO_MIC_FREQ)),
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

ring_buf out_rb;
dbl_buffer write_bf;
dbl_buffer read_bf;

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t out_rb_data[2048];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t write_buffer[AUDIO_IN_PACKET << 1];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t read_buffer[AUDIO_OUT_PACKET << 1];

#define EP_TX_FLAG     1
#define EP_RX_FLAG     2
volatile uint8_t ep_busy_flag = 0;
volatile uint8_t open_flag = 0;


// call from interrupt
void usbd_audio_open(uint8_t intf) {
    printf("OPEN%d\r\n", intf);
    if (intf == 1) {
        // output speaker interface
        open_flag |= EP_RX_FLAG;
        ep_busy_flag |= EP_RX_FLAG;
        // setup first out ep read transfer
        dbl_buffer_reset(&read_bf);
        // start reading
        usbd_ep_start_read(AUDIO_OUT_EP, read_bf.buffer, read_bf.halfsize);
    } else if(intf == 2) {
        // input mic interface
        // prepare in ep transfer
        open_flag |= EP_TX_FLAG;
        dbl_buffer_reset(&write_bf);
    }
}

// call from interrupt
void usbd_audio_close(uint8_t intf) {
    printf("CLOSE%d\r\n", intf);
    if (intf == 1) {
        open_flag &= ~EP_RX_FLAG;
        ep_busy_flag &= ~EP_RX_FLAG;
    } else {
        open_flag &= ~EP_TX_FLAG;
        ep_busy_flag &= ~EP_TX_FLAG;
    }
}

// call from interrupt
void usbd_configure_done_callback(void) {
}

// call from interrupt after received data from host
void usbd_audio_out_callback(uint8_t ep, uint32_t nbytes) {
    // received nbytes in (read_bf.buffer + (read_bf.buffer_pos ? 0 : read_bf.halfsize))
    read_bf.count = nbytes;
    // select next buffer
    read_bf.buffer_pos ^= 1;
    ep_busy_flag &= ~EP_RX_FLAG;
    usbd_ep_start_read(AUDIO_OUT_EP, read_bf.buffer + (read_bf.buffer_pos ? read_bf.halfsize : 0), AUDIO_OUT_PACKET);
    //USB_LOG_RAW("actual out len:%d\r\n", nbytes);
}

// call from interrupt after sending data to host
void usbd_audio_in_callback(uint8_t ep, uint32_t nbytes)
{
    // successfully writen nbytes from (write_bf.buffer + (write_bf.buffer_pos ? 0 : write_bf.halfsize))
    ep_busy_flag &= ~EP_TX_FLAG;
    // USB_LOG_RAW("actual in len:%d\r\n", nbytes);
}

static struct usbd_endpoint audio_in_ep = {
    .ep_cb = usbd_audio_in_callback,
    .ep_addr = AUDIO_IN_EP
};

static struct usbd_endpoint audio_out_ep = {
    .ep_cb = usbd_audio_out_callback,
    .ep_addr = AUDIO_OUT_EP
};

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

    usbd_audio_add_entity(0x02, AUDIO_CONTROL_FEATURE_UNIT);
    usbd_audio_add_entity(0x05, AUDIO_CONTROL_FEATURE_UNIT);

    usbd_initialize();
}

// call from main programm
void read_samples() {
    const uint8_t* data;
    uint32_t len;
    if((ep_busy_flag & EP_RX_FLAG) == 0) {
        // now we have one full and and one filled buffer
        data = read_bf.buffer + (read_bf.buffer_pos ? 0 : read_bf.halfsize);
        len = read_bf.count;
        ring_buf_write(&out_rb, data, len);
        ep_busy_flag |= EP_RX_FLAG;
    }
}

// call from main programm
void write_samples() {
    uint8_t* data;
    if((ep_busy_flag & EP_TX_FLAG) == 0) {
        // now we sent one buffer and need to fill and write next one
        write_bf.buffer_pos ^= 1;
        data = write_bf.buffer + (write_bf.buffer_pos ? 0 : write_bf.halfsize);
        if(write_bf.count == 0) {
            // no data from out ep yet
            write_bf.count = AUDIO_IN_PACKET;
            memset(write_bf.buffer + (write_bf.buffer_pos ? write_bf.halfsize : 0), 0, write_bf.count);
        }
        usbd_ep_start_write(AUDIO_IN_EP, write_bf.buffer + (write_bf.buffer_pos ? write_bf.halfsize : 0), write_bf.count);
        // prepare next buffer
        write_bf.count = ring_buf_read(&out_rb, data, AUDIO_IN_PACKET);
        ep_busy_flag |= EP_TX_FLAG;
    }
}

void audio_test()
{
	//uint16_t i;
    ring_buf_init(&out_rb, out_rb_data, sizeof(out_rb_data));
    dbl_buffer_init(&write_bf, write_buffer, AUDIO_IN_PACKET);
    dbl_buffer_init(&read_bf, read_buffer, AUDIO_OUT_PACKET);

    while (1) {
        if (open_flag) {
            if(open_flag & EP_RX_FLAG)
                read_samples();
            if(open_flag & EP_TX_FLAG)
                write_samples();
        }
    }
}

#endif // CONFIG_USB_FS
