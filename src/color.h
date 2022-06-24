#ifndef __COLOR
#define __COLOR
/*
 * color.h - Data structures and function prototypes for coloring algorithm
 *             to determine register allocation.
 */
#include <stdio.h>

#include "graph.h"
#include "temp.h"
struct COL_result {
    Temp_map coloring;
    Temp_tempList spills;
};
struct COL_result COL_Color(G_nodeList ig);
void COL_PrintMap(FILE*, Temp_map, G_nodeList);
#endif