/* IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 */

#include "color.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "assem.h"
#include "flowgraph.h"
#include "graph.h"
#include "ig.h"
#include "liveness.h"
#include "myutils.h"
#include "rpi.h"
#include "symbol.h"
#include "table.h"
#include "temp.h"
#include "util.h"

void COL_PrintMap(FILE* out, Temp_map tm, G_nodeList ig) {
    G_nodeList l = ig;
    Temp_temp t;
    fprintf(out, "----coloring result----\n");
    for (; l; l = l->tail) {
        t = G_nodeInfo(l->head);
        fprintf(out, "Temp %s mapped to %s\n", Temp_look(Temp_name(), t),
                Temp_look(tm, t));
    }
    return;
}

/* IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 IMPLEMENTATION NEEDED IMPLEMENTATION NEEDED
 */

struct COL_result COL_Color(G_nodeList ig) {
    struct COL_result cr;
    cr.coloring = Temp_empty();
    cr.spills = NULL;

    int cnt = 0;
    G_nodeList sta = NULL;
    Temp_map degs = Temp_empty();
    for (int i = 0; i <= 14; i++) {
        Temp_enter(cr.coloring, F_Ri(i), concats(2, "r", i2str(i)));
    }
    for (G_nodeList l = ig; l; l = l->tail) {
        G_node gn = l->head;
        Temp_temp n = G_nodeInfo(gn);
        string tn = Temp_look(Temp_name(), n);
        int d = G_degree(gn);
        Temp_enter(degs, n, i2str(d));
        if (atoi(tn) < 100) continue;
        cnt++;
    }
    while (cnt) {
        // printf("round:\n");
        bool flag = FALSE;
        for (G_nodeList l = ig; l; l = l->tail) {
            G_node gn = l->head;
            Temp_temp n = G_nodeInfo(gn);
            string tn = Temp_look(Temp_name(), n);
            if (atoi(tn) < 100) continue;
            int d = atoi(Temp_look(degs, n));
            if (d >= 0 && d < 10) {
                // printf("[[%d]]\n", d);
                flag = TRUE;
                Temp_enter(degs, n, i2str(-1));
                sta = G_NodeList(gn, sta);  // simplify
                eraseNode(gn, degs);
                cnt--;
            }
        }
        if (flag == FALSE) {
            for (G_nodeList l = ig; l; l = l->tail) {
                G_node gn = l->head;
                Temp_temp n = G_nodeInfo(gn);
                string tn = Temp_look(Temp_name(), n);
                if (atoi(tn) < 100) continue;
                int d = atoi(Temp_look(degs, n));
                if (d >= 0) {
                    // printf("[%d]\n", d);
                    assert(d >= 10);
                    Temp_enter(degs, n, i2str(-1));
                    Temp_enter(cr.coloring, n, String("Spill"));
                    assert(n);
                    cr.spills = Temp_TempList(n, cr.spills);
                    eraseNode(gn, degs);
                    cnt--;
                    break;
                }
            }
        }
    }
    while (sta) {
        G_node gn = sta->head;
        sta = sta->tail;
        Temp_temp n = G_nodeInfo(gn);
        string tn = Temp_look(Temp_name(), n);
        // printf("%s:\n", tn);
        int mask = 0;

        for (G_nodeList p = G_succ(gn); p; p = p->tail) {
            G_node gv = p->head;
            Temp_temp v = G_nodeInfo(gv);
            string tv = Temp_look(Temp_name(), v);
            string s = Temp_look(cr.coloring, v);
            if (s != NULL) {
                if (s[0] == 'r') {
                    // printf("%s\t%s\n", tv, s);
                    int vc = atoi(s + 1);
                    mask |= 1 << vc;
                } else {
                    assert(s[0] == 'S');
                }
            }
        }
        for (G_nodeList p = G_pred(gn); p; p = p->tail) {
            G_node gv = p->head;
            Temp_temp v = G_nodeInfo(gv);
            string tv = Temp_look(Temp_name(), v);
            string s = Temp_look(cr.coloring, v);
            if (s != NULL) {
                if (s[0] == 'r') {
                    // printf("%s\t%s\n", tv, s);
                    int vc = atoi(s + 1);
                    mask |= 1 << vc;
                } else {
                    assert(s[0] == 'S');
                }
            }
        }
        int i = 0;
        while (mask & (1 << i)) i++;
        if (i >= 9) i = 14;
        assert(!(mask & (1 << i)));
        // printf("decide: %d\n", i);
        Temp_enter(cr.coloring, n, concats(2, "r", i2str(i)));
    }

    // check
    for (G_nodeList l = ig; l; l = l->tail) {
        G_node gn = l->head;
        Temp_temp n = G_nodeInfo(gn);
        for (G_nodeList p = G_adj(gn); p; p = p->tail) {
            G_node gv = p->head;
            Temp_temp v = G_nodeInfo(gv);
            assert(n != v);
        }
    }
    return cr;
}
