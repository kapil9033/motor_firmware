#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

void Encoder_Init(void);
int32_t Encoder_GetCount(void);
void Encoder_ResetCount(void);
uint8_t Encoder_GetButtonState(void);

#endif // ENCODER_H
