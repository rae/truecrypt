/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#include "System.h"
#include <wx/stdpaths.h>
#include "Main.h"
#include "Application.h"
#include "CommandLineInterface.h"
#include "GraphicUserInterface.h"
#include "TextUserInterface.h"

namespace TrueCrypt
{
	wxApp* Application::CreateConsoleApp ()
	{
		mUserInterface = new TextUserInterface;
		return mUserInterface;
	} 

	wxApp* Application::CreateGuiApp ()
	{
		mUserInterface = new GraphicUserInterface;
		return mUserInterface;
	} 

	FilePath Application::GetConfigFilePath (const wxString &configFileName, bool createConfigDir)
	{
		wxStandardPaths stdPaths;
		DirectoryPath configDir;
		
		if (!Core->IsInTravelMode())
		{
#ifdef TC_MACOSX
			wxFileName configPath (L"~/Library/Application Support/TrueCrypt");
			configPath.Normalize();
			configDir = wstring (configPath.GetFullPath());
#else
			configDir = wstring (stdPaths.GetUserDataDir());
#endif
		}
		else
			configDir = GetExecutableDirectory();

		if (createConfigDir && !configDir.IsDirectory())
			Directory::Create (configDir);

		FilePath filePath = wstring (wxFileName (wstring (configDir), configFileName).GetFullPath());
		return filePath;
	}

	DirectoryPath Application::GetExecutableDirectory ()
	{
		return wstring (wxFileName (wxStandardPaths().GetExecutablePath()).GetPath());
	}

	FilePath Application::GetExecutablePath ()
	{
		return wstring (wxStandardPaths().GetExecutablePath());
	}

	void Application::Initialize (UserInterfaceType::Enum type)
	{
		switch (type)
		{
		case UserInterfaceType::Text:
			{
				wxAppInitializer wxTheAppInitializer((wxAppInitializerFunction) CreateConsoleApp);
				break;
			}
		case UserInterfaceType::Graphic:
			{
				wxAppInitializer wxTheAppInitializer((wxAppInitializerFunction) CreateGuiApp);
				break;
			}

		default:
			throw ParameterIncorrect (SRC_POS);
		}
	}

	int Application::ExitCode = 0;
	UserInterface *Application::mUserInterface = nullptr;
}
