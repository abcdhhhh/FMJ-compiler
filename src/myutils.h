#ifndef __MYUTILS
#define __MYUTILS
#include "assem.h"
#include "graph.h"
#include "temp.h"
#include "tree.h"
#include "util.h"

string concats(int num, ...);

void eraseNode(G_node, Temp_map);

typedef struct AS_instrListing_* AS_instrListing;
struct AS_instrListing_ {
    AS_instrList heading, tailing;
};
AS_instrListing AS_InstrListingEmpty();
void emit(AS_instrListing, AS_instr);

string i2str(int);

int myLog2(int);
T_relOp negOp(T_relOp);
string op2a(T_relOp);

T_stm safeSeq(T_stm, T_stm);
T_stm superSeq(int, ...);

#endif