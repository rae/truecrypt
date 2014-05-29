/*
 Copyright (c) TrueCrypt Foundation. All rights reserved.

 Covered by the TrueCrypt License 2.3 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#include <atlcomcli.h>
#include <atlconv.h>
#include <strsafe.h>
#include <windows.h>
#include "BaseCom.h"
#include "Dlgcode.h"
#include "MainCom.h"
#include "MainCom_h.h"
#include "MainCom_i.c"
#include "Password.h"

static volatile LONG ObjectCount = 0;

class TrueCrypt : public ITrueCrypt
{

public:
	TrueCrypt (DWORD messageThreadId) : RefCount (0), MessageThreadId (messageThreadId)
	{
		InterlockedIncrement (&ObjectCount);
	}

	~TrueCrypt ()
	{
		if (InterlockedDecrement (&ObjectCount) == 0)
			PostThreadMessage (MessageThreadId, WM_APP, 0, 0);
	}

	virtual ULONG STDMETHODCALLTYPE AddRef ()
	{
		return InterlockedIncrement (&RefCount);
	}

	virtual ULONG STDMETHODCALLTYPE Release ()
	{
		if (!InterlockedDecrement (&RefCount))
		{
			delete this;
			return 0;
		}

		return RefCount;
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface (REFIID riid, void **ppvObject)
	{
		if (riid == IID_IUnknown || riid == IID_ITrueCrypt)
			*ppvObject = this;
		else
		{
			*ppvObject = NULL;
			return E_NOINTERFACE;
		}

		AddRef ();
		return S_OK;
	}
	
	virtual int STDMETHODCALLTYPE BackupVolumeHeader (LONG_PTR hwndDlg, BOOL bRequireConfirmation, BSTR lpszVolume)
	{
		USES_CONVERSION;
		return ::BackupVolumeHeader ((HWND) hwndDlg, bRequireConfirmation, CW2A (lpszVolume));
	}

	virtual int STDMETHODCALLTYPE RestoreVolumeHeader (LONG_PTR hwndDlg, BSTR lpszVolume)
	{
		USES_CONVERSION;
		return ::RestoreVolumeHeader ((HWND) hwndDlg, CW2A (lpszVolume));
	}

	virtual int STDMETHODCALLTYPE ChangePassword (BSTR volumePath, Password *oldPassword, Password *newPassword, int pkcs5, LONG_PTR hWnd)
	{
		USES_CONVERSION;
		return ::ChangePwd (CW2A (volumePath), oldPassword, newPassword, pkcs5, (HWND) hWnd);
	}

protected:
	DWORD MessageThreadId;
	LONG RefCount;
};


extern "C" BOOL ComServerMain ()
{
	TrueCryptFactory<TrueCrypt> factory (GetCurrentThreadId ());
	DWORD cookie;

	if (IsUacSupported ())
		UacElevated = TRUE;

	if (CoRegisterClassObject (CLSID_TrueCrypt, (LPUNKNOWN) &factory,
		CLSCTX_LOCAL_SERVER, REGCLS_SINGLEUSE, &cookie) != S_OK)
		return FALSE;

	MSG msg;
	while (int r = GetMessage (&msg, NULL, 0, 0))
	{
		if (r == -1)
			return FALSE;

		TranslateMessage (&msg);
		DispatchMessage (&msg);

		if (msg.message == WM_APP
			&& ObjectCount < 1
			&& !factory.IsServerLocked ())
			break;
	}
	CoRevokeClassObject (cookie);

	return TRUE;
}


static BOOL ComGetInstance (HWND hWnd, ITrueCrypt **tcServer)
{
	return ComGetInstanceBase (hWnd, CLSID_TrueCrypt, IID_ITrueCrypt, (void **) tcServer);
}


extern "C" int UacBackupVolumeHeader (HWND hwndDlg, BOOL bRequireConfirmation, char *lpszVolume)
{
	CComPtr<ITrueCrypt> tc;
	int r;

	CoInitialize (NULL);

	if (ComGetInstance (hwndDlg, &tc))
		r = tc->BackupVolumeHeader ((LONG_PTR) hwndDlg, bRequireConfirmation, CComBSTR (lpszVolume));
	else
		r = -1;

	CoUninitialize ();

	return r;
}


extern "C" int UacRestoreVolumeHeader (HWND hwndDlg, char *lpszVolume)
{
	CComPtr<ITrueCrypt> tc;
	int r;

	CoInitialize (NULL);

	if (ComGetInstance (hwndDlg, &tc))
		r = tc->RestoreVolumeHeader ((LONG_PTR) hwndDlg, CComBSTR (lpszVolume));
	else
		r = -1;

	CoUninitialize ();

	return r;
}


extern "C" int UacChangePwd (char *lpszVolume, Password *oldPassword, Password *newPassword, int pkcs5, HWND hwndDlg)
{
	CComPtr<ITrueCrypt> tc;
	int r;

	if (ComGetInstance (hwndDlg, &tc))
	{
		WaitCursor ();
		r = tc->ChangePassword (CComBSTR (lpszVolume), oldPassword, newPassword, 0, (LONG_PTR) hwndDlg);
		NormalCursor ();
	}
	else
		r = -1;

	return r;
}
