#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

/* Event flags */
#define EVT_RESPONSE (1 << 0)

/**
 * Initialize receiver module with event group for signaling
 * @param events Event group handle for response notification
 */
void receiver_init(EventGroupHandle_t events);

/**
 * Single polling iteration - call repeatedly in a tight loop
 * Monitors RX line and decodes controller responses without blocking
 * Signals EVT_RESPONSE when a complete frame is received
 */
void receiver_poll(void);
