/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#ifndef TC_HEADER_Driver_Fuse_FuseService
#define TC_HEADER_Driver_Fuse_FuseService

#include "Platform/Platform.h"
#include "Platform/Unix/Process.h"
#include "Volume/VolumeInfo.h"
#include "Volume/Volume.h"

namespace TrueCrypt
{

	class FuseService
	{
	protected:
		struct ExecFunctor : public ProcessExecFunctor
		{
			ExecFunctor (shared_ptr <Volume> openVolume, VolumeSlotNumber slotNumber)
				: MountedVolume (openVolume), SlotNumber (slotNumber)
			{
			}
			virtual void operator() (int argc, char *argv[]);

		protected:
			shared_ptr <Volume> MountedVolume;
			VolumeSlotNumber SlotNumber;
		};

		friend class ExecFunctor;

	public:
		static bool CheckAccessRights ();
		static void Dismount ();
		static int ExceptionToErrorCode ();
		static const char *GetControlPath () { return "/control"; }
		static const char *GetVolumeImagePath ();
		static string GetDeviceType () { return "truecrypt"; }
		static uid_t GetGroupId () { return GroupId; }
		static DevicePath GetLoopDevice () { return OpenVolumeInfo.LoopDevice; }
		static uid_t GetUserId () { return UserId; }
		static shared_ptr <Buffer> GetVolumeInfo ();
		static uint64 GetVolumeSize ();
		static uint64 GetVolumeSectorSize () { return MountedVolume->GetSectorSize(); }
		static void Mount (shared_ptr <Volume> openVolume, VolumeSlotNumber slotNumber, const string &fuseMountPoint);
		static void ReadVolumeSectors (const BufferPtr &buffer, uint64 byteOffset);
		static void ReceiveLoopDevice (const ConstBufferPtr &buffer);
		static void SendLoopDevice (const DirectoryPath &fuseMountPoint, const DevicePath &loopDevice);
		static void WriteVolumeSectors (const ConstBufferPtr &buffer, uint64 byteOffset);

	protected:
		FuseService ();

		static VolumeInfo OpenVolumeInfo;
		static Mutex OpenVolumeInfoMutex;
		static shared_ptr <Volume> MountedVolume;
		static VolumeSlotNumber SlotNumber;
		static uid_t UserId;
		static gid_t GroupId;
	};
}

#endif // TC_HEADER_Driver_Fuse_FuseService
