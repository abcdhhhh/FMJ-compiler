#ifndef __TYPE
#define __TYPE
/*
 * types.h -
 *
 * All types and functions declared in this header file begin with "Ty_"
 * Linked list types end with "..list"
 */
#include "symbol.h"

typedef struct Ty_ty_ *Ty_ty;
typedef struct Ty_tyList_ *Ty_tyList;
typedef struct Ty_field_ *Ty_field;
typedef struct Ty_fieldList_ *Ty_fieldList;
typedef struct Ty_method_ *Ty_method;
typedef struct Ty_methodList_ *Ty_methodList;

struct Ty_ty_ {
    enum {
        Ty_record,
        Ty_nil,
        Ty_int,
        Ty_bool,
        Ty_array,
        Ty_name,
        Ty_void
    } kind;
    union {
        struct {
            S_symbol parent;
            Ty_fieldList fields;
            Ty_methodList methods;
        } record;
        struct {
            Ty_ty ty;
            int arity;
        } array;
        struct {
            S_symbol sym;
            Ty_ty ty;
        } name;
    } u;
};

struct Ty_tyList_ {
    Ty_ty head;
    Ty_tyList tail;
};
struct Ty_field_ {
    S_symbol name;
    Ty_ty ty;
};
struct Ty_fieldList_ {
    Ty_field head;
    Ty_fieldList tail;
};

struct Ty_method_ {
    S_symbol name;
    Ty_ty ty;
    Ty_tyList formals;
};

struct Ty_methodList_ {
    Ty_method head;
    Ty_methodList tail;
};

Ty_tyList reverseTyList(Ty_tyList list);

Ty_ty Ty_Nil(void);
Ty_ty Ty_Int(void);
Ty_ty Ty_Bool(void);
Ty_ty Ty_Void(void);

Ty_ty Ty_Record(S_symbol parent, Ty_fieldList fields, Ty_methodList methods);
Ty_ty Ty_Array(Ty_ty ty, int arity);
Ty_ty Ty_Name(S_symbol sym, Ty_ty ty);

Ty_tyList Ty_TyList(Ty_ty head, Ty_tyList tail);
Ty_field Ty_Field(S_symbol name, Ty_ty ty);
Ty_fieldList Ty_FieldList(Ty_field head, Ty_fieldList tail);

Ty_method Ty_Method(S_symbol name, Ty_ty ty, Ty_tyList formals);
Ty_methodList Ty_MethodList(Ty_method head, Ty_methodList tail);

void Ty_print(Ty_ty t);
void TyList_print(Ty_tyList list);
#endif