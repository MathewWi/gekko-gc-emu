// hw_pi.h
// (c) 2005,2008 Gekko Team

#ifndef _HW_PI_H_
#define _HW_PI_H_

////////////////////////////////////////////////////////////////////////////////

#define REGPI16(X)			(*((u16 *) &PIRegisters[REG_SIZE - (X & REG_MASK) - 2]))
#define REGPI32(X)			(*((u32 *) &PIRegisters[REG_SIZE - (X & REG_MASK) - 4]))

extern u8 PIRegisters[REG_SIZE];

////////////////////////////////////////////////////////////////////////////////

#define PI_INTSR				0xCC003000
#define PI_INTMR				0xCC003004
#define PI_FIFO_BASESTART		0xCC00300C
#define PI_FIFO_BASEEND			0xCC003010
#define PI_FIFO_WRITEPOINTER	0xCC003014 
#define PI_CONSOLE_TYPE         0xCC00302c

#define PI_MASK_RSWST			0x00010000
#define PI_MASK_HSP				0x00002000
#define PI_MASK_DEBUG			0x00001000
#define PI_MASK_CP				0x00000800
#define PI_MASK_PEFINISH		0x00000400
#define PI_MASK_PETOKEN			0x00000200
#define PI_MASK_VI				0x00000100
#define PI_MASK_MEM				0x00000080
#define PI_MASK_DSP				0x00000040
#define PI_MASK_AI				0x00000020
#define PI_MASK_EXI				0x00000010
#define PI_MASK_SI				0x00000008
#define PI_MASK_DI				0x00000004
#define PI_MASK_RSW				0x00000002
#define PI_MASK_ERROR			0x00000001

#define PI_FIFO_WRITE_POINTER   REGPI32(PI_FIFO_WRITEPOINTER)

////////////////////////////////////////////////////////////////////////////////

void __fastcall PI_Fifo_Write8(u32 addr, u32 data);
void __fastcall PI_Fifo_Write16(u32 addr, u32 data);
void __fastcall PI_Fifo_Write32(u32 addr, u32 data);

extern inline u32		PI_CheckForInterrupts();
extern inline void	PI_RequestInterrupt(u32 mask);
extern inline void	PI_ClearInterrupt(u32 mask);
void	PI_Open(void);
void	PI_Update(void);

////////////////////////////////////////////////////////////////////////////////

u8		__fastcall	PI_Read8(u32 addr);
void	__fastcall	PI_Write8(u32 addr, u32 data);
u16		__fastcall	PI_Read16(u32 addr);
void	__fastcall	PI_Write16(u32 addr, u32 data);
u32		__fastcall	PI_Read32(u32 addr);
void	__fastcall	PI_Write32(u32 addr, u32 data);

//

#endif
