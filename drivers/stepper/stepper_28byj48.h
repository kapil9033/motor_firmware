#ifndef STEPPER_28BYJ48_H
#define STEPPER_28BYJ48_H

#include <stdbool.h>

bool Stepper28BYJ48_Init(void);
void Stepper28BYJ48_Step(bool clockwise);
void Stepper28BYJ48_Release(void);

#endif
