/* Nintendulator Mapper DLLs
 * Copyright (C) QMT Productions
 */
//added by rveach

#include	"..\DLL\d_iNES.h"
#include	"..\Hardware\h_MMC3.h"

namespace
{

const uint8_t protection[4] = {0x83,0x83,0x42,0x00};

FCPURead _Read5;

uint8_t exCtrl, exLast;
BOOL exMode, suppressIrq;

void	Sync (void)
{
	if (ROM->INES_Flags & 0x08)
		EMU->Mirror_4S();
	else
		MMC3::SyncMirror();

	MMC3::SyncWRAM();	// assume WRAM is here

	if (exCtrl & 0x80) {
		if (exCtrl & 0x20)
			EMU->SetPRG_ROM32(0x8, (exCtrl & 0x1F) >> 2);
		else {
			EMU->SetPRG_ROM16(0x8, exCtrl & 0x1F);
			EMU->SetPRG_ROM16(0xC, exCtrl & 0x1F);
		}
	} else {
		MMC3::SyncPRG(0x3F, 0);
	}

	if (ROM->INES_CHRSize) {
		MMC3::SyncCHR_ROM(0xFF, 0);
	} else
		MMC3::SyncCHR_RAM(0xFF, 0);
}

int	MAPINT	SaveLoad (STATE_TYPE mode, int x, unsigned char *data)
{
	SAVELOAD_BYTE(mode, x, data, exCtrl);
	SAVELOAD_BYTE(mode, x, data, exLast);
	SAVELOAD_BYTE(mode, x, data, exMode);
	SAVELOAD_BYTE(mode, x, data, suppressIrq);
	
	return MMC3::SaveLoad(mode, x, data);
}

int	MAPINT	Read5 (int Bank, int Addr)
{
	if (Addr) {
		return _Read5(Bank, Addr);
	} else {
		return protection[exLast & 0x3];
	}
}

void	MAPINT	Write5 (int Bank, int Addr, int Val)
{
	exLast = Val;

	if (Addr) {
		if (suppressIrq) {
			suppressIrq = FALSE;

			EMU->GetCPUWriteHandler(0x4)(0x4, 0x017, 0x40);
		}
	} else {
		if (exCtrl != Val) {
			exCtrl = Val;

			Sync();
		}
	}
}
void	MAPINT	Write8 (int Bank, int Addr, int Val)
{
	switch (Addr & 3) {
	case 0:
		exMode = TRUE;
		break;
	case 1:
		if (exMode)
			break;
	case 2:
		return;
	case 3:
		exMode = FALSE;

		if (Val == 0x28 || Val == 0x2A || Val == 0x06) {
			EMU->SetPRG_ROM8(0xA, Val == 0x2A ? 0x0F : 0x1F);
			EMU->SetPRG_ROM8(0xC, Val == 0x06 ? 0x1F : 0x17);
		}

		Sync();
		return;
	}
	
	MMC3::CPUWrite89(Bank, Addr, Val);
}
BOOL	MAPINT	Load (void)
{
	MMC3::Load(Sync, TRUE);

	iNES_SetSRAM();
	return TRUE;
}
void	MAPINT	Reset (RESET_TYPE ResetType)
{
	if (ResetType == RESET_HARD) {
		exCtrl = exLast = 0;
		exMode = FALSE;
		suppressIrq = TRUE;
	}

	MMC3::Reset(ResetType);

	_Read5 = EMU->GetCPUReadHandler(0x5);
	EMU->SetCPUReadHandler(0x5, Read5);
	EMU->SetCPUWriteHandler(0x5, Write5);

	for (int i = 0x8; i < 0xA; i++)
		EMU->SetCPUWriteHandler(i, Write8);
}
void	MAPINT	Unload (void)
{
	MMC3::Unload();
}

} // namespace

const MapperInfo	MapperInfo_187 =
{
	187,
	_T("Mapper 187"),
	COMPAT_PARTIAL,
	Load,
	Reset,
	Unload,
	NULL,
	MMC3::PPUCycle,
	SaveLoad,
	NULL,
	NULL
};
