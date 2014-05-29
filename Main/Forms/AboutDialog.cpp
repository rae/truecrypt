/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#include "System.h"
#include "Volume/Version.h"
#include "Main/Application.h"
#include "Main/GraphicUserInterface.h"
#include "Main/Resources.h"
#include "AboutDialog.h"

namespace TrueCrypt
{
	AboutDialog::AboutDialog (wxWindow* parent) : AboutDialogBase (parent)
	{
		LogoBitmap->SetBitmap (Resources::GetTextualLogoBitmap());

		VersionStaticText->SetLabel (Application::GetName() + L" " + StringConverter::ToWide (Version::String()));
		CopyrightStaticText->SetLabel (L"Copyright \xA9 2003-2008 TrueCrypt Foundation");
		WebsiteHyperlink->SetLabel (L"www.truecrypt.org");

		CreditsTextCtrl->SetMinSize (wxSize (
			Gui->GetCharWidth (CreditsTextCtrl) * 70,
			Gui->GetCharHeight (CreditsTextCtrl) * 6
#ifdef TC_WINDOWS
			- 5
#else
			- 11
#endif
			));

		Layout();
		Fit();
		Center();

		CreditsTextCtrl->ChangeValue (
			L"Portions of this software are based in part on the works of the following people: "
			L"Paul Le Roux, "
			L"Bruce Schneier, John Kelsey, Doug Whiting, David Wagner, Chris Hall, Niels Ferguson, "
			L"Lars Knudsen, Ross Anderson, Eli Biham, "
			L"Joan Daemen, Vincent Rijmen, "
			L"Phillip Rogaway, "
			L"Hans Dobbertin, Antoon Bosselaers, Bart Preneel, "
			L"Paulo Barreto, Brian Gladman, Wei Dai, Peter Gutmann, and many others.\n\n"

			L"Portions of this software:\n"
			L"Copyright \xA9 2003-2008 TrueCrypt Foundation. All Rights Reserved.\n"
			L"Copyright \xA9 1998-2000 Paul Le Roux. All Rights Reserved.\n"
			L"Copyright \xA9 1999-2006 Brian Gladman. All Rights Reserved.\n"
			L"Copyright \xA9 1995-1997 Eric Young. All Rights Reserved.\n"
			L"Copyright \xA9 2001 Markus Friedl. All Rights Reserved.\n\n"

			L"This software as a whole:\n"
			L"Copyright \xA9 2008 TrueCrypt Foundation. All rights reserved.\n\n"

			L"This software uses wxWidgets library, which is copyright \xA9 1998-2008 Julian Smart, Robert Roebling et al.\n\n"

			L"A TrueCrypt Foundation Release");
	}
}
