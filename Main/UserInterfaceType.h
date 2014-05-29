/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#ifndef TC_HEADER_Main_UserInterfaceType
#define TC_HEADER_Main_UserInterfaceType

namespace TrueCrypt
{
	struct UserInterfaceType
	{
		enum Enum
		{
			Unknown,
			Graphic,
			Text
		};
	};
}

#endif // TC_HEADER_Main_UserInterfaceType
