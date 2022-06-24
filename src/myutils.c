#include "myutils.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "assem.h"
#include "graph.h"
#include "temp.h"
#include "tree.h"
#include "util.h"

string concats(int num, ...) {
    va_list valist;
    va_start(valist, num);
    string* table = (string*)malloc(num * sizeof(string));
    int len = 0;
    for (int i = 0; i < num; i++) {
        string s = va_arg(valist, string);
        table[i] = (string)malloc((strlen(s) + 1) * sizeof(char));
        strcpy(table[i], s);
        len += strlen(s);
    }
    va_end(valist);
    string ret = (string)malloc((len + 1) * sizeof(char));
    ret[0] = '\0';
    for (int i = 0; i < num; i++) strcat(ret, table[i]);
    return ret;
}

void eraseNode(G_node gn, Temp_map degs) {
    for (G_nodeList p = G_succ(gn); p; p = p->tail) {
        G_node gv = p->head;
        Temp_temp v = G_nodeInfo(gv);
        int vd = atoi(Temp_look(degs, v));
        assert(vd != 0);
        if (vd > 0) Temp_enter(degs, v, i2str(vd - 1));
    }
}

AS_instrListing AS_InstrListingEmpty() {
    AS_instrListing iling = (AS_instrListing)checked_malloc(sizeof(*iling));
    iling->heading = NULL;
    iling->tailing = NULL;
    return iling;
}
void emit(AS_instrListing iling, AS_instr i) {
    if (iling->heading == NULL) {
        iling->heading = iling->tailing = AS_InstrList(i, NULL);
    } else {
        iling->tailing->tail = AS_InstrList(i, NULL);
        iling->tailing = iling->tailing->tail;
    }
}

string i2str(int i) {
    string ret = (string)malloc(10 * sizeof(char));
    sprintf(ret, "%d", i);
    return ret;
}

int myLog2(int val) {
    int k = 0;
    while (!(val & 1)) {
        k++;
        val = (unsigned)val >> 1;
    }
    assert(val == 1);
    return k;
}

T_relOp negOp(T_relOp op) {
    switch (op) {
        case T_eq: {
            return T_ne;
        } break;
        case T_ne: {
            return T_eq;
        } break;
        case T_lt: {
            return T_ge;
        } break;
        case T_gt: {
            return T_le;
        } break;
        case T_le: {
            return T_gt;
        } break;
        case T_ge: {
            return T_lt;
        } break;
        default: {
            assert(0);
        } break;
    }
    assert(0);
    return T_eq;
}

string op2a(T_relOp op) {
    switch (op) {
        case T_eq: {
            return "eq";
        } break;
        case T_ne: {
            return "ne";
        } break;
        case T_lt: {
            return "lt";
        } break;
        case T_gt: {
            return "gt";
        } break;
        case T_le: {
            return "le";
        } break;
        case T_ge: {
            return "ge";
        } break;
        default: {
            assert(0);
        } break;
    }
    assert(0);
    return "";
}

T_stm safeSeq(T_stm left, T_stm right) {
    if (left == NULL) return right;
    if (right == NULL) return left;
    return T_Seq(left, right);
}
T_stm superSeq(int num, ...) {
    va_list valist;
    va_start(valist, num);
    T_stm ret = NULL;
    for (int i = 0; i < num; i++) {
        ret = safeSeq(ret, va_arg(valist, T_stm));
    }
    va_end(valist);
    return ret;
}
