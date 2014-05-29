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

#include "Tcdefs.h"

#include "Crypto.h"
#include "Fat.h"
#include "Format.h"
#include "Volumes.h"
#include "Password.h"
#include "Apidrvr.h"
#include "Dlgcode.h"
#include "Language.h"
#include "Pkcs5.h"
#include "Common/Endian.h"
#include "Resource.h"
#include "Random.h"

#include <io.h>

void
VerifyPasswordAndUpdate (HWND hwndDlg, HWND hButton, HWND hPassword,
			 HWND hVerify, unsigned char *szPassword,
			 char *szVerify,
			 BOOL keyFilesEnabled)
{
	char szTmp1[MAX_PASSWORD + 1];
	char szTmp2[MAX_PASSWORD + 1];
	int k = GetWindowTextLength (hPassword);
	BOOL bEnable = FALSE;

	if (hwndDlg);		/* Remove warning */

	GetWindowText (hPassword, szTmp1, sizeof (szTmp1));
	GetWindowText (hVerify, szTmp2, sizeof (szTmp2));

	if (strcmp (szTmp1, szTmp2) != 0)
		bEnable = FALSE;
	else
	{
		if (k >= MIN_PASSWORD || keyFilesEnabled)
			bEnable = TRUE;
		else
			bEnable = FALSE;
	}

	if (szPassword != NULL)
		memcpy (szPassword, szTmp1, sizeof (szTmp1));

	if (szVerify != NULL)
		memcpy (szVerify, szTmp2, sizeof (szTmp2));

	burn (szTmp1, sizeof (szTmp1));
	burn (szTmp2, sizeof (szTmp2));

	EnableWindow (hButton, bEnable);
}


BOOL CheckPasswordCharEncoding (HWND hPassword, Password *ptrPw)
{
	int i, len;
	
	if (hPassword == NULL)
	{
		unsigned char *pw;
		len = ptrPw->Length;
		pw = (unsigned char *) ptrPw->Text;

		for (i = 0; i < len; i++)
		{
			if (pw[i] >= 0x7f || pw[i] < 0x20)	// A non-ASCII or non-printable character?
				return FALSE;
		}
	}
	else
	{
		wchar_t s[MAX_PASSWORD + 1];
		len = GetWindowTextLength (hPassword);

		if (len > MAX_PASSWORD)
			return FALSE; 

		GetWindowTextW (hPassword, s, sizeof (s) / sizeof (wchar_t));

		for (i = 0; i < len; i++)
		{
			if (s[i] >= 0x7f || s[i] < 0x20)	// A non-ASCII or non-printable character?
				break;
		}

		burn (s, sizeof(s));

		if (i < len)
			return FALSE; 
	}

	return TRUE;
}


BOOL CheckPasswordLength (HWND hwndDlg, HWND hwndItem)
{
	if (GetWindowTextLength (hwndItem) < PASSWORD_LEN_WARNING)
	{
		if (MessageBoxW (hwndDlg, GetString ("PASSWORD_LENGTH_WARNING"), lpszTitle, MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2) != IDYES)
			return FALSE;
	}
	return TRUE;
}

int
ChangePwd (char *lpszVolume, Password *oldPassword, Password *newPassword, int pkcs5, HWND hwndDlg)
{
	int nDosLinkCreated = 1, nStatus = ERR_OS_ERROR;
	char szDiskFile[TC_MAX_PATH], szCFDevice[TC_MAX_PATH];
	char szDosDevice[TC_MAX_PATH];
	char buffer[HEADER_SIZE];
	PCRYPTO_INFO cryptoInfo = NULL, ci = NULL;
	void *dev = INVALID_HANDLE_VALUE;
	DWORD dwError;
	BOOL bDevice;
	unsigned __int64 volSize = 0;
	int volumeType;
	int wipePass;
	FILETIME ftCreationTime;
	FILETIME ftLastWriteTime;
	FILETIME ftLastAccessTime;
	BOOL bTimeStampValid = FALSE;

	if (oldPassword->Length == 0 || newPassword->Length == 0) return -1;

	WaitCursor ();

	CreateFullVolumePath (szDiskFile, lpszVolume, &bDevice);

	if (bDevice == FALSE)
	{
		strcpy (szCFDevice, szDiskFile);
	}
	else
	{
		nDosLinkCreated = FakeDosNameForDevice (szDiskFile, szDosDevice, szCFDevice, FALSE);
		
		if (nDosLinkCreated != 0)
			goto error;
	}

	dev = CreateFile (szCFDevice, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	if (bDevice)
	{
		/* This is necessary to determine the hidden volume header offset */

		if (dev == INVALID_HANDLE_VALUE)
		{
			goto error;
		}
		else
		{
			PARTITION_INFORMATION diskInfo;
			DWORD dwResult;
			BOOL bResult;

			bResult = GetPartitionInfo (lpszVolume, &diskInfo);

			if (bResult)
			{
				volSize = diskInfo.PartitionLength.QuadPart;
			}
			else
			{
				DISK_GEOMETRY driveInfo;

				bResult = DeviceIoControl (dev, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0,
					&driveInfo, sizeof (driveInfo), &dwResult, NULL);

				if (!bResult)
					goto error;

				volSize = driveInfo.Cylinders.QuadPart * driveInfo.BytesPerSector *
					driveInfo.SectorsPerTrack * driveInfo.TracksPerCylinder;
			}

			if (volSize == 0)
			{
				nStatus = ERR_VOL_SIZE_WRONG;
				goto error;
			}
		}
	}

	if (dev == INVALID_HANDLE_VALUE) 
		goto error;

	if (Randinit ())
		goto error;

	if (!bDevice && bPreserveTimestamp)
	{
		/* Remember the container modification/creation date and time, (used to reset file date and time of
		file-hosted volumes after password change (or attempt to), in order to preserve plausible deniability
		of hidden volumes (last password change time is stored in the volume header). */

		if (GetFileTime ((HANDLE) dev, &ftCreationTime, &ftLastAccessTime, &ftLastWriteTime) == 0)
		{
			bTimeStampValid = FALSE;
			MessageBoxW (hwndDlg, GetString ("GETFILETIME_FAILED_PW"), L"TrueCrypt", MB_OK | MB_ICONEXCLAMATION);
		}
		else
			bTimeStampValid = TRUE;
	}

	for (volumeType = VOLUME_TYPE_NORMAL; volumeType < NBR_VOLUME_TYPES; volumeType++)
	{

		/* Read in volume header */

		if (volumeType == VOLUME_TYPE_HIDDEN)
		{
			if (!SeekHiddenVolHeader ((HFILE) dev, volSize, bDevice))
			{
				nStatus = ERR_VOL_SEEKING;
				goto error;
			}
		}

		nStatus = _lread ((HFILE) dev, buffer, sizeof (buffer));
		if (nStatus != sizeof (buffer))
		{
			nStatus = ERR_VOL_SIZE_WRONG;
			goto error;
		}

		/* Try to decrypt the header */

		nStatus = VolumeReadHeader (FALSE, buffer, oldPassword, &cryptoInfo, NULL);
		if (nStatus == ERR_CIPHER_INIT_WEAK_KEY)
			nStatus = 0;	// We can ignore this error here

		if (nStatus == ERR_PASSWORD_WRONG)
		{
			continue;		// Try next volume type
		}
		else if (nStatus != 0)
		{
			cryptoInfo = NULL;
			goto error;
		}
		else 
			break;
	}

	if (nStatus != 0)
	{
		cryptoInfo = NULL;
		goto error;
	}

	// Change the PKCS-5 PRF if requested by user
	if (pkcs5 != 0)
		cryptoInfo->pkcs5 = pkcs5;


	/* Re-encrypt the volume header */ 

	/* The header will be re-encrypted PRAND_DISK_WIPE_PASSES times to prevent adversaries from using 
	techniques such as magnetic force microscopy or magnetic force scanning tunnelling microscopy
	to recover the overwritten header. According to Peter Gutmann, data should be overwritten 22
	times (ideally, 35 times) using non-random patterns and pseudorandom data. However, as users might
	impatiently interupt the process (etc.) we will not use the Gutmann's patterns but will write the
	valid re-encrypted header, i.e. pseudorandom data, and there will be many more passes than Guttman
	recommends. During each pass we will write a valid working header. Each pass will use the same master
	key, and also the same header key, secondary key (XTS), etc., derived from the new password. The only
	item that will be different for each pass will be the salt. This is sufficient to cause each "version"
	of the header to differ substantially and in a random manner from the versions written during the
	other passes. */
	for (wipePass = 0; wipePass < PRAND_DISK_WIPE_PASSES; wipePass++)
	{
		// Seek the volume header
		if (volumeType == VOLUME_TYPE_HIDDEN)
		{
			if (!SeekHiddenVolHeader ((HFILE) dev, volSize, bDevice))
			{
				nStatus = ERR_VOL_SEEKING;
				goto error;
			}
		}
		else
		{
			nStatus = _llseek ((HFILE) dev, 0, FILE_BEGIN);

			if (nStatus != 0)
			{
				nStatus = ERR_VOL_SEEKING;
				goto error;
			}
		}

		// Prepare new volume header
		nStatus = VolumeWriteHeader (FALSE,
			buffer,
			cryptoInfo->ea,
			cryptoInfo->mode,
			newPassword,
			cryptoInfo->pkcs5,
			cryptoInfo->master_keydata,
			cryptoInfo->volume_creation_time,
			&ci,
			cryptoInfo->VolumeSize.Value,
			volumeType == VOLUME_TYPE_HIDDEN ? cryptoInfo->hiddenVolumeSize : 0,
			cryptoInfo->EncryptedAreaStart.Value,
			cryptoInfo->EncryptedAreaLength.Value,
			wipePass < PRAND_DISK_WIPE_PASSES - 1);

		if (ci != NULL)
			crypto_close (ci);

		if (nStatus != 0)
			goto error;

		// Write the new header 
		nStatus = _lwrite ((HFILE) dev, buffer, HEADER_SIZE);
		if (nStatus != HEADER_SIZE)
		{
			nStatus = ERR_VOL_WRITING;
			goto error;
		}
		FlushFileBuffers (dev);
	}

	/* Password successfully changed */
	nStatus = 0;

error:
	dwError = GetLastError ();

	burn (buffer, sizeof (buffer));

	if (cryptoInfo != NULL)
		crypto_close (cryptoInfo);

	if (bTimeStampValid)
	{
		// Restore the container timestamp (to preserve plausible deniability of possible hidden volume). 
		if (SetFileTime (dev, &ftCreationTime, &ftLastAccessTime, &ftLastWriteTime) == 0)
			MessageBoxW (hwndDlg, GetString ("SETFILETIME_FAILED_PW"), L"TrueCrypt", MB_OK | MB_ICONEXCLAMATION);
	}

	if (dev != INVALID_HANDLE_VALUE)
		CloseHandle ((HANDLE) dev);

	if (nDosLinkCreated == 0)
		RemoveFakeDosName (szDiskFile, szDosDevice);

	NormalCursor ();
	Randfree ();

	SetLastError (dwError);

	if (nStatus == ERR_OS_ERROR && dwError == ERROR_ACCESS_DENIED
		&& bDevice
		&& !UacElevated
		&& IsUacSupported ())
		return nStatus;

	if (nStatus != 0)
		handleError (hwndDlg, nStatus);

	return nStatus;
}

