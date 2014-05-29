/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#include "System.h"
#include "InfoWizardPage.h"

namespace TrueCrypt
{
	void InfoWizardPage::SetMaxStaticTextWidth (int width)
	{
		InfoStaticText->Wrap (width);
	}
}
