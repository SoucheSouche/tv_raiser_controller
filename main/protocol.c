#include "protocol.h"
#include <string.h>
#include <assert.h>

/* Remote control frame definitions (matching analyzer's known frames) */
const uint8_t FRAME_IDLE[6]     = { 0x4A, 0x03, 0x08, 0xA0, 0x80, 0x05 };
const uint8_t FRAME_PRESET_1[6] = { 0x4A, 0x03, 0x48, 0xA0, 0x80, 0x06 };
const uint8_t FRAME_PRESET_2[6] = { 0x4A, 0x03, 0x88, 0xA0, 0x80, 0x0A };

/* Controller response frames: populate as they become known */
const frame_def_t CTRL_FRAMES[] = {
    // Example placeholder: fill when real controller responses are captured
    // { "CTRL_STATUS", { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, false },
};

const size_t NUM_CTRL_FRAMES = sizeof(CTRL_FRAMES) / sizeof(CTRL_FRAMES[0]);

const char *identify_ctrl_frame(const uint8_t *frame)
{
    assert(frame != NULL);  // Validate input
    
    if (NUM_CTRL_FRAMES == 0) {
        return NULL;  // No controller frames defined yet
    }
    
    for (int i = 0; i < (int)NUM_CTRL_FRAMES; i++) {
        if (CTRL_FRAMES[i].ignore_last_byte) {
            if (memcmp(frame, CTRL_FRAMES[i].bytes, 5) == 0) {
                return CTRL_FRAMES[i].name;
            }
        } else {
            if (memcmp(frame, CTRL_FRAMES[i].bytes, 6) == 0) {
                return CTRL_FRAMES[i].name;
            }
        }
    }
    return NULL;
}
