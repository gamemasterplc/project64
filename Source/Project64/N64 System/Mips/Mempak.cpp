/****************************************************************************
*                                                                           *
* Project 64 - A Nintendo 64 emulator.                                      *
* http://www.pj64-emu.com/                                                  *
* Copyright (C) 2012 Project64. All rights reserved.                        *
*                                                                           *
* License:                                                                  *
* GNU/GPLv2 http://www.gnu.org/licenses/gpl-2.0.html                        *
*                                                                           *
****************************************************************************/
#include "stdafx.h"
static BYTE Mempaks[4][0x8000];
HANDLE hMempakFile[4];

void Mempak::Close(void)
{
	for (int i = 0; i < 4; i++)
	{
		if (hMempakFile[i]) 
		{
			CloseHandle(hMempakFile[i]);
			hMempakFile[i] = NULL;
		}
	}
}

void LoadMempak (int Control)
{
	CPath FileName;
	DWORD dwRead;
	stdstr MempakName;
	bool bFormatMempak = false;

	MempakName.Format("%s_Cont_%d", g_Settings->LoadString(Game_GameName).c_str(), Control + 1);

	FileName.SetDriveDirectory(g_Settings->LoadString(Directory_NativeSave).c_str());
	FileName.SetName(MempakName.c_str());
	FileName.SetExtension("mpk");
	
	if (!FileName.Exists())
	{
		bFormatMempak = true;

		if (!FileName.DirectoryExists())
		{
			FileName.CreateDirectory();
		}

		BYTE Initilize[] = {
			0x81,0x01,0x02,0x03, 0x04,0x05,0x06,0x07, 0x08,0x09,0x0a,0x0b, 0x0C,0x0D,0x0E,0x0F,
			0x10,0x11,0x12,0x13, 0x14,0x15,0x16,0x17, 0x18,0x19,0x1A,0x1B, 0x1C,0x1D,0x1E,0x1F,
			0xFF,0xFF,0xFF,0xFF, 0x05,0x1A,0x5F,0x13, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0x01,0xFF, 0x66,0x25,0x99,0xCD,
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0xFF,0xFF,0xFF,0xFF, 0x05,0x1A,0x5F,0x13, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0x01,0xFF, 0x66,0x25,0x99,0xCD,
			0xFF,0xFF,0xFF,0xFF, 0x05,0x1A,0x5F,0x13, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0x01,0xFF, 0x66,0x25,0x99,0xCD,
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0xFF,0xFF,0xFF,0xFF, 0x05,0x1A,0x5F,0x13, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0xFF,0xFF, 0xFF,0xFF,0x01,0xFF, 0x66,0x25,0x99,0xCD,
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
			0x00,0x71,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03, 0x00,0x03,0x00,0x03,
		};

		memcpy(&Mempaks[Control][0], Initilize, 0x110);

		for (int count = 0x110; count < 0x8000; count += 2) 
		{
			Mempaks[Control][count] = 0x00;
			Mempaks[Control][count + 1] = 0x03;
		}
	}

	hMempakFile[Control] = CreateFile(FileName, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);

	if (hMempakFile[Control] == INVALID_HANDLE_VALUE)
	{
		WriteTraceF(TraceError,__FUNCTION__ ": Failed to open (%s), lastError = %X",(LPCTSTR)FileName, GetLastError());
		return;
	}

	SetFilePointer(hMempakFile[Control], 0, NULL, FILE_BEGIN);
	if (bFormatMempak)
	{
		WriteFile(hMempakFile[Control], Mempaks[Control], 0x8000, &dwRead, NULL);
	}
	else
	{
		ReadFile(hMempakFile[Control], Mempaks[Control], 0x8000, &dwRead, NULL);
	}
}

BYTE Mempak::CalculateCrc(BYTE * DataToCrc)
{
	DWORD Count;
	DWORD XorTap;

	int Length;
	BYTE CRC = 0;

	for (Count = 0; Count < 0x21; Count++)
	{
		for (Length = 0x80; Length >= 1; Length >>= 1)
		{
			XorTap = (CRC & 0x80) ? 0x85 : 0;
			CRC <<= 1;
			if (Count == 0x20)
			{
				CRC &= 0xFF;
			}
			else
			{
				if ((*DataToCrc & Length) != 0)
				{
					CRC |= 1;
				}
			}
			CRC ^= XorTap;
		}
		DataToCrc++;
	}

	return CRC;
}

void Mempak::ReadFrom(int Control, int Address, BYTE * Buffer)
{
	if (Address < 0x8000)
	{
		if (hMempakFile[Control] == NULL)
		{
			LoadMempak(Control);
		}
		memcpy(Buffer, &Mempaks[Control][Address], 0x20);
	}
	else
	{
		memset(Buffer, 0, 0x20);
		/* Rumble pack area */
	}

	Buffer[0x20] = CalculateCrc(Buffer);
}

void Mempak::WriteTo(int Control, int Address, BYTE * Buffer)
{
	DWORD dwWritten;

	if (Address < 0x8000)
	{
		if (hMempakFile[Control] == NULL)
		{
			LoadMempak(Control);
		}
		memcpy(&Mempaks[Control][Address], Buffer, 0x20);

		SetFilePointer(hMempakFile[Control], 0,NULL,FILE_BEGIN);
		WriteFile(hMempakFile[Control], &Mempaks[Control][0], 0x8000, &dwWritten, NULL);
	}
	else
	{
		/* Rumble pack area */
	}
	Buffer[0x20] = CalculateCrc(Buffer);
}
