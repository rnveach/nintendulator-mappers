/* Nintendulator Mapper DLLs
 * Copyright (C) QMT Productions
 */
//added by rveach

#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_MMC3.h"

// need access to these internals, since I don't feel like copy/pasting the entire MMC3 here
namespace MMC3
{
extern uint8_t Cmd;
}

namespace
{

const uint8_t protection[4] = {0x83,0x83,0x42,0x00};

FCPURead _Read4;
FCPUWrite _Write4;

uint16_t Regs[2];

void	Sync (void)
{
	if (ROM->INES_Flags & 0x08)
		EMU->Mirror_4S();
	else
		MMC3::SyncMirror();

	MMC3::SyncPRG(0x3F, 0);
	
	if (ROM->INES_CHRSize) {
		uint8_t position = (MMC3::Cmd >> 7) & 1;
		int i;

		for (i = 0; i < 4; i++)
			EMU->SetCHR_ROM1(i, MMC3::GetCHRBank(i) + Regs[position ^ 0]);
		for (; i < 8; i++)
			EMU->SetCHR_ROM1(i, MMC3::GetCHRBank(i) + Regs[position ^ 1]);
	} else
		MMC3::SyncCHR_RAM(0xFF, 0);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int x, unsigned char *data)
{
	SAVELOAD_WORD(mode, x, data, Regs[0]);
	SAVELOAD_WORD(mode, x, data, Regs[1]);
	
	return MMC3::SaveLoad(mode, x, data);
}

int	MAPINT	Read4 (int Bank, int Addr)
{
	if ((Bank == 5) || (Addr >= 0x100))
		return 1;

	return _Read4(Bank, Addr);
}

void	MAPINT	Write4 (int Bank, int Addr, int Val)
{
	if ((Bank == 5) || (Addr >= 0x100)) {
		Regs[0] = (Val & 0x01) << 8;
		Regs[1] = (Val & 0x10) << 4;

		Sync();
	} else
		_Write4(Bank, Addr, Val);
}
BOOL	MAPINT	Load (void)
{
	MMC3::Load(Sync, TRUE);
	return TRUE;
}
void	MAPINT	Reset (RESET_TYPE ResetType)
{
	if (ResetType == RESET_HARD) {
		Regs[0] = Regs[1] = 0;
	}

	MMC3::Reset(ResetType);

	_Read4 = EMU->GetCPUReadHandler(0x4);
	_Write4 = EMU->GetCPUWriteHandler(0x4);

	EMU->SetCPUReadHandler(0x4, Read4);
	EMU->SetCPUReadHandler(0x5, Read4);
	EMU->SetCPUReadHandler(0x6, Read4);
	EMU->SetCPUReadHandler(0x7, Read4);

	EMU->SetCPUWriteHandler(0x4, Write4);
	EMU->SetCPUWriteHandler(0x5, Write4);
}
void	MAPINT	Unload (void)
{
	MMC3::Unload();
}

} // namespace

const MapperInfo	MapperInfo_012 =
{
	12,
	_T("Mapper 12"),
	COMPAT_FULL,
	Load,
	Reset,
	Unload,
	NULL,
	MMC3::PPUCycle,
	SaveLoad,
	NULL,
	NULL
};
