#include <string.h>

#include "assem.h"
#include "canon.h"
#include "check.h"
#include "color.h"
#include "flowgraph.h"
#include "fmjAST.h"
#include "graph.h"
#include "ig.h"
#include "liveness.h"
#include "myutils.h"
#include "printtree.h"
#include "prog.h"
#include "regalloc.h"
#include "rpi.h"
#include "tile.h"
#include "translate.h"
#include "tree.h"

void show(AS_instr ins) { FG_Showinfo(stdout, ins, Temp_name()); }

AS_instrList alloc(AS_instrList l, string methodname) {
    G_graph G = FG_AssemFlowGraph(l);
    // G_show(stdout, G_nodes(G), (void*)show);  // show it

    G_nodeList lg = Liveness(G_nodes(G));
    // Show_Liveness(stdout, lg);

    G_nodeList ig = Create_ig(lg);
    // printf("------Interference Graph---------\n");
    // Show_ig(stdout, ig);

    struct COL_result cr = COL_Color(ig);
    // printf("------Here are the assignments------\n");
    // Temp_dumpMap(stdout, cr.coloring);

    /* Take a look what are spilled */
    // printf("------Here are the spill temps------\n");
    int num_spills = 0;
    for (Temp_tempList t = cr.spills; t; t = t->tail) {
        // printf("Temp %s\n", Temp_look(Temp_name(), t->head));
        num_spills++;
    }
    //
    /* Now we assign the colors back to the instruction list l */
    AS_instrList ril =
        RA_RegAlloc(String(methodname), l, cr.coloring, num_spills);

    // printf("----instructions after reg alloc without spilling----\n");
    // AS_printInstrList(stdout, ril, Temp_name());

    /* Now we deal with the spills */
    AS_instrList final_il =
        RA_Spill(ril, cr.spills);  // This code need completion!!

    return final_il;
}

T_stmList canon_method(T_stm s) {
    // printf("Original IR Tree:\n");
    // printStmList(stdout, T_StmList(s, NULL));
    // printf("\n");

    T_stmList sl = C_linearize(s);
    // printf("\nLinearized IR Tree:\n");
    // printStmList(stdout, sl);
    // printf("\n");

    struct C_block c = C_basicBlocks(sl);

    // printf("\n\nHow It's Broken Up:\n");
    // for (C_stmListList sList = c.stmLists; sList; sList = sList->tail) {
    //     printf("\n\nFor Label=%s\n", S_name(sList->head->head->u.LABEL));
    //     printStmList(stdout, sList->head);
    // }
    // printf("\n");

    // printf("\n\nThe Final Canonical Tree:\n");
    // printStmList(stdout, C_traceSchedule(c));

    return C_traceSchedule(c);
}

int main() {
    A_prog p = prog("test/final_bubblesort.fmj");
    // printFMJfromAST(p);

    // check_prog(p);

    T_stmList sl = trans_prog(p);

    for (T_stmList slist = sl; slist; slist = slist->tail) {
        // printStmList(stdout, T_StmList(slist->head, NULL));
        T_stmList msl = canon_method(slist->head);
        // printStmList(stdout, msl);
        // printf("\n\n");

        assert(msl->head->kind == T_LABEL);
        string methodname = Temp_labelstring(msl->head->u.LABEL);

        AS_instrList l = tileSl(msl);
        // AS_printInstrList(stdout, l, Temp_name());  // print it

        AS_instrList final_il = alloc(l, methodname);
        // printf("------The final assembly instructions for the
        // method !-- -- --\n ");

        FILE* fp = fopen(concats(2, methodname, ".s"), "w");
        fprintf(fp, ".extern _sysy_starttime\n");
        fprintf(fp, ".extern _sysy_stoptime\n");
        fprintf(fp, ".extern putint\n");
        fprintf(fp, ".extern putch\n");
        fprintf(fp, ".extern putarray\n");
        fprintf(fp, ".extern getint\n");
        fprintf(fp, ".extern getch\n");
        fprintf(fp, ".extern getarray\n\n");
        fprintf(fp, ".balign 4\n");
        fprintf(fp, "%s", concats(3, ".global ", methodname, "\n"));
        fprintf(fp, ".section .text\n\n");

        AS_printInstrList(fp, final_il, Temp_name());
    }

    return 0;
}