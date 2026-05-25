/* Nintendulator Mapper DLLs
 * Copyright (C) QMT Productions
 */
//added by rveach
//not tested

#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_Latch.h"

namespace
{

void	Sync (void)
{
	if (ROM->INES_Flags & 0x02)
		EMU->SetPRG_RAM8(0x6, 0);

	EMU->SetPRG_ROM32(0x8, Latch::Addr.s0 & 0xFF);
	EMU->SetCHR_ROM8(0, Latch::Addr.s0 & 0xFF);
}

BOOL	MAPINT	Load (void)
{
	Latch::Load(Sync, FALSE, TRUE);

	iNES_SetSRAM();
	return TRUE;
}
void	MAPINT	Reset (RESET_TYPE ResetType)
{
	iNES_SetMirroring();

	Latch::Reset(ResetType);
}
void	MAPINT	Unload (void)
{
	Latch::Unload();
}

} // namespace

const MapperInfo	MapperInfo_054 =
{
	54,
	_T("Mapper 54"),
	COMPAT_FULL,
	Load,
	Reset,
	Unload,
	NULL,
	NULL,
	Latch::SaveLoad_A,
	NULL,
	NULL
};
