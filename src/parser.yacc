%{
#include<stdio.h>
#include<assert.h>
#include "fmjAST.h"
#include "util.h"
#define curPos A_Pos(yylloc.first_line, yylloc.first_column)
int base;

extern int yylex();
extern void yyerror(char*);
extern int yywrap();
A_prog root;
int yylineno;

A_varDeclList reverseVarDeclList(A_varDeclList dl) {
    A_varDeclList ret = NULL;
    while (dl) {
        A_varDeclList tail = dl->tail;
        dl->tail = ret;
        ret = dl;
        dl = tail;
    }
    return ret;
}

%}
%locations

%union {
    // A_dsList dsl; A_ds ds;
    A_prog p; 
    A_mainClass m;
    A_classDeclList cdl; A_classDecl cd;
    A_varDeclList dl; A_varDecl d;
    A_methodDeclList mdl; A_methodDecl md;
    A_formal f; A_formalList fl;
    A_type t; 
    A_stmList sl; A_stm s; 
    A_expList el; A_exp e; 
    int i; 
    A_binop binop; 
    A_pos pos; 
    string str;
}

%token <str> CLASS PUBLIC STATIC INT MAIN STRING EXTENDS BOOL
%token <str> IF ELSE WHILE CONTINUE BREAK RETURN PUTINT PUTCH PUTARRAY STARTTIME STOPTIME
%token <str> LENGTH THIS NEW GETINT GETCH GETARRAY
%token <str> TRUE_ FALSE_
%token <str> AND OR LE EQ  
%token <str> id
%token <i> DIGIT
%token <str> EOF_

/* %type <dsl> DsList
%type <ds> Ds */
%type <p> Program
%type <m> MainClass
%type <cdl> ClassDeclList
%type <cd> ClassDecl
%type <dl> VarDeclList
%type <d> VarDecl
%type <mdl> MethodDeclList 
%type <md> MethodDecl 
%type <fl> FormalList FormalRest
%type <f> Formal
%type <t> Type ArrayType
%type <sl> StatementList
%type <s> Statement
%type <i> INTEGER_LITERAL
%type <e> Exp
%type <el> ExpList ExpRest


%left ';'
%left ','
%right '='
%left OR
%left AND
%left EQ
%left '<' LE
%left '+' '-'
%left '*' '/'
%right '!'
%left '[' ']' '(' ')' '.'

%start Program

%%                   /* beginning of rules section */

/* DsList: {
    $$ = NULL;
} | Ds DsList{
    $$ = A_DsList($1, $2);
}

Ds: VarDecl {
    $$ = A_VarDeclDs($1);
} | Statement {
    $$ = A_StmDs($1);
} */

Program: MainClass ClassDeclList EOF_ {
    root = A_Prog(curPos, $1, $2);
    // printf("ok\n");
} | error EOF_ {
    yyerrok;
};

MainClass: CLASS id '{' PUBLIC STATIC INT MAIN '(' STRING '[' ']' id ')' '{' VarDeclList StatementList '}' '}'{
    // printf("end of mainclass\n");
    $$ = A_MainClass(curPos, $2, reverseVarDeclList($15), $16);
};

ClassDeclList: {
    $$ = NULL;
} | ClassDecl ClassDeclList {
    $$ = A_ClassDeclList($1, $2);
};

ClassDecl: CLASS id '{' VarDeclList MethodDeclList '}' {
    $$ = A_ClassDecl(curPos, $2, NULL, reverseVarDeclList($4), $5);
} | CLASS id EXTENDS id '{' VarDeclList MethodDeclList '}' {
    $$ = A_ClassDecl(curPos, $2, $4, reverseVarDeclList($6), $7);
};

VarDeclList: {
    $$ = NULL;
} | VarDeclList VarDecl {
    $$ = A_VarDeclList($2, $1); // reversed
};

VarDecl: Type id ';' {
    $$ = A_VarDecl(curPos, $1, $2);
} | ArrayType id ';' {
    $$ = A_VarDecl(curPos, $1, $2);
};

MethodDeclList: {
    $$ = NULL;
} | MethodDecl MethodDeclList {
    $$ = A_MethodDeclList($1, $2);
};

MethodDecl: PUBLIC Type id '(' FormalList ')' '{' VarDeclList StatementList '}' {
    $$ = A_MethodDecl(curPos, $2, $3, $5, reverseVarDeclList($8), $9);
} | PUBLIC ArrayType id '(' FormalList ')' '{' VarDeclList StatementList '}' {
    $$ = A_MethodDecl(curPos, $2, $3, $5, reverseVarDeclList($8), $9);
};

FormalList: {
    $$ = NULL;
} | Formal FormalRest {
    $$ = A_FormalList($1, $2);
};

FormalRest: {
    $$ = NULL;
} | ',' Formal FormalRest {
    $$ = A_FormalList($2, $3);
};

Formal: Type id {
    $$ = A_Formal(curPos, $1, $2);
};

Type: INT {
    $$ = A_Type(curPos, A_intType, NULL, 0);
} | BOOL {
    $$ = A_Type(curPos, A_boolType, NULL, 0);
} | id {
    $$ = A_Type(curPos, A_idType, $1, 0);
};

ArrayType: INT '[' ']' {
    $$ = A_Type(curPos, A_intArrType, NULL, 1);
} | ArrayType '[' ']' {
    $$ = A_Type(curPos, A_intArrType, NULL, $1->arity+1);
};

StatementList: {
    $$ = NULL;
} | Statement StatementList {
    $$ = A_StmList($1, $2);
};

Statement: '{' StatementList '}' {
    $$ = A_NestedStm($2);
} | IF '(' Exp ')' Statement ELSE Statement{
    $$ = A_IfStm(curPos, $3, $5, $7);
} | WHILE '(' Exp ')' Statement{
    $$ = A_WhileStm(curPos, $3, $5);
} | id '=' Exp ';' {
    $$ = A_AssignStm(curPos, $1, NULL, $3);
} | id '[' ExpList ']' '=' Exp ';' {
    $$ = A_AssignStm(curPos, $1, $3, $6);
} | CONTINUE ';' {
    $$ = A_ContinueStm(curPos);
} | BREAK ';' {
    $$ = A_BreakStm(curPos);
} | RETURN '(' Exp ')' ';' {
    $$ = A_ReturnStm(curPos, $3);
} | PUTINT '(' Exp ')' ';' {
    $$ = A_PutintStm(curPos, $3);
} | PUTCH '(' Exp ')' ';' {
    $$ = A_PutchStm(curPos, $3);
} | PUTARRAY '(' Exp ',' Exp ')' ';' {
    $$ = A_PutarrayStm(curPos, $3, $5);
} | STARTTIME '(' ')' ';' {
    $$ = A_StarttimeStm(curPos);
} | STOPTIME '(' ')' ';' {
    $$ = A_StoptimeStm(curPos);
} | error ';' {
    yyerrok;
};

Exp: Exp AND Exp {
    $$ = A_OpExp($1, A_and, $3);
} | Exp OR Exp {
    $$ = A_OpExp($1, A_or, $3);
} | Exp '<' Exp {
    $$ = A_OpExp($1, A_less, $3);
} | Exp LE Exp {
    $$ = A_OpExp($1, A_le, $3);
} | Exp EQ Exp {
    $$ = A_OpExp($1, A_eq, $3);
} | Exp '+' Exp {
    $$ = A_OpExp($1, A_plus, $3);
} | Exp '-' Exp {
    $$ = A_OpExp($1, A_minus, $3);
} | Exp '*' Exp {
    $$ = A_OpExp($1, A_times, $3);
} | Exp '/' Exp {
    $$ = A_OpExp($1, A_div, $3);
} | Exp '[' ExpList ']' {
    $$ = A_ArrayExp($1, $3);
} | Exp '.' LENGTH {
    $$ = A_LengthExp(curPos, $1);
} | Exp '.' id '(' ExpList ')' {
    $$ = A_CallExp(curPos, $1, $3, $5);
} | INTEGER_LITERAL {
    $$ = A_NumExp(curPos, $1);
} | TRUE_ {
    $$ = A_BoolExp(curPos, TRUE);
} | FALSE_ {
    $$ = A_BoolExp(curPos, FALSE);
} | id {
    $$ = A_IdExp(curPos, $1);
} | THIS {
    $$ = A_ThisExp(curPos);
} | NEW INT '[' ExpList ']' {
    $$ = A_NewIntArrExp(curPos, $4);
} | NEW id '(' ')' {
    $$ = A_NewObjExp(curPos, $2);
} | '!' Exp {
    $$ = A_NotExp($2);
} | '-' Exp %prec '!' {
    $$ = A_OpExp(A_NumExp(curPos, 0), A_minus, $2);
} | '(' Exp ')' {
    $$ = $2;
} | GETINT '(' ')' {
    $$ = A_GetintExp(curPos);
} | GETCH '(' ')' {
    $$ = A_GetchExp(curPos);
} | GETARRAY '(' Exp ')' {
    $$ = A_GetarrayExp(curPos, $3);
};

ExpList: {
    $$ = NULL;
} | Exp ExpRest {
    $$ = A_ExpList($1, $2);
};

ExpRest: {
    $$ = NULL;
} | ',' Exp ExpRest {
    $$ = A_ExpList($2, $3);
};

INTEGER_LITERAL: DIGIT {
    $$ = $1;
    base = 10;
} | INTEGER_LITERAL DIGIT {
    $$ = base * $1 + $2;
};

%%
/* int main() {
    return (yyparse());
} */
void yyerror(s) char *s; {
    fprintf(stderr, "error on line %d pos %d: %s\n", yylloc.first_line, yylloc.first_column, s);
}

int yywrap(){
    return(1);
}

