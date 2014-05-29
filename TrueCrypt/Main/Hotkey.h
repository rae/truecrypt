/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#ifndef TC_HEADER_Main_Hotkey
#define TC_HEADER_Main_Hotkey

#include "System.h"
#include "Main.h"

namespace TrueCrypt
{
	struct Hotkey;
	typedef list < shared_ptr <Hotkey> > HotkeyList;

	struct Hotkey
	{
	public:
		struct Id
		{
			enum
			{
				DismountAll = 0,
				ForceDismountAllWipeCache,
				ForceDismountAllWipeCacheExit,
				MountAllDevices,
				MountAllFavorites,
				ShowHideApplication,
				WipeCache
			};
		};

		Hotkey (int id, const wstring &name, const wxString &description, int virtualKeyCode = 0, int virtualKeyModifiers = 0)
			: Description (description), Id (id), Name (name), VirtualKeyCode (virtualKeyCode), VirtualKeyModifiers (virtualKeyModifiers) { }

		virtual ~Hotkey () { }

		static HotkeyList GetAvailableHotkeys ();
		wxString GetShortcutString () const;
		static wxString GetVirtualKeyCodeString (int virtualKeyCode);
		static HotkeyList LoadList ();
		static void RegisterList (wxWindow *handler, const HotkeyList &hotkeys);
		static void SaveList (const HotkeyList &hotkeys);
		static void UnregisterList (wxWindow *handler, const HotkeyList &hotkeys);

		wxString Description;
		int Id;
		wstring Name;
		int VirtualKeyCode;
		int VirtualKeyModifiers;

	protected:
		static wxString GetFileName () { return L"Hotkeys.xml"; }
	};
}

#endif // TC_HEADER_Main_Hotkey
