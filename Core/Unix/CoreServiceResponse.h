/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#ifndef TC_HEADER_Core_Unix_CoreServiceResponse
#define TC_HEADER_Core_Unix_CoreServiceResponse

#include "Platform/Serializable.h"
#include "Core/Core.h"

namespace TrueCrypt
{
	struct CoreServiceResponse : public Serializable
	{
	};

	struct ChangePasswordResponse : CoreServiceResponse
	{
		ChangePasswordResponse () { }
		TC_SERIALIZABLE (ChangePasswordResponse);
	};

	struct CheckFilesystemResponse : CoreServiceResponse
	{
		CheckFilesystemResponse () { }
		TC_SERIALIZABLE (CheckFilesystemResponse);
	};

	struct DismountVolumeResponse : CoreServiceResponse
	{
		DismountVolumeResponse () { }
		TC_SERIALIZABLE (DismountVolumeResponse);
	};

	struct GetDeviceSizeResponse : CoreServiceResponse
	{
		GetDeviceSizeResponse () { }
		GetDeviceSizeResponse (uint64 size) : Size (size) { }
		TC_SERIALIZABLE (GetDeviceSizeResponse);

		uint64 Size;
	};

	struct GetHostDevicesResponse : CoreServiceResponse
	{
		GetHostDevicesResponse () { }
		GetHostDevicesResponse (const HostDeviceList &hostDevices) : HostDevices (hostDevices) { }
		TC_SERIALIZABLE (GetHostDevicesResponse);

		HostDeviceList HostDevices;
	};

	struct MountVolumeResponse : CoreServiceResponse
	{
		MountVolumeResponse () { }
		MountVolumeResponse (shared_ptr <VolumeInfo> volumeInfo) : MountedVolumeInfo (volumeInfo) { }
		TC_SERIALIZABLE (MountVolumeResponse);

		shared_ptr <VolumeInfo> MountedVolumeInfo;
	};

	struct SetFileOwnerResponse : CoreServiceResponse
	{
		SetFileOwnerResponse () { }
		TC_SERIALIZABLE (SetFileOwnerResponse);
	};
}

#endif // TC_HEADER_Core_Unix_CoreServiceResponse
