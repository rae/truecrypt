/* 
Copyright (c) 2004-2006 TrueCrypt Foundation. All rights reserved. 

Covered by TrueCrypt License 2.1 the full text of which is contained in the file
License.txt included in TrueCrypt binary and source code distribution archives. 
*/

enum
{
	/* When adding/removing hot keys, update the following functions in Mount.c:
	DisplayHotkeyList()
	SaveSettings()
	LoadSettings()
	HandleHotKey()	*/

	HK_AUTOMOUNT_DEVICES = 0,
	HK_DISMOUNT_ALL,
	HK_FORCE_DISMOUNT_ALL_AND_WIPE,
	HK_FORCE_DISMOUNT_ALL_AND_WIPE_AND_EXIT,
	HK_MOUNT_FAVORITE_VOLUMES,
	HK_SHOW_HIDE_MAIN_WINDOW,
	NBR_HOTKEYS
};

typedef struct
{
	UINT vKeyCode;
	UINT vKeyModifiers;
} TCHOTKEY;

extern TCHOTKEY	Hotkeys [NBR_HOTKEYS];

BOOL WINAPI HotkeysDlgProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL GetKeyName (UINT vKey, wchar_t *keyName);
void UnregisterAllHotkeys (HWND hwndDlg, TCHOTKEY hotkeys[]);
void RegisterAllHotkeys (HWND hwndDlg, TCHOTKEY hotkeys[]);

