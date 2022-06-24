#include "fmjAST.h"

#include <stdlib.h>

#include "util.h"

A_pos A_Pos(int line, int pos) {
    A_pos p = checked_malloc(sizeof *p);
    p->line = line;
    p->pos = pos;
    return (p);
}

A_type A_Type(A_pos pos, A_dataType dt, string id, int arity) {
    A_type t = checked_malloc(sizeof *t);
    t->pos = pos;
    t->t = dt;
    t->id = id;
    t->arity = arity;
    return (t);
}

A_prog A_Prog(A_pos pos, A_mainClass m, A_classDeclList cdl) {
    A_prog p = checked_malloc(sizeof *p);
    p->pos = pos;
    p->m = m;
    p->cdl = cdl;
    return (p);
}

A_mainClass A_MainClass(A_pos pos, string id, A_varDeclList dl, A_stmList sl) {
    A_mainClass p = checked_malloc(sizeof *p);
    p->pos = pos;
    p->id = id;
    p->dl = dl;
    p->sl = sl;
    return (p);
}

A_methodDecl A_MethodDecl(A_pos pos, A_type t, string id, A_formalList fl,
                          A_varDeclList vdl, A_stmList sl) {
    A_methodDecl p = checked_malloc(sizeof *p);
    p->pos = pos;
    p->t = t;
    p->id = id;
    p->fl = fl;
    p->vdl = vdl;
    p->sl = sl;
    return (p);
};

A_methodDeclList A_MethodDeclList(A_methodDecl head, A_methodDeclList tail) {
    A_methodDeclList p = checked_malloc(sizeof *p);
    p->head = head;
    p->tail = tail;
    return (p);
}

A_classDecl A_ClassDecl(A_pos pos, string id, string parentID,
                        A_varDeclList vdl, A_methodDeclList mdl) {
    A_classDecl p = checked_malloc(sizeof *p);
    p->pos = pos;
    p->id = id;
    p->parentID = parentID;
    p->vdl = vdl;
    p->mdl = mdl;
    return (p);
};

A_classDeclList A_ClassDeclList(A_classDecl head, A_classDeclList tail) {
    A_classDeclList p = checked_malloc(sizeof *p);
    p->head = head;
    p->tail = tail;
    return (p);
};

A_formal A_Formal(A_pos pos, A_type t, string id) {
    A_formal p = checked_malloc(sizeof *p);
    p->pos = pos;
    p->t = t;
    p->id = id;
    return (p);
};

A_formalList A_FormalList(A_formal head, A_formalList tail) {
    A_formalList p = checked_malloc(sizeof *p);
    p->head = head;
    p->tail = tail;
    return (p);
}

A_stmList A_StmList(A_stm head, A_stmList tail) {
    A_stmList p = checked_malloc(sizeof *p);
    p->head = head;
    p->tail = tail;
    return (p);
};

A_varDecl A_VarDecl(A_pos pos, A_type t, string v) {
    A_varDecl d = checked_malloc(sizeof *d);
    d->pos = pos;
    d->t = t;
    d->v = v;
    return (d);
}

A_varDeclList A_VarDeclList(A_varDecl head, A_varDeclList tail) {
    A_varDeclList dl = checked_malloc(sizeof *dl);
    dl->head = head;
    dl->tail = tail;
    return (dl);
}

A_stm A_NestedStm(A_stmList sl) {
    A_stm s = checked_malloc(sizeof *s);
    s->pos = sl->head->pos;
    s->kind = A_nestedStm;
    s->u.ns = sl;
    return s;
}

A_stm A_IfStm(A_pos pos, A_exp e, A_stm s1, A_stm s2) {
    A_stm s = checked_malloc(sizeof *s);
    s->pos = pos;
    s->kind = A_ifStm;
    s->u.if_stat.e = e;
    s->u.if_stat.s1 = s1;
    s->u.if_stat.s2 = s2;
    return s;
}

A_stm A_WhileStm(A_pos pos, A_exp e, A_stm s) {
    A_stm s0 = checked_malloc(sizeof *s0);
    s0->pos = pos;
    s0->kind = A_whileStm;
    s0->u.while_stat.e = e;
    s0->u.while_stat.s = s;
    return s0;
}

A_stm A_AssignStm(A_pos pos, string id, A_expList l, A_exp r) {
    // for array assign l<>NULL, simple assignme l==NULL
    A_stm s = checked_malloc(sizeof *s);
    s->pos = pos;
    s->kind = A_assignStm;
    s->u.assign.elist = l;
    s->u.assign.v = id;
    s->u.assign.e = r;
    return s;
}

A_stm A_ContinueStm(A_pos pos) {
    A_stm s = checked_malloc(sizeof *s);
    s->pos = pos;
    s->kind = A_continue;
    return s;
};

A_stm A_ReturnStm(A_pos pos, A_exp e) {
    A_stm s = checked_malloc(sizeof *s);
    s->pos = pos;
    s->kind = A_return;
    s->u.e = e;
    return s;
}
A_stm A_BreakStm(A_pos pos) {
    A_stm s = checked_malloc(sizeof *s);
    s->pos = pos;
    s->kind = A_break;
    return s;
}

A_stm A_PutintStm(A_pos pos, A_exp e) {
    A_stm s = checked_malloc(sizeof *s);
    s->pos = pos;
    s->kind = A_putint;
    s->u.e = e;
    return s;
}

A_stm A_PutchStm(A_pos pos, A_exp e) {
    A_stm s = checked_malloc(sizeof *s);
    s->pos = pos;
    s->kind = A_putch;
    s->u.e = e;
    return s;
}

A_stm A_PutarrayStm(A_pos pos, A_exp e1, A_exp e2) {
    A_stm s = checked_malloc(sizeof *s);
    s->pos = pos;
    s->kind = A_putarray;
    s->u.putarray.e1 = e1;
    s->u.putarray.e2 = e2;
    return s;
}

A_stm A_StarttimeStm(A_pos pos) {
    A_stm s = checked_malloc(sizeof *s);
    s->pos = pos;
    s->kind = A_starttime;
    return s;
}

A_stm A_StoptimeStm(A_pos pos) {
    A_stm s = checked_malloc(sizeof *s);
    s->pos = pos;
    s->kind = A_stoptime;
    return s;
}

A_exp A_OpExp(A_exp left, A_binop oper, A_exp right) {
    A_exp e = checked_malloc(sizeof *e);
    e->pos = left->pos;
    e->kind = A_opExp;
    e->u.op.left = left;
    e->u.op.oper = oper;
    e->u.op.right = right;
    return e;
}

A_exp A_ArrayExp(A_exp a, A_expList elist) {
    A_exp e = checked_malloc(sizeof *e);
    e->pos = a->pos;
    e->kind = A_arrayExp;
    e->u.array.e = a;
    e->u.array.elist = elist;
    return (e);
}

A_exp A_LengthExp(A_pos pos, A_exp a) {
    A_exp e = checked_malloc(sizeof *e);
    e->pos = pos;
    e->kind = A_lengthExp;
    e->u.e = a;
    return (e);
}

A_exp A_CallExp(A_pos pos, A_exp c, string id, A_expList l) {
    A_exp e = checked_malloc(sizeof *e);
    e->pos = pos;
    e->kind = A_callExp;
    e->u.call.e = c;      // the class
    e->u.call.id = id;    // the method name
    e->u.call.elist = l;  // the parameters
    return (e);
}

A_exp A_IdExp(A_pos pos, string s) {
    A_exp e = checked_malloc(sizeof *e);
    e->pos = pos;
    e->kind = A_idExp;
    e->u.v = s;
    return e;
}

A_exp A_NumExp(A_pos pos, int num) {
    A_exp e = checked_malloc(sizeof *e);
    e->pos = pos;
    e->kind = A_numExp;
    e->u.num = num;
    return e;
}

A_exp A_BoolExp(A_pos pos, bool b) {
    A_exp e = checked_malloc(sizeof *e);
    e->pos = pos;
    e->kind = A_boolExp;
    e->u.b = b;
    return e;
}

A_exp A_ThisExp(A_pos pos) {
    A_exp e = checked_malloc(sizeof *e);
    e->pos = pos;
    e->kind = A_thisExp;
    return (e);
}

A_exp A_NewIntArrExp(A_pos pos, A_expList l) {
    A_exp e = checked_malloc(sizeof *e);
    e->pos = pos;
    e->kind = A_newIntArrExp;
    e->u.newIntArr.elist = l;
    return (e);
}

A_exp A_NewObjExp(A_pos pos, string id) {
    A_exp e = checked_malloc(sizeof *e);
    e->pos = pos;
    e->kind = A_newObjExp;
    e->u.v = id;
    return (e);
}

A_exp A_NotExp(A_exp e) {
    A_exp e0 = checked_malloc(sizeof *e0);
    e0->pos = e->pos;
    e0->kind = A_notExp;
    e0->u.e = e;
    return e0;
}

A_exp A_GetintExp(A_pos pos) {
    A_exp e = checked_malloc(sizeof *e);
    e->pos = pos;
    e->kind = A_getint;
    return e;
}

A_exp A_GetchExp(A_pos pos) {
    A_exp e = checked_malloc(sizeof *e);
    e->pos = pos;
    e->kind = A_getch;
    return e;
}

A_exp A_GetarrayExp(A_pos pos, A_exp e) {
    A_exp e0 = checked_malloc(sizeof *e);
    e0->pos = pos;
    e0->kind = A_getarray;
    e0->u.e = e;
    return e0;
}

A_expList A_ExpList(A_exp head, A_expList tail) {
    A_expList p = checked_malloc(sizeof *p);
    p->head = head;
    p->tail = tail;
    return (p);
}
