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

#pragma once

#include "Tcdefs.h"
#include "Common.h"
#include "Crypto.h"
#include "Wipe.h"

#ifdef _WIN32

// Modifying the following values can introduce incompatibility with previous versions

#define TC_IOCTL(CODE) (CTL_CODE (FILE_DEVICE_UNKNOWN, 0x800 + (CODE), METHOD_BUFFERED, FILE_ANY_ACCESS))

#define TC_IOCTL_GET_DRIVER_VERSION						TC_IOCTL (1)
#define TC_IOCTL_GET_BOOT_LOADER_VERSION				TC_IOCTL (2)
#define TC_IOCTL_MOUNT_VOLUME							TC_IOCTL (3)
#define TC_IOCTL_DISMOUNT_VOLUME						TC_IOCTL (4)
#define TC_IOCTL_DISMOUNT_ALL_VOLUMES					TC_IOCTL (5)
#define TC_IOCTL_GET_MOUNTED_VOLUMES					TC_IOCTL (6)
#define TC_IOCTL_GET_VOLUME_PROPERTIES					TC_IOCTL (7)
#define TC_IOCTL_GET_DEVICE_REFCOUNT					TC_IOCTL (8)
#define TC_IOCTL_WAS_REFERENCED_DEVICE_DELETED			TC_IOCTL (9)
#define TC_IOCTL_IS_ANY_VOLUME_MOUNTED					TC_IOCTL (10)
#define TC_IOCTL_GET_PASSWORD_CACHE_STATUS				TC_IOCTL (11)
#define TC_IOCTL_WIPE_PASSWORD_CACHE					TC_IOCTL (12)
#define TC_IOCTL_OPEN_TEST								TC_IOCTL (13)
#define TC_IOCTL_GET_DRIVE_PARTITION_INFO				TC_IOCTL (14)
#define TC_IOCTL_GET_DRIVE_GEOMETRY						TC_IOCTL (15)
#define TC_IOCTL_PROBE_REAL_DRIVE_SIZE					TC_IOCTL (16)
#define TC_IOCTL_GET_RESOLVED_SYMLINK					TC_IOCTL (17)
#define TC_IOCTL_GET_BOOT_ENCRYPTION_STATUS				TC_IOCTL (18)
#define TC_IOCTL_BOOT_ENCRYPTION_SETUP					TC_IOCTL (19)
#define TC_IOCTL_ABORT_BOOT_ENCRYPTION_SETUP			TC_IOCTL (20)
#define TC_IOCTL_GET_BOOT_ENCRYPTION_SETUP_RESULT		TC_IOCTL (21)
#define TC_IOCTL_GET_BOOT_DRIVE_VOLUME_PROPERTIES		TC_IOCTL (22)
#define TC_IOCTL_REOPEN_BOOT_VOLUME_HEADER				TC_IOCTL (23)

// Legacy IOCTLs used before version 5.0
#define TC_IOCTL_LEGACY_GET_DRIVER_VERSION		466968
#define TC_IOCTL_LEGACY_GET_MOUNTED_VOLUMES		466948


/* Start of driver interface structures, the size of these structures may
   change between versions; so make sure you first send DRIVER_VERSION to
   check that it's the correct device driver */

#pragma pack (push)
#pragma pack(1)

typedef struct
{
	int nReturnCode;					/* Return code back from driver */
	short wszVolume[TC_MAX_PATH];		/* Volume to be mounted */
	Password VolumePassword;			/* User password */
	BOOL bCache;						/* Cache passwords in driver */
	int nDosDriveNo;					/* Drive number to mount */
	int BytesPerSector;
	BOOL bMountReadOnly;				/* Mount volume in read-only mode */
	BOOL bMountRemovable;				/* Mount volume as removable media */
	BOOL bExclusiveAccess;				/* Open host file/device in exclusive access mode */
	BOOL bMountManager;					/* Announce volume to mount manager */
	BOOL bUserContext;					/* Mount volume in user process context */
	BOOL bPreserveTimestamp;			/* Preserve file container timestamp */
	// Hidden volume protection
	BOOL bProtectHiddenVolume;			/* TRUE if the user wants the hidden volume within this volume to be protected against being overwritten (damaged) */
	Password ProtectedHidVolPassword;	/* Password to the hidden volume to be protected against overwriting */
} MOUNT_STRUCT;

typedef struct
{
	int nDosDriveNo;	/* Drive letter to unmount */
	BOOL ignoreOpenFiles;
	int nReturnCode;	/* Return code back from driver */
} UNMOUNT_STRUCT;

typedef struct
{
	unsigned __int32 ulMountedDrives;	/* Bitfield of all mounted drive letters */
	short wszVolume[26][TC_MAX_PATH];	/* Volume names of mounted volumes */
	unsigned __int64 diskLength[26];
	int ea[26];
	int volumeType[26];	/* Volume type (e.g. PROP_VOL_TYPE_OUTER, PROP_VOL_TYPE_OUTER_VOL_WRITE_PREVENTED, etc.) */
} MOUNT_LIST_STRUCT;

typedef struct
{
	int driveNo;
	int uniqueId;
	short wszVolume[TC_MAX_PATH];
	unsigned __int64 diskLength;
	int ea;
	int mode;
	int pkcs5;
	int pkcs5Iterations;
	BOOL hiddenVolume;
	BOOL readOnly;
	unsigned __int64 volumeCreationTime;
	unsigned __int64 headerCreationTime;
	unsigned __int64 totalBytesRead;
	unsigned __int64 totalBytesWritten;
	int hiddenVolProtection;	/* Hidden volume protection status (e.g. HIDVOL_PROT_STATUS_NONE, HIDVOL_PROT_STATUS_ACTIVE, etc.) */
} VOLUME_PROPERTIES_STRUCT;

typedef struct
{
	WCHAR symLinkName[TC_MAX_PATH];
	WCHAR targetName[TC_MAX_PATH];
} RESOLVE_SYMLINK_STRUCT;

typedef struct
{
	WCHAR deviceName[TC_MAX_PATH];
	PARTITION_INFORMATION partInfo;
	BOOL IsGPT;
}
DISK_PARTITION_INFO_STRUCT;

typedef struct
{
	WCHAR deviceName[TC_MAX_PATH];
	DISK_GEOMETRY diskGeometry;
}
DISK_GEOMETRY_STRUCT;

typedef struct
{
	WCHAR DeviceName[TC_MAX_PATH];
	LARGE_INTEGER RealDriveSize;
} ProbeRealDriveSizeRequest;

typedef struct
{
	short wszFileName[TC_MAX_PATH];	/* Volume to be "open tested" */
} OPEN_TEST_STRUCT;


typedef enum
{
	SetupNone = 0,
	SetupEncryption,
	SetupDecryption
} BootEncryptionSetupMode;


typedef struct
{
	BOOL DeviceFilterActive;

	uint16 BootLoaderVersion;

	BOOL DriveMounted;
	BOOL VolumeHeaderPresent;
	BOOL DriveEncrypted;

	LARGE_INTEGER BootDriveLength;

	int64 ConfiguredEncryptedAreaStart;
	int64 ConfiguredEncryptedAreaEnd;
	int64 EncryptedAreaStart;
	int64 EncryptedAreaEnd;

	uint32 VolumeHeaderSaltCrc32;

	BOOL SetupInProgress;
	BootEncryptionSetupMode SetupMode;
	BOOL TransformWaitingForIdle;

	uint32 HibernationPreventionCount;

} BootEncryptionStatus;


typedef struct
{
	BootEncryptionSetupMode SetupMode;
	WipeAlgorithmId WipeAlgorithm;
} BootEncryptionSetupRequest;


typedef struct
{
	Password VolumePassword;
} ReopenBootVolumeHeaderRequest;


#pragma pack (pop)

#ifdef NT4_DRIVER
#define DRIVER_STR WIDE
#else
#define DRIVER_STR
#endif

/* NT only */

#define TC_UNIQUE_ID_PREFIX "TrueCrypt"
#define TC_MOUNT_PREFIX L"\\Device\\TrueCryptVolume"

#define NT_MOUNT_PREFIX DRIVER_STR("\\Device\\TrueCryptVolume")
#define NT_ROOT_PREFIX DRIVER_STR("\\Device\\TrueCrypt")
#define DOS_MOUNT_PREFIX DRIVER_STR("\\DosDevices\\")
#define DOS_ROOT_PREFIX DRIVER_STR("\\DosDevices\\TrueCrypt")
#define WIN32_ROOT_PREFIX DRIVER_STR("\\\\.\\TrueCrypt")

#endif		/* _WIN32 */
