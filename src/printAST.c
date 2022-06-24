#include <stdio.h>
#include <stdlib.h>

#include "fmjAST.h"
#include "util.h"

void printExpList(A_expList);

void printExp(A_exp e) {
    switch (e->kind) {
        case A_opExp:
            printf("(");
            printExp(e->u.op.left);
            switch (e->u.op.oper) {
                case A_and:
                    printf("&&");
                    break;
                case A_or:
                    printf("||");
                    break;
                case A_less:
                    printf("<");
                    break;
                case A_le:
                    printf("<=");
                    break;
                case A_eq:
                    printf("==");
                    break;
                case A_plus:
                    printf("+");
                    break;
                case A_minus:
                    printf("-");
                    break;
                case A_times:
                    printf("*");
                    break;
                case A_div:
                    printf("/");
                    break;
            }
            printExp(e->u.op.right);
            printf(")");
            break;
        case A_arrayExp:
            printExp(e->u.array.e);
            printf("[");
            printExpList(e->u.array.elist);
            printf("]");
            break;
        case A_lengthExp:
            printExp(e->u.e);
            printf(".length");
            break;
        case A_callExp:
            printExp(e->u.call.e);
            printf(".%s(", e->u.call.id);
            printExpList(e->u.call.elist);
            printf(")");
            break;
        case A_numExp:
            printf("%d", e->u.num);
            break;
        case A_boolExp:
            if (e->u.b)
                printf("true");
            else
                printf("false");
            break;
        case A_idExp:
            printf("%s", e->u.v);
            break;
        case A_thisExp:
            printf("this");
            break;
        case A_newIntArrExp:
            printf("new int[");
            printExpList(e->u.newIntArr.elist);
            printf("]");
            break;
        case A_newObjExp:
            printf("new %s()", e->u.v);
            break;
        case A_notExp:
            printf("!");
            printExp(e->u.e);
            break;
        case A_getint:
            printf("getint()");
            break;
        case A_getch:
            printf("getch()");
            break;
        case A_getarray:
            printf("getarray(");
            printExp(e->u.e);
            printf(")");
            break;
    }
    return;
}

void printExpList(A_expList el) {
    if (el != NULL) {
        printExp(el->head);
        if (el->tail != NULL) printf(", ");
        printExpList(el->tail);
    }
    return;
}

void printStmtList(A_stmList);

void printStmt(A_stm s) {
    switch (s->kind) {
        case A_nestedStm:
            printf("{\n");
            printStmtList(s->u.ns);
            printf("}\n");
            break;
        case A_ifStm:
            printf("if (");
            printExp(s->u.if_stat.e);
            printf(") \n");
            printStmt(s->u.if_stat.s1);
            printf(" else \n");
            printStmt(s->u.if_stat.s2);
            break;
        case A_whileStm:
            printf("while (");
            printExp(s->u.while_stat.e);
            printf(")\n");
            printStmt(s->u.while_stat.s);
            break;
        case A_assignStm:
            printf("%s", s->u.assign.v);
            if (s->u.assign.elist != NULL) {
                printf("[");
                printExpList(s->u.assign.elist);
                printf("]");
            }
            printf("=");
            printExp(s->u.assign.e);
            printf(";\n");
            break;
        case A_continue:
            printf("continue;\n");
            break;
        case A_break:
            printf("break;\n");
            break;
        case A_return:
            printf("return(");
            printExp(s->u.e);
            printf(");\n");
            break;
        case A_putint:
            printf("putint(");
            printExp(s->u.e);
            printf(");\n");
            break;
        case A_putarray:
            printf("putarray(");
            printExp(s->u.putarray.e1);
            printf(", ");
            printExp(s->u.putarray.e2);
            printf(");\n");
            break;
        case A_putch:
            printf("putch(");
            printExp(s->u.e);
            printf(");\n");
            break;
        case A_starttime:
            printf("starttime();\n");
            break;
        case A_stoptime:
            printf("stoptime();\n");
            break;
    }
    return;
}

void printStmtList(A_stmList sl) {
    if (sl != NULL) {
        printStmt(sl->head);
        printStmtList(sl->tail);
    }
    return;
}

void printType(A_type t) {
    switch (t->t) {
        case A_intType:
            printf("int");
            break;
        case A_intArrType:
            printf("int");
            for (int i = 0; i < t->arity; i++) printf("[]");
            break;
        case A_boolType:
            printf("bool");
            break;
        case A_idType:
            printf("%s", t->id);
            break;
    }
    return;
}

void printVarDec(A_varDecl v) {
    printType(v->t);
    printf(" %s;\n", v->v);
    return;
}

void printVarDeclList(A_varDeclList l) {
    if (l != NULL) {
        printVarDec(l->head);
        printVarDeclList(l->tail);
    }
    return;
}

void printfFormal(A_formal f) {
    printType(f->t);
    printf(" %s", f->id);
    return;
}

void printfFormalList(A_formalList fl) {
    if (fl != NULL) {
        printfFormal(fl->head);
        if (fl->tail != NULL) printf(", ");
        printfFormalList(fl->tail);
    }
    return;
}

void printMethodDecl(A_methodDecl m) {
    printf("public ");
    printType(m->t);
    printf(" %s(", m->id);
    printfFormalList(m->fl);
    printf(")\n{");
    printVarDeclList(m->vdl);
    printStmtList(m->sl);
    printf("} //end of method %s declaration\n", m->id);
    return;
}

void printMethodDeclList(A_methodDeclList ml) {
    if (ml != NULL) {
        printMethodDecl(ml->head);
        printMethodDeclList(ml->tail);
    }
    return;
}

void printClassDecl(A_classDecl d) {
    printf("class %s ", d->id);
    if (d->parentID != NULL) printf("extends %s", d->parentID);
    printf("{\n");
    printVarDeclList(d->vdl);
    printMethodDeclList(d->mdl);
    printf("} //end of class %s declaration\n", d->id);
    return;
}

void printClassDeclList(A_classDeclList dl) {
    if (dl != NULL) {
        printClassDecl(dl->head);
        printClassDeclList(dl->tail);
    }
    return;
}

void printMainClass(A_mainClass m) {
    printf("class %s {public static int main (String[] id) {\n", m->id);
    printVarDeclList(m->dl);
    printStmtList(m->sl);
    printf("}} //end of main class\n");
    return;
}

void printFMJfromAST(A_prog p) {
    printMainClass(p->m);
    printClassDeclList(p->cdl);
}
