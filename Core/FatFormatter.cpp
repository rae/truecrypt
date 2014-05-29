/*
 Legal Notice: Some portions of the source code contained in this file were
 derived from the source code of Encryption for the Masses 2.02a, which is
 Copyright (c) 1998-2000 Paul Le Roux and which is governed by the 'License
 Agreement for Encryption for the Masses'. Modifications and additions to
 the original source code (contained in this file) and all other portions of
 this file are Copyright (c) 2003-2008 TrueCrypt Foundation and are governed
 by the TrueCrypt License 2.4 the full text of which is contained in the
 file License.txt included in TrueCrypt binary and source code distribution
 packages. */

#include <time.h>
#include "Common/Tcdefs.h"
#include "Platform/Platform.h"
#include "FatFormatter.h"

namespace TrueCrypt
{
	struct fatparams
	{
		char volume_name[11];
		uint32 num_sectors;	/* total number of sectors */
		uint32 cluster_count;	/* number of clusters */
		uint32 size_root_dir;	/* size of the root directory in bytes */
		uint32 size_fat;		/* size of FAT */
		uint32 fats;
		uint32 create_time;
		uint32 media;
		uint32 cluster_size;
		uint32 fat_length;
		uint16 dir_entries;
		uint16 sector_size;
		uint32 hidden;
		uint16 reserved;
		uint16 sectors;
		uint32 total_sect;

		uint16 heads;
		uint16 secs_track;

	};

	static void GetFatParams (fatparams * ft)
	{
		uint32 fatsecs;
		if(ft->cluster_size == 0)	// 'Default' cluster size
		{
			if (ft->num_sectors * 512LL >= 256*BYTES_PER_GB)
				ft->cluster_size = 128;
			else if (ft->num_sectors * 512LL >= 64*BYTES_PER_GB)
				ft->cluster_size = 64;
			else if (ft->num_sectors * 512LL >= 16*BYTES_PER_GB)
				ft->cluster_size = 32;
			else if (ft->num_sectors * 512LL >= 8*BYTES_PER_GB)
				ft->cluster_size = 16;
			else if (ft->num_sectors * 512LL >= 128*BYTES_PER_MB)
				ft->cluster_size = 8;
			else if (ft->num_sectors * 512LL >= 64*BYTES_PER_MB)
				ft->cluster_size = 4;
			else if (ft->num_sectors * 512LL >= 32*BYTES_PER_MB)
				ft->cluster_size = 2;
			else
				ft->cluster_size = 1;
		}

		// Geometry always set to SECTORS/1/1
		ft->secs_track = 1; 
		ft->heads = 1; 

		ft->dir_entries = 512;
		ft->fats = 2;
		ft->create_time = (uint32) time (NULL);
		ft->media = 0xf8;
		ft->sector_size = SECTOR_SIZE;
		ft->hidden = 0;

		ft->size_root_dir = ft->dir_entries * 32;

		// FAT12
		ft->size_fat = 12;
		ft->reserved = 2;
		fatsecs = ft->num_sectors - (ft->size_root_dir + SECTOR_SIZE - 1) / SECTOR_SIZE - ft->reserved;
		ft->cluster_count = (int) (((int64) fatsecs * SECTOR_SIZE) / (ft->cluster_size * SECTOR_SIZE + 3));
		ft->fat_length = (((ft->cluster_count * 3 + 1) >> 1) + SECTOR_SIZE - 1) / SECTOR_SIZE;

		if (ft->cluster_count >= 4085) // FAT16
		{
			ft->size_fat = 16;
			ft->reserved = 2;
			fatsecs = ft->num_sectors - (ft->size_root_dir + SECTOR_SIZE - 1) / SECTOR_SIZE - ft->reserved;
			ft->cluster_count = (int) (((int64) fatsecs * SECTOR_SIZE) / (ft->cluster_size * SECTOR_SIZE + 4));
			ft->fat_length = (ft->cluster_count * 2 + SECTOR_SIZE - 1) / SECTOR_SIZE;
		}

		if(ft->cluster_count >= 65525) // FAT32
		{
			ft->size_fat = 32;
			ft->reserved = 32;
			fatsecs = ft->num_sectors - ft->reserved;
			ft->size_root_dir = ft->cluster_size * SECTOR_SIZE;
			ft->cluster_count = (int) (((int64) fatsecs * SECTOR_SIZE) / (ft->cluster_size * SECTOR_SIZE + 8));
			ft->fat_length = (ft->cluster_count * 4 + SECTOR_SIZE - 1) / SECTOR_SIZE;
		}

		if (ft->num_sectors >= 65536 || ft->size_fat == 32)
		{
			ft->sectors = 0;
			ft->total_sect = ft->num_sectors;
		}
		else
		{
			ft->sectors = (uint16) ft->num_sectors;
			ft->total_sect = 0;
		}
	}

	static void PutBoot (fatparams * ft, byte *boot)
	{
		int cnt = 0;

		boot[cnt++] = 0xeb;	/* boot jump */
		boot[cnt++] = 0x3c;
		boot[cnt++] = 0x90;
		memcpy (boot + cnt, "MSDOS5.0", 8); /* system id */
		cnt += 8;
		*(int16 *)(boot + cnt) = Endian::Little (ft->sector_size);	/* bytes per sector */
		cnt += 2;
		boot[cnt++] = (int8) ft->cluster_size;			/* sectors per cluster */
		*(int16 *)(boot + cnt) = Endian::Little (ft->reserved);		/* reserved sectors */
		cnt += 2;
		boot[cnt++] = (int8) ft->fats;					/* 2 fats */

		if(ft->size_fat == 32)
		{
			boot[cnt++] = 0x00;
			boot[cnt++] = 0x00;
		}
		else
		{
			*(int16 *)(boot + cnt) = Endian::Little (ft->dir_entries);	/* 512 root entries */
			cnt += 2;
		}

		*(int16 *)(boot + cnt) = Endian::Little (ft->sectors);		/* # sectors */
		cnt += 2;
		boot[cnt++] = (int8) ft->media;					/* media byte */

		if(ft->size_fat == 32)	
		{
			boot[cnt++] = 0x00;
			boot[cnt++] = 0x00;
		}
		else 
		{ 
			*(uint16 *)(boot + cnt) = Endian::Little ((uint16) ft->fat_length);	/* fat size */
			cnt += 2;
		}

		*(int16 *)(boot + cnt) = Endian::Little (ft->secs_track);	/* # sectors per track */
		cnt += 2;
		*(int16 *)(boot + cnt) = Endian::Little (ft->heads);			/* # heads */
		cnt += 2;
		*(int32 *)(boot + cnt) = Endian::Little (ft->hidden);		/* # hidden sectors */
		cnt += 4;
		*(int32 *)(boot + cnt) = Endian::Little (ft->total_sect);	/* # huge sectors */
		cnt += 4;

		if(ft->size_fat == 32)
		{
			*(int32 *)(boot + cnt) = Endian::Little (ft->fat_length); cnt += 4;	/* fat size 32 */
			boot[cnt++] = 0x00;	/* ExtFlags */
			boot[cnt++] = 0x00;
			boot[cnt++] = 0x00;	/* FSVer */
			boot[cnt++] = 0x00;
			boot[cnt++] = 0x02;	/* RootClus */
			boot[cnt++] = 0x00;
			boot[cnt++] = 0x00;
			boot[cnt++] = 0x00;
			boot[cnt++] = 0x01;	/* FSInfo */
			boot[cnt++] = 0x00;
			boot[cnt++] = 0x06;	/* BkBootSec */
			boot[cnt++] = 0x00;
			memset(boot+cnt, 0, 12); cnt+=12;	/* Reserved */
		}

		boot[cnt++] = 0x00;	/* drive number */   // FIXED 80 > 00
		boot[cnt++] = 0x00;	/* reserved */
		boot[cnt++] = 0x29;	/* boot sig */
		*(int32 *)(boot + cnt) = Endian::Little (ft->create_time);	/* vol id */
		cnt += 4;
		memcpy (boot + cnt, ft->volume_name, 11);	/* vol title */
		cnt += 11;

		switch(ft->size_fat) /* filesystem type */
		{
		case 12: memcpy (boot + cnt, "FAT12   ", 8); break;
		case 16: memcpy (boot + cnt, "FAT16   ", 8); break;
		case 32: memcpy (boot + cnt, "FAT32   ", 8); break;
		}
		cnt += 8;

		memset (boot + cnt, 0, ft->size_fat==32 ? 420:448);	/* boot code */
		cnt += ft->size_fat==32 ? 420:448;
		boot[cnt++] = 0x55;
		boot[cnt++] = 0xaa;	/* boot sig */
	}


	/* FAT32 FSInfo */
	static void PutFSInfo (byte *sector)
	{
		memset (sector, 0, 512);
		sector[3] = 0x41; /* LeadSig */
		sector[2] = 0x61; 
		sector[1] = 0x52; 
		sector[0] = 0x52; 
		sector[484+3] = 0x61; /* StrucSig */
		sector[484+2] = 0x41; 
		sector[484+1] = 0x72; 
		sector[484+0] = 0x72; 
		sector[488+3] = 0xff; /* Free_Count */
		sector[488+2] = 0xff;
		sector[488+1] = 0xff;
		sector[488+0] = 0xff;
		sector[492+3] = 0xff; /* Nxt_Free */
		sector[492+2] = 0xff;
		sector[492+1] = 0xff;
		sector[492+0] = 0xff;
		sector[508+3] = 0xaa; /* TrailSig */
		sector[508+2] = 0x55;
		sector[508+1] = 0x00;
		sector[508+0] = 0x00;
	}

	void FatFormatter::Format (WriteSectorCallback &writeSector, uint64 deviceSize, uint32 clusterSize)
	{
		fatparams fatParams;

		if (deviceSize / SECTOR_SIZE > 0xffffFFFF)
			throw ParameterIncorrect (SRC_POS);

		fatParams.num_sectors = (uint32) (deviceSize / SECTOR_SIZE);
		fatParams.cluster_size = clusterSize / SECTOR_SIZE;
		memcpy (fatParams.volume_name, "NO NAME    ", 11);
		GetFatParams (&fatParams); 
		fatparams *ft = &fatParams;

		SecureBuffer sector (SECTOR_SIZE);
		uint32 sectorNumber = 0;

		/* Write the data area */

		sector.Zero();

		PutBoot (ft, (byte *) sector);
		writeSector (sector); ++sectorNumber;

		/* fat32 boot area */
		if (ft->size_fat == 32)				
		{
			/* fsinfo */
			PutFSInfo((byte *) sector);
			writeSector (sector); ++sectorNumber;

			/* reserved */
			while (sectorNumber < 6)
			{
				sector.Zero();
				sector[508+3] = 0xaa; /* TrailSig */
				sector[508+2] = 0x55;
				writeSector (sector); ++sectorNumber;
			}

			/* bootsector backup */
			sector.Zero();
			PutBoot (ft, (byte *) sector);
			writeSector (sector); ++sectorNumber;

			PutFSInfo((byte *) sector);
			writeSector (sector); ++sectorNumber;
		}

		/* reserved */
		while (sectorNumber < (uint32)ft->reserved)
		{
			sector.Zero();
			writeSector (sector); ++sectorNumber;
		}

		/* write fat */
		for (uint32 x = 1; x <= ft->fats; x++)
		{
			for (uint32 n = 0; n < ft->fat_length; n++)
			{
				sector.Zero();

				if (n == 0)
				{
					byte fat_sig[12];
					if (ft->size_fat == 32)
					{
						fat_sig[0] = (byte) ft->media;
						fat_sig[1] = fat_sig[2] = 0xff;
						fat_sig[3] = 0x0f;
						fat_sig[4] = fat_sig[5] = fat_sig[6] = 0xff;
						fat_sig[7] = 0x0f;
						fat_sig[8] = fat_sig[9] = fat_sig[10] = 0xff;
						fat_sig[11] = 0x0f;
						memcpy (sector, fat_sig, 12);
					}				
					else if (ft->size_fat == 16)
					{
						fat_sig[0] = (byte) ft->media;
						fat_sig[1] = 0xff;
						fat_sig[2] = 0xff;
						fat_sig[3] = 0xff;
						memcpy (sector, fat_sig, 4);
					}
					else if (ft->size_fat == 12)
					{
						fat_sig[0] = (byte) ft->media;
						fat_sig[1] = 0xff;
						fat_sig[2] = 0xff;
						fat_sig[3] = 0x00;
						memcpy (sector, fat_sig, 4);
					}
				}

				if (!writeSector (sector))
					return;
			}
		}

		/* write rootdir */
		for (uint32 x = 0; x < ft->size_root_dir / SECTOR_SIZE; x++)
		{
			sector.Zero();
			if (!writeSector (sector))
				return;
		}
	}
}
