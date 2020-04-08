/*
Copyright (C) 2017 Baker

*/
// coref.c

#include "quakedef.h"
#include "menu_ex.h"	// Related
#include "menu.h"		// Courtesy



#if 0
#include "menu_ex_vartypes.h"

typedef struct {
	int a;
	float b;
} some_struct_t;

typedef struct {
	int a;
	float b;
//	some_struct_t ss;
} print_this_struct_t;

print_this_struct_t printme;

typedef struct {
	char	*name;
	float b;
//	some_struct_t ss;
} structo_t;




void PO_f (lparse_t *line)
{
	// line->count == 1
}





#endif // 0

void Memories_f (lparse_t *line) {}
void Objects_f (lparse_t *line) {}
void Classes_f (lparse_t *line) {}
void Enum_f (lparse_t *line) {}
void Arrays_f (lparse_t *line) {}
void Names_f (lparse_t *line) {}
void Types_f (lparse_t *line) {}