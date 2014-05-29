/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#ifndef TC_HEADER_Main_Forms_VolumeLocationWizardPage
#define TC_HEADER_Main_Forms_VolumeLocationWizardPage

#include "Forms.h"

namespace TrueCrypt
{
	class VolumeLocationWizardPage : public VolumeLocationWizardPageBase
	{
	public:
		VolumeLocationWizardPage (wxPanel* parent, bool selectExisting = false);
		~VolumeLocationWizardPage ();

		VolumePath GetVolumePath () const { return VolumePath (wstring (VolumePathComboBox->GetValue())); }
		bool IsValid () { return !VolumePathComboBox->GetValue().IsEmpty(); }
		void OnPageChanging (bool forward);
		void SetVolumePath (const VolumePath &path);
		void SetMaxStaticTextWidth (int width) { InfoStaticText->Wrap (width); }
		void SetPageText (const wxString &text) { InfoStaticText->SetLabel (text); }

	protected:
		void OnVolumePathTextChanged (wxCommandEvent& event) { PageUpdatedEvent.Raise(); }
		void OnNoHistoryCheckBoxClick (wxCommandEvent& event);
		void OnSelectDeviceButtonClick (wxCommandEvent& event);
		void OnSelectFileButtonClick (wxCommandEvent& event);
		void OnPreferencesUpdated (EventArgs &args);

		bool SelectExisting;
	};
}

#endif // TC_HEADER_Main_Forms_VolumeLocationWizardPage
