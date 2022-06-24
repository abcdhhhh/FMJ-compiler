/* May 22, 2022 version:
 to make sure each F_Ri(i) will return the same temp everytime it is called!
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symbol.h"
#include "table.h"
#include "temp.h"
#include "util.h"
/*
 * This is register defs for RPi
 *
 */

struct Temp_temp_ {
    int num;
};
static Temp_temp FRi_table[20];

static void FRi_newTable(void) {
    if (FRi_table[0]) return;       // already initialized
    for (int i = 0; i < 20; i++) {  // otherwise, initialize!
        Temp_temp p = (Temp_temp)checked_malloc(sizeof(*p));
        p->num = i;
        FRi_table[i] = p;
        if (Temp_look(Temp_name(), p) ==
            NULL) {  // if ri is not yet in the Temp_map
            char r[16];
            sprintf(r, "r%d", i);
            Temp_enter(Temp_name(), p, String(r));
        }
    }
    return;
}

static Temp_temp FRi_lookup(int i) {
    FRi_newTable();
    return (FRi_table[i]);
}

Temp_temp F_Ri(int i) {  // for R0-R11
    return (FRi_lookup(i));
}

/*
Temp_temp F_Ri(int i) { //for R0-R11
    Temp_temp p = (Temp_temp) checked_malloc(sizeof (*p));
    p->num=i;
    if (Temp_look(Temp_name(), p) == NULL) { //if ri is not yet in the Temp_map
        char r[16];
        sprintf(r, "r%d", i);
        Temp_enter(Temp_name(), p, String(r));
    }
    return(p);
}
 */

Temp_temp F_FP(void) {
    return (F_Ri(11));  // r11 is FP register in RPi
}

Temp_temp F_SP(void) {
    return (F_Ri(13));  // r13 is SP register in RPi
}

Temp_temp F_LR(void) {
    return (F_Ri(14));  // r14 is LR register in RPi
}
