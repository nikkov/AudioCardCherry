#include "debug.h"
#include "ring_buf.h"
#include "usb_descr.h"

ring_buf out_rb;
dbl_buffer write_bf;
dbl_buffer read_bf;

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t out_rb_data[2048];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t write_buffer[AUDIO_IN_PACKET_SZ << 1];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t read_buffer[AUDIO_OUT_PACKET_SZ << 1];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t feedback_buffer[4];

#define EP_TX_FLAG     1
#define EP_RX_FLAG     2
#define EP_FB_FLAG     4
volatile uint8_t ep_busy_flag = 0;
volatile uint8_t open_flag = 0;

uint32_t cur_speaker_freq = AUDIO_SPEAKER_FREQ2;
uint32_t cur_microphone_freq = AUDIO_MIC_FREQ1;
volatile uint32_t cur_feedback_rate = 48 << 14;


static inline uint32_t get_mic_packet_size() {
    return cur_microphone_freq * AUDIO_MIC_FRAME_SIZE_BYTE * AUDIO_MIC_CHANNEL_NUM / 1000;
}

static inline uint32_t get_spk_packet_size() {
    return cur_speaker_freq * AUDIO_SPEAKER_FRAME_SIZE_BYTE * AUDIO_SPEAKER_CHANNEL_NUM / 1000;
}

static inline void set_feedback_value() {
    cur_feedback_rate = (cur_speaker_freq / 1000) << 14;
}

// call from interrupt
void usbd_audio_open(uint8_t intf) {
    printf("OPEN%d\r\n", intf);
    if (intf == 1) {
        // output speaker interface
        open_flag |= EP_RX_FLAG;
        open_flag |= EP_FB_FLAG;
        ep_busy_flag |= EP_RX_FLAG;
        ep_busy_flag |= EP_FB_FLAG;
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
    usbd_ep_start_read(AUDIO_OUT_EP, read_bf.buffer + (read_bf.buffer_pos ? read_bf.halfsize : 0), read_bf.halfsize);
    //USB_LOG_RAW("actual out len:%d\r\n", nbytes);
}

// call from interrupt after sending data to host
void usbd_audio_in_callback(uint8_t ep, uint32_t nbytes)
{
    // successfully writen nbytes from (write_bf.buffer + (write_bf.buffer_pos ? 0 : write_bf.halfsize))
    write_bf.buffer_pos ^= 1;
    ep_busy_flag &= ~EP_TX_FLAG;
    usbd_ep_start_write(AUDIO_IN_EP, write_bf.buffer + (write_bf.buffer_pos ? write_bf.halfsize : 0), write_bf.count);
    // USB_LOG_RAW("actual in len:%d\r\n", nbytes);
}

void usbd_audio_in_fb_callback(uint8_t ep, uint32_t nbytes)
{
    // write feedback ratio
    feedback_buffer[0] = (uint8_t)cur_feedback_rate;
    feedback_buffer[1] = (uint8_t)(cur_feedback_rate >> 8);
    feedback_buffer[2] = (uint8_t)(cur_feedback_rate >> 16);
#ifdef CONFIG_USB_HS
    feedback_buffer[3] = (uint8_t)(cur_feedback_rate >> 24);
    usbd_ep_start_write(AUDIO_OUT_FB_EP, feedback_buffer, 4);
#else        
    usbd_ep_start_write(AUDIO_OUT_FB_EP, feedback_buffer, 3);
#endif        
    ep_busy_flag &= ~EP_FB_FLAG;
}

void usbd_audio_set_volume(uint8_t entity_id, uint8_t ch, float dB)
{
    USB_LOG_RAW("audio set volume:%d,%d,%d\r\n", entity_id, ch, (int)dB);
}

void usbd_audio_set_mute(uint8_t entity_id, uint8_t ch, uint8_t enable)
{
    USB_LOG_RAW("audio set mute:%d,%d,%d\r\n", entity_id, ch, enable);
}

void usbd_audio_set_sampling_freq(uint8_t entity_id, uint8_t ep_ch, uint32_t sampling_freq)
{
    if(ep_ch == AUDIO_OUT_EP) {
        cur_speaker_freq = sampling_freq;
        set_feedback_value();
        USB_LOG_RAW("spk set freq:%d,%d,%lu\r\n", entity_id, ep_ch, sampling_freq);
    } 
    else {
        cur_microphone_freq = sampling_freq;
        USB_LOG_RAW("mic set freq:%d,%d,%lu\r\n", entity_id, ep_ch, sampling_freq);
    }
}

uint32_t usbd_audio_get_sampling_freq(uint8_t entity_id, uint8_t ep_ch)
{
    if(ep_ch == AUDIO_OUT_EP) {
        USB_LOG_RAW("spk get freq:%d,%d,%lu\r\n", entity_id, ep_ch, cur_speaker_freq);
        return cur_speaker_freq;
    } 
    else {
        USB_LOG_RAW("mic get freq:%d,%d,%lu\r\n", entity_id, ep_ch, cur_microphone_freq);
        return cur_microphone_freq;
    }
}

static struct usbd_endpoint audio_in_ep = {
    .ep_cb = usbd_audio_in_callback,
    .ep_addr = AUDIO_IN_EP
};

static struct usbd_endpoint audio_out_ep = {
    .ep_cb = usbd_audio_out_callback,
    .ep_addr = AUDIO_OUT_EP
};

static struct usbd_endpoint audio_out_fb_ep = {
    .ep_cb = usbd_audio_in_fb_callback,
    .ep_addr = AUDIO_OUT_FB_EP
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
    usbd_add_endpoint(&audio_out_fb_ep);

    usbd_audio_add_entity(0x02, AUDIO_CONTROL_FEATURE_UNIT);
    usbd_audio_add_entity(0x05, AUDIO_CONTROL_FEATURE_UNIT);

    usbd_initialize();
}

// call from main programm
void read_samples() {
    const uint8_t* data;
    uint32_t len;
    if((ep_busy_flag & EP_RX_FLAG) == 0) {
        // now we have one full and and one filling buffer
        data = read_bf.buffer + (read_bf.buffer_pos ? 0 : read_bf.halfsize);
        len = read_bf.count;
        // write filled data to ring buffer
        ring_buf_write(&out_rb, data, len);
        ep_busy_flag |= EP_RX_FLAG;
    }
}

// call from main programm
void write_feedback() {
    if((ep_busy_flag & EP_FB_FLAG) == 0) {
        // calc feedback
        ep_busy_flag |= EP_FB_FLAG;
    }
}

// call from main programm
void write_samples() {
    uint8_t* data;
    if((ep_busy_flag & EP_TX_FLAG) == 0) {
        // now we sent one buffer and need to fill and write next one
        // write_bf.buffer_pos ^= 1;
        data = write_bf.buffer + (write_bf.buffer_pos ? 0 : write_bf.halfsize);
        // prepare next buffer
        uint32_t need_len = get_mic_packet_size();
        write_bf.count = ring_buf_read(&out_rb, data, need_len);
        if(write_bf.count < need_len) {
            memset(data + write_bf.count, 0, need_len - write_bf.count);
            write_bf.count = need_len;
        }
        ep_busy_flag |= EP_TX_FLAG;
    }
}

void audio_test()
{
    ring_buf_init(&out_rb, out_rb_data, sizeof(out_rb_data));
    dbl_buffer_init(&write_bf, write_buffer, AUDIO_IN_PACKET_SZ);
    dbl_buffer_init(&read_bf, read_buffer, AUDIO_OUT_PACKET_SZ);

    while (1) {
        if (open_flag) {
            if(open_flag & EP_RX_FLAG)
                read_samples();
            if(open_flag & EP_TX_FLAG)
                write_samples();
            if(open_flag & EP_FB_FLAG)
                write_feedback();
        }
    }
}
