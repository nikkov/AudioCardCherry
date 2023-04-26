#include "debug.h"
#include "ring_buf.h"
#include "usb_descr.h"

ring_buf out_rb;
dbl_buffer write_bf;
dbl_buffer read_bf;

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t out_rb_data[2048];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t write_buffer[AUDIO_INP_PACKET_SZ << 1];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t read_buffer[AUDIO_OUT_PACKET_SZ << 1];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t feedback_buffer[4];

#define EP_TX_FLAG     1
#define EP_RX_FLAG     2
#define EP_FB_FLAG     4
volatile uint8_t ep_busy_flag = 0;
volatile uint8_t open_flag = 0;

#if CONFIG_USBDEV_AUDIO_VERSION < 0x0200
uint32_t cur_out_freq = AUDIO_OUT_FREQ1;
uint32_t cur_inp_freq = AUDIO_INP_FREQ1;
static inline uint32_t get_inp_packet_size() {
    return ((uint32_t)((cur_inp_freq * AUDIO_INP_FRAME_SIZE_BYTE * AUDIO_INP_CHANNEL_NUM) / 1000));
}
#else
uint32_t cur_out_freq = AUDIO_OUT_FREQ_MIN;
uint32_t cur_inp_freq = AUDIO_INP_FREQ_MIN;
static inline uint32_t get_inp_packet_size() {
    return ((uint32_t)((cur_inp_freq * AUDIO_INP_FRAME_SIZE_BYTE * AUDIO_INP_CHANNEL_NUM) / 1000 / (8 / (1 << (EP_INTERVAL - 1)))));
}
#endif

volatile uint32_t cur_feedback_rate = 48 << 14;

void set_feedback_value() {
    cur_feedback_rate = (cur_out_freq / 1000) << 14;
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
void usbd_audio_in_callback(uint8_t ep, uint32_t nbytes) {
    // successfully writen nbytes from (write_bf.buffer + (write_bf.buffer_pos ? 0 : write_bf.halfsize))
    write_bf.buffer_pos ^= 1;
    ep_busy_flag &= ~EP_TX_FLAG;
    usbd_ep_start_write(AUDIO_INP_EP, write_bf.buffer + (write_bf.buffer_pos ? write_bf.halfsize : 0), write_bf.count);
    // USB_LOG_RAW("actual in len:%d\r\n", nbytes);
}

void usbd_audio_in_fb_callback(uint8_t ep, uint32_t nbytes) {
    // write feedback ratio
    feedback_buffer[0] = (uint8_t)cur_feedback_rate;
    feedback_buffer[1] = (uint8_t)(cur_feedback_rate >> 8);
    feedback_buffer[2] = (uint8_t)(cur_feedback_rate >> 16);
#ifdef CONFIG_USB_HS
    feedback_buffer[3] = (uint8_t)(cur_feedback_rate >> 24);
    usbd_ep_start_write(AUDIO_OFB_EP, feedback_buffer, 4);
#else        
    usbd_ep_start_write(AUDIO_OFB_EP, feedback_buffer, 3);
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
        uint32_t need_len = get_inp_packet_size();
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
    dbl_buffer_init(&write_bf, write_buffer, AUDIO_INP_PACKET_SZ);
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
