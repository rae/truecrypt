/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#include "Platform.h"
#include "BootMemory.h"
#include "BootConfig.h"
#include "BootConsoleIo.h"
#include "BootDebug.h"
#include "BootDefs.h"
#include "BootDiskIo.h"
#include "BootEncryptedIo.h"
#include "IntFilter.h"

static byte InterruptStack[4096];

static uint32 OriginalInt13Handler;
static uint32 OriginalInt15Handler;

static Registers IntRegisters;


bool Int13Filter ()
{
	Registers regs;
	memcpy (&regs, &IntRegisters, sizeof (regs));
	__asm sti

	static int ReEntryCount = -1;
	++ReEntryCount;

	byte function = (byte) (regs.AX >> 8);

#ifdef TC_TRACE_INT13
	DisableScreenOutput();

	PrintHex (function);

	Print (" EN:"); Print (ReEntryCount);
	Print (" SS:"); PrintHex (regs.SS);

	PrintChar (' '); PrintHex ((uint16) InterruptStack);
	uint16 spdbg;
	__asm mov spdbg, sp
	PrintChar ('<'); PrintHex ((uint16) spdbg);
	PrintChar ('>'); PrintHex ((uint16) InterruptStack + sizeof (InterruptStack));

#endif

	bool passOriginalRequest = true;

	switch (function)
	{
	case 0x2: // Read sectors
	case 0x3: // Write sectors
		{
			byte drive = (byte) regs.DX;

			ChsAddress chs;
			chs.Cylinder = ((regs.CX << 2) & 0x300) | (regs.CX >> 8);
			chs.Head = regs.DX >> 8;
			chs.Sector = regs.CX & 0x3f;

			byte sectorCount = (byte) regs.AX;

#ifdef TC_TRACE_INT13
			PrintVal (": Drive", drive - TC_FIRST_BIOS_DRIVE, false);
			Print (" Chs: "); Print (chs);
#endif

			uint64 sector;
			if (drive == BootDrive)
			{
				ChsToLba (BootDriveGeometry, chs, sector);
#ifdef TC_TRACE_INT13
				PrintVal (" Sec", sector.LowPart, false);
#endif
			}

#ifdef TC_TRACE_INT13
			PrintVal (" Count", sectorCount, false);
			Print (" Buf: "); PrintHex (regs.ES); PrintChar (':'); PrintHex (regs.BX);
			PrintEndl();
#endif

			if (ReEntryCount == 0 && drive == EncryptedVirtualPartition.Drive)
			{
				BiosResult result;
				
				if (function == 0x3)
					result = WriteEncryptedSectors (regs.ES, regs.BX, drive, sector, sectorCount);
				else
					result = ReadEncryptedSectors (regs.ES, regs.BX, drive, sector, sectorCount);

				__asm cli

				memcpy (&IntRegisters, &regs, sizeof (regs));
				IntRegisters.AX = (uint16) result << 8;

				if (result == BiosResultSuccess)
				{
					IntRegisters.AX |= sectorCount;
					IntRegisters.Flags &= ~TC_X86_CARRY_FLAG;
				}
				else
					IntRegisters.Flags |= TC_X86_CARRY_FLAG;

				passOriginalRequest = false;
			}
		}
		break;

	case 0x42: // Read sectors LBA
	case 0x43: // Write sectors LBA
		{
			byte drive = (byte) regs.DX;
			
			BiosLbaPacket lba;
			CopyMemory (regs.DS, regs.SI, (byte *) &lba, sizeof (lba));

#ifdef TC_TRACE_INT13
			PrintVal (": Drive", drive - TC_FIRST_BIOS_DRIVE, false);
			PrintVal (" Sec", lba.Sector.LowPart, false);
#endif
			
			if (drive == BootDrive)
			{
				ChsAddress chs;
				LbaToChs (BootDriveGeometry, lba.Sector, chs);
#ifdef TC_TRACE_INT13
				Print (" Chs: ");
				Print (chs);
#endif
			}

#ifdef TC_TRACE_INT13
			PrintVal (" Count", lba.SectorCount, false);
			PrintVal (" Buf", lba.Buffer, false, true);
			PrintEndl();
#endif

			if (ReEntryCount == 0 && drive == EncryptedVirtualPartition.Drive)
			{
				BiosResult result;
				
				uint16 segment = (uint16) (lba.Buffer >> 16);
				uint16 offset = (uint16) lba.Buffer;

				if (function == 0x43)
					result = WriteEncryptedSectors (segment, offset, drive, lba.Sector, lba.SectorCount);
				else
					result = ReadEncryptedSectors (segment, offset, drive, lba.Sector, lba.SectorCount);

				__asm cli

				memcpy (&IntRegisters, &regs, sizeof (regs));
				IntRegisters.AX = (IntRegisters.AX & 0xff) | ((uint16) result << 8);

				if (result == BiosResultSuccess)
					IntRegisters.Flags &= ~TC_X86_CARRY_FLAG;
				else
					IntRegisters.Flags |= TC_X86_CARRY_FLAG;

				passOriginalRequest = false;
			}
		}
		break;

	default:
#ifdef TC_TRACE_INT13
		PrintEndl();
#endif
		break;
	}

#ifdef TC_TRACE_INT13
	EnableScreenOutput();
#endif
	--ReEntryCount;

	return passOriginalRequest;
}


#define TC_MAX_MEMORY_MAP_SIZE 32

BiosMemoryMapEntry BiosMemoryMap[TC_MAX_MEMORY_MAP_SIZE];
static size_t BiosMemoryMapSize;


static void CreateBootLoaderMemoryMapEntry (BiosMemoryMapEntry *newMapEntry, uint32 bootLoaderStart)
{
	newMapEntry->Type = 0x2;
	newMapEntry->BaseAddress.HighPart = 0;
	newMapEntry->BaseAddress.LowPart = bootLoaderStart;
	newMapEntry->Length.HighPart = 0;
	newMapEntry->Length.LowPart = TC_BOOT_MEMORY_REQUIRED * 1024UL;
}


static bool CreateNewBiosMemoryMap ()
{
	// Create a new BIOS memory map presenting the memory area of the loader as reserved

	BiosMemoryMapSize = 0;
	BiosMemoryMapEntry entry;
	BiosMemoryMapEntry *newMapEntry = BiosMemoryMap;

	const BiosMemoryMapEntry *mapEnd = BiosMemoryMap + TC_MAX_MEMORY_MAP_SIZE;

	uint64 bootLoaderStart;
	bootLoaderStart.HighPart = 0;

	uint16 codeSeg;
	__asm mov codeSeg, cs
	bootLoaderStart.LowPart = GetLinearAddress (codeSeg, 0);

	uint64 bootLoaderEnd;
	bootLoaderEnd.HighPart = 0;
	bootLoaderEnd.LowPart = bootLoaderStart.LowPart + TC_BOOT_MEMORY_REQUIRED * 1024UL;

	bool loaderEntryInserted = false;

	if (GetFirstBiosMemoryMapEntry (entry))
	{
		do
		{
			uint64 entryEnd = entry.BaseAddress + entry.Length;

			if (entry.Type == 0x1 && RegionsIntersect (bootLoaderStart, TC_BOOT_MEMORY_REQUIRED * 1024UL, entry.BaseAddress, entryEnd - 1))
			{
				// Free map entry covers the boot loader area

				if (entry.BaseAddress < bootLoaderStart)
				{
					// Create free entry below the boot loader area
					if (newMapEntry >= mapEnd)
						goto mapOverflow;

					*newMapEntry = entry;
					newMapEntry->Length = bootLoaderStart - entry.BaseAddress;
					++newMapEntry;
				}

				if (!loaderEntryInserted)
				{
					// Create reserved entry for the boot loader if it has not been done yet
					if (newMapEntry >= mapEnd)
						goto mapOverflow;

					CreateBootLoaderMemoryMapEntry (newMapEntry, bootLoaderStart.LowPart);
					++newMapEntry;
					loaderEntryInserted = true;
				}

				if (bootLoaderEnd < entryEnd)
				{
					// Create free entry above the boot loader area
					if (newMapEntry >= mapEnd)
						goto mapOverflow;

					newMapEntry->Type = 0x1;
					newMapEntry->BaseAddress = bootLoaderEnd;
					newMapEntry->Length = entryEnd - bootLoaderEnd;
					++newMapEntry;
				}
			}
			else
			{
				if (newMapEntry >= mapEnd)
					goto mapOverflow;

				if (!loaderEntryInserted && entry.BaseAddress > bootLoaderStart)
				{
					// Create reserved entry for the boot loader if it has not been done yet
					CreateBootLoaderMemoryMapEntry (newMapEntry, bootLoaderStart.LowPart);
					++newMapEntry;
					loaderEntryInserted = true;
				}

				// Copy map entry
				*newMapEntry++ = entry;
			}

		} while (GetNextBiosMemoryMapEntry (entry));
	}

	BiosMemoryMapSize = newMapEntry - BiosMemoryMap;
	return true;

mapOverflow:
	PrintError ("BIOS memory map too long");
	return false;
}


bool Int15Filter ()
{
#ifdef TC_TRACE_INT15
	DisableScreenOutput();

	Print ("15-");
	PrintHex (IntRegisters.AX);

	Print (" SS:"); PrintHex (IntRegisters.SS);

	PrintChar (' '); PrintHex ((uint16) InterruptStack);
	uint16 spdbg;
	__asm mov spdbg, sp
		PrintChar ('<'); PrintHex ((uint16) spdbg);
	PrintChar ('>'); PrintHex ((uint16) InterruptStack + sizeof (InterruptStack));
	PrintEndl();

	Print ("EAX:"); PrintHex (IntRegisters.EAX);
	Print (" EBX:"); PrintHex (IntRegisters.EBX);
	Print (" ECX:"); PrintHex (IntRegisters.ECX);
	Print (" EDX:"); PrintHex (IntRegisters.EDX);
	Print (" DI:"); PrintHex (IntRegisters.DI);
	PrintEndl();

#endif

	if (IntRegisters.EBX >= BiosMemoryMapSize)
	{
		IntRegisters.Flags |= TC_X86_CARRY_FLAG;
		IntRegisters.EBX = 0;
		IntRegisters.AX = -1;
	}
	else
	{
		CopyMemory ((byte *) &BiosMemoryMap[IntRegisters.EBX], IntRegisters.ES, IntRegisters.DI, sizeof (BiosMemoryMap[0]));

		IntRegisters.Flags &= ~TC_X86_CARRY_FLAG;
		IntRegisters.EAX = 0x534D4150UL;

		++IntRegisters.EBX;
		if (IntRegisters.EBX >= BiosMemoryMapSize)
			IntRegisters.EBX = 0;

		IntRegisters.ECX = sizeof (BiosMemoryMap[0]);
	}

#ifdef TC_TRACE_INT15
	BiosMemoryMapEntry entry;
	CopyMemory (IntRegisters.ES, IntRegisters.DI, (byte *) &entry, sizeof (entry));
	PrintHex (entry.Type); PrintChar (' ');
	PrintHex (entry.BaseAddress); PrintChar (' ');
	PrintHex (entry.Length); PrintChar (' ');
	PrintHex (entry.BaseAddress + entry.Length); PrintEndl();

	Print ("EAX:"); PrintHex (IntRegisters.EAX);
	Print (" EBX:"); PrintHex (IntRegisters.EBX);
	Print (" ECX:"); PrintHex (IntRegisters.ECX);
	Print (" EDX:"); PrintHex (IntRegisters.EDX);
	Print (" DI:"); PrintHex (IntRegisters.DI);
	Print (" FL:"); PrintHex (IntRegisters.Flags);
	PrintEndl (2);
#endif

#ifdef TC_TRACE_INT15
	EnableScreenOutput();
#endif
	return false;
}


void IntFilterEntry ()
{
	// No automatic variables should be used in this scope as SS may change
	static uint16 OrigStackPointer;
	static uint16 OrigStackSegment;

	__asm
	{
		pushf
		pushad

		cli
		mov cs:IntRegisters.DI, di

		lea di, cs:IntRegisters.EAX
		TC_ASM_EMIT4 (66,2E,89,05) // mov [cs:di], eax
		lea di, cs:IntRegisters.EBX
		TC_ASM_EMIT4 (66,2E,89,1D) // mov [cs:di], ebx
		lea di, cs:IntRegisters.ECX
		TC_ASM_EMIT4 (66,2E,89,0D) // mov [cs:di], ecx
		lea di, cs:IntRegisters.EDX
		TC_ASM_EMIT4 (66,2E,89,15) // mov [cs:di], edx

		mov ax, [bp + 8]
		mov cs:IntRegisters.Flags, ax

		mov cs:IntRegisters.SI, si
		mov si, [bp + 2] // Int number

		mov cs:IntRegisters.DS, ds
		mov cs:IntRegisters.ES, es
		mov cs:IntRegisters.SS, ss

		// Compiler assumes SS == DS - use private stack if this condition is not met
		mov ax, ss
		mov bx, cs
		cmp ax, bx
		jz stack_ok

		mov cs:OrigStackPointer, sp
		mov cs:OrigStackSegment, ss
		mov ax, cs
		mov ss, ax
		lea ax, InterruptStack + SIZE InterruptStack
		mov sp, ax

	stack_ok:
		// DS = CS
		push ds
		push es
		mov ax, cs
		mov ds, ax
		mov es, ax

		push si // Int number

		// Filter request
		cmp si, 0x15
		je filter15
		cmp si, 0x13
		jne $

		call Int13Filter
		jmp s0

	filter15:
		call Int15Filter

	s0:
		pop si // Int number
		pop es
		pop ds

		// Restore original SS:SP if the private stack is empty
		cli
		lea bx, InterruptStack + SIZE InterruptStack
		cmp bx, sp
		jnz stack_in_use

		mov ss, cs:OrigStackSegment
		mov sp, cs:OrigStackPointer
	stack_in_use:

		test ax, ax // passOriginalRequest
		jnz pass_request

		// Return results of filtered request
		popad
		popf
		mov ax, cs:IntRegisters.Flags
		mov [bp + 8], ax
		leave

		lea di, cs:IntRegisters.EAX
		TC_ASM_EMIT4 (66,2E,8B,05) // mov eax, [cs:di]
		lea di, cs:IntRegisters.EBX
		TC_ASM_EMIT4 (66,2E,8B,1D) // mov ebx, [cs:di]
		lea di, cs:IntRegisters.ECX
		TC_ASM_EMIT4 (66,2E,8B,0D) // mov ecx, [cs:di]
		lea di, cs:IntRegisters.EDX
		TC_ASM_EMIT4 (66,2E,8B,15) // mov edx, [cs:di]

		mov di, cs:IntRegisters.DI
		mov si, cs:IntRegisters.SI
		mov es, cs:IntRegisters.ES
		mov ds, cs:IntRegisters.DS

		sti
		add sp, 2
		iret

		// Pass original request
	pass_request:
		sti
		cmp si, 0x15
		je pass15
		cmp si, 0x13
		jne $

		popad
		popf
		leave
		add sp, 2
		jmp cs:OriginalInt13Handler	

	pass15:
		popad
		popf
		leave
		add sp, 2
		jmp cs:OriginalInt15Handler
	}
}


void Int13FilterEntry ()
{
	__asm
	{
		leave
		push 0x13
		jmp IntFilterEntry
	}
}


static void Int15FilterEntry ()
{
	__asm
	{
		pushf
		cmp ax, 0xe820 // Get system memory map
		je filter
		
		popf
		leave
		jmp cs:OriginalInt15Handler

	filter:
		leave
		push 0x15
		jmp IntFilterEntry
	}
}


bool InstallInterruptFilters ()
{
	if (!CreateNewBiosMemoryMap())
		return false;

	__asm
	{
		cli
		push es

		// Save original INT 13 handler
		xor ax, ax
		mov es, ax
		
		mov si, 0x13 * 4
		lea di, OriginalInt13Handler

		mov ax, es:[si]
		mov [di], ax
		mov ax, es:[si + 2]
		mov [di + 2], ax
		
		// Install INT 13 filter
		lea ax, Int13FilterEntry
		mov es:[si], ax
		mov es:[si + 2], cs

		// Save original INT 15 handler
		mov si, 0x15 * 4	
		lea di, OriginalInt15Handler

		mov ax, es:[si]
		mov [di], ax
		mov ax, es:[si + 2]
		mov [di + 2], ax

		// Install INT 15 filter
		lea ax, Int15FilterEntry
		mov es:[si], ax
		mov es:[si + 2], cs

		// If the BIOS does not support system memory map (INT15 0xe820),
		// set amount of available memory to CS:0000 - 0:0000
		cmp BiosMemoryMapSize, 1
		jg mem_map_ok
		mov ax, cs
		shr ax, 10 - 4		// CS * 16 / 1024
		mov es:[0x413], ax	// = KBytes available
	mem_map_ok:

		pop es
		sti
	}

	return true;
}
