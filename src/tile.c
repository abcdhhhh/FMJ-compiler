#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assem.h"
#include "flowgraph.h"
#include "graph.h"
#include "ig.h"
#include "liveness.h"
#include "myutils.h"
#include "printtree.h"
#include "prog.h"
#include "rpi.h"
#include "temp.h"
#include "translate.h"
#include "tree.h"

#define IL(a, b) AS_InstrList(a, b)
#define MI(a, b, c) AS_Move(a, b, c)
#define LI(a, b) AS_Label(a, b)
#define OI(a, b, c, d) AS_Oper(a, b, c, d)
#define T(a) Temp_TempList(a, NULL)
#define TL(a, b) Temp_TempList(a, b)
#define LL(a, b) Temp_LabelList(a, b)
#define L(a) Temp_LabelList(a, NULL)
#define Targets(a) AS_Targets(a)

// utils
AS_instrListing iling;

Temp_temp tileE(T_exp);

void doLdrStr(T_exp reg, T_exp addr, bool isLdr) {
    Temp_temp _reg = tileE(reg);
    Temp_temp _addr;
    int offset;
    bool flag = FALSE;
    if (addr->kind == T_BINOP) {
        T_binOp op = addr->u.BINOP.op;
        T_exp left = addr->u.BINOP.left, right = addr->u.BINOP.right;
        if (op == T_plus) {
            if (right->kind == T_CONST) {
                flag = TRUE;
                _addr = tileE(left);
                offset = right->u.CONST;
            } else if (left->kind == T_CONST) {
                flag = TRUE;
                _addr = tileE(right);
                offset = left->u.CONST;
            }
        } else if (op == T_minus) {
            if (right->kind == T_CONST) {
                flag = TRUE;
                _addr = tileE(left);
                offset = -(right->u.CONST);
            }
        }
    }
    if (flag == TRUE) {
        assert(offset >= -4095 && offset <= 4095);
        if (isLdr) {
            emit(iling, OI(concats(3, "ldr `d0, [`s0, #", i2str(offset), "]"),
                           T(_reg), T(_addr), NULL));
        } else {
            emit(iling, OI(concats(3, "str `s0, [`s1, #", i2str(offset), "]"),
                           NULL, TL(_reg, T(_addr)), NULL));
        }
    } else {
        _addr = tileE(addr);
        if (isLdr) {
            emit(iling, OI("ldr `d0, [`s0]", T(_reg), T(_addr), NULL));
        } else {
            emit(iling, OI("str `s0, [`s1]", NULL, TL(_reg, T(_addr)), NULL));
        }
    }
}

void doBinOp(T_binOp op, Temp_temp _dst, T_exp left, T_exp right) {
    switch (op) {
        case T_plus: {
            Temp_temp _left, _right;
            if (right->kind == T_CONST) {
                _left = tileE(left);
                emit(iling,
                     OI(concats(2, "add `d0, `s0, #", i2str(right->u.CONST)),
                        T(_dst), T(_left), NULL));
            } else if (left->kind == T_CONST) {
                _right = tileE(right);
                emit(iling,
                     OI(concats(2, "add `d0, `s0, #", i2str(left->u.CONST)),
                        T(_dst), T(_right), NULL));
            } else {
                _left = tileE(left);
                _right = tileE(right);
                emit(iling, OI("add `d0, `s0, `s1", T(_dst),
                               TL(_left, T(_right)), NULL));
            }
        } break;
        case T_minus: {
            Temp_temp _left, _right;
            if (right->kind == T_CONST) {
                _left = tileE(left);
                emit(iling,
                     OI(concats(2, "sub `d0, `s0, #", i2str(right->u.CONST)),
                        T(_dst), T(_left), NULL));
            } else if (left->kind == T_CONST) {
                _right = tileE(right);
                emit(iling,
                     OI(concats(2, "rsb `d0, `s0, #", i2str(left->u.CONST)),
                        T(_dst), T(_right), NULL));
            } else {
                _left = tileE(left);
                _right = tileE(right);
                emit(iling, OI("sub `d0, `s0, `s1", T(_dst),
                               TL(_left, T(_right)), NULL));
            }
        } break;
        case T_mul: {
            bool flag = FALSE;
            Temp_temp _src;
            int k;
            if (right->kind == T_CONST) {
                int x = right->u.CONST;
                if (x > 0 && (x & x - 1) == 0) {
                    flag = TRUE;
                    _src = tileE(left);
                    k = myLog2(x);
                }
            } else if (left->kind == T_CONST) {
                int x = left->u.CONST;
                if (x > 0 && (x & x - 1) == 0) {
                    flag = TRUE;
                    _src = tileE(right);
                    k = myLog2(x);
                }
            }
            if (flag == TRUE) {
                emit(iling, MI(concats(2, "mov `d0, `s0, ASL #", i2str(k)),
                               T(_dst), T(_src)));
            } else {
                Temp_temp _left = tileE(left);
                Temp_temp _right = tileE(right);
                emit(iling, OI("mul `d0, `s0, `s1", T(_dst),
                               TL(_left, T(_right)), NULL));
            }
        } break;
        case T_div: {
            assert(right->kind == T_CONST);
            int x = right->u.CONST;
            assert(x > 0 && (x & x - 1) == 0);
            int k = myLog2(x);
            Temp_temp _left = tileE(left);
            emit(iling, MI(concats(2, "mov `d0, `s0, ASR #", i2str(k)), T(_dst),
                           T(_left)));
        } break;
        default: {
            assert(0);
        } break;
    }
}

void doMove(T_exp dst, T_exp src) {
    // printf("%d\n", dst->kind);
    switch (dst->kind) {
        case T_MEM: {
            doLdrStr(src, dst->u.MEM, 0);
        } break;
        case T_TEMP: {
            Temp_temp _dst = dst->u.TEMP;
            switch (src->kind) {
                case T_MEM: {
                    doLdrStr(dst, src->u.MEM, 1);
                } break;
                case T_NAME: {
                    emit(iling, OI(concats(2, "ldr `d0, =",
                                           Temp_labelstring(src->u.NAME)),
                                   T(_dst), NULL, NULL));
                } break;
                case T_CONST: {
                    emit(iling,
                         OI(concats(2, "mov `d0, #", i2str(src->u.CONST)),
                            T(_dst), NULL, NULL));
                } break;
                case T_TEMP: {
                    // printf("here\n");
                    Temp_temp _src = src->u.TEMP;
                    emit(iling, MI("mov `d0, `s0", T(_dst), T(_src)));
                } break;
                case T_BINOP: {
                    doBinOp(src->u.BINOP.op, _dst, src->u.BINOP.left,
                            src->u.BINOP.right);
                } break;
                default: {
                    assert(0);
                } break;
            }
        } break;
        default: {
            assert(0);
        } break;
    }
}

Temp_temp tileE(T_exp e) {
    switch (e->kind) {
        case T_TEMP: {
            return e->u.TEMP;
        } break;
        case T_BINOP:
        case T_MEM:
        case T_NAME:
        case T_CONST: {
            Temp_temp temp = Temp_newtemp();
            doMove(T_Temp(temp), e);
            return temp;
        } break;
        default: {
            assert(0);
        } break;
    }
}

void tileS(T_stm s) {
    switch (s->kind) {
        case T_SEQ: {
            assert(0);
        } break;
        case T_LABEL: {
            Temp_label label = s->u.LABEL;
            emit(iling,
                 LI(concats(2, Temp_labelstring(label), ": nop"), label));
        } break;
        case T_JUMP: {
            T_exp e = s->u.JUMP.exp;
            if (e->kind == T_NAME) {
                string name = Temp_labelstring(e->u.NAME);
                Temp_tempList params = NULL;
                if (strcmp(name, concats(2, "L", i2str(atoi(name + 1)))) == 0) {
                    // printf("branch(new) %s\n", name);
                } else {
                    // printf("branch(extern) %s\n", name);
                    params = TL(F_Ri(0), TL(F_Ri(1), TL(F_Ri(2), T(F_Ri(3)))));
                }
                emit(iling,
                     OI(concats(2, "b ", Temp_labelstring(e->u.NAME)), params,
                        params, Targets(L(s->u.JUMP.jumps->head))));
            } else {
                Temp_temp reg = tileE(e);
                if (reg == F_LR()) {  // bx lr
                    emit(iling, OI("bx r14", NULL, TL(F_LR(), T(F_Ri(0))),
                                   Targets(L(s->u.JUMP.jumps->head))));
                } else {
                    // string tmp = Temp_labelstring(s->u.JUMP.jumps->head);
                    // printf("bx(method) %s\n", tmp);
                    Temp_tempList params =
                        TL(F_Ri(0), TL(F_Ri(1), TL(F_Ri(2), T(F_Ri(3)))));
                    emit(iling, OI("bx `s0", params, TL(reg, params),
                                   Targets(L(s->u.JUMP.jumps->head))));
                }
            }
        } break;
        case T_CJUMP: {
            Temp_temp left = tileE(s->u.CJUMP.left);
            Temp_temp right = tileE(s->u.CJUMP.right);
            emit(iling, OI("cmp `s0, `s1", NULL, TL(left, T(right)), NULL));
            emit(iling, OI(concats(4, "b", op2a(s->u.CJUMP.op), " ",
                                   Temp_labelstring(s->u.CJUMP.true)),
                           NULL, NULL,
                           Targets(LL(s->u.CJUMP.true, L(s->u.CJUMP.false)))));
            emit(iling, OI(concats(2, "b ", Temp_labelstring(s->u.CJUMP.false)),
                           NULL, NULL, Targets(L(s->u.CJUMP.false))));
        } break;
        case T_MOVE: {
            doMove(s->u.MOVE.dst, s->u.MOVE.src);
        } break;
        case T_EXP: {
            tileE(s->u.EXP);
        } break;
        default: {
            assert(0);
        } break;
    }
}

AS_instrList tileSl(T_stmList slHead) {
    iling = AS_InstrListingEmpty();
    for (T_stmList sl = slHead; sl; sl = sl->tail) {
        T_stm s = sl->head;
        AS_instr instr;
        switch (s->kind) {
            case T_SEQ: {
                assert(0);
            } break;
            case T_MOVE: {
                T_exp dst = s->u.MOVE.dst, src = s->u.MOVE.src;
                if (dst->kind == T_TEMP && dst->u.TEMP == F_LR() &&
                    src->kind == T_NAME && sl->tail && sl->tail->tail) {
                    T_stm s1 = sl->tail->head, s2 = sl->tail->tail->head;
                    Temp_label return_label = src->u.NAME;
                    if (s1->kind == T_JUMP && s2->kind == T_LABEL &&
                        s2->u.LABEL == return_label) {
                        // callee-saved r0-r3,lr
                        Temp_tempList params = TL(
                            F_LR(),
                            TL(F_Ri(0), TL(F_Ri(1), TL(F_Ri(2), T(F_Ri(3))))));
                        assert(s1->u.JUMP.jumps->head == return_label);
                        if (s1->u.JUMP.exp->kind == T_NAME) {
                            Temp_label label = s1->u.JUMP.exp->u.NAME;
                            // string tmp = Temp_labelstring(label);
                            // printf("bl(extern) %s\n", tmp);
                            emit(iling,
                                 OI(concats(2, "bl ", Temp_labelstring(label)),
                                    params, params, Targets(L(return_label))));
                        } else {
                            // string tmp = Temp_labelstring(return_label);
                            // printf("blx(method) %s\n", tmp);
                            Temp_temp reg = tileE(s1->u.JUMP.exp);
                            emit(iling, OI("blx `s0", params, TL(reg, params),
                                           Targets(L(return_label))));
                        }
                        // return
                        emit(iling,
                             LI(concats(2, Temp_labelstring(return_label),
                                        ": nop"),
                                return_label));
                        sl = sl->tail->tail;
                    } else {
                        tileS(s);
                    }
                } else {
                    tileS(s);
                }
            } break;
            case T_CJUMP: {
                if (sl->tail) {
                    T_stm s1 = sl->tail->head;
                    if (s1->kind == T_LABEL &&
                        s1->u.LABEL == s->u.CJUMP.false) {
                        Temp_temp left = tileE(s->u.CJUMP.left);
                        Temp_temp right = tileE(s->u.CJUMP.right);
                        emit(iling, OI("cmp `s0, `s1", NULL, TL(left, T(right)),
                                       NULL));
                        emit(iling,
                             OI(concats(4, "b", op2a(s->u.CJUMP.op), " ",
                                        Temp_labelstring(s->u.CJUMP.true)),
                                NULL, NULL,
                                Targets(
                                    LL(s->u.CJUMP.true, L(s->u.CJUMP.false)))));
                        emit(iling,
                             LI(concats(2, Temp_labelstring(s->u.CJUMP.false),
                                        ": nop"),
                                s->u.CJUMP.false));
                        sl = sl->tail;
                    } else {
                        tileS(s);
                    }
                } else {
                    tileS(s);
                }
            } break;
            default: {
                tileS(s);
            } break;
        }
    }
    emit(iling, LI("END: nop", Temp_namedlabel("END")));
    return iling->heading;
}
