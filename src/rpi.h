#ifndef __RPI
#define __RPI
/*
 * This is register defs for RPi
 *
 */
#include "temp.h"
Temp_temp F_Ri(int);  // for R0-R11
Temp_temp F_SP(void);
Temp_temp F_FP(void);
Temp_temp F_LR(void);
#endif