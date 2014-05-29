/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#ifndef TC_HEADER_Core_CoreException
#define TC_HEADER_Core_CoreException

#include "Platform/Platform.h"

namespace TrueCrypt
{
	struct ElevationFailed : public ExecutedProcessFailed
	{
		ElevationFailed () { }
		ElevationFailed (const string &message, const string &command, int exitCode, const string &errorOutput)
			: ExecutedProcessFailed (message, command, exitCode, errorOutput) { }
		TC_SERIALIZABLE_EXCEPTION (ElevationFailed);
	};

	TC_EXCEPTION_DECL (RootDeviceUnavailable, SystemException);

#define TC_EXCEPTION(NAME) TC_EXCEPTION_DECL(NAME,Exception)

#undef TC_EXCEPTION_SET
#define TC_EXCEPTION_SET \
	TC_EXCEPTION_NODECL (ElevationFailed); \
	TC_EXCEPTION_NODECL (RootDeviceUnavailable); \
	TC_EXCEPTION (DriveLetterUnavailable); \
	TC_EXCEPTION (DriverError); \
	TC_EXCEPTION (MountPointRequired); \
	TC_EXCEPTION (MountPointUnavailable); \
	TC_EXCEPTION (NoDriveLetterAvailable); \
	TC_EXCEPTION (NoLoopbackDeviceAvailable); \
	TC_EXCEPTION (UnsupportedSectorSize); \
	TC_EXCEPTION (VolumeAlreadyMounted); \
	TC_EXCEPTION (VolumeSlotUnavailable);

	TC_EXCEPTION_SET;

#undef TC_EXCEPTION
}

#endif // TC_HEADER_Core_CoreException
