#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Protocol timing constants */
#define BIT_US           104            // Microseconds per bit (calibrated from real remote)
#define FRAME_GAP_US     7000           // Gap between frames (µs)
#define FRAME_BITS       48             // Bits per frame
#define FRAME_BYTES      6              // Bytes per frame (FRAME_BITS / 8)
#define BIT_TOLERANCE    35             // Tolerance for bit timing (±35%)
#define MIN_PULSE_US     50             // Minimum pulse width (noise filter)

/* Bit manipulation helpers - used by both transmitter and receiver */
static inline __attribute__((always_inline)) size_t bit_to_byte_index(size_t bit_index) {
    return bit_index >> 3;  // bit_index / 8
}

static inline __attribute__((always_inline)) size_t bit_to_bit_index(size_t bit_index) {
    return bit_index & 7;   // bit_index % 8
}

/* Frame definitions */
extern const uint8_t FRAME_IDLE[6];
extern const uint8_t FRAME_PRESET_1[6];
extern const uint8_t FRAME_PRESET_2[6];

/* Frame type descriptor */
typedef struct {
    const char *name;
    uint8_t bytes[6];
    bool ignore_last_byte;
} frame_def_t;

/* Controller response frame database */
extern const frame_def_t CTRL_FRAMES[];
extern const size_t NUM_CTRL_FRAMES;

/**
 * Identify a controller frame by matching against known patterns
 * @param frame 6-byte frame to identify
 * @return Frame name if recognized, NULL otherwise
 */
const char *identify_ctrl_frame(const uint8_t *frame);
