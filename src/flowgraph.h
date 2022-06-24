
#ifndef __FLOWGRAPH
#define __FLOWGRAPH
/*
 * flowgraph.h - Function prototypes to represent control flow graphs.
 */
#include <stdio.h>

#include "assem.h"
#include "graph.h"
#include "temp.h"
#include "util.h"
Temp_tempList FG_def(G_node n);
Temp_tempList FG_use(G_node n);
bool FG_isMove(G_node n);
G_graph FG_AssemFlowGraph(AS_instrList il);
void FG_Showinfo(FILE*, AS_instr, Temp_map);
#endif