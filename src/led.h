#pragma once

#include "types.h"

#define LED_GREEN 12
#define LED_ORANGE 13
#define LED_RED 14
#define LED_BLUE 15

void ledOn(u32 pin);

void ledOff(u32 pin);

void ledInitAll(void);