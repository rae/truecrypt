/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#include "System.h"
#include "Main/GraphicUserInterface.h"
#include "VolumeSizeWizardPage.h"

namespace TrueCrypt
{
	VolumeSizeWizardPage::VolumeSizeWizardPage (wxPanel* parent, const VolumePath &volumePath)
		: VolumeSizeWizardPageBase (parent)
	{
		VolumeSizePrefixChoice->Append (LangString["KB"], reinterpret_cast <void *> (1024));
		VolumeSizePrefixChoice->Append (LangString["MB"], reinterpret_cast <void *> (1024 * 1024));
		VolumeSizePrefixChoice->Append (LangString["GB"], reinterpret_cast <void *> (1024 * 1024 * 1024));
		VolumeSizePrefixChoice->Select (Prefix::MB);

		wxLongLong diskSpace = 0;
		if (!wxGetDiskSpace (wxFileName (wstring (volumePath)).GetPath(), nullptr, &diskSpace))
		{
			VolumeSizeTextCtrl->Disable();
			VolumeSizeTextCtrl->SetValue (L"");
		}

		FreeSpaceStaticText->SetFont (Gui->GetDefaultBoldFont (this));

#ifdef TC_WINDOWS
		wxString drive = wxFileName (wstring (volumePath)).GetVolume();
		if (!drive.empty())
		{
			FreeSpaceStaticText->SetLabel (StringFormatter (_("Free space on drive {0}: is {1}."),
				drive, Gui->SizeToString (diskSpace.GetValue())));
		}
		else
#endif
		{
			FreeSpaceStaticText->SetLabel (StringFormatter (_("Free space available: {0}"),
				Gui->SizeToString (diskSpace.GetValue())));
		}

		VolumeSizeTextCtrl->SetMinSize (wxSize (Gui->GetCharWidth (VolumeSizeTextCtrl) * 20, -1));

		wxTextValidator validator (wxFILTER_INCLUDE_CHAR_LIST);  // wxFILTER_NUMERIC does not exclude - . , etc.
		const wxChar *valArr[] = { L"0", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9" };
		validator.SetIncludes (wxArrayString (array_capacity (valArr), (const wxChar **) &valArr));
		VolumeSizeTextCtrl->SetValidator (validator);
	}

	uint64 VolumeSizeWizardPage::GetVolumeSize () const
	{
		uint64 prefixMult = 1;
		if (VolumeSizePrefixChoice->GetSelection() != wxNOT_FOUND)
			prefixMult = reinterpret_cast <int> (VolumeSizePrefixChoice->GetClientData (VolumeSizePrefixChoice->GetSelection()));
		
		uint64 val = StringConverter::ToUInt64 (wstring (VolumeSizeTextCtrl->GetValue()));
		if (val <= 0x7fffFFFFffffFFFFull / prefixMult)
			return val * prefixMult;
		else
			return 0;
	}

	bool VolumeSizeWizardPage::IsValid ()
	{
		if (!VolumeSizeTextCtrl->IsEmpty() && Validate())
		{
			try
			{
				if (GetVolumeSize() > 0)
					return true;
			}
			catch (...) { }
		}
		return false;
	}

	void VolumeSizeWizardPage::SetVolumeSize (uint64 size)
	{
		if (size == 0)
		{
			VolumeSizePrefixChoice->Select (Prefix::MB);
			VolumeSizeTextCtrl->SetValue (L"");
			return;
		}
		
		if (size % (1024 * 1024 * 1024) == 0)
		{
			size /= 1024 * 1024 * 1024;
			VolumeSizePrefixChoice->Select (Prefix::GB);
		}
		else if (size % (1024 * 1024) == 0)
		{
			size /= 1024 * 1024;
			VolumeSizePrefixChoice->Select (Prefix::MB);
		}
		else
		{
			size /= 1024;
			VolumeSizePrefixChoice->Select (Prefix::KB);
		}

		VolumeSizeTextCtrl->SetValue (StringConverter::FromNumber (size));
	}
}
