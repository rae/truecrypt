/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#ifndef TC_HEADER_Core_Unix_CoreServiceRequest
#define TC_HEADER_Core_Unix_CoreServiceRequest

#include "Platform/Serializable.h"
#include "Core/Core.h"

namespace TrueCrypt
{
	struct CoreServiceRequest : public Serializable
	{
		CoreServiceRequest () : ElevateUserPrivileges (false), FastElevation (false) { }
		TC_SERIALIZABLE (CoreServiceRequest);

		virtual bool RequiresElevation () const { return false; }

		string AdminPassword;
		FilePath ApplicationExecutablePath;
		bool ElevateUserPrivileges;
		bool FastElevation;
	};

	struct ChangePasswordRequest : CoreServiceRequest
	{
		ChangePasswordRequest () { }
		ChangePasswordRequest (shared_ptr <VolumePath> volumePath, bool preserveTimestamps, shared_ptr <VolumePassword> password, shared_ptr <KeyfileList> keyfiles, shared_ptr <VolumePassword> newPassword, shared_ptr <KeyfileList> newKeyfiles, shared_ptr <Pkcs5Kdf> newPkcs5Kdf)
			: Keyfiles (keyfiles), NewKeyfiles (newKeyfiles), NewPassword (newPassword), NewPkcs5Kdf (newPkcs5Kdf), Password (password), Path (volumePath), PreserveTimestamps (preserveTimestamps) { }
		TC_SERIALIZABLE (ChangePasswordRequest);

		virtual bool RequiresElevation () const;

		shared_ptr <KeyfileList> Keyfiles;
		shared_ptr <KeyfileList> NewKeyfiles;
		shared_ptr <VolumePassword> NewPassword;
		shared_ptr <Pkcs5Kdf> NewPkcs5Kdf;
		shared_ptr <VolumePassword> Password;
		shared_ptr <VolumePath> Path;
		bool PreserveTimestamps;
	};

	struct CheckFilesystemRequest : CoreServiceRequest
	{
		CheckFilesystemRequest () { }
		CheckFilesystemRequest (shared_ptr <VolumeInfo> volumeInfo, bool repair)
			: MountedVolumeInfo (volumeInfo), Repair (repair) { }
		TC_SERIALIZABLE (CheckFilesystemRequest);

		virtual bool RequiresElevation () const;

		shared_ptr <VolumeInfo> MountedVolumeInfo;
		bool Repair;
	};

	struct DismountVolumeRequest : CoreServiceRequest
	{
		DismountVolumeRequest () { }
		DismountVolumeRequest (shared_ptr <VolumeInfo> volumeInfo, bool ignoreOpenFiles)
			: IgnoreOpenFiles (ignoreOpenFiles), MountedVolumeInfo (volumeInfo) { }
		TC_SERIALIZABLE (DismountVolumeRequest);

		virtual bool RequiresElevation () const;

		bool IgnoreOpenFiles;
		shared_ptr <VolumeInfo> MountedVolumeInfo;
	};

	struct GetDeviceSizeRequest : CoreServiceRequest
	{
		GetDeviceSizeRequest () { }
		GetDeviceSizeRequest (const DevicePath &path) : Path (path) { }
		TC_SERIALIZABLE (GetDeviceSizeRequest);

		virtual bool RequiresElevation () const;

		DevicePath Path;
	};

	struct GetHostDevicesRequest : CoreServiceRequest
	{
		GetHostDevicesRequest () { }
		GetHostDevicesRequest (bool pathListOnly) : PathListOnly (pathListOnly) { }
		TC_SERIALIZABLE (GetHostDevicesRequest);

		virtual bool RequiresElevation () const;

		bool PathListOnly;
	};

	struct ExitRequest : CoreServiceRequest
	{
		TC_SERIALIZABLE (ExitRequest);
	};

	struct MountVolumeRequest : CoreServiceRequest
	{
		MountVolumeRequest () { }
		MountVolumeRequest (MountOptions *options) : Options (options) { }
		TC_SERIALIZABLE (MountVolumeRequest);

		virtual bool RequiresElevation () const;

		MountOptions *Options;

	protected:
		shared_ptr <MountOptions> DeserializedOptions;
	};


	struct SetFileOwnerRequest : CoreServiceRequest
	{
		SetFileOwnerRequest () { }
		SetFileOwnerRequest (const FilesystemPath &path, const UserId &owner) : Owner (owner), Path (path) { }
		TC_SERIALIZABLE (SetFileOwnerRequest);

		virtual bool RequiresElevation () const;

		UserId Owner;
		FilesystemPath Path;
	};
}

#endif // TC_HEADER_Core_Unix_CoreServiceRequest
