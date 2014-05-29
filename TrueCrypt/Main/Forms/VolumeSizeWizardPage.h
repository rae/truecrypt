/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#ifndef TC_HEADER_Main_Forms_VolumeSizeWizardPage
#define TC_HEADER_Main_Forms_VolumeSizeWizardPage

#include "Forms.h"

namespace TrueCrypt
{
	class VolumeSizeWizardPage : public VolumeSizeWizardPageBase
	{
	public:
		VolumeSizeWizardPage (wxPanel* parent, const VolumePath &volumePath);

		uint64 GetVolumeSize () const;
		bool IsValid ();
		void SetMaxStaticTextWidth (int width) { InfoStaticText->Wrap (width); }
		void SetPageText (const wxString &text) { InfoStaticText->SetLabel (text); }
		void SetVolumeSize (uint64 size);

	protected:
		struct Prefix
		{
			enum
			{
				KB = 0,
				MB,
				GB
			};
		};

		void OnBrowseButtonClick (wxCommandEvent& event);
		void OnVolumeSizePrefixSelected (wxCommandEvent& event) { PageUpdatedEvent.Raise(); }
		void OnVolumeSizeTextChanged (wxCommandEvent& event) { PageUpdatedEvent.Raise(); }
	};
}

#endif // TC_HEADER_Main_Forms_VolumeSizeWizardPage
