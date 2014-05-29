/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#ifndef TC_HEADER_Main_Resources
#define TC_HEADER_Main_Resources

#include "System.h"
#include "Platform/Platform.h"

namespace TrueCrypt
{
	class Resources
	{
	public:
		static wxBitmap GetDriveIconBitmap ();
		static wxBitmap GetDriveIconMaskBitmap ();
		static string GetLanguageXml ();
		static string GetLegalNotices ();
		static wxBitmap GetLogoBitmap ();
		static wxBitmap GetTextualLogoBitmap ();
		static wxIcon GetTrueCryptIcon ();
		static wxBitmap GetVolumeCreationWizardBitmap (int height = -1);

	protected:
	};
}

#endif // TC_HEADER_Main_Resources
