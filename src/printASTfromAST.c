#include <stdio.h>
#include <stdlib.h>

#include "fmjAST.h"
#include "util.h"

void printASTfromASTVarDecl(A_varDecl);
void printASTfromASTVarDeclList(A_varDeclList);
void printASTfromASTType(A_type);
void printASTfromASTStmList(A_stmList);
void printASTfromASTExpList(A_expList);

void printASTfromASTFormal(A_formal f) {
    printf("A_Formal(A_Pos(%d, %d), ", f->pos->line, f->pos->pos);
    printASTfromASTType(f->t);
    printf(", String(\"%s\"))", f->id);
    return;
}

void printASTfromASTFormalList(A_formalList fl) {
    if (fl != NULL) {
        printf("A_FormalList(\n");
        printASTfromASTFormal(fl->head);
        if (fl->tail != NULL) {
            printf(",\n");
            printASTfromASTFormalList(fl->tail);
            printf(")");
        } else
            printf(", NULL)");
    }
    return;
}

void printASTfromASTMethodDecl(A_methodDecl md) {
    printf("A_MethodDecl(A_Pos(%d, %d), ", md->pos->line, md->pos->pos);
    printASTfromASTType(md->t);
    printf(", String(\"%s\")", md->id);
    if (md->fl != NULL) {
        printf(",\n");
        printASTfromASTFormalList(md->fl);
    } else
        printf(", NULL");
    if (md->vdl != NULL) {
        printf(",\n");
        printASTfromASTVarDeclList(md->vdl);
    } else
        printf(", NULL");
    if (md->sl != NULL) {
        printf(",\n");
        printASTfromASTStmList(md->sl);
    } else
        printf(", NULL");
    printf(")");
    return;
}

void printASTfromASTMethodDeclList(A_methodDeclList mdl) {
    if (mdl != NULL) {
        printf("A_MethodDeclList(\n");
        printASTfromASTMethodDecl(mdl->head);
        if (mdl->tail != NULL) {
            printf(",\n");
            printASTfromASTMethodDeclList(mdl->tail);
            printf(")");
        } else
            printf(", NULL)");
    }
    return;
}

void printASTfromASTClassDecl(A_classDecl cd) {
    printf("A_ClassDecl(A_Pos(%d, %d), String(\"%s\")", cd->pos->line,
           cd->pos->pos, cd->id);
    if (cd->parentID != NULL)
        printf(", String(\"%s\"), ", cd->parentID);
    else
        printf(", NULL, ");
    if (cd->vdl != NULL) {
        printf("\n");
        printASTfromASTVarDeclList(cd->vdl);
    } else
        printf("NULL");
    if (cd->mdl != NULL) {
        printf(",\n");
        printASTfromASTMethodDeclList(cd->mdl);
    } else
        printf(", NULL");
    printf(")");
    return;
}

void printASTfromASTClassDeclList(A_classDeclList cdl) {
    if (cdl != NULL) {
        printf("A_ClassDeclList(\n");
        printASTfromASTClassDecl(cdl->head);
        if (cdl->tail != NULL) {
            printf(",\n");
            printASTfromASTClassDeclList(cdl->tail);
            printf(")");
        } else
            printf(", NULL)");
    }
    return;
}

void printASTfromASTExp(A_exp e) {
    switch (e->kind) {
        case A_opExp:
            printf("A_OpExp(");
            printASTfromASTExp(e->u.op.left);
            switch (e->u.op.oper) {
                case A_and:
                    printf(", A_and, ");
                    break;
                case A_or:
                    printf(", A_or, ");
                    break;
                case A_less:
                    printf(", A_less, ");
                    break;
                case A_le:
                    printf(", A_le, ");
                    break;
                case A_eq:
                    printf(", A_eq, ");
                    break;
                case A_plus:
                    printf(", A_plus, ");
                    break;
                case A_minus:
                    printf(", A_minus, ");
                    break;
                case A_times:
                    printf(", A_times, ");
                    break;
                case A_div:
                    printf(", A_div, ");
                    break;
            }
            printASTfromASTExp(e->u.op.right);
            printf(")");
            break;
        case A_arrayExp:
            printf("A_ArrayExp(A_Pos(%d, %d), ", e->pos->line, e->pos->pos);
            printASTfromASTExp(e->u.array.e);
            printf(", ");
            if (e->u.array.elist != NULL) {
                printf("\n");
                printASTfromASTExpList(e->u.array.elist);
            } else
                printf("NULL");
            printf(")");
        case A_lengthExp:
            printf("A_LengthExp(A_Pos(%d, %d), ", e->pos->line, e->pos->pos);
            printASTfromASTExp(e->u.e);
            printf(")");
        case A_callExp:
            printf("A_CallExp(A_Pos(%d, %d), ", e->pos->line, e->pos->pos);
            printASTfromASTExp(e->u.call.e);
            printf(", String(\"%s\"), ", e->u.call.id);
            if (e->u.call.elist != NULL) {
                printf("\n");
                printASTfromASTExpList(e->u.call.elist);
            } else
                printf("NULL");
            printf(")");
            break;
        case A_numExp:
            printf("A_NumExp(A_Pos(%d, %d), %d)", e->pos->line, e->pos->pos,
                   e->u.num);
            break;
        case A_boolExp:
            printf("A_BoolExp(A_Pos(%d, %d), ", e->pos->line, e->pos->pos);
            if (e->u.b)
                printf("TRUE)");
            else
                printf("FALSE)");
            break;
        case A_idExp:
            printf("A_IdExp(A_Pos(%d, %d), String(\"%s\"))", e->pos->line,
                   e->pos->pos, e->u.v);
            break;
        case A_thisExp:
            printf("A_ThisExp(A_Pos(%d, %d), %d)", e->pos->line, e->pos->pos,
                   e->u.num);
            break;
        case A_newIntArrExp:
            printf("A_NewIntArrExp(A_Pos(%d, %d), ", e->pos->line, e->pos->pos);
            if (e->u.newIntArr.elist != NULL) {
                printf("\n");
                printASTfromASTExpList(e->u.newIntArr.elist);
            } else
                printf("NULL");
            printf(")");
            break;
        case A_newObjExp:
            printf("A_NewObjExp(A_Pos(%d, %d), ", e->pos->line, e->pos->pos);
            printf("String(\"%s\"))", e->u.v);
            break;
        case A_notExp:
            printf("A_NotExp(A_Pos(%d, %d), ", e->pos->line, e->pos->pos);
            printASTfromASTExp(e->u.e);
            printf(")");
            break;
        case A_getint:
            printf("A_GetintExp(A_Pos(%d, %d))", e->pos->line, e->pos->pos);
            break;
        case A_getch:
            printf("A_Getchxp(A_Pos(%d, %d), %d)", e->pos->line, e->pos->pos,
                   e->u.num);
            break;
        case A_getarray:
            printf("A_GetarrayExp(A_Pos(%d, %d), ", e->pos->line, e->pos->pos);
            printASTfromASTExp(e->u.e);
            printf(")");
            break;
    }
    return;
}

void printASTfromASTExpList(A_expList el) {
    if (el != NULL) {
        printf("A_ExpList(\n");
        printASTfromASTExp(el->head);
        if (el->tail != NULL) {
            printf(",\n");
            printASTfromASTExpList(el->tail);
            printf(")");
        } else
            printf(",\nNULL)");
    }
    return;
}

void printASTfromASTStm(A_stm s) {
    switch (s->kind) {
        case A_nestedStm:
        case A_ifStm:
            printf("A_IfStm(A_Pos(%d, %d), ", s->pos->line, s->pos->pos);
            printASTfromASTExp(s->u.if_stat.e);
            printf(",\n");
            printASTfromASTStm(s->u.if_stat.s1);
            printf(",\n");
            printASTfromASTStm(s->u.if_stat.s2);
            printf(")");
            break;
        case A_whileStm:
            printf("A_WhileStm(A_Pos(%d, %d), ", s->pos->line, s->pos->pos);
            printASTfromASTExp(s->u.while_stat.e);
            printf(",\n");
            printASTfromASTStm(s->u.while_stat.s);
            printf(")");
            break;
        case A_assignStm:
            printf("A_AssignStm(A_Pos(%d, %d), String(\"%s\"), ", s->pos->line,
                   s->pos->pos, s->u.assign.v);
            if (s->u.assign.elist != NULL) {
                printf("\n");
                printASTfromASTExpList(s->u.assign.elist);
                printf(", ");
            } else
                printf("NULL, ");
            printASTfromASTExp(s->u.assign.e);
            printf(")");
            break;
        case A_continue:
            printf("A_ContinueStm(A_Pos(%d, %d))", s->pos->line, s->pos->pos);
            break;
        case A_break:
            printf("A_BreakStm(A_Pos(%d, %d))", s->pos->line, s->pos->pos);
            break;
        case A_return:
            printf("A_ReturnStm(A_Pos(%d, %d), ", s->pos->line, s->pos->pos);
            printASTfromASTExp(s->u.e);
            printf(")");
            break;
        case A_putint:
            printf("A_PutintStm(A_Pos(%d, %d), ", s->pos->line, s->pos->pos);
            printASTfromASTExp(s->u.e);
            printf(")");
            break;
        case A_putarray:
            printf("A_PutarrayStm(A_Pos(%d, %d), ", s->pos->line, s->pos->pos);
            printASTfromASTExp(s->u.putarray.e1);
            printf(", ");
            printASTfromASTExp(s->u.putarray.e2);
            printf(")");
            break;
        case A_putch:
            printf("A_PutchaStm(A_Pos(%d, %d), ", s->pos->line, s->pos->pos);
            printASTfromASTExp(s->u.e);
            printf(")");
            break;
        case A_starttime:
            printf("A_StarttimeStm(A_Pos(%d, %d))", s->pos->line, s->pos->pos);
            break;
        case A_stoptime:
            printf("A_StoptimeStm(A_Pos(%d, %d))", s->pos->line, s->pos->pos);
            break;
    }
    return;
}

void printASTfromASTStmList(A_stmList sl) {
    if (sl != NULL) {
        printf("A_StmList(\n");
        printASTfromASTStm(sl->head);
        if (sl->tail != NULL) {
            printf(",\n");
            printASTfromASTStmList(sl->tail);
            printf(")");
        } else
            printf(",\nNULL)");
    }
    return;
}

void printASTfromASTType(A_type t) {
    printf("A_Type(A_Pos(%d, %d), ", t->pos->line, t->pos->pos);
    switch (t->t) {
        case A_intType:
            printf("A_intType, NULL, 0)");
            break;
        case A_boolType:
            printf("A_boolType, NULL, 0)");
            break;
        case A_idType:
            printf("A_idType, String(\"%s\"), 0)", t->id);
            break;
        case A_intArrType:
            printf("A_intArrType, NULL, %d)", t->arity);
            break;
    }
    return;
}

void printASTfromASTVarDecl(A_varDecl vd) {
    printf("A_VarDecl(A_Pos(%d, %d), ", vd->pos->line, vd->pos->pos);
    printASTfromASTType(vd->t);
    printf(", String(\"%s\"))", vd->v);
    return;
}

void printASTfromASTVarDeclList(A_varDeclList dl) {
    if (dl != NULL) {
        printf("A_VarDeclList(\n");
        printASTfromASTVarDecl(dl->head);
        if (dl->tail != NULL) {
            printf(",\n");
            printASTfromASTVarDeclList(dl->tail);
            printf(")");
        } else
            printf(",\nNULL)");
    }
    return;
}

void printASTfromASTMainClass(A_mainClass m) {
    printf("A_MainClass(A_Pos(%d, %d), String(\"%s\"), ", m->pos->line,
           m->pos->pos, m->id);
    if (m->dl != NULL) {
        printf("\n");
        printASTfromASTVarDeclList(m->dl);
        printf(", ");
    } else
        printf("NULL, ");
    if (m->sl != NULL) {
        printf("\n");
        printASTfromASTStmList(m->sl);
    } else
        printf("NULL");
    printf(")");
}

void printFMJASTfromAST(A_prog p) {
    printf("A_Prog(A_Pos(%d, %d), ", p->pos->line, p->pos->pos);
    printASTfromASTMainClass(p->m);
    if (p->cdl != NULL) {
        printf(",\n");
        printASTfromASTClassDeclList(p->cdl);
    } else
        printf(", NULL");
    printf(")\n");
}
