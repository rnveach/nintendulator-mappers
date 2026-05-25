/* Nintendulator Mapper DLLs
 * Copyright (C) QMT Productions
 */
//added by rveach

#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace
{

uint8_t PRG;
uint16_t Address;

void	Sync (void)
{
	if (ROM->INES_Flags & 0x02)
		EMU->SetPRG_RAM8(0x6, 0);

	if (PRG) {	//C000+
		EMU->SetPRG_ROM32(0x8, (Address & 0xFF) >> 1);
	} else {						//<C000
		EMU->SetPRG_ROM16(0x8, Address & 0xFF);
		EMU->SetPRG_ROM16(0xC, Address & 0xFF);
	}
	
	EMU->SetCHR_ROM8(0, Address & 0xFF);

	if (Address & 0x08)
		EMU->Mirror_A11();
	else
		EMU->Mirror_A10();
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int x, unsigned char *data)
{
	SAVELOAD_BYTE(mode, x, data, PRG);
	SAVELOAD_WORD(mode, x, data, Address);

	if (mode == STATE_LOAD)
		Sync();

	return x;
}

void	MAPINT	Write8 (int Bank, int Addr, int Val)
{
	Bank = 0;
	Address = (uint16_t)Addr;
	
	Sync();
}

void	MAPINT	WriteC (int Bank, int Addr, int Val)
{
	Bank = 1;
	Address = (uint16_t)Addr;
	
	Sync();
}

BOOL	MAPINT	Load (void)
{
	iNES_SetSRAM();
	return TRUE;
}
void	MAPINT	Reset (RESET_TYPE ResetType)
{
	int i;
	
	for (i = 0x8; i < 0xC; i++)
		EMU->SetCPUWriteHandler(i, Write8);
	for (; i < 0x10; i++)
		EMU->SetCPUWriteHandler(i, WriteC);

	if (ResetType == RESET_HARD)
	{
		PRG = 1;
		Address = -1;
	}

	Sync();
}

} // namespace

const MapperInfo	MapperInfo_212 =
{
	212,
	_T("Mapper 212"),
	COMPAT_FULL,
	Load,
	Reset,
	NULL,
	NULL,
	NULL,
	SaveLoad,
	NULL,
	NULL
};
