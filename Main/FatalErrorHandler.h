/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#ifndef TC_HEADER_Main_FatalErrorHandler
#define TC_HEADER_Main_FatalErrorHandler

#include "System.h"

namespace TrueCrypt
{
	class FatalErrorHandler
	{
	public:
		static void Deregister();
		static void Register();

	protected:
		static void OnSignal (int signal);
		static void OnTerminate ();

	private:
		FatalErrorHandler ();
	};
}

#endif // TC_HEADER_Main_FatalErrorHandler
