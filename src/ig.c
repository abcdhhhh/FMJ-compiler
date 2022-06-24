#include "ig.h"

#include <stdio.h>

#include "assem.h"
#include "flowgraph.h"
#include "graph.h"
#include "liveness.h"
#include "symbol.h"
#include "table.h"
#include "temp.h"
#include "util.h"

static G_graph RA_ig;  // info of this graph is a Temp_temp

void Ig_empty() { RA_ig = G_Graph(); }

G_graph Ig_graph() { return RA_ig; }

G_node Look_ig(Temp_temp t) {
    G_node n1 = NULL;
    for (G_nodeList n = G_nodes(RA_ig); n != NULL; n = n->tail) {
        if ((Temp_temp)G_nodeInfo(n->head) == t) {
            n1 = n->head;
            break;
        }
    }
    if (n1 == NULL)
        return (G_Node(RA_ig, t));
    else
        return n1;
}

void Enter_ig(Temp_temp t1, Temp_temp t2) {
    G_node n1 = Look_ig(t1);
    G_node n2 = Look_ig(t2);
    // bidirectional
    G_addEdge(n1, n2);
    G_addEdge(n2, n1);
    return;
}

// input flowgraph after liveness analysis (so FG_In and FG_Out are available)

G_nodeList Create_ig(G_nodeList flowgraph) {
    // You need to fill in here!
    RA_ig = G_Graph();
    for (G_nodeList n = flowgraph; n != NULL; n = n->tail) {
        G_node n1 = n->head;
        for (Temp_tempList use = FG_use(n1); use; use = use->tail) {
            Temp_temp temp_use = use->head;
            Look_ig(temp_use);  // build node
        }
        for (Temp_tempList def = FG_def(n1); def != NULL; def = def->tail) {
            Temp_temp temp_def = def->head;
            Look_ig(temp_def);  // build node
            for (Temp_tempList out = FG_Out(n1); out; out = out->tail) {
                Temp_temp temp_out = out->head;
                bool flag = (temp_def != temp_out);
                if (FG_isMove(n1)) {
                    for (Temp_tempList use = FG_use(n1); use; use = use->tail) {
                        Temp_temp temp_use = use->head;
                        if (temp_use == temp_out) flag = FALSE;
                    }
                }
                if (flag) {
                    Enter_ig(temp_def, temp_out);
                }
            }
        }
    }
    // printf("here!!\n");
    assert(RA_ig);
    // printf("here!!!\n");
    return G_nodes(RA_ig);
}

static void show_temp(Temp_temp t) {
    fprintf(stdout, "%s, ", Temp_look(Temp_name(), t));
}

void Show_ig(FILE* out, G_nodeList l) { G_show(out, l, (void*)show_temp); }
