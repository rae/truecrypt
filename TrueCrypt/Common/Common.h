/*
 Copyright (c) TrueCrypt Foundation. All rights reserved.

 Covered by the TrueCrypt License 2.3 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#ifndef COMMON_H
#define COMMON_H

#include "Crypto.h"

/* Volume types */
enum
{
	VOLUME_TYPE_NORMAL = 0,
	VOLUME_TYPE_HIDDEN,
	NBR_VOLUME_TYPES
};

/* Prop volume types */
enum
{
	PROP_VOL_TYPE_NORMAL = 0,
	PROP_VOL_TYPE_HIDDEN,
	PROP_VOL_TYPE_OUTER,						/* Outer/normal (hidden volume protected) */
	PROP_VOL_TYPE_OUTER_VOL_WRITE_PREVENTED,	/* Outer/normal (hidden volume protected AND write already prevented) */
	PROP_NBR_VOLUME_TYPES
};

/* Hidden volume protection status */
enum
{
	HIDVOL_PROT_STATUS_NONE = 0,
	HIDVOL_PROT_STATUS_ACTIVE,
	HIDVOL_PROT_STATUS_ACTION_TAKEN			/* Active + action taken (write operation has already been denied) */
};

typedef struct
{
	BOOL ReadOnly;
	BOOL Removable;
	BOOL ProtectHiddenVolume;
	BOOL PreserveTimestamp;
	BOOL SystemVolume;
	BOOL PersistentVolume;
	Password ProtectedHidVolPassword;	/* Password of hidden volume to protect against overwriting */
} MountOptions;

#endif
