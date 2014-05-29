/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#ifndef TC_HEADER_Core_CoreMacOSX
#define TC_HEADER_Core_CoreMacOSX

#include "System.h"
#include "Core/Unix/FreeBSD/CoreFreeBSD.h"

namespace TrueCrypt
{
	class CoreMacOSX : public CoreFreeBSD
	{
	public:
		CoreMacOSX ();
		virtual ~CoreMacOSX ();

		virtual void DismountVolume (shared_ptr <VolumeInfo> mountedVolume, bool ignoreOpenFiles = false);
		virtual string GetDefaultMountPointPrefix () const { return "/Volumes/truecrypt"; }

	protected:
		virtual void MountAuxVolumeImage (const DirectoryPath &auxMountPoint, const MountOptions &options) const;

	private:
		CoreMacOSX (const CoreMacOSX &);
		CoreMacOSX &operator= (const CoreMacOSX &);
	};
}

#endif // TC_HEADER_Core_CoreMacOSX
