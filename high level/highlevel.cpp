// highlevel.cpp
// (c) 2005,2006 Gekko Team

#include "../emu.h"
#include "hle_func.h"
#include "../cpu core/cpu_core_regs.h"
#include "hle_crc.h"

using namespace std;

//

mapFile mf;
map<u32, Function> maps;
multimap<u32, u32>	funcAddresses;

bool	DisableHLEPatches = 0;

u32 hle_ranges[4][2]=
{
	{0x80003000,0x80070000},
	{0x80070000,0x80300000},
	{0x80300000,0x80700000},
	{0x00000000,0x00000000}
};

//mask for opcodes to contain the most detail we can without things like offsets messing us up
//generated by looking at all possible known libraries and generating a list of functions used for references
//masking out addic, addi, lis, bl, ori, lwz, lwzu, lbz, stw, stb, stbu, lhz, lha, sth, sthu, lfs, lfd, stfs
u32 hle_op_mask[] = 
{
	//0x00
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	//0x08
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFF0000, 0xFFFF0000, 0xFFFF0000, 
	//0x10
	0xFFFFFFFF, 0xFFFFFFFF, 0xFC000003, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	//0x18
	0xFFFF0000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	//0x20
	0xFFFF0000, 0xFFFF0000, 0xFFFF0000, 0xFFFFFFFF, 0xFFFF0000, 0xFFFFFFFF, 0xFFFF0000, 0xFFFF0000, 
	//0x28
	0xFFFF0000, 0xFFFFFFFF, 0xFFFF0000, 0xFFFFFFFF, 0xFFFF0000, 0xFFFF0000, 0xFFFFFFFF, 0xFFFFFFFF, 
	//0x30
	0xFFFF0000, 0xFFFFFFFF, 0xFFFF0000, 0xFFFFFFFF, 0xFFFF0000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	//0x38
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
};

typedef struct _HLEPatchFuncType
{
	char *FuncName;
	u32	 FuncPtr;
} HLEPatchFuncType;

HLEPatchFuncType	HLEPatchFuncs[] =
{
	{"ignore", (u32)HLE_PTR(ignore)},
	{"ignore_return_true", (u32)HLE_PTR(ignore_return_true)},
	{"ignore_return_false", (u32)HLE_PTR(ignore_return_false)},
	{"osreport", (u32)HLE_PTR(OSReport)},
	{"dbprintf", (u32)HLE_PTR(DBPrintf)},
	{"ospanic", (u32)HLE_PTR(OSPanic)},
	{0, 0}
};

////////////////////////////////////////////////////////////

HLE(ignore)
{
}

HLE(ignore_return_true)
{
	HLE_RETURN_TRUE;
}

HLE(ignore_return_false)
{
	HLE_RETURN_FALSE;
}

////////////////////////////////////////////////////////////

u32 HLE_GetTotalCRC(OSCallFormat *oscallptr)
{
	u32 os_index=0;
	while(oscallptr[os_index].Vals[11]!=-1)
	{
		os_index++;
	};
	return os_index;
}

void HLE_ExecuteLowLevel(void)				// hack ~ShizZy
{
	/** Adds the return address to the stack **/
	GPR(0) = LR;							// mflr		r0
	Memory_Write32( GPR(1) + 4, GPR(0) );	// stw		r0,4(sp)
	u32 ea = (GPR(1) - 0x98);
	Memory_Write32(ea, GPR(1));				// stwu		sp,-0x0098(sp)
	GPR(1) = ea;

	/** Skip additional HLE information **/
	ireg.PC += 8;
//	cpu->pPC = (u32*)&RAM[ireg.PC & RAM_MASK];
}

void HLE_PatchFunction(u32 addr, u32 functionPTR)
{
	if(functionPTR)
	{
		Memory_Write32(addr,(3<<26));
		Memory_Write32(addr+4,0x4E800020);
		Memory_Write32(addr+8,functionPTR);
	}
}

void HLE_GetGameCRC(char *gameCRC, u8 *Header, u8 BannerCRC)
{
	sprintf(gameCRC, "%.4s%02X", Header, BannerCRC);

	if(strlen(gameCRC) < 6)
	{
		//if less than 6 chars, then 0 pad it
		memset(&gameCRC[strlen(gameCRC)], 0x30, 6 - strlen(gameCRC));
		gameCRC[6] = 0;
	}

	return;
}

void HLE_DetectFunctions()
{
    char    buf[1024];
    FILE    *mapfile;
    
    u32     procSize, procCRC;
    char    procName[512];
	char	funcName[512];

	//find all possible functions
	u32			Addr;
	u32			FuncSize;
	u32			FuncCRC;
	u32			FuncPatch;
	Function	NewFunc;
	u32			FuncsFound;
	u32			TotalFuncs;
	u32			x;
	Function	rFunction;
	pair<std::multimap<u32, u32>::iterator,std::multimap<u32, u32>::iterator> itr;
	std::multimap<u32, u32>::iterator itrend;
	std::map<u32, Function>::iterator mapitr;

	//go thru memory detecting functions and generate CRCs
	//add the functions to the function list if they do not already exist
	
	maps.clear();
	funcAddresses.clear();

	TotalFuncs = 0;
	for(Addr = 0x80002000; Addr < 0x81000000; Addr+= 4)
	{
		//see if we have something other than 0x00000000
		if(*(u32 *)&Mem_RAM[Addr & RAM_MASK])
		{
			//have a value, detect the function
			FuncSize = HLE_DetectFunctionSize(Addr);
			FuncCRC = HLE_GenerateFunctionCRC(Addr, FuncSize);

			memset(&NewFunc, 0, sizeof(NewFunc));

			//see if the value exists
			NewFunc.address = Addr;
			NewFunc.CRC = FuncCRC;
			NewFunc.DetectedSize = FuncSize;
			maps[Addr] = NewFunc;
			funcAddresses.insert(pair<u32, u32>(FuncCRC, Addr));

			Addr += FuncSize - 4;
			TotalFuncs++;
		}
	}

	FuncsFound = 0;
/*
	//now open up the map file and start scanning for matches
	sprintf(procName, "%smemcrc.txt", ProgramDirectory);
	mapfile = fopen(procName, "r");
    if(!mapfile) return;

    while(!feof(mapfile))
    {
        fgets(buf, 1024, mapfile);

        if(sscanf(buf, "%s\t%08x\t%08x", 
            procName, &procSize, &procCRC) != 3) continue;
*/
	u32 HLECount = 0;

	while(HLE_CRCs[HLECount].FuncName != 0)
	{
		memcpy(procName, HLE_CRCs[HLECount].FuncName, strlen(HLE_CRCs[HLECount].FuncName) + 1);
		procSize = HLE_CRCs[HLECount].FuncSize;
		procCRC = HLE_CRCs[HLECount].FuncHash;
		HLECount++;

		if(funcAddresses.count(procCRC))
		{
			itr = funcAddresses.equal_range(procCRC);

			itrend = itr.first;
			while(itrend != itr.second)
			{
				mapitr = maps.find(itrend->second);
				rFunction = mapitr->second;
				if(rFunction.CRC == procCRC && rFunction.DetectedSize == procSize && rFunction.funcSize == 0)
				{
					rFunction.funcSize = procSize;
					rFunction.funcName = procName;
					maps[rFunction.address] = rFunction;
					FuncsFound++;
				}

				itrend++;
			}
		}
    }

//    fclose(mapfile);

	/*
	sprintf(procName, "%smemcrc-patch.txt", ProgramDirectory);
	mapfile = fopen(procName, "r");
    if(mapfile)
	{
		while(!feof(mapfile))
		{
			fgets(buf, 1024, mapfile);

			funcName[0] = 0;
			if(sscanf(buf, "%s\t%08x\t%08x\t%s", 
				procName, &procSize, &procCRC, funcName) < 3) continue;
*/

	HLECount = 0;
	while(HLE_CRCPatch[HLECount].FuncName != 0)
	{
		memcpy(procName, HLE_CRCPatch[HLECount].FuncName, strlen(HLE_CRCPatch[HLECount].FuncName) + 1);
		procSize = HLE_CRCPatch[HLECount].FuncSize;
		procCRC = HLE_CRCPatch[HLECount].FuncHash;
		memcpy(funcName, HLE_CRCPatch[HLECount].PatchFuncName, strlen(HLE_CRCPatch[HLECount].PatchFuncName) + 1);
		HLECount++;

		if(funcAddresses.count(procCRC))
		{
			itr = funcAddresses.equal_range(procCRC);
			itrend = itr.first;
			while(itrend != itr.second)
			{
				mapitr = maps.find(itrend->second);
				rFunction = mapitr->second;

				if(rFunction.CRC == procCRC && rFunction.DetectedSize == procSize)
				{
					rFunction.funcSize = procSize;
					rFunction.funcName = procName;
					maps[rFunction.address] = rFunction;
					FuncsFound++;

					//if the HLE function to use doesn't exist then use the name of the function
					if(funcName[0] == 0)
						strcpy(funcName, procName);

					for(FuncPatch=0; HLEPatchFuncs[FuncPatch].FuncPtr != 0; FuncPatch++)
					{
						if(stricmp(funcName, HLEPatchFuncs[FuncPatch].FuncName) == 0)
						{
							printf("Patching %s with HLE_%s\n", procName, funcName);
							HLE_PatchFunction(rFunction.address, HLEPatchFuncs[FuncPatch].FuncPtr);
							break;
						}
					}

					if(HLEPatchFuncs[FuncPatch].FuncPtr == 0)
					{
						printf("Unable to patch %s, no HLE_%s!\n", procName, funcName);
					}
				}
				itrend++;
			}
		}
/*
		fclose(mapfile);
*/
	}

	printf("Identified %d of %d functions\n", FuncsFound, TotalFuncs);

	//go thru and rename all unknowns
/*
	itr = maps.begin();
	while(itr != maps.end())
	{
		rFunction = itr->second;
		itr++;
		if(rFunction.funcSize == 0)
			maps.erase(rFunction.address);
	}
*/

	mapitr = maps.begin();
	while(mapitr != maps.end())
	{
		rFunction = mapitr->second;
		mapitr++;
		if(rFunction.funcName.length() == 0)
		{
			sprintf(procName, "U-%08X-%X", rFunction.CRC, rFunction.DetectedSize);
			rFunction.funcName = procName;
			rFunction.funcSize = rFunction.DetectedSize;
			maps[rFunction.address] = rFunction;
		}
	}

}

void HLE_ScanForPatches()
{
	HLE_DetectFunctions();

	if(DisableINIPatches)
	{
		printf("INI patches disabled\n");
	}
	else
	{
		printf("Looking for ini CRC entry %s...", CurrentGameCRC);
		if (!findINIEntry(CurrentGameCRC,&RomInfo, 0))
			printf("\nUnable to find an ini entry. No patches applied\n");
	}
}

////////////////////////////////////////////////////////////

BOOL HLE_Map_LoadFile(char * filename) // Code Warrior Only
{
    BOOL    started = FALSE;
    char    buf[1024], token1[256];
    FILE    *mapfile;
    
    u32     moduleOffset, procSize, procAddr;
    int     flags;
    char    procName[512];

    mapfile = fopen(filename, "r");
    if(!mapfile) return false;

	maps.clear();

    while(!feof(mapfile))
    {
        fgets(buf, 1024, mapfile);
        sscanf(buf, "%s", token1);

        if(!strcmp(buf, ".init section layout\n")) { started = TRUE; continue; }
        if(!strcmp(buf, ".text section layout\n")) { started = TRUE; continue; }
        if(!strcmp(buf, ".data section layout\n")) break;

        #define IFIS(str) if(!strcmp(token1, #str)) continue;
        IFIS(Starting);
        IFIS(address);
        IFIS(-----------------------);
        IFIS(UNUSED);

        if(token1[strlen(token1) - 1] == ']') continue;
        if(started == FALSE) continue;

        if(sscanf(buf, "  %08x %08x %08x  %i %s", 
            &moduleOffset, &procSize, &procAddr,
            &flags,
            procName) != 5) continue;

        if(flags != 1)
        {
			Function f;
            f.address = procAddr;
			f.funcName = procName;
            f.funcSize = procSize;
			maps[procAddr] = f;
        }
    }

    fclose(mapfile);

    printf("\nCode Warrior format map loaded : %s\n\n", filename);

    return true;
}

void HLE_Map_OpenFile(void)
{
	static OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize		= sizeof(OPENFILENAME);
	ofn.hwndOwner		= wnd.hWnd;
	ofn.lpstrFile		= mf.path;
	ofn.nMaxFile		= 255;
	ofn.lpstrFilter		= "Code Warrior Dolphin SDK Map File (*.map)\0*.map\0";
	ofn.nFilterIndex	= 1;
	ofn.nMaxFileTitle	= 255;
	ofn.lpstrFileTitle	= mf.title;
	ofn.lpstrInitialDir	= NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if(GetOpenFileName(&ofn)>0)
		HLE_Map_LoadFile(mf.path);

	HLE_FindFuncsAndGenerateCRCs();
}

void HLE_MapSetDebugSymbol(u32 add, std::string name)
{
	Function f;
	int size = 4;

    f.address = add;
	f.funcName = name;

	u32 i = add;
	while(Memory_Read32(i) != 0x4E800020)
	{
		i += 4; 
		size += 4;
		if(size >= 10000)
		{
			break;
		}
	}

    f.funcSize = size;
	maps[add] = f;
}

bool HLE_MapGetDebugSymbol(u32 add, std::string& name)
{
    std::map<u32, Function>::const_iterator itr = maps.begin();
    while(itr != maps.end())
    {
        const Function& rFunction = itr->second;
        
        if ((add >= rFunction.address) && (add < rFunction.address + rFunction.funcSize))
        {
            name = rFunction.funcName;
            return true;
        }  

        itr++;
    }
    return false;
}

bool HLE_MapGetDebugSymbolFunctionAddress(const std::string& _name, u32& _addr)
{
    std::map<u32, Function>::const_iterator itr = maps.begin();
    while(itr != maps.end())
    {
        const Function& rFunction = itr->second;
        if (stricmp(_name.c_str(), rFunction.funcName.c_str()) == 0)
        {
            _addr = rFunction.address;
            return true;
        }
        itr++;
    }
    return false;
}

u32 HLE_DetectFunctionSize(u32 addr)
{
	u32 CurAddr;
	u32 BranchAddr;
	u32 OpCode;
	u32	NewBranchAddr;

	//figure out the size of a function
	CurAddr = addr;
	BranchAddr = CurAddr;
	for(;;)
	{
		//get an opcode
		OpCode = *(u32 *)&Mem_RAM[CurAddr & RAM_MASK];
		if(!OpCode)
			break;
		else if((OpCode & 0xFC000000) == 0x40000000)
		{
			//a conditional branch, figure out where it is branching
			if(!(OpCode & 0x02))
			{
				//i only care about conditional branches as a function
				//is not going to set the top 16 bits
				NewBranchAddr = CurAddr + (s16)(OpCode & 0xFFFC);
				if(NewBranchAddr > BranchAddr)
					BranchAddr = NewBranchAddr;			//new branch is higher, log where it goes
				else if(NewBranchAddr < addr)
					break;								//shouldn't jump out of our function, exit
				
/*				if(BranchAddr > CurAddr)
				{
					//the branch is higher than the current position, skip ahead
					//offset by 1 opcode due to the auto add below
					CurAddr = BranchAddr - 4;
					OpCode = 0;
				}
*/			}
		}
		else if(CurAddr >= BranchAddr)
		{
			if((OpCode & 0xFC000000) == 0x48000000)
			{
				//straight branch, check for a mflr r0 immediately after which could indicate
				//the start of another function
				if(*(u32*)&Mem_RAM[(CurAddr + 4) & RAM_MASK] == 0x7C0802A6)
					break;
				else if(((OpCode & 3) == 0) && (CurAddr + (s16)(OpCode & 0x3FFFFFC) < addr))
					break;		//branch outside of the start without a return
			}
			else if(OpCode == 0x4E800020 || OpCode == 0x4E800021)
			{
				//blr
				break;
			}
			else if(OpCode == 0x4C000064)
			{
				//rfi
				//some map files indicate a nop after the rfi, if there is a nop then count it
				if(*(u32 *)&Mem_RAM[(CurAddr + 4) & RAM_MASK] == 0x60000000)
					CurAddr += 4;

				break;
			}
		}

		//next opcode
		CurAddr += 4;
	}

	//return the size of the function
	return CurAddr - addr + 4;
}

u32 HLE_GenerateFunctionCRC(u32 Addr, u32 FuncSize)
{
	//generate a CRC of the function specified

	u32		ulCRC = -1;
	u32		CurOp;

	//calculate 2 opcodes at a time
	for(; FuncSize > 7; FuncSize-=8)
	{
		CurOp = *(u32 *)&Mem_RAM[Addr & RAM_MASK];
		CurOp = CurOp & hle_op_mask[CurOp >> 26];
		ulCRC ^= CurOp;
		ulCRC = crc32_table[3][((ulCRC) & 0xFF)] ^
			    crc32_table[2][((ulCRC >> 8) & 0xFF)] ^
			    crc32_table[1][((ulCRC >> 16) & 0xFF)] ^
				crc32_table[0][((ulCRC >> 24))];
		CurOp = *(u32 *)&Mem_RAM[Addr & RAM_MASK];
		CurOp = CurOp & hle_op_mask[CurOp >> 26];
		ulCRC ^= CurOp;
		ulCRC = crc32_table[3][((ulCRC) & 0xFF)] ^
			    crc32_table[2][((ulCRC >> 8) & 0xFF)] ^
			    crc32_table[1][((ulCRC >> 16) & 0xFF)] ^
				crc32_table[0][((ulCRC >> 24))];
		Addr+=8;
	}

	//if anything left then handle it
	if(FuncSize)
	{
		CurOp = *(u32 *)&Mem_RAM[Addr & RAM_MASK];
		CurOp = CurOp & hle_op_mask[CurOp >> 26];
		ulCRC ^= CurOp;
		ulCRC = crc32_table[3][((ulCRC) & 0xFF)] ^
			    crc32_table[2][((ulCRC >> 8) & 0xFF)] ^
			    crc32_table[1][((ulCRC >> 16) & 0xFF)] ^
				crc32_table[0][((ulCRC >> 24))];
	}

	return ulCRC;
}

void HLE_FindFuncsAndGenerateCRCs()
{
	u32			Addr;
	u32			FuncSize;
	u32			FuncCRC;
	Function	NewFunc;

	return;

	//go thru memory detecting functions and generate CRCs
	//add the functions to the function list if they do not already exist
	
	for(Addr = 0x80002000; Addr < 0x81000000; Addr+= 4)
	{
		//see if we have something other than 0x00000000
		if(*(u32 *)&Mem_RAM[Addr & RAM_MASK])
		{
			//have a value, detect the function
			FuncSize = HLE_DetectFunctionSize(Addr);
			FuncCRC = HLE_GenerateFunctionCRC(Addr, FuncSize);

			memset(&NewFunc, 0, sizeof(NewFunc));

			//see if the value exists
			if(maps[Addr].address)
				NewFunc = maps[Addr];

			NewFunc.address = Addr;
			NewFunc.CRC = FuncCRC;
			NewFunc.DetectedSize = FuncSize;
			maps[Addr] = NewFunc;

			Addr += FuncSize - 4;
		}
	}

	HANDLE	fHandle = CreateFile("e:\\gekko\\memcrc-mismatch.txt", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	HANDLE	fHandle2 = CreateFile("e:\\gekko\\memcrc-found.txt", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	char	fData[1024];
	DWORD	dataOut;
	long	TotalFound, FoundMatch, MismatchFound, FoundNoMap;

	TotalFound = FoundMatch = MismatchFound = FoundNoMap = 0;

    std::map<u32, Function>::const_iterator itr = maps.begin();
    while(itr != maps.end())
    {
        Function rFunction = itr->second;
        
		if(rFunction.funcSize == 0xcccccccc)
		{
			FoundNoMap++;
			rFunction.funcSize = 0;
		}

		if(rFunction.funcSize != rFunction.DetectedSize)
		{
			MismatchFound++;
			sprintf(fData, "0x%08X:\t%s\t0x%08X/0x%08X\t0x%08X\n", rFunction.address, rFunction.funcName.c_str(), rFunction.funcSize, rFunction.DetectedSize, rFunction.CRC);
			WriteFile(fHandle, fData, strlen(fData), &dataOut, 0);
		}
		else
		{
			sprintf(fData, "%s\t%08X\t%08X\n", rFunction.funcName.c_str(), rFunction.DetectedSize, rFunction.CRC);
			WriteFile(fHandle2, fData, strlen(fData), &dataOut, 0);
			FoundMatch++;
		}

		TotalFound++;
        itr++;
    }

	CloseHandle(fHandle);
	CloseHandle(fHandle2);
	printf("HLE CRC Stats:\n");
	printf("\t%d total found\n", TotalFound);
	printf("\t%d found with map\n", TotalFound - FoundNoMap);
	printf("\t%d matches\n", FoundMatch);
	printf("\t%d total mismatches\n", MismatchFound);
	if((MismatchFound - FoundNoMap) <= 0)
		MismatchFound = FoundNoMap;
	printf("\t%d mismatches with map\n", MismatchFound - FoundNoMap);
	printf("Map CRC dump complete\n");
}

////////////////////////////////////////////////////////////