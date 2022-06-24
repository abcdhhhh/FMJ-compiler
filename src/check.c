#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fmjAST.h"
#include "prog.h"
#include "symbol.h"
#include "types.h"
#include "util.h"

void check_stmList(S_table, S_table, A_stmList, string);
void check_varDeclList(S_table, S_table, A_varDeclList);

void show_pos(A_pos pos) {
    if (pos == NULL)
        printf("NULL");
    else
        printf("(%d, %d)", pos->line, pos->pos);
}

void EM_error(A_pos pos, string s) {
    show_pos(pos);
    printf(" ERROR: %s\n", s);
    assert(0);
}

Ty_ty A2Ty(A_type t) {
    switch (t->t) {
        case A_intType: {
            return Ty_Int();
        } break;
        case A_boolType: {
            return Ty_Bool();
        } break;
        case A_idType: {
            return Ty_Name(S_Symbol(t->id), NULL);
        } break;
        case A_intArrType: {
            return Ty_Array(Ty_Int(), t->arity);
        } break;
    }
    assert(0);
    return Ty_Nil();
}

int convertible(S_table tenv, Ty_ty t_l, Ty_ty t_r) {
    assert(t_l && t_r);
    int f_l = (t_l->kind == Ty_int || t_l->kind == Ty_bool ||
               t_l->kind == Ty_array && t_l->u.array.arity == 0);
    int f_r = (t_r->kind == Ty_int || t_r->kind == Ty_bool ||
               t_r->kind == Ty_array && t_r->u.array.arity == 0);
    if (f_l && f_r)
        return 1;
    else if (f_l || f_r)
        return 0;
    switch (t_l->kind) {
        case Ty_name: {
            // if (t_r->kind != Ty_name) return 0;
            // S_symbol s_l = t_l->u.name.sym, s_r = t_r->u.name.sym;
            // while (s_r) {
            //     // printf(" %s %s\n", s_l, s_r);
            //     if (s_r == s_l) return 1;
            //     Ty_ty t = S_look(tenv, s_r);
            //     assert(t);
            //     assert(t->kind == Ty_record);
            //     s_r = t->u.record.parent;
            // }
            // return 0;
            return 1;
        } break;
        case Ty_array: {
            // printf("t_l arity: %d\n", t_l->u.array.arity);
            return (t_r->kind == Ty_array &&
                    t_l->u.array.ty == t_r->u.array.ty);
        } break;
        case Ty_nil: {
            return 0;
        } break;
    }
    assert(0);
    return 0;
}

Ty_ty transExp(S_table venv, S_table tenv, A_exp e, string className) {
    // show_pos(e->pos);
    // printf("trans exp in class %s\n", className);
    switch (e->kind) {
        case A_opExp: {
            Ty_ty left = transExp(venv, tenv, e->u.op.left, className);
            Ty_ty right = transExp(venv, tenv, e->u.op.right, className);
            if (!convertible(tenv, left, Ty_Int())) {
                EM_error(e->u.op.left->pos,
                         "(opExp left) int or bool required");
            }
            if (!convertible(tenv, right, Ty_Int())) {
                EM_error(e->u.op.right->pos,
                         "(opExp right) int or bool required");
            }
            switch (e->u.op.oper) {
                case A_and:
                case A_or:
                case A_less:
                case A_le:
                case A_eq: {
                    return Ty_Bool();
                } break;
                case A_plus:
                case A_minus:
                case A_times:
                case A_div: {
                    return Ty_Int();
                } break;
            }
            assert(0);
        } break;
        case A_arrayExp: {
            Ty_ty t = transExp(venv, tenv, e->u.array.e, className);
            int arity = 0;
            for (A_expList elist = e->u.array.elist; elist;
                 elist = elist->tail) {
                arity++;
            }
            if (t->kind == Ty_array) {
                return Ty_Array(t->u.array.ty, t->u.array.arity - arity);
            } else {
                EM_error(e->pos, "(exp) not an array");
            }
        } break;
        case A_lengthExp: {
            return Ty_Int();
        } break;
        case A_callExp: {
            // printf("call method %s\n", e->u.call.id);
            Ty_ty t = transExp(venv, tenv, e->u.call.e, className);
            if (t->kind != Ty_name) {
                EM_error(e->pos, "not a class");
                return Ty_Nil();
            }
            // printf("class successful\n");
            Ty_ty tt = S_look(tenv, t->u.name.sym);
            if (tt == NULL) {
                EM_error(e->pos, "method not found");
                return Ty_Nil();
            }
            Ty_methodList methods = tt->u.record.methods;
            // printf("1\n");
            while (methods) {
                // printf("searching method ...\n");
                Ty_method method = methods->head;
                if (method->name == S_Symbol(e->u.call.id)) {
                    break;
                }
                methods = methods->tail;
            }
            if (methods == NULL) {
                EM_error(e->pos, "method doesn't exist");
                return Ty_Nil();
            }
            // printf("method found\n");
            Ty_method method = methods->head;
            int cnt_formals = 0;
            for (Ty_tyList formals = method->formals; formals;
                 formals = formals->tail) {
                cnt_formals++;
            }
            Ty_tyList formals = method->formals;
            A_expList elist = e->u.call.elist;
            while (cnt_formals--) {
                Ty_ty t_e = transExp(venv, tenv, elist->head, className);
                assert(formals != NULL);
                if (elist == NULL) {
                    EM_error(e->pos, "fewer parameters");
                    return Ty_Nil();
                }
                if (!convertible(tenv, t_e, formals->head)) {
                    EM_error(e->pos, "formal not the same type");
                    return Ty_Nil();
                }
                formals = formals->tail;
                elist = elist->tail;
            }
            if (elist != NULL) {
                EM_error(e->pos, "more parameters");
                return Ty_Nil();
            }
            return method->ty;
        } break;
        case A_numExp: {
            return Ty_Int();
        } break;
        case A_boolExp: {
            return Ty_Bool();
        } break;
        case A_idExp: {
            if (S_look(venv, S_Symbol(e->u.v)) == NULL) {
                EM_error(e->pos, "id not found");
            }
            return S_look(venv, S_Symbol(e->u.v));
        } break;
        case A_thisExp: {
            return Ty_Name(S_Symbol(className), NULL);
        } break;
        case A_newIntArrExp: {
            int arity = 0;
            for (A_expList elist = e->u.newIntArr.elist; elist;
                 elist = elist->tail) {
                arity++;
            }
            return Ty_Array(Ty_Int(), arity);
        } break;
        case A_newObjExp: {
            // printf("new object %s\n", e->u.v);
            return Ty_Name(S_Symbol(e->u.v), NULL);
        } break;
        case A_notExp: {
            return Ty_Bool();
        } break;
        case A_getint: {
            return Ty_Int();
        } break;
        case A_getch: {
            return Ty_Int();
        } break;
        case A_getarray: {
            // printf("getarray\n");
            Ty_ty t_e = transExp(venv, tenv, e->u.e, className);
            if (t_e->kind != Ty_array || t_e->u.array.arity <= 0) {
                EM_error(e->pos, "not an array");
                return Ty_Nil();
            }
            return Ty_Int();
        } break;
    }
    assert(0);
    return Ty_Nil();
}

void check_varDecl(S_table venv, S_table tenv, S_table temp, A_varDecl d) {
    //  show_pos(d->pos);
    //  printf("\n");
    if (S_look(temp, S_Symbol(d->v)) != NULL) {
        EM_error(d->pos, "duplicate declaration of variable");
        return;
    }
    switch (d->t->t) {
        case A_intType: {
            S_enter(venv, S_Symbol(d->v), Ty_Int());
            S_enter(temp, S_Symbol(d->v), Ty_Int());
        } break;
        case A_boolType: {
            S_enter(venv, S_Symbol(d->v), Ty_Bool());
            S_enter(temp, S_Symbol(d->v), Ty_Bool());
        } break;
        case A_idType: {
            S_enter(venv, S_Symbol(d->v), Ty_Name(S_Symbol(d->t->id), NULL));
            S_enter(temp, S_Symbol(d->v), Ty_Name(S_Symbol(d->t->id), NULL));
        } break;
        case A_intArrType: {
            S_enter(venv, S_Symbol(d->v), Ty_Array(Ty_Int(), d->t->arity));
            S_enter(temp, S_Symbol(d->v), Ty_Array(Ty_Int(), d->t->arity));
        } break;
        default: {
            assert(0);
        }
    }
}

void check_varDeclList(S_table venv, S_table tenv, A_varDeclList dl) {
    S_table temp = S_empty();
    while (dl) {
        check_varDecl(venv, tenv, temp, dl->head);
        dl = dl->tail;
    }
}

void check_formalList(S_table venv, S_table tenv, A_formalList fl) {
    S_table temp = S_empty();
    while (fl) {
        A_formal f = fl->head;
        check_varDecl(venv, tenv, temp, A_VarDecl(f->pos, f->t, f->id));
        fl = fl->tail;
    }
}

void check_methodDecl(S_table venv, S_table tenv, A_methodDecl md,
                      string className) {
    S_beginScope(venv);
    check_formalList(venv, tenv, md->fl);
    check_varDeclList(venv, tenv, md->vdl);
    check_stmList(venv, tenv, md->sl, className);
    S_endScope(venv);
}
void check_methodDeclList(S_table venv, S_table tenv, A_methodDeclList mdl,
                          string className) {
    // printf("methods:\n");
    while (mdl) {
        check_methodDecl(venv, tenv, mdl->head, className);
        mdl = mdl->tail;
    }
}

void check_stm(S_table venv, S_table tenv, A_stm s, string className) {
    // show_pos(s->pos);
    // printf("\n");
    //   printf("checking stm in class %s\n", className);
    switch (s->kind) {
        case A_nestedStm: {
            check_stmList(venv, tenv, s->u.ns, className);
        } break;
        case A_ifStm: {
            Ty_ty t = transExp(venv, tenv, s->u.if_stat.e, className);
            if (!convertible(tenv, t, Ty_Int())) {
                EM_error(s->pos, "(if_stat) integer or bool required");
            }
            check_stm(venv, tenv, s->u.if_stat.s1, className);
            check_stm(venv, tenv, s->u.if_stat.s2, className);
        } break;
        case A_whileStm: {
            Ty_ty t = transExp(venv, tenv, s->u.while_stat.e, className);
            if (!convertible(tenv, t, Ty_Int())) {
                EM_error(s->pos, "(while_stat) integer or bool required");
            }
            check_stm(venv, tenv, s->u.while_stat.s, className);
        } break;
        case A_assignStm: {
            int arity = 0;
            for (A_expList elist = s->u.assign.elist; elist;
                 elist = elist->tail) {
                arity++;
            }
            // printf("arity=%d\n", arity);
            Ty_ty t_l =
                transExp(venv, tenv, A_IdExp(s->pos, s->u.assign.v), className);
            Ty_ty t_r = transExp(venv, tenv, s->u.assign.e, className);
            if (arity) {
                if (t_l->kind == Ty_array) {
                    t_l = Ty_Array(t_l->u.array.ty, t_l->u.array.arity - arity);
                } else {
                    EM_error(s->pos, "not an array");
                    return;
                }
            }
            if (!convertible(tenv, t_l, t_r)) {
                EM_error(s->pos, "not convertible");
                return;
            }
        } break;
        case A_continue:
        case A_break: {
        } break;
        case A_return: {
            //   printf("checking return\n");
        } break;
        case A_putint:
        case A_putch: {
            Ty_ty t = transExp(venv, tenv, s->u.e, className);
            if (!convertible(tenv, t, Ty_Int())) {
                EM_error(s->pos, "(putint/putch) integer or bool required");
            }
        } break;
        case A_putarray: {
            Ty_ty t1 = transExp(venv, tenv, s->u.putarray.e1, className),
                  t2 = transExp(venv, tenv, s->u.putarray.e2, className);
            if (!convertible(tenv, t1, Ty_Int())) {
                EM_error(s->pos, "(putarray) integer required");
            }
            if (t2->kind != Ty_array || t2->u.array.arity <= 0) {
                EM_error(s->pos, "(putarray) array required");
            }
        } break;
        case A_starttime: {
        } break;
        case A_stoptime: {
        } break;
    }
}

void check_stmList(S_table venv, S_table tenv, A_stmList sl, string className) {
    while (sl) {
        assert(sl->head);
        check_stm(venv, tenv, sl->head, className);
        sl = sl->tail;
    }
}

void check_classDecl(S_table venv, S_table tenv, A_classDecl cd) {
    //  printf("checking cd %s\n", cd->id);
    S_beginScope(venv);
    check_varDeclList(venv, tenv, cd->vdl);
    //  printf("vd checked\n");
    check_methodDeclList(venv, tenv, cd->mdl, cd->id);
    //  printf("methods checked\n");
    S_endScope(venv);
}

void check_classDeclList(S_table venv, S_table tenv, A_classDeclList cdl) {
    while (cdl) {
        check_classDecl(venv, tenv, cdl->head);
        cdl = cdl->tail;
    }
}

void check_mainClass(S_table venv, S_table tenv, A_mainClass m) {
    S_beginScope(venv);
    check_varDeclList(venv, tenv, m->dl);
    // printf("vd checked\n");
    check_stmList(venv, tenv, m->sl, m->id);
    // printf("stms checked\n");
    S_endScope(venv);
}

void scan_classDecl(S_table venv, S_table tenv, A_classDecl cd) {
    printf("scanning classDecl %s ...\n", cd->id);

    if (S_look(tenv, S_Symbol(cd->id)) != NULL) {
        EM_error(cd->pos, "multiple definition of class");
        return;
    }

    Ty_fieldList fields = NULL;
    Ty_methodList methods = NULL;
    if (cd->parentID) {
        Ty_ty t = S_look(tenv, S_Symbol(cd->parentID));
        if (t == NULL) {
            EM_error(cd->pos, "parent not found");
            return;
        }
        assert(t->kind == Ty_record);
        fields = t->u.record.fields;
        methods = t->u.record.methods;
    }

    for (A_varDeclList vdl = cd->vdl; vdl; vdl = vdl->tail) {
        A_varDecl vd = vdl->head;
        printf("scanning vd %s ...\n", vd->v);

        Ty_field field = Ty_Field(S_Symbol(vd->v), A2Ty(vd->t));
        fields = Ty_FieldList(field, fields);
    }
    // printf("fields get\n");

    for (A_methodDeclList mdl = cd->mdl; mdl; mdl = mdl->tail) {
        A_methodDecl md = mdl->head;
        printf("scanning md %s ...\n", md->id);

        S_table temp = S_empty();
        Ty_tyList formals = NULL;
        for (A_formalList fl = md->fl; fl; fl = fl->tail) {
            A_formal f = fl->head;
            //  printf("scanning formal %s ...\n", f->id);

            if (S_look(temp, S_Symbol(f->id)) != NULL) {
                EM_error(md->pos, "duplicate variable in method's formals");
                continue;
            }
            S_enter(temp, S_Symbol(f->id), A2Ty(f->t));

            formals = Ty_TyList(A2Ty(f->t), formals);
        }
        formals = reverseTyList(formals);
        // printf("formals get\n");
        Ty_method method = Ty_Method(S_Symbol(md->id), A2Ty(md->t), formals);
        methods = Ty_MethodList(method, methods);
    }
    // printf("methods get\n");

    Ty_ty record;
    if (cd->parentID != NULL) {
        record = Ty_Record(S_Symbol(cd->parentID), fields, methods);
    } else {
        record = Ty_Record(NULL, fields, methods);
    }
    // printf("record get\n");
    S_enter(tenv, S_Symbol(cd->id), record);
}

void scan_classDeclList(S_table venv, S_table tenv, A_classDeclList cdl) {
    while (cdl) {
        scan_classDecl(venv, tenv, cdl->head);
        cdl = cdl->tail;
    }
}

void check_prog(A_prog p) {
    S_table venv = S_empty(), tenv = S_empty();
    scan_classDeclList(venv, tenv, p->cdl);
    // printf("cdl scanned\n");

    check_mainClass(venv, tenv, p->m);
    // printf("mainClass checked\n");

    check_classDeclList(venv, tenv, p->cdl);
    // printf("cdl checked\n");
}
