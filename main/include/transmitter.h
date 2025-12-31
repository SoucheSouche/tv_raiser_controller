#pragma once

#include <stdint.h>

/**
 * Send a complete 48-bit frame on the TX line
 * @param frame 6-byte frame to transmit (LSB first)
 */
void transmitter_send_frame(const uint8_t *frame);
