/**
 * @file Pico-USB-Host-MIDI-Adapter.c
 * @brief A USB Host to Serial Port MIDI adapter that runs on a Raspberry Pi
 * Pico board
 * 
 * MIT License

 * Copyright (c) 2022 rppicomidi

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "midi_uart_lib.h"
#include "bsp/board.h"
#include "tusb.h"
#include "class/midi/midi_device.h"
// On-board LED mapping. If no LED, set to NO_LED_GPIO
const uint NO_LED_GPIO = 255;
const uint LED_GPIO = 25;
// UART selection Pin mapping. You can move these for your design if you want to
// Make sure all these values are consistent with your choice of midi_uart
#define MIDI_UART_NUM 1
const uint MIDI_UART_TX_GPIO = 4;
const uint MIDI_UART_RX_GPIO = 5;

static void *midi_uart_instance;
static uint8_t midi_dev_addr = 0;

static void blink_led(void)
{
    static absolute_time_t previous_timestamp = {0};

    static bool led_state = false;

    // This design has no on-board LED
    if (NO_LED_GPIO == LED_GPIO)
        return;
    absolute_time_t now = get_absolute_time();
    
    int64_t diff = absolute_time_diff_us(previous_timestamp, now);
    if (diff > 1000000) {
        gpio_put(LED_GPIO, led_state);
        led_state = !led_state;
        previous_timestamp = now;
    }
}

static void poll_usb_rx(bool connected)
{
    // device must be attached and have at least one endpoint ready to receive a message
    if (!connected)
    {
        return;
    }
    uint8_t rx[48];
    uint32_t nread = tud_midi_stream_read(rx, sizeof(rx));
    uint8_t npushed = midi_uart_write_tx_buffer(midi_uart_instance,rx,nread);
    if (npushed != nread) {
        TU_LOG1("Warning: Dropped %d bytes sending to UART MIDI Out\r\n", nread - npushed);
    }
}


static void poll_midi_uart_rx(bool connected)
{
    uint8_t rx[48];
    // Pull any bytes received on the MIDI UART out of the receive buffer and
    // send them out via USB MIDI on virtual cable 0
    uint8_t nread = midi_uart_poll_rx_buffer(midi_uart_instance, rx, sizeof(rx));
    if (nread > 0 && connected)
    {
        uint32_t nwritten = tud_midi_stream_write(0, rx, nread);
        if (nwritten != nread) {
            TU_LOG1("Warning: Dropped %d bytes receiving from UART MIDI In\r\n", nread - nwritten);
        }
    }
}

int main() {

    bi_decl(bi_program_description("Provide a USB host interface for Serial Port MIDI."));
    bi_decl(bi_1pin_with_name(LED_GPIO, "On-board LED"));
    bi_decl(bi_2pins_with_names(MIDI_UART_TX_GPIO, "MIDI UART TX", MIDI_UART_RX_GPIO, "MIDI UART RX"));

    board_init();
    printf("Pico MIDI USB Device to MIDI UART Adapter\r\n");
    tusb_init();

    // Map the pins to functions
    gpio_init(LED_GPIO);
    gpio_set_dir(LED_GPIO, GPIO_OUT);
    midi_uart_instance = midi_uart_configure(MIDI_UART_NUM, MIDI_UART_TX_GPIO, MIDI_UART_RX_GPIO);
    printf("Configured MIDI UART %u for 31250 baud\r\n", MIDI_UART_NUM);
    while (1) {
        tud_task();

        blink_led();
        bool connected = tud_midi_mounted();

        poll_midi_uart_rx(connected);
        poll_usb_rx(connected);

        midi_uart_drain_tx_buffer(midi_uart_instance);
    }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
    // TODO
    TU_LOG1("Mounted\r\n");
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    // TODO
    TU_LOG1("Unmounted\r\n");
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    // TODO
    TU_LOG1("Suspended\r\n");
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    // TODO
    TU_LOG1("Resumed\r\n");
}
