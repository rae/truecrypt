/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#ifndef TC_HEADER_Main_UserPreferences
#define TC_HEADER_Main_UserPreferences

#include "System.h"
#include "Main.h"
#include "Hotkey.h"

namespace TrueCrypt
{
	struct UserPreferences
	{
		UserPreferences ()
			:
			BackgroundTaskEnabled (true),
			BackgroundTaskMenuDismountItemsEnabled (true),
			BackgroundTaskMenuMountItemsEnabled (true),
			BackgroundTaskMenuOpenItemsEnabled (true),
			BeepAfterHotkeyMountDismount (false),
			CloseBackgroundTaskOnNoVolumes (false),
			CloseExplorerWindowsOnDismount (true),
			DismountOnInactivity (false),
			DismountOnLogOff (true),
			DismountOnPowerSaving (true),
			DismountOnScreenSaver (false),
			DisplayMessageAfterHotkeyDismount (false),
			ForceAutoDismount (true),
			LastSelectedSlotNumber (0),
			MaxVolumeIdleTime (60),
			MountDevicesOnLogon (false),
			MountFavoritesOnLogon (false),
			NonInteractive (false),
			OpenExplorerWindowAfterMount (false),
			SaveHistory (false),
			StartOnLogon (false),
			UseKeyfiles (false),
			Verbose (false),
			WipeCacheOnAutoDismount (true),
			WipeCacheOnClose (false)
		{
		}

		virtual ~UserPreferences ()
		{
		}
		void Load();
		void Save() const;

		HotkeyList Hotkeys;
		KeyfileList DefaultKeyfiles;
		MountOptions DefaultMountOptions;

		bool BackgroundTaskEnabled;
		bool BackgroundTaskMenuDismountItemsEnabled;
		bool BackgroundTaskMenuMountItemsEnabled;
		bool BackgroundTaskMenuOpenItemsEnabled;
		bool BeepAfterHotkeyMountDismount;
		bool CloseBackgroundTaskOnNoVolumes;
		bool CloseExplorerWindowsOnDismount;
		bool DismountOnInactivity;
		bool DismountOnLogOff;
		bool DismountOnPowerSaving;
		bool DismountOnScreenSaver;
		bool DisplayMessageAfterHotkeyDismount;
		bool ForceAutoDismount;
		uint64 LastSelectedSlotNumber;
		int32 MaxVolumeIdleTime;
		bool MountDevicesOnLogon;
		bool MountFavoritesOnLogon;
		bool NonInteractive;
		bool OpenExplorerWindowAfterMount;
		bool SaveHistory;
		bool StartOnLogon;
		bool UseKeyfiles;
		bool Verbose;
		bool WipeCacheOnAutoDismount;
		bool WipeCacheOnClose;

	protected:
		wxString GetDefaultKeyfilesFileName () const { return L"Default Keyfiles.xml"; }
		wxString GetPreferencesFileName () const { return L"Configuration.xml"; }
		void SetValue (const wxString &cfgText, bool &cfgVar);
		void SetValue (const wxString &cfgText, int &cfgVar);
		void SetValue (const wxString &cfgText, uint64 &cfgVar);
		void SetValue (const wxString &cfgText, wstring &cfgVar);
		void SetValue (const wxString &cfgText, wxString &cfgVar);
	};
}

#endif // TC_HEADER_Main_UserPreferences
