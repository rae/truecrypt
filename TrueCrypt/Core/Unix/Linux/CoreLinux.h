/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#ifndef TC_HEADER_Core_CoreLinux
#define TC_HEADER_Core_CoreLinux

#include "System.h"
#include "Core/Unix/CoreUnix.h"

namespace TrueCrypt
{
	class CoreLinux : public CoreUnix
	{
	public:
		CoreLinux ();
		virtual ~CoreLinux ();

		virtual HostDeviceList GetHostDevices (bool pathListOnly = false) const; 

	protected:
		virtual DevicePath AttachFileToLoopDevice (const FilePath &filePath) const;
		virtual void DetachLoopDevice (const DevicePath &devicePath) const;
		virtual MountedFilesystemList GetMountedFilesystems (const DevicePath &devicePath = DevicePath(), const DirectoryPath &mountPoint = DirectoryPath()) const;
		virtual void MountFilesystem (const DevicePath &devicePath, const DirectoryPath &mountPoint, const string &filesystemType, bool readOnly, const string &systemMountOptions) const;

	private:
		CoreLinux (const CoreLinux &);
		CoreLinux &operator= (const CoreLinux &);
	};
}

#endif // TC_HEADER_Core_CoreLinux
