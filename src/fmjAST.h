#ifndef __FMJAST
#define __FMJAST
#include "util.h"
// This is an AST definition for FMJ1.4
//  April 16, 2022
typedef struct A_pos_* A_pos;  // position information
typedef struct A_type_* A_type;
typedef struct A_prog_* A_prog;
typedef struct A_mainClass_* A_mainClass;
typedef struct A_classDecl_* A_classDecl;
typedef struct A_classDeclList_* A_classDeclList;
typedef struct A_methodDecl_* A_methodDecl;
typedef struct A_methodDeclList_* A_methodDeclList;
typedef struct A_formal_* A_formal;
typedef struct A_formalList_* A_formalList;
typedef struct A_varDecl_* A_varDecl;
typedef struct A_varDeclList_* A_varDeclList;
typedef struct A_stmList_* A_stmList;
typedef struct A_stm_* A_stm;
typedef struct A_exp_* A_exp;
typedef struct A_expList_* A_expList;

struct A_pos_ {
    int line, pos;
};

typedef enum { A_intType, A_boolType, A_idType, A_intArrType } A_dataType;

struct A_type_ {
    A_pos pos;
    A_dataType t;
    string id;  // for class type, otherwise ignore
    int arity;  // for non-array types: arity=0
};

struct A_prog_ {
    A_pos pos;
    A_mainClass m;
    A_classDeclList cdl;
};

struct A_mainClass_ {
    A_pos pos;
    string id;  // class name
    A_varDeclList dl;
    A_stmList sl;
};

struct A_methodDecl_ {
    A_pos pos;
    A_type t;   // return type
    string id;  // method name
    A_formalList fl;
    A_varDeclList vdl;
    A_stmList sl;
};

struct A_methodDeclList_ {
    A_methodDecl head;
    A_methodDeclList tail;
};

struct A_classDecl_ {
    A_pos pos;
    string id;        // class name
    string parentID;  // parent class id if not NULL
    A_varDeclList vdl;
    A_methodDeclList mdl;
};

struct A_classDeclList_ {
    A_classDecl head;
    A_classDeclList tail;
};

struct A_formal_ {
    A_pos pos;
    A_type t;  // This can't be array type (probably shouldn't be like this)
    string id;
};

struct A_formalList_ {
    A_formal head;
    A_formalList tail;
};

struct A_varDecl_ {
    A_pos pos;
    A_type t;
    string v;
};

struct A_varDeclList_ {
    A_varDecl head;
    A_varDeclList tail;
};

struct A_stmList_ {
    A_stm head;
    A_stmList tail;
};

struct A_stm_ {
    A_pos pos;
    enum {
        A_nestedStm,
        A_ifStm,
        A_whileStm,
        A_assignStm,
        A_continue,
        A_break,
        A_return,
        A_putint,
        A_putarray,
        A_putch,
        A_starttime,
        A_stoptime
    } kind;
    union {
        A_stmList ns;
        struct {
            A_exp e;
            A_stm s1, s2;
        } if_stat;
        struct {
            A_exp e;
            A_stm s;
        } while_stat;
        struct {
            string v;
            A_expList elist;  // empty if not array on the left hand side
            A_exp e;
        } assign;
        struct {
            A_exp e1, e2;
        } putarray;
        A_exp e;
    } u;
};

typedef enum {
    A_and,
    A_or,
    A_less,
    A_le,
    A_eq,
    A_plus,
    A_minus,
    A_times,
    A_div
} A_binop;

struct A_exp_ {
    A_pos pos;
    enum {
        A_opExp,
        A_arrayExp,
        A_lengthExp,  // only works for array
        A_callExp,
        A_numExp,
        A_boolExp,
        A_idExp,
        A_thisExp,
        A_newIntArrExp,
        A_newObjExp,
        A_notExp,
        A_getint,
        A_getch,
        A_getarray
    } kind;
    union {
        struct {
            A_exp left;
            A_binop oper;
            A_exp right;
        } op;
        struct {
            A_exp e;
            A_expList elist;
        } array;
        struct {
            A_exp e;
            string id;
            A_expList elist;
        } call;
        struct {
            A_expList elist;
        } newIntArr;
        int num;   // this is for numExp
        bool b;    // This is for boolExp
        string v;  // This is for callExp, idExp, newObjExp
        A_exp e;   // This is for lengthExp, notExp, getarray (note: -Exp is
                   // 0-Exp, (Exp) doesn't need its own node)
    } u;
};

struct A_expList_ {
    A_exp head;
    A_expList tail;
};

A_pos A_Pos(int, int);
A_type A_Type(A_pos, A_dataType, string, int);
A_prog A_Prog(A_pos, A_mainClass, A_classDeclList);
A_mainClass A_MainClass(A_pos, string, A_varDeclList, A_stmList);
A_classDecl A_ClassDecl(A_pos, string, string, A_varDeclList, A_methodDeclList);
A_classDeclList A_ClassDeclList(A_classDecl, A_classDeclList);
A_varDecl A_VarDecl(A_pos, A_type, string);
A_varDeclList A_VarDeclList(A_varDecl, A_varDeclList);
A_methodDecl A_MethodDecl(A_pos, A_type, string, A_formalList, A_varDeclList,
                          A_stmList);
A_methodDeclList A_MethodDeclList(A_methodDecl, A_methodDeclList);
A_formal A_Formal(A_pos, A_type, string);
A_formalList A_FormalList(A_formal, A_formalList);
A_stmList A_StmList(A_stm, A_stmList);
A_stm A_NestedStm(A_stmList);
A_stm A_IfStm(A_pos, A_exp, A_stm, A_stm);
A_stm A_WhileStm(A_pos, A_exp, A_stm);
A_stm A_AssignStm(
    A_pos, string, A_expList,
    A_exp);  // id[elist] on the left.. if elist==null, then simply id
A_stm A_ContinueStm(A_pos);
A_stm A_BreakStm(A_pos);
A_stm A_ReturnStm(A_pos, A_exp);
A_stm A_PutintStm(A_pos, A_exp);
A_stm A_PutchStm(A_pos, A_exp);
A_stm A_PutarrayStm(A_pos, A_exp, A_exp);
A_stm A_StarttimeStm(A_pos);
A_stm A_StoptimeStm(A_pos);

A_exp A_OpExp(A_exp, A_binop, A_exp);
A_exp A_ArrayExp(A_exp, A_expList);
A_exp A_CallExp(A_pos, A_exp, string v, A_expList);
A_exp A_LengthExp(A_pos, A_exp);
A_exp A_NumExp(A_pos, int);
A_exp A_BoolExp(A_pos, bool);
A_exp A_IdExp(A_pos, string);
A_exp A_ThisExp(A_pos);
A_exp A_NewIntArrExp(A_pos, A_expList);
A_exp A_NewObjExp(A_pos, string);
A_exp A_NotExp(A_exp);
A_exp A_GetintExp(A_pos);
A_exp A_GetchExp(A_pos);
A_exp A_GetarrayExp(A_pos, A_exp);

A_expList A_ExpList(A_exp, A_expList);

void printFMJfromAST(A_prog);
// void printFMJASTfromAST(A_prog);

#endif