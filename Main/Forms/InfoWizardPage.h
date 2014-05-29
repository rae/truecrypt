/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#ifndef TC_HEADER_Main_Forms_InfoWizardPage
#define TC_HEADER_Main_Forms_InfoWizardPage

#include "Forms.h"

namespace TrueCrypt
{
	class InfoWizardPage : public InfoWizardPageBase
	{
	public:
		InfoWizardPage (wxPanel *parent) : InfoWizardPageBase (parent) { InfoStaticText->SetFocus();  }

		bool IsValid () { return true; }
		void SetMaxStaticTextWidth (int width);
		void SetPageText (const wxString &text) { InfoStaticText->SetLabel (text); }
	};
}

#endif // TC_HEADER_Main_Forms_InfoWizardPage
