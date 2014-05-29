/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#include "CoreUnix.h"
#include <iostream>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "Platform/FileStream.h"
#include "Driver/Fuse/FuseService.h"
#include "Volume/VolumePasswordCache.h"

namespace TrueCrypt
{
	CoreUnix::CoreUnix ()
	{
		signal (SIGPIPE, SIG_IGN);
		
		char *loc = setlocale (LC_ALL, "");
		if (!loc || string (loc) == "C")
			setlocale (LC_ALL, "en_US.UTF-8");
	}

	CoreUnix::~CoreUnix ()
	{
	}
	
	void CoreUnix::CheckFilesystem (shared_ptr <VolumeInfo> mountedVolume, bool repair) const
	{
		if (!mountedVolume->MountPoint.IsEmpty())
			DismountFilesystem (mountedVolume->MountPoint, false);

		list <string> args;

		args.push_back ("-T");
		args.push_back ("fsck");

		args.push_back ("-e");

		string xargs = "fsck ";
		if (!repair)
			xargs += "-n ";
		else
			xargs += "-r ";

		xargs += string (mountedVolume->VirtualDevice) + "; echo '[Done]'; read W";
		args.push_back (xargs);

		try
		{
			Process::Execute ("xterm", args, 1000);
		} catch (TimeOut&) { }
	}

	void CoreUnix::DismountFilesystem (const DirectoryPath &mountPoint, bool force) const
	{
		list <string> args;

		args.push_back ("--");
		args.push_back (mountPoint);

		Process::Execute ("umount", args);
	}

	void CoreUnix::DismountVolume (shared_ptr <VolumeInfo> mountedVolume, bool ignoreOpenFiles)
	{
		if (!mountedVolume->MountPoint.IsEmpty())
		{
			DismountFilesystem (mountedVolume->MountPoint, ignoreOpenFiles);

			// Delete mount directory if a default path has been used
			if (string (mountedVolume->MountPoint).find (GetDefaultMountPointPrefix()) == 0)
				mountedVolume->MountPoint.Delete();
		}

		if (!mountedVolume->LoopDevice.IsEmpty())
		{
			try
			{
				DetachLoopDevice (mountedVolume->LoopDevice);
			}
			catch (ExecutedProcessFailed&) { }
		}

		if (mountedVolume->Protection == VolumeProtection::HiddenVolumeReadOnly)
		{
			sync();
			VolumeInfoList ml = GetMountedVolumes (mountedVolume->Path);

			if (ml.size() > 0)
				mountedVolume->HiddenVolumeProtectionTriggered = ml.front()->HiddenVolumeProtectionTriggered;
		}

		list <string> args;
		args.push_back ("--");
		args.push_back (mountedVolume->AuxMountPoint);

		for (int t = 0; true; t++)
		{
			try
			{
				Process::Execute ("umount", args);
				break;
			}
			catch (ExecutedProcessFailed&)
			{
				if (t > 10)
					throw;
				Thread::Sleep (200);
			}
		}

		try
		{
			mountedVolume->AuxMountPoint.Delete();
		}
		catch (...)	{ }

		VolumeEventArgs eventArgs (mountedVolume);
		VolumeDismountedEvent.Raise (eventArgs);
	}

	string CoreUnix::GetDefaultMountPointPrefix () const
	{
		const char *envPrefix = getenv ("TRUECRYPT_MOUNT_PREFIX");
		if (envPrefix && !string (envPrefix).empty())
			return envPrefix;
		
		if (FilesystemPath ("/media").IsDirectory())
			return "/media/truecrypt";
		
		if (FilesystemPath ("/mnt").IsDirectory())
			return "/mnt/truecrypt";
		
		return GetTempDirectory() + "/truecrypt_mnt";
	}

	uint64 CoreUnix::GetDeviceSize (const DevicePath &devicePath) const
	{
		File dev;
		dev.Open (devicePath);
		return dev.Length();
	}

	DirectoryPath CoreUnix::GetDeviceMountPoint (const DevicePath &devicePath) const
	{
		MountedFilesystemList mountedFilesystems = GetMountedFilesystems (devicePath);

		if (mountedFilesystems.size() < 1)
			return DirectoryPath();

		return mountedFilesystems.front()->MountPoint;
	}

	VolumeInfoList CoreUnix::GetMountedVolumes (const VolumePath &volumePath) const
	{
		VolumeInfoList volumes;

		foreach_ref (const MountedFilesystem &mf, GetMountedFilesystems ())
		{
			if (string (mf.MountPoint).find (GetFuseMountDirPrefix()) == string::npos)
				continue;

			shared_ptr <VolumeInfo> mountedVol;
			try
			{
				shared_ptr <File> controlFile (new File);
				controlFile->Open (string (mf.MountPoint) + FuseService::GetControlPath());

				shared_ptr <Stream> controlFileStream (new FileStream (controlFile));
				mountedVol = Serializable::DeserializeNew <VolumeInfo> (controlFileStream);
			}
			catch (...)
			{
				continue;
			}
			
			if (!volumePath.IsEmpty() && wstring (mountedVol->Path).compare (volumePath) != 0)
				continue;

			mountedVol->AuxMountPoint = mf.MountPoint;

			if (!mountedVol->LoopDevice.IsEmpty())
			{
				MountedFilesystemList mpl = GetMountedFilesystems (mountedVol->LoopDevice);

				if (mpl.size() > 0)
					mountedVol->MountPoint = mpl.front()->MountPoint;
			}

			volumes.push_back (mountedVol);

			if (!volumePath.IsEmpty())
				break;
		}

		return volumes;
	}
	
	gid_t CoreUnix::GetRealGroupId () const
	{
		const char *env = getenv ("SUDO_GID");
		if (env)
		{
			try
			{
				string s (env);
				return static_cast <gid_t> (StringConverter::ToUInt64 (s));
			}
			catch (...) { }
		}

		throw ParameterIncorrect (SRC_POS);
	}

	uid_t CoreUnix::GetRealUserId () const
	{
		const char *env = getenv ("SUDO_UID");
		if (env)
		{
			try
			{
				string s (env);
				return static_cast <uid_t> (StringConverter::ToUInt64 (s));
			}
			catch (...) { }
		}

		throw ParameterIncorrect (SRC_POS);
	}
	
	string CoreUnix::GetTempDirectory () const
	{
		char *envDir = getenv ("TMPDIR");
		return envDir ? envDir : "/tmp";
	}

	bool CoreUnix::IsMountPointAvailable (const DirectoryPath &mountPoint) const
	{
		return GetMountedFilesystems (DevicePath(), mountPoint).size() == 0;
	}

	void CoreUnix::MountFilesystem (const DevicePath &devicePath, const DirectoryPath &mountPoint, const string &filesystemType, bool readOnly, const string &systemMountOptions) const
	{
		if (GetMountedFilesystems (DevicePath(), mountPoint).size() > 0)
			throw MountPointUnavailable (SRC_POS);

		list <string> args;
		string options;

		if (!filesystemType.empty())
		{
			args.push_back ("-t");
			args.push_back (filesystemType);
		}

		if (readOnly)
			options = "-oro";

		if (!systemMountOptions.empty())
		{
			if (options.empty())
				options = "-o";
			else
				options += ",";

			options += systemMountOptions;
		}

		if (!options.empty())
			args.push_back (options);

		args.push_back ("--");
		args.push_back (devicePath);
		args.push_back (mountPoint);

		Process::Execute ("mount", args);
	}

	VolumeSlotNumber CoreUnix::MountPointToSlotNumber (const DirectoryPath &mountPoint) const
	{
		string mountPointStr (mountPoint);
		if (mountPointStr.find (GetDefaultMountPointPrefix()) == 0)
		{
			try
			{
				return StringConverter::ToUInt32 (StringConverter::GetTrailingNumber (mountPointStr));
			}
			catch (...) { }
		}
		return GetFirstFreeSlotNumber();
	}

	shared_ptr <VolumeInfo> CoreUnix::MountVolume (MountOptions &options)
	{
		CoalesceSlotNumberAndMountPoint (options);

		if (IsVolumeMounted (*options.Path))
			throw VolumeAlreadyMounted (SRC_POS);

		shared_ptr <Volume> volume (OpenVolume (
			options.Path,
			options.PreserveTimestamps,
			options.Password,
			options.Keyfiles,
			options.Protection,
			options.ProtectionPassword,
			options.ProtectionKeyfiles,
			options.SharedAccessAllowed)
		);

		// Find a free mount point for FUSE service
		MountedFilesystemList mountedFilesystems = GetMountedFilesystems ();
		string fuseMountPoint;
		for (int i = 1; true; i++)
		{
			stringstream path;
			path << GetTempDirectory() << "/" << GetFuseMountDirPrefix() << i;
			FilesystemPath fsPath (path.str());

			bool inUse = false;

			foreach_ref (const MountedFilesystem &mf, mountedFilesystems)
			{
				if (mf.MountPoint == path.str())
				{
					inUse = true;
					break;
				}
			}

			if (!inUse)
			{
				try
				{
					if (fsPath.IsDirectory())
						fsPath.Delete();

					throw_sys_sub_if (mkdir (path.str().c_str(), S_IRUSR | S_IXUSR) == -1, path.str());

					fuseMountPoint = fsPath;
					break;
				}
				catch (...)
				{
					if (i > 255)
						throw;
				}
			}
		}

		try
		{
			FuseService::Mount (volume, options.SlotNumber, fuseMountPoint);
		}
		catch (...)
		{
			try
			{
				DirectoryPath (fuseMountPoint).Delete();
			}
			catch (...) { }
			throw;
		}

		try
		{
			// Create a mount directory if a default path has been specified
			bool mountDirCreated = false;
			string mountPoint;
			if (!options.NoFilesystem && options.MountPoint)
			{
				mountPoint = *options.MountPoint;

				if (mountPoint.find (GetDefaultMountPointPrefix()) == 0 && !options.MountPoint->IsDirectory())
				{
					Directory::Create (*options.MountPoint);
					try
					{
						throw_sys_sub_if (chown (mountPoint.c_str(), GetRealUserId(), GetRealGroupId()) == -1, mountPoint);
					} catch (ParameterIncorrect&) { }

					mountDirCreated = true;
				}
			}

			try
			{
				MountAuxVolumeImage (fuseMountPoint, options);
			}
			catch (...)
			{
				if (mountDirCreated)
					remove (mountPoint.c_str());
				throw;
			}
		}
		catch (...)
		{
			try
			{
				VolumeInfoList mountedVolumes = GetMountedVolumes (*options.Path);
				if (mountedVolumes.size() > 0)
				{
					shared_ptr <VolumeInfo> mountedVolume (mountedVolumes.front());
					DismountVolume (mountedVolume);
				}
			}
			catch (...) { }
			throw;
		}

		VolumeInfoList mountedVolumes = GetMountedVolumes (*options.Path);
		if (mountedVolumes.size() != 1)
			throw ParameterIncorrect (SRC_POS);

		VolumeEventArgs eventArgs (mountedVolumes.front());
		VolumeMountedEvent.Raise (eventArgs);

		return shared_ptr <VolumeInfo> (mountedVolumes.front());
	}

	void CoreUnix::MountAuxVolumeImage (const DirectoryPath &auxMountPoint, const MountOptions &options) const
	{
		DevicePath loopDev = AttachFileToLoopDevice (string (auxMountPoint) + FuseService::GetVolumeImagePath());

		try
		{
			FuseService::SendLoopDevice (auxMountPoint, loopDev);
		}
		catch (...)
		{
			try
			{
				DetachLoopDevice (loopDev);
			}
			catch (...) { }
			throw;
		}

		if (!options.NoFilesystem && options.MountPoint && !options.MountPoint->IsEmpty())
		{
			MountFilesystem (loopDev, *options.MountPoint,
				StringConverter::ToSingle (options.FilesystemType),
				options.Protection == VolumeProtection::ReadOnly,
				StringConverter::ToSingle (options.FilesystemOptions));
		}
	}

	void CoreUnix::SetFileOwner (const FilesystemPath &path, const UserId &owner) const
	{
		throw_sys_if (chown (string (path).c_str(), owner.SystemId, (gid_t) -1) == -1);
	}

	DirectoryPath CoreUnix::SlotNumberToMountPoint (VolumeSlotNumber slotNumber) const
	{
		if (slotNumber < GetFirstSlotNumber() || slotNumber > GetLastSlotNumber())
			throw ParameterIncorrect (SRC_POS);

		stringstream s;
		s << GetDefaultMountPointPrefix() << slotNumber;
		return s.str();
	}
}
