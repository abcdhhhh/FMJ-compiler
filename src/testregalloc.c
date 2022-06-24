#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assem.h"
#include "color.h"
#include "flowgraph.h"
#include "graph.h"
#include "ig.h"
#include "liveness.h"
#include "regalloc.h"
#include "rpi.h"
#include "symbol.h"
#include "temp.h"
#include "util.h"

#define IL(a, b) AS_InstrList(a, b)
#define MI(a, b, c) AS_Move(a, b, c)
#define LI(a, b) AS_Label(a, b)
#define OI(a, b, c, d) AS_Oper(a, b, c, d)
#define T(a) Temp_TempList(a, NULL)
#define TL(a, b) Temp_TempList(a, b)
#define LL(a, b) Temp_LabelList(a, b)
#define L(a) Temp_LabelList(a, NULL)
#define Targets(a) AS_Targets(a)

/*
 The following program is like Graph 10.1 in the testbook, but each var is a
 temp

    mov a, #0
    lbegin: nop
    add b, a, #1
    add c, c, b
    mov b, b, ASL #1
    cmp a, #10
    blt lbegin
    ft:   nop
    b lr
 */

static Temp_temp lr, r0, a, b, c;
static Temp_label lbegin, ft;

AS_instrList list1(void) {
    lr = F_LR();
    r0 = F_Ri(0);
    a = Temp_newtemp();
    b = Temp_newtemp();
    c = Temp_newtemp();
    lbegin = Temp_newlabel();
    ft = Temp_newlabel();
    Temp_label label_main = Temp_newlabel();
    return (
        IL(LI("main: nop", label_main),
           IL(OI("mov `d0, #0", T(a), NULL, NULL),
              IL(LI("lbegin: nop", lbegin),
                 IL(OI("add `d0, `s0, #1", T(b), T(a), NULL),
                    IL(OI("add `d0, `s0, `s1", T(c), TL(c, T(b)), NULL),
                       IL(OI("mov `d0, `s0, ASL #1", T(b), T(b), NULL),
                          IL(OI("cmp `s0, #10", NULL, T(a), NULL),
                             IL(OI("blt lbegin", NULL, NULL,
                                   Targets(LL(lbegin, L(ft)))),
                                IL(LI("ft: nop", ft),
                                   IL(MI("mov `d0, `s0", T(r0), T(c)),
                                      IL(OI("bx lr", NULL, TL(lr, T(r0)), NULL),
                                         NULL))))))))))));
}

void show(AS_instr ins) { FG_Showinfo(stdout, ins, Temp_name()); }

/* hand coded interference graph for list1 program*/

void Ig_empty();
G_graph Ig_graph();
void Enter_ig(Temp_temp, Temp_temp);

G_nodeList Create_ig1() {
    Ig_empty();

    Enter_ig(a, lr);
    Enter_ig(a, c);
    Enter_ig(c, lr);
    Enter_ig(c, b);
    Enter_ig(b, lr);
    Enter_ig(lr, r0);

    return G_nodes(Ig_graph());
}

/* create a hand-coded interference graph
   Graph 11.1 in textbook (without "move" edges)
 */
G_nodeList Create_ig11_1() {
    Temp_temp k = F_Ri(0);  // Temp_newtemp();
    Temp_temp j = Temp_newtemp();
    Temp_temp g = Temp_newtemp();
    Temp_temp h = Temp_newtemp();
    Temp_temp f = Temp_newtemp();
    Temp_temp e = Temp_newtemp();
    Temp_temp m = Temp_newtemp();
    Temp_temp b = Temp_newtemp();
    Temp_temp c = Temp_newtemp();
    Temp_temp d = Temp_newtemp();

    Ig_empty();

    Enter_ig(j, f);
    Enter_ig(j, e);
    Enter_ig(j, k);
    Enter_ig(j, d);
    Enter_ig(j, h);
    Enter_ig(j, g);
    Enter_ig(f, e);
    Enter_ig(f, m);
    Enter_ig(e, b);
    Enter_ig(e, m);
    Enter_ig(k, b);
    Enter_ig(k, d);
    Enter_ig(k, g);
    Enter_ig(h, g);
    Enter_ig(d, b);
    Enter_ig(d, m);
    Enter_ig(b, m);
    Enter_ig(c, b);
    Enter_ig(c, m);
    return G_nodes(Ig_graph());
}

#ifdef TEST_MY_IG
#undef TEST_MY_IG
#endif
//#define TEST_MY_IG

#ifdef TEXT11
#undef TEXT11
#endif
//#define TEXT11

#ifdef CREATE_IG_CODE
#undef CREATE_IG_CODE
#endif
//#define CREATE_IG_CODE

void test_text11() {
    G_nodeList ig1 = Create_ig11_1();
    printf("------Interference Graph---------\n");
    Show_ig(stdout, ig1);
    // color the interference graph
    struct COL_result cr = COL_Color(ig1);
    printf("------Here are the assignments------\n");
    Temp_dumpMap(stdout, cr.coloring);
    // dump the spills
    if (!cr.spills) {
        printf("---No spill needed--- we are done!-----\n");
    } else {
        printf("------Here are the spill temps------\n");
        for (Temp_tempList t = cr.spills; t; t = t->tail) {
            printf("Temp %s\n", Temp_look(Temp_name(), t->head));
        }
    }
    return;
}

int main() {
    G_nodeList ig1;

#ifdef TEXT11  // if only to test text book figure 11.1 interference graph
    test_text11();
    return 0;
#endif

    AS_instrList l = list1();                   // hand-coded instrList
    AS_printInstrList(stdout, l, Temp_name());  // print it

    G_graph G = FG_AssemFlowGraph(l);         // getting the flowgraph
    G_show(stdout, G_nodes(G), (void*)show);  // show it
    G_nodeList lg = Liveness(G_nodes(
        G));  // liveness analysis (print the iterations, and storing the result
              // in/out lists in FG_In/Out functions for each node of lg)
    Show_Liveness(stdout, lg);  // show the result

#ifdef TEST_MY_IG        /* here's the code for getting the ig graph */
    ig1 = Create_ig(lg); /* test your create_ig function */
#else
    ig1 = Create_ig1(); /* here's a hand-coded one */
#endif

#ifdef CREATE_IG_CODE
    Create_ig_Code(stdout, ig1); /* may use this to create the IG code from ig
                                  data structure for debugging/later use */
#endif

    printf("------Interference Graph---------\n");
    Show_ig(stdout, ig1);

    /* Now we color the interference graph, cr has coloring map and spill
     * templist*/
    struct COL_result cr = COL_Color(ig1);
    printf("------Here are the assignments------\n");
    Temp_dumpMap(stdout, cr.coloring);

    /* Take a look what are spilled */
    int num_spills = 0;
    if (!cr.spills) {
        printf("---No spill needed----------------\n");
    } else {
        printf("------Here are the spill temps------\n");
        for (Temp_tempList t = cr.spills; t; t = t->tail) {
            printf("Temp %s\n", Temp_look(Temp_name(), t->head));
            num_spills++;
        }
    }
    /* Now we assign the colors back to the instruction list l */
    AS_instrList ril = RA_RegAlloc(String("main"), l, cr.coloring, num_spills);

    printf("----instructions after reg alloc without spilling----\n");
    AS_printInstrList(stdout, ril, Temp_name());

    /* Now we deal with the spills */
    AS_instrList final_il =
        RA_Spill(ril, cr.spills);  // This code need completion!!

    printf("------The final assembly instructions for the method!------\n");
    AS_printInstrList(stdout, final_il, Temp_name());
    return 0;
}
