#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "canon.h"
#include "fmjAST.h"
#include "myutils.h"
#include "printtree.h"
#include "prog.h"
#include "rpi.h"
#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "util.h"

// utils

typedef struct stringList_ *stringList;
struct stringList_ {
    string head;
    stringList tail;
};
stringList StringList(string head, stringList tail) {
    stringList strl = checked_malloc(sizeof *strl);
    strl->head = head;
    strl->tail = tail;
    return (strl);
}

stringList methods;
S_table funcs;           // extern functions
S_table vars2offset;     // plused 1
S_table methods2offset;  // plused 1
S_table classMethods2label;
int recordSize;  // size of classes

T_stm begin_frame() {  // push {lr,fp}, fp=(old)sp
    return superSeq(
        4,
        T_Move(T_Mem(T_Binop(T_minus, T_Temp(F_SP()), T_Const(4))),
               T_Temp(F_LR())),
        T_Move(T_Mem(T_Binop(T_minus, T_Temp(F_SP()), T_Const(8))),
               T_Temp(F_FP())),
        T_Move(T_Temp(F_FP()), T_Temp(F_SP())),
        T_Move(T_Temp(F_SP()), T_Binop(T_minus, T_Temp(F_SP()), T_Const(8))));
}

T_stm end_frame() {              // pop {lr,fp}, jump{lr}
    Temp_temp temp_lr = F_LR();  // bx lr
    return superSeq(
        4, T_Move(T_Temp(F_SP()), T_Binop(T_plus, T_Temp(F_SP()), T_Const(8))),
        T_Move(T_Temp(F_FP()),
               T_Mem(T_Binop(T_minus, T_Temp(F_SP()), T_Const(8)))),
        T_Move(T_Temp(temp_lr),
               T_Mem(T_Binop(T_minus, T_Temp(F_SP()), T_Const(4)))),
        T_Jump(T_Temp(temp_lr),
               Temp_LabelList(
                   Temp_namedlabel(String("END")),
                   Temp_LabelList(Temp_namedlabel(String("END")), NULL))));
}

T_exp myCall(T_exp fun, T_expList args, string methodname) {
    assert(methodname != NULL);
    Temp_label return_l = Temp_newlabel();
    Temp_temp temp_r0 = Temp_newtemp();
    T_stm s = NULL;
    // add args
    int i = 0;
    for (T_expList arg_ = args; arg_; arg_ = arg_->tail) {
        T_exp arg = arg_->head;
        s = safeSeq(s, T_Move(T_Temp(F_Ri(i)), arg));
        i++;
    }

    s = superSeq(
        5, s, T_Move(T_Temp(F_LR()), T_Name(return_l)),
        T_Jump(fun,
               Temp_LabelList(
                   return_l,
                   Temp_LabelList(
                       Temp_namedlabel(String(methodname)),
                       Temp_LabelList(Temp_namedlabel(String("CALL")), NULL)))),
        T_Label(return_l), T_Move(T_Temp(temp_r0), T_Temp(F_Ri(0))));
    return T_Eseq(s, T_Temp(temp_r0));
}

T_stm myReturn(T_exp e) {
    return safeSeq(T_Move(T_Temp(F_Ri(0)), e), end_frame());
}

T_exp branchE(T_relOp op, T_exp opleft, T_exp opright, T_exp trueExp,
              T_exp falseExp) {
    Temp_label lt = Temp_newlabel();
    Temp_label lf = Temp_newlabel();
    Temp_label lend = Temp_newlabel();

    S_table temp = S_empty();  // temporary var environment
    Temp_temp temp_x = Temp_newtemp();
    S_enter(temp, S_Symbol("temp"), temp_x);

    T_stm s =
        superSeq(7, T_Cjump(op, opleft, opright, lt, lf), T_Label(lt),
                 T_Move(T_Temp(temp_x), trueExp),
                 T_Jump(T_Name(lend), Temp_LabelList(lend, NULL)), T_Label(lf),
                 T_Move(T_Temp(temp_x), falseExp), T_Label(lend));
    return T_Eseq(s, T_Temp(temp_x));
}

// translation

T_stm translateSl(A_stmList, S_table, Temp_label, Temp_label);
T_expList translateEl(A_expList, S_table);
T_exp translateE(A_exp e, S_table vars) {
    switch (e->kind) {
        case A_opExp: {
            T_exp left = translateE(e->u.op.left, vars);
            T_exp right = translateE(e->u.op.right, vars);
            T_binOp op;
            switch (e->u.op.oper) {
                case A_and: {
                    return branchE(T_eq, left, T_Const(0), T_Const(0),
                                   branchE(T_eq, right, T_Const(0), T_Const(0),
                                           T_Const(1)));
                } break;
                case A_or: {
                    return branchE(T_ne, left, T_Const(0), T_Const(1),
                                   branchE(T_ne, right, T_Const(0), T_Const(1),
                                           T_Const(0)));
                } break;
                case A_less: {
                    return branchE(T_lt, left, right, T_Const(1), T_Const(0));
                } break;
                case A_le: {
                    return branchE(T_le, left, right, T_Const(1), T_Const(0));
                } break;
                case A_eq: {
                    return branchE(T_eq, left, right, T_Const(1), T_Const(0));
                } break;
                case A_plus: {
                    op = T_plus;
                } break;
                case A_minus: {
                    op = T_minus;
                } break;
                case A_times: {
                    op = T_mul;
                } break;
                case A_div: {
                    op = T_div;
                } break;
                default: {
                    assert(0);
                } break;
            }
            return T_Binop(op, left, right);
        } break;
        case A_arrayExp: {  // get address
            T_expList elist = NULL, el = NULL;
            A_exp e_ = e;
            while (e_->kind == A_arrayExp) {
                T_expList elist_ = translateEl(e_->u.array.elist, vars);
                assert(elist_ != NULL);
                if (el == NULL) {
                    elist = el = elist_;
                    while (el->tail != NULL) el = el->tail;
                } else {
                    el->tail = elist_;
                    while (el->tail != NULL) el = el->tail;
                }
                e_ = e_->u.array.e;
            }

            Temp_temp temp_arr = Temp_newtemp();

            int arity = 0;
            for (T_expList elist_ = elist; elist_; elist_ = elist_->tail)
                arity++;
            assert(arity >= 1 && arity <= 3);
            while (arity < 3) {
                elist = T_ExpList(T_Const(0), elist);
                arity++;
            }

            T_exp d2 = T_Mem(T_Temp(temp_arr)),
                  d3 = T_Mem(T_Binop(T_plus, T_Temp(temp_arr), T_Const(4)));
            T_exp i1 = elist->head, i2 = elist->tail->head,
                  i3 = elist->tail->tail->head;
            T_exp offset = T_Binop(
                T_plus,
                T_Binop(T_mul, T_Binop(T_plus, T_Binop(T_mul, i1, d2), i2), d3),
                i3);
            offset =
                T_Binop(T_mul, T_Binop(T_plus, T_Const(2), offset), T_Const(4));
            return T_Eseq(T_Move(T_Temp(temp_arr), translateE(e_, vars)),
                          T_Mem(T_Binop(T_plus, T_Temp(temp_arr), offset)));

        } break;
        case A_lengthExp: {
            assert(0);
        } break;
        case A_callExp: {
            T_exp e_ = translateE(e->u.call.e, vars);
            string methodname = e->u.call.id;
            T_expList elist = translateEl(e->u.call.elist, vars);
            int offset = ((int)(long long)S_look(methods2offset,
                                                 S_Symbol(e->u.call.id)) -
                          1) *
                         4;
            Temp_temp temp_a = Temp_newtemp();
            return T_Eseq(
                T_Move(T_Temp(temp_a), e_),
                myCall(T_Mem(T_Binop(T_plus, T_Temp(temp_a), T_Const(offset))),
                       T_ExpList(T_Temp(temp_a), elist), methodname));
        } break;
        case A_numExp: {
            return T_Const(e->u.num);
        } break;
        case A_boolExp: {
            return T_Const(e->u.b);
        } break;
        case A_idExp: {
            if (S_look(vars, S_Symbol(e->u.v)) != NULL) {
                return T_Temp(S_look(vars, S_Symbol(e->u.v)));
            } else if (S_look(vars2offset, S_Symbol(e->u.v)) != NULL) {
                assert(S_look(vars, S_Symbol("this")) != NULL);
                int offset =
                    ((int)(long long)S_look(vars2offset, S_Symbol(e->u.v)) -
                     1) *
                    4;
                return T_Mem(T_Binop(T_plus,
                                     T_Temp(S_look(vars, S_Symbol("this"))),
                                     T_Const(offset)));
            } else {
                assert(0);
            }
        } break;
        case A_thisExp: {
            assert(S_look(vars, S_Symbol("this")) != NULL);
            return T_Temp(S_look(vars, S_Symbol("this")));
        } break;
        case A_newIntArrExp: {  // malloc space and record dimension size
            T_expList elist = translateEl(e->u.newIntArr.elist, vars);

            int arity = 0;
            for (T_expList el = elist; el; el = el->tail) arity++;
            assert(arity >= 1 && arity <= 3);
            while (arity < 3) {
                elist = T_ExpList(T_Const(1), elist);
                arity++;
            }

            T_exp d1 = elist->head, d2 = elist->tail->head,
                  d3 = elist->tail->tail->head;
            Temp_temp temp_a = Temp_newtemp(), temp_d2 = Temp_newtemp(),
                      temp_d3 = Temp_newtemp();
            T_exp arrSize = T_Binop(
                T_mul, d1, T_Binop(T_mul, T_Temp(temp_d2), T_Temp(temp_d3)));
            arrSize = T_Binop(T_mul, T_Binop(T_plus, T_Const(2), arrSize),
                              T_Const(4));

            T_stm s = superSeq(
                5, T_Move(T_Temp(temp_d2), d2), T_Move(T_Temp(temp_d3), d3),
                T_Move(T_Temp(temp_a),
                       myCall(T_Name(S_look(funcs, S_Symbol(String("malloc")))),
                              T_ExpList(arrSize, NULL), "malloc")),
                T_Move(T_Mem(T_Temp(temp_a)), T_Temp(temp_d2)),
                T_Move(T_Mem(T_Binop(T_plus, T_Temp(temp_a), T_Const(4))),
                       T_Temp(temp_d3)));
            return T_Eseq(s, T_Temp(temp_a));
        } break;
        case A_newObjExp: {  // traverse all (method, offset) and move label to
                             // address
            Temp_temp temp_a = Temp_newtemp();
            T_stm s = T_Move(
                T_Temp(temp_a),
                myCall(T_Name(S_look(funcs, S_Symbol(String("malloc")))),
                       T_ExpList(T_Const(recordSize * 4), NULL), "malloc"));
            for (stringList meth = methods; meth; meth = meth->tail) {
                string method = meth->head;
                string cm = concats(3, e->u.v, ".", method);
                if (S_look(classMethods2label, S_Symbol(cm)) != NULL) {
                    int offset = ((int)(long long)S_look(methods2offset,
                                                         S_Symbol(method)) -
                                  1) *
                                 4;
                    s = safeSeq(s, T_Move(T_Mem(T_Binop(T_plus, T_Temp(temp_a),
                                                        T_Const(offset))),
                                          T_Name(S_look(classMethods2label,
                                                        S_Symbol(cm)))));
                }
            }
            return T_Eseq(s, T_Temp(temp_a));
        } break;
        case A_notExp: {
            T_exp e_ = translateE(e->u.e, vars);
            return branchE(T_ne, e_, T_Const(0), T_Const(0), T_Const(1));
        } break;
        case A_getint: {
            return myCall(T_Name(S_look(funcs, S_Symbol(String("getint")))),
                          NULL, "getint");
        } break;
        case A_getch: {
            return myCall(T_Name(S_look(funcs, S_Symbol(String("getch")))),
                          NULL, "getch");
        } break;
        case A_getarray: {
            T_exp e_ = translateE(e->u.e, vars);
            return myCall(T_Name(S_look(funcs, S_Symbol(String("getarray")))),
                          T_ExpList(T_Binop(T_plus, T_Const(8), e_), NULL),
                          "getarray");
        } break;
        default: {
            assert(0);
        } break;
    }
}
T_expList translateEl(A_expList el, S_table vars) {
    T_expList ret = NULL, cur = NULL;
    while (el) {
        A_exp e = el->head;
        if (cur == NULL) {
            ret = cur = T_ExpList(translateE(e, vars), NULL);
        } else {
            cur->tail = T_ExpList(translateE(e, vars), NULL);
            cur = cur->tail;
        }
        el = el->tail;
    }
    return ret;
}

T_stm translateS(A_stm s, S_table vars, Temp_label _lbegin, Temp_label _lf) {
    switch (s->kind) {
        case A_nestedStm: {
            return translateSl(s->u.ns, vars, _lbegin, _lf);
        } break;
        case A_ifStm: {
            Temp_label lt = Temp_newlabel();
            Temp_label lf = Temp_newlabel();
            Temp_label lend = Temp_newlabel();
            T_exp e = translateE(s->u.if_stat.e, vars);
            T_stm s1 = translateS(s->u.if_stat.s1, vars, _lbegin, _lf);
            T_stm s2 = translateS(s->u.if_stat.s2, vars, _lbegin, _lf);

            return superSeq(7, T_Cjump(T_ne, e, T_Const(0), lt, lf),
                            T_Label(lt), s1,
                            T_Jump(T_Name(lend), Temp_LabelList(lend, NULL)),
                            T_Label(lf), s2, T_Label(lend));
        } break;
        case A_whileStm: {
            Temp_label lbegin = Temp_newlabel();
            Temp_label lt = Temp_newlabel();
            Temp_label lf = Temp_newlabel();
            T_exp e = translateE(s->u.while_stat.e, vars);
            T_stm s_ = translateS(s->u.while_stat.s, vars, lbegin, lf);

            return superSeq(
                6, T_Label(lbegin), T_Cjump(T_ne, e, T_Const(0), lt, lf),
                T_Label(lt), s_,
                T_Jump(T_Name(lbegin), Temp_LabelList(lbegin, NULL)),
                T_Label(lf));
        } break;
        case A_assignStm: {
            T_exp left;
            if (s->u.assign.elist != NULL) {
                left = translateE(A_ArrayExp(A_IdExp(s->pos, s->u.assign.v),
                                             s->u.assign.elist),
                                  vars);
            } else {
                left = translateE(A_IdExp(s->pos, s->u.assign.v), vars);
            }
            return T_Move(left, translateE(s->u.assign.e, vars));
        } break;
        case A_continue: {
            assert(_lbegin);
            return T_Jump(T_Name(_lbegin), Temp_LabelList(_lbegin, NULL));
        } break;
        case A_break: {
            assert(_lf);
            return T_Jump(T_Name(_lf), Temp_LabelList(_lf, NULL));
        } break;
        case A_return: {
            return myReturn(translateE(s->u.e, vars));

        } break;
        case A_putint: {
            return T_Exp(
                myCall(T_Name(S_look(funcs, S_Symbol(String("putint")))),
                       T_ExpList(translateE(s->u.e, vars), NULL), "putint"));
        } break;
        case A_putarray: {
            return T_Exp(myCall(
                T_Name(S_look(funcs, S_Symbol(String("putarray")))),
                T_ExpList(translateE(s->u.putarray.e1, vars),
                          T_ExpList(T_Binop(T_plus, T_Const(8),
                                            translateE(s->u.putarray.e2, vars)),
                                    NULL)),
                "putarray"));
        } break;
        case A_putch: {
            return T_Exp(
                myCall(T_Name(S_look(funcs, S_Symbol(String("putch")))),
                       T_ExpList(translateE(s->u.e, vars), NULL), "putch"));
        } break;
        case A_starttime: {
            return T_Exp(myCall(
                T_Name(S_look(funcs, S_Symbol(String("_sysy_starttime")))),
                NULL, "_sysy_starttime"));
        } break;
        case A_stoptime: {
            return T_Exp(myCall(
                T_Name(S_look(funcs, S_Symbol(String("_sysy_stoptime")))), NULL,
                "_sysy_stoptime"));
        } break;
        default: {
            assert(0);
        } break;
    }
}

T_stm translateSl(A_stmList sl, S_table vars, Temp_label _lbegin,
                  Temp_label _lf) {
    T_stm ret = NULL, tmp;
    while (sl) {
        ret = safeSeq(ret, translateS(sl->head, vars, _lbegin, _lf));
        sl = sl->tail;
    }
    return ret;
}

void scanD(A_varDecl d, S_table vars) {
    S_enter(vars, S_Symbol(d->v), Temp_newtemp());
}
void scanDl(A_varDeclList dl, S_table vars) {
    while (dl) {
        scanD(dl->head, vars);
        dl = dl->tail;
    }
}

T_stm translateMd(A_methodDecl md, string classId) {
    string cm = concats(3, classId, ".", md->id);
    Temp_label label = S_look(classMethods2label, S_Symbol(cm));
    assert(label == S_Symbol(cm));
    T_stm s = safeSeq(T_Label(label), begin_frame());
    S_table vars = S_empty();
    Temp_temp temp = Temp_newtemp();
    S_enter(vars, S_Symbol("this"), temp);
    s = safeSeq(s, T_Move(T_Temp(temp), T_Temp(F_Ri(0))));
    A_formalList fl = md->fl;
    int i = 1;
    while (fl) {
        A_formal f = fl->head;
        temp = Temp_newtemp();
        S_enter(vars, S_Symbol(f->id), temp);
        s = safeSeq(s, T_Move(T_Temp(temp), T_Temp(F_Ri(i))));
        i++;
        fl = fl->tail;
    }
    scanDl(md->vdl, vars);
    return superSeq(2, s, translateSl(md->sl, vars, NULL, NULL));
}
T_stmList translateMdl(A_methodDeclList mdl, string classId) {
    T_stmList ret = NULL, rettail = NULL;
    while (mdl) {
        if (ret == NULL) {
            ret = rettail = T_StmList(translateMd(mdl->head, classId), NULL);
        } else {
            rettail->tail = T_StmList(translateMd(mdl->head, classId), NULL);
            rettail = rettail->tail;
        }
        mdl = mdl->tail;
    }
    return ret;
}

T_stmList translateCd(A_classDecl cd) { return translateMdl(cd->mdl, cd->id); }
T_stmList translateCdl(A_classDeclList cdl) {
    T_stmList ret = NULL, rettail = NULL;
    while (cdl) {
        if (ret == NULL) {
            ret = rettail = translateCd(cdl->head);
        } else {
            rettail->tail = translateCd(cdl->head);
        }
        while (rettail && rettail->tail) rettail = rettail->tail;
        cdl = cdl->tail;
    }
    return ret;
}

void scanCdl(A_classDeclList cdl) {
    // printf("member vars:\n");
    for (A_classDeclList x = cdl; x; x = x->tail) {
        A_classDecl cd = x->head;
        A_varDeclList vdl = cd->vdl;
        while (vdl) {
            A_varDecl vd = vdl->head;
            if (S_look(vars2offset, S_Symbol(vd->v)) == NULL) {
                S_enter(vars2offset, S_Symbol(vd->v),
                        (void *)(long long)(recordSize + 1));
                recordSize += 1;
                // printf("%s\n", vd->v);
                assert(S_look(vars2offset, S_Symbol(vd->v)) != NULL);
            }
            vdl = vdl->tail;
        }
    }

    // printf("original methods:\n");
    for (A_classDeclList x = cdl; x; x = x->tail) {
        A_classDecl cd = x->head;
        A_methodDeclList mdl = cd->mdl;
        while (mdl) {
            A_methodDecl md = mdl->head;
            if (S_look(methods2offset, S_Symbol(md->id)) == NULL) {
                S_enter(methods2offset, S_Symbol(md->id),
                        (void *)(long long)(recordSize + 1));
                methods = StringList(md->id, methods);
                recordSize += 1;
            }
            string cm = concats(3, cd->id, ".", md->id);
            S_enter(classMethods2label, S_Symbol(cm), Temp_namedlabel(cm));
            // printf("%s\n", cm);
            mdl = mdl->tail;
        }
    }
    // printf("inherited methods:\n");
    for (A_classDeclList x = cdl; x; x = x->tail) {
        A_classDecl cd = x->head;
        if (cd->parentID != NULL) {
            for (stringList strl = methods; strl; strl = strl->tail) {
                string str = strl->head;
                string cm = concats(3, cd->id, ".", str),
                       parentCm = concats(3, cd->parentID, ".", str);
                if (S_look(classMethods2label, S_Symbol(cm)) == NULL &&
                    S_look(classMethods2label, S_Symbol(parentCm)) != NULL) {
                    S_enter(classMethods2label, S_Symbol(cm),
                            S_look(classMethods2label, S_Symbol(parentCm)));
                    // printf("%s\n", cm);
                }
            }
        }
    }
}

T_stm translateM(A_mainClass m) {
    // vars
    Temp_label label = Temp_namedlabel("main");
    S_table vars = S_empty();
    scanDl(m->dl, vars);
    return superSeq(3, T_Label(label), begin_frame(),
                    translateSl(m->sl, vars, NULL, NULL));
}

T_stmList translateProg(A_prog p) {
    // funcs
    funcs = S_empty();
    S_enter(funcs, S_Symbol(String("_sysy_starttime")),
            Temp_namedlabel(String("_sysy_starttime")));
    S_enter(funcs, S_Symbol(String("_sysy_stoptime")),
            Temp_namedlabel(String("_sysy_stoptime")));
    S_enter(funcs, S_Symbol(String("putint")),
            Temp_namedlabel(String("putint")));
    S_enter(funcs, S_Symbol(String("putch")), Temp_namedlabel(String("putch")));
    S_enter(funcs, S_Symbol(String("putarray")),
            Temp_namedlabel(String("putarray")));
    S_enter(funcs, S_Symbol(String("getint")),
            Temp_namedlabel(String("getint")));
    S_enter(funcs, S_Symbol(String("getch")), Temp_namedlabel(String("getch")));
    S_enter(funcs, S_Symbol(String("getarray")),
            Temp_namedlabel(String("getarray")));
    // S_enter(funcs, S_Symbol(String("return")),
    //         Temp_namedlabel(String("return")));
    S_enter(funcs, S_Symbol(String("malloc")),
            Temp_namedlabel(String("malloc")));

    // build records
    methods = NULL;
    vars2offset = S_empty();
    methods2offset = S_empty();
    classMethods2label = S_empty();  // "class.method" to label
    recordSize = 0;
    scanCdl(p->cdl);
    // printf("begin translation\n");
    T_stmList ret = T_StmList(translateM(p->m), translateCdl(p->cdl));

    // printf("end translation\n");
    return ret;
}

T_stmList trans_prog(A_prog p) {
    // check_prog(p);
    // printf("checked!\n");
    T_stmList sl = translateProg(p);

    return sl;
}

// int main(int argc, const char *argv[]) {
//     A_prog p = prog();
//     printf("\n\nThe Final Canonical Tree:\n");
//     printStmList(stdout, trans_prog(p));
//     return 0;
// }
