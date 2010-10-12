// Symbols.h

#pragma once

#define MAX_SYM_NAME 64

typedef struct
{
	u32 size;
	u32 value;

	u8 bind;
	u8 type;

	u16 sec;

	char name[MAX_SYM_NAME];

} Symbol_Table, SymTable;

extern SymTable *vSym;
