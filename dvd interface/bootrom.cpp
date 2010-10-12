// bootrom.cpp

#include "../emu.h"

////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////

// Desc: Initialize Exception Handlers
//

void Bootrom(u32 FSTStart)
{
	int i;

	printf(".Bootrom: Loading OS Values\n");

	// Desc: Initialize Exception Handlers
	//

	for(i = 0; i < sizeof(GEX_Reset_Handler) / 4; i++)
	{
		Memory_Write32 (GekkoCPU::GEX_RESET + i*4, GEX_Reset_Handler[i]);
		Memory_Write32 (0x80001800 + i*4, GEX_Reset_Handler[i]);
	}

	//for(int i = 0; i < sizeof(GEX_Decrementer_Handler) / 4; i++)
		Memory_Write32 (GekkoCPU::GEX_DEC, 0x4c000064);
/*
	for(i = 0; i < sizeof(GEX_Empty_Handler) / 4; i++)
	{
		Memory_Write32 (GekkoCPU::GEX_SCALL + i*4, GEX_Empty_Handler[i]);
		Memory_Write32 (GekkoCPU::GEX_EXT + i*4, GEX_Empty_Handler[i]);
	}
*/
	//setup a dummy system handler, rfi
    Memory_Write32(GekkoCPU::GEX_SCALL, 0x4c000064);
	Memory_Write32(GekkoCPU::GEX_EXT, 0x4c000064);

	//setup the cpu
	for(i=0; i < 16; i++)
		set_ireg_sr(i, 0x80000000);
    set_ireg_spr(I_IBAT0U, 0x80001fff);
	set_ireg_spr(I_IBAT0L, 0x00000002);
    set_ireg_spr(I_IBAT1U, 0xc0001fff);
	set_ireg_spr(I_IBAT1L, 0x0000002a);
	set_ireg_spr(I_DBAT0U, 0x80001fff);
	set_ireg_spr(I_DBAT0L, 0x00000002);
    set_ireg_spr(I_DBAT1U, 0xc0001fff);
	set_ireg_spr(I_DBAT1L, 0x0000002a);
	set_ireg_spr(287, 0x00083214);


	//setup the memory for the dolphin os
    Memory_Write32(DOLPHIN_GEKKOCLOCK, GEKKO_CLOCK);
    Memory_Write32(DOLPHIN_GEKKOBUS, GEKKO_CLOCK / 3);
	Memory_Write32(DOLPHIN_RAM_SIZE, RAM_24MB);
	Memory_Write32(DOLPHIN_SIM_RAM_SIZE, RAM_24MB);
	Memory_Write32(DOLPHIN_CONSOLE_TYPE, CONSOLE_TYPE_LATEST_PB);
	Memory_Write32(DOLPHIN_ARENALO, ARENA_LO);
	Memory_Write32(DOLPHIN_ARENAHI, FSTStart);
	Memory_Write32(DOLPHIN_MAGIC, MAGIC_NORMAL);

	//setup a dummy system handler, rfi
//    Memory_Write32(0x80000C00, 0x4c000064);

//	Memory_Write32(DOLPHIN_CONTEXT_PHYSICAL,CONTEXT_PHYSICAL);
//	Memory_Write32(DOLPHIN_CONTEXT_LOGICAL,	CONTEXT_LOGICAL);

	// Initialize Hardware
	//

//	Memory_Initialize();
//	Flipper_Open();
//	cpu->Open(entry_point);
}