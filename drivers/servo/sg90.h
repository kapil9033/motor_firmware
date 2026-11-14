#ifndef SG90_H
#define SG90_H

#include <stdbool.h>
#include <stdint.h>

bool SG90_Init(void);
bool SG90_SetAngle(uint16_t angle_degrees);

#endif
