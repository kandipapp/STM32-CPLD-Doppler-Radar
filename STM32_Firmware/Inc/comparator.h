#ifndef COMPARATOR_H
#define COMPARATOR_H
#include "main.h"

// Global variable to share data with main.c (for display)
extern volatile float measured_frequency;

void Comparator_Start(void);

#endif
