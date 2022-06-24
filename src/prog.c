#include <stdio.h>
#include <stdlib.h>

#include "fmjAST.h"
#include "util.h"

extern int yyparse();
extern A_prog root;
A_prog prog(string fn) {
    freopen(fn, "r", stdin);
    (yyparse());
    return root;
}

/*
 Prog1 below is the AST tree for the following program:

 class try { public static int main (String [] id) {
   int a; bool b; int c;
   starttime();
   b=true;
   if (b) a=3; else a=2;
   while (a<10) a=a+1;
   putint(a);
   stoptime();
   return(a);
 }
 */

A_prog prog1(void) {
    return (A_Prog(
        A_Pos(1, 1),
        A_MainClass(
            A_Pos(1, 1), "try",
            A_VarDeclList(
                A_VarDecl(A_Pos(2, 3), A_Type(A_Pos(2, 3), A_intType, NULL, 0),
                          String("a")),
                A_VarDeclList(
                    A_VarDecl(A_Pos(2, 10),
                              A_Type(A_Pos(2, 10), A_boolType, NULL, 0),
                              String("b")),
                    A_VarDeclList(
                        A_VarDecl(A_Pos(2, 18),
                                  A_Type(A_Pos(2, 18), A_intType, NULL, 0),
                                  String("c")),
                        NULL))),  // no other var decl

            A_StmList(
                A_StarttimeStm(A_Pos(3, 3)),
                A_StmList(
                    A_AssignStm(A_Pos(4, 3), String("b"), NULL,
                                A_BoolExp(A_Pos(4, 5), TRUE)),
                    A_StmList(
                        A_IfStm(A_Pos(5, 3), A_IdExp(A_Pos(5, 7), String("b")),
                                A_AssignStm(A_Pos(5, 10), String("a"), NULL,
                                            A_NumExp(A_Pos(5, 12), 3)),
                                A_AssignStm(A_Pos(5, 20), String("a"), NULL,
                                            A_NumExp(A_Pos(5, 22), 2))),
                        A_StmList(
                            A_WhileStm(
                                A_Pos(6, 3),
                                A_OpExp(A_IdExp(A_Pos(6, 10), String("a")),
                                        A_less, A_NumExp(A_Pos(6, 12), 10)),
                                A_AssignStm(
                                    A_Pos(6, 16), String("a"), NULL,
                                    A_OpExp(A_IdExp(A_Pos(6, 18), String("a")),
                                            A_plus,
                                            A_NumExp(A_Pos(6, 20), 1)))),
                            A_StmList(
                                A_PutintStm(A_Pos(7, 3),
                                            A_IdExp(A_Pos(7, 10), String("a"))),
                                A_StmList(
                                    A_StoptimeStm(A_Pos(8, 3)),
                                    A_StmList(A_ReturnStm(A_Pos(9, 3),
                                                          A_IdExp(A_Pos(9, 10),
                                                                  String("a"))),
                                              NULL)))))))),  // no other stmts
        NULL));                                              // no other classes
}

/*
 Prog2 below is the AST tree for the following program:

 class try2 { public static int main (String [] id) {
   return(new C2().f(0));
 }

 class C1 {
    int[][] a;
    public int f(int x) {
      a=new int[3,getint()];
      a[1,1]=3;
      return(1);
    }

    public int g() {
       return(2);
    }
 }

 class C2 extends C1 {
    public int f(int x) {
        C1 c1;
        c1 = new C2();
        return(c1.g());
    }
 }
 */

A_prog prog2(void) {
    return (A_Prog(
        A_Pos(1, 1),

        /*main*/
        A_MainClass(
            A_Pos(1, 1), "try2", NULL,
            /*return*/
            A_StmList(A_ReturnStm(
                          A_Pos(2, 4),
                          /*new C2.f(0)*/ A_CallExp(
                              A_Pos(2, 11), A_NewObjExp(A_Pos(2, 11), "C2"),
                              "f", A_ExpList(A_NumExp(A_Pos(2, 22), 0), NULL))),
                      NULL)),  // end of main class

        /*C1*/
        A_ClassDeclList(
            A_ClassDecl(
                A_Pos(5, 1), "C1", NULL,
                A_VarDeclList(
                    /*int[][] a*/ A_VarDecl(
                        A_Pos(6, 4), A_Type(A_Pos(6, 4), A_intArrType, NULL, 2),
                        String("a")),
                    NULL),
                /*int f(x)*/
                A_MethodDeclList(
                    A_MethodDecl(
                        A_Pos(7, 4), A_Type(A_Pos(7, 11), A_intType, NULL, 0),
                        "f",
                        /* x */
                        A_FormalList(A_Formal(A_Pos(7, 17),
                                              A_Type(A_Pos(7, 17), A_intArrType,
                                                     NULL, 0),
                                              "x"),
                                     NULL),
                        NULL,  // empty A_VarDeclList(),
                        A_StmList(
                            A_AssignStm(
                                A_Pos(8, 6), "a", NULL,  // simple id
                                /*a=new int[3,getint()]*/
                                A_NewIntArrExp(
                                    A_Pos(8, 8),
                                    A_ExpList(
                                        A_NumExp(A_Pos(8, 16), 3),
                                        A_ExpList(A_GetintExp(A_Pos(8, 18)),
                                                  NULL)))),
                            /*a[1,1]=3*/ A_StmList(
                                A_AssignStm(
                                    A_Pos(9, 6), "a",
                                    A_ExpList(
                                        A_NumExp(A_Pos(9, 8), 1),
                                        A_ExpList(A_NumExp(A_Pos(9, 10), 1),
                                                  NULL)),
                                    A_NumExp(A_Pos(9, 13), 3)),
                                /*return(1)*/ A_StmList(
                                    A_ReturnStm(A_Pos(10, 6),
                                                A_NumExp(A_Pos(10, 13), 1)),
                                    NULL)))  // no more statements!
                        ),                   // end of first method
                    /* int g()*/ A_MethodDeclList(
                        A_MethodDecl(
                            A_Pos(11, 4),
                            A_Type(A_Pos(11, 11), A_intType, NULL, 0), "g",
                            NULL,  // no formal
                            NULL,  // no var decl
                            /*return(2)*/
                            A_StmList(A_ReturnStm(A_Pos(12, 7),
                                                  A_NumExp(A_Pos(12, 14), 2)),
                                      NULL)),
                        NULL))),  // no more methods for C1
            /*C2*/ A_ClassDeclList(
                A_ClassDecl(
                    A_Pos(16, 1), "C2", "C1",
                    NULL,  // no class variables
                    /*int f(x) */
                    A_MethodDeclList(
                        A_MethodDecl(
                            A_Pos(17, 4),
                            A_Type(A_Pos(17, 11), A_intType, NULL, 0), "f",
                            A_FormalList(A_Formal(A_Pos(17, 17),
                                                  A_Type(A_Pos(17, 17),
                                                         A_intType, NULL, 0),
                                                  "x"),
                                         NULL),
                            /* C1 c1*/
                            A_VarDeclList(A_VarDecl(A_Pos(18, 8),
                                                    A_Type(A_Pos(18, 8),
                                                           A_idType, "C1", 0),
                                                    "c1"),
                                          NULL),
                            /*c1=new C2()*/
                            A_StmList(
                                A_AssignStm(A_Pos(19, 8), "c1", NULL,
                                            A_NewObjExp(A_Pos(19, 13), "C2")),
                                /*return(c1.g())*/
                                A_StmList(
                                    A_ReturnStm(
                                        A_Pos(20, 8),
                                        A_CallExp(A_Pos(20, 15),
                                                  A_IdExp(A_Pos(20, 15), "c1"),
                                                  "g", NULL)),
                                    NULL))),
                        NULL)),  // no more methods
                NULL))           // no more classes
        ));
}