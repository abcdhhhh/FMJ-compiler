/* IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 */

/*
   This file contains the following functions:
1. AS_instrList RA_RegAlloc(string method_name, AS_instrList il, Temp_map tm)
   This is to give the final instructions for the method with the method name
   il is the instruction list after the instructions selection
   tm is the register assignment map (temp->register_string: r0, r1, .. or
Spill, which means the temp is a spill. The resulting instructions have the new
dst and src lists so that they have the right registers for each d and s, except
for the spilled temps.

2. AS_instrList RA_Spill(AS_instrList il, Temp_tempList spills)
   The input il is assumed to have assigned with registers (in dst and src)
except for the temps in spills. This function is to (1) find a stack location
(offset) for each of the spilled temps, (2) for each instruction with spilled
temps, insert the appropriate instructions (using r9-10 for spilling), and (3)
replace the spilled temps in dst and src with r9 or r10 appropriately.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assem.h"
#include "flowgraph.h"
#include "graph.h"
#include "ig.h"
#include "liveness.h"
#include "myutils.h"
#include "rpi.h"
#include "symbol.h"
#include "table.h"
#include "temp.h"
#include "util.h"

// This is to find the register that's assigned to temp t based on tm
// If return NULL, then it's a Spill
static Temp_temp RA_FindColor(Temp_map tm, Temp_temp t) {
    // if (t) printf("find color for =%s\n", Temp_look(Temp_name(), t));
    string color = Temp_look(tm, t);
    assert(color);
    // printf("found color %s\n",color);
    if (!strcmp(color, "Spill")) {
        return NULL;
    }
    assert(strlen(color) > 1);
    int i = atoi(&color[1]);
    // printf("Temp %s assigned to register i=%d\n", Temp_look(Temp_name(), t),
    // i);
    return (F_Ri(i));
}

static Temp_label nthLabel(Temp_labelList list, int i) {
    assert(list);
    if (i == 0)
        return list->head;
    else
        return nthLabel(list->tail, i - 1);
}

static Temp_temp nthTemp(Temp_tempList list, int i) {
    assert(list);
    if (i == 0)
        return list->head;
    else
        return nthTemp(list->tail, i - 1);
}

// This one does two things:
// 1) clean the def/use list so that only the ones appearing in
//    the instruction string assem (i.e., used by `di `si) will remain
// 2) change the temp in the def/use lists to the assigned regiters
//    (however, if the temp is spilled, then the original temp stays).
//    we need another pass to perform the actual spill wit RA_Spill()

static AS_instr RA_AssignColor(Temp_map tm, AS_instr instr) {
    Temp_tempList new_dst = NULL, new_src = NULL;
    Temp_tempList last_dst = NULL, last_src = NULL;
    Temp_tempList old_dst, old_src;
    char* assem;
    Temp_temp t, nth;
    char* p;

    if (instr->kind == I_LABEL) return instr;  // nothing to do if it's a label

    if (instr->kind == I_OPER) {
        old_dst = instr->u.OPER.dst;
        old_src = instr->u.OPER.src;
        assem = instr->u.OPER.assem;
        // printf("instr=%s\n", assem);
    } else {  // it's a MOVE instruction
        old_dst = instr->u.MOVE.dst;
        old_src = instr->u.MOVE.src;
        assem = instr->u.MOVE.assem;
        // printf("instr=%s\n", assem);
    }

    // now scan the assem string to find `d and `s
    for (p = assem; p && *p != '\0'; p++)
        if (*p == '`') switch (*(++p)) {
                case 's': {
                    int n = atoi(++p);
                    nth = nthTemp(old_src, n);  // this is the source temp
                    t = RA_FindColor(tm, nth);  // this is the register for it
                    // if (t)
                    //     printf("s: color=%s\n", Temp_look(Temp_name(), t));
                    // else
                    //     printf("s: spill\n");
                    if (t ==
                        NULL) {  // this is a spill, so keep the original temp
                        if (last_src)
                            last_src =
                                (last_src->tail = Temp_TempList(nth, NULL));
                        else
                            last_src = (new_src = Temp_TempList(nth, NULL));
                    } else {  // found its register t
                        if (last_src)
                            last_src =
                                (last_src->tail = Temp_TempList(t, NULL));
                        else
                            last_src = (new_src = Temp_TempList(t, NULL));
                    }
                } break;
                case 'd': {
                    int n = atoi(++p);
                    nth = nthTemp(old_dst, n);
                    t = RA_FindColor(tm, nth);
                    // if (t)
                    //     printf("d: color=%s\n", Temp_look(Temp_name(), t));
                    // else
                    //     printf("d: spill\n");
                    if (t == NULL) {  // Spill temp
                        if (last_dst)
                            last_dst =
                                (last_dst->tail = Temp_TempList(nth, NULL));
                        else
                            last_dst = (new_dst = Temp_TempList(nth, NULL));
                    } else {  // register t
                        if (last_dst)
                            last_dst =
                                (last_dst->tail = Temp_TempList(t, NULL));
                        else
                            last_dst = (new_dst = Temp_TempList(t, NULL));
                    }
                } break;
                case 'j':  // we don't use this
                    break;
                case '`':
                    break;
                default:
                    assert(0);
            }

    // printf("new assem=%s\n", new_assem);

    if (instr->kind == I_OPER) {
        instr->u.OPER.dst = new_dst;
        instr->u.OPER.src = new_src;
    } else {  // it's a MOVE instruction
        instr->u.MOVE.dst = new_dst;
        instr->u.MOVE.src = new_src;
    }
    return instr;
}

/* IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 */

/* You need to implement the following function to deal with the spills */

AS_instrList RA_Spill(AS_instrList il, Temp_tempList spills) {
    /* (1) decide the offset for each of the spills: may use a Temp_map to
       remember the offset for each (2) then go through the instructions in il.
       If any of the dst or src have the spills, insert the right instructions,
       and change the spilled temp to r9-r10 in dst and src (3) return the
       resulting instruction list
     */
    Temp_map t2offset = Temp_empty();

    int offset = 0;
    for (Temp_tempList tl = spills; tl; tl = tl->tail) {
        Temp_temp t = tl->head;
        Temp_enter(t2offset, t, i2str(offset));
        offset += 4;
    }

    AS_instrListing ret = AS_InstrListingEmpty();

    for (AS_instrList ilCur = il; ilCur; ilCur = ilCur->tail) {
        AS_instr i = ilCur->head;
        Temp_tempList dst = NULL, src = NULL;
        switch (i->kind) {
            case I_OPER: {
                dst = i->u.OPER.dst, src = i->u.OPER.src;
            } break;
            case I_MOVE: {
                dst = i->u.MOVE.dst, src = i->u.MOVE.src;
            } break;
            default: {
            } break;
        }
        int id = 9;
        for (Temp_tempList tl = src; tl; tl = tl->tail) {
            Temp_temp t = tl->head;
            string s_offset = Temp_look(t2offset, t);
            if (s_offset) {
                emit(ret, AS_Oper(concats(3, "ldr `d0, [`s0, #", s_offset, "]"),
                                  Temp_TempList(F_Ri(id), NULL),
                                  Temp_TempList(F_FP(), NULL), NULL));
                tl->head = F_Ri(id);  // this changes src
                id++;
            }
        }
        assert(id <= 11);

        AS_instrListing tmp =
            AS_InstrListingEmpty();  // temporarily store str operations
        id = 9;
        for (Temp_tempList tl = dst; tl; tl = tl->tail) {
            Temp_temp t = tl->head;
            string s_offset = Temp_look(t2offset, t);
            if (s_offset) {
                emit(tmp,
                     AS_Oper(
                         concats(3, "str `s0, [`s1, #", s_offset, "]"), NULL,
                         Temp_TempList(F_Ri(id), Temp_TempList(F_FP(), NULL)),
                         NULL));
                tl->head = F_Ri(id);  // this changes dst
                id++;
            }
        }
        assert(id <= 11);
        if (i->kind == I_MOVE && strstr(i->u.MOVE.assem, "S") == NULL && dst &&
            src && dst->head == src->head) {
        } else {
            // printf("%d\n", i->kind);
            emit(ret, i);
        }
        for (AS_instrList dstIl = tmp->heading; dstIl; dstIl = dstIl->tail) {
            emit(ret, dstIl->head);
        }
    }
    return ret->heading;
}

// given a temp_map that gives assigns registers to temps (with some of them as
// "Spill") Update the input instruction list to generate the final instruction
// list method_name: method name string for the instruction list

// In this implementation, I assume the last instruction of the il is
//"bx lr", and this is the only return point for the method.
// I will insert pop {r4-r10} before that one.
//  In your implementation, you need to adjust to your situation!!
/* IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 */

AS_instrList RA_RegAlloc(string method_name, AS_instrList il, Temp_map tm,
                         int num_spills) {
    AS_instrList ril = NULL, last = NULL;
    Temp_tempList params = Temp_TempList(F_SP(), NULL);
    for (int i = 4; i <= 10; i++) params = Temp_TempList(F_Ri(i), params);
    AS_instr push = AS_Oper("push {r4-r10}", NULL, params, NULL);
    AS_instr spill_push = RA_AssignColor(
        tm, AS_Oper(concats(2, "sub `d0, `s0, #", i2str(num_spills * 4)),
                    Temp_TempList(F_SP(), NULL), Temp_TempList(F_SP(), NULL),
                    NULL));

    AS_instr spill_pop = RA_AssignColor(
        tm, AS_Oper(concats(2, "add `d0, `s0, #", i2str(num_spills * 4)),
                    Temp_TempList(F_SP(), NULL), Temp_TempList(F_SP(), NULL),
                    NULL));
    AS_instr pop = AS_Oper("pop {r4-r10}", params, NULL, NULL);

    // now change the def and use lists of each instruction
    for (; il; il = il->tail) {
        AS_instr i = RA_AssignColor(tm, il->head);
        if (ril == NULL) {
            ril = AS_InstrList(
                i, AS_InstrList(push, AS_InstrList(spill_push, NULL)));
            last = ril->tail->tail;
        } else {
            if (il->head->kind == I_OPER &&
                (strcmp(i->u.OPER.assem, "bx r14") == 0 ||
                 strcmp(i->u.OPER.assem, "bx lr") == 0)) {
                last->tail = AS_InstrList(spill_pop, AS_InstrList(pop, NULL));
                last = last->tail->tail;
            }
            last->tail = AS_InstrList(i, NULL);
            last = last->tail;
        }
    }
    return ril;
}
