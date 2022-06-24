#ifndef __LIVENESS
#define __LIVENESS
#include <stdio.h>

#include "graph.h"
#include "temp.h"
G_nodeList Liveness(G_nodeList);
void Show_Liveness(FILE*, G_nodeList);
Temp_tempList FG_Out(G_node);
Temp_tempList FG_In(G_node);
#endif