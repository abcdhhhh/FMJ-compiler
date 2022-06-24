#ifndef __REGALLOC
#define __REGALLOC
/* function prototype from regalloc.c */
// given a temp_map that gives assigns registers to temps (with some of them as
// "Spill") Update the input instruction list to generate the final instruction
// list
AS_instrList RA_RegAlloc(string method_name, AS_instrList il, Temp_map tm,
                         int num_spills);
// The following one performs the actual spills using r9-10
AS_instrList RA_Spill(AS_instrList il, Temp_tempList spills);
#endif