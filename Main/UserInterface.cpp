/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#include "System.h"
#include <set>
#include <typeinfo>
#include <wx/apptrait.h>
#include <wx/cmdline.h>
#include "Platform/PlatformTest.h"
#ifdef TC_UNIX
#include "Platform/Unix/Process.h"
#endif
#include "Volume/EncryptionTest.h"
#include "Application.h"
#include "FavoriteVolume.h"
#include "UserInterface.h"

namespace TrueCrypt
{
	UserInterface::UserInterface ()
	{
	}

	UserInterface::~UserInterface ()
	{
		Core->WarningEvent.Disconnect (this);
		Core->VolumeMountedEvent.Disconnect (this);
	}

	void UserInterface::CloseExplorerWindows (shared_ptr <VolumeInfo> mountedVolume) const
	{
#ifdef TC_WINDOWS
		struct Args
		{
			HWND ExplorerWindow;
			string DriveRootPath;
		};

		struct Enumerator
		{
			static BOOL CALLBACK ChildWindows (HWND hwnd, LPARAM argsLP)
			{
				Args *args = reinterpret_cast <Args *> (argsLP);
				
				char s[4096];
				SendMessageA (hwnd, WM_GETTEXT, sizeof (s), (LPARAM) s);

				if (strstr (s, args->DriveRootPath.c_str()) != NULL)
				{
					PostMessage (args->ExplorerWindow, WM_CLOSE, 0, 0);
					return FALSE;
				}

				return TRUE;
			}

			static BOOL CALLBACK TopLevelWindows (HWND hwnd, LPARAM argsLP)
			{
				Args *args = reinterpret_cast <Args *> (argsLP);

				char s[4096];
				GetClassNameA (hwnd, s, sizeof s);
				if (strcmp (s, "CabinetWClass") == 0)
				{
					GetWindowTextA (hwnd, s, sizeof s);
					if (strstr (s, args->DriveRootPath.c_str()) != NULL)
					{
						PostMessage (hwnd, WM_CLOSE, 0, 0);
						return TRUE;
					}

					args->ExplorerWindow = hwnd;
					EnumChildWindows (hwnd, ChildWindows, argsLP);
				}

				return TRUE;
			}
		};

		Args args;

		string mountPoint = mountedVolume->MountPoint;
		if (mountPoint.size() < 2 || mountPoint[1] != ':')
			return;

		args.DriveRootPath = string() + mountPoint[0] + string (":\\");
		
		EnumWindows (Enumerator::TopLevelWindows, (LPARAM) &args);
#endif
	}

	void UserInterface::DismountAllVolumes (bool ignoreOpenFiles, bool interactive) const
	{
		try
		{
			VolumeInfoList mountedVolumes = Core->GetMountedVolumes();

			if (mountedVolumes.size() < 1)
				ShowInfo (LangString["NO_VOLUMES_MOUNTED"]);

			BusyScope busy (this);
			DismountVolumes (mountedVolumes, ignoreOpenFiles, interactive);
		}
		catch (exception &e)
		{
			ShowError (e);
		}
	}

	void UserInterface::DismountVolume (shared_ptr <VolumeInfo> volume, bool ignoreOpenFiles, bool interactive) const
	{
		VolumeInfoList volumes;
		volumes.push_back (volume);

		DismountVolumes (volumes, ignoreOpenFiles, interactive);
	}

	void UserInterface::DismountVolumes (VolumeInfoList volumes, bool ignoreOpenFiles, bool interactive) const
	{
		BusyScope busy (this);

		wxString message;
		bool twoPassMode = volumes.size() > 1;
		bool volumesInUse = false;
		bool firstPass = true;

#ifdef TC_WINDOWS
		if (Preferences.CloseExplorerWindowsOnDismount)
		{
			foreach (shared_ptr <VolumeInfo> volume, volumes)
				CloseExplorerWindows (volume);
		}
#endif
		while (!volumes.empty())
		{
			VolumeInfoList volumesLeft;
			foreach (shared_ptr <VolumeInfo> volume, volumes)
			{
				try
				{
					BusyScope busy (this);
					Core->DismountVolume (volume, ignoreOpenFiles);
				}
				catch (MountedVolumeInUse&)
				{
					if (!firstPass)
						throw;

					if (twoPassMode || !interactive)
					{
						volumesInUse = true;
						volumesLeft.push_back (volume);
						continue;
					}
					else
					{
						if (AskYesNo (StringFormatter (LangString["UNMOUNT_LOCK_FAILED"], wstring (volume->Path)), true, true))
						{
							BusyScope busy (this);
							Core->DismountVolume (volume, true);
						}
						else
							throw UserAbort (SRC_POS);
					}
				}
				catch (...)
				{
					if (twoPassMode && firstPass)
						volumesLeft.push_back (volume);
					else
						throw;
				}

				if (volume->HiddenVolumeProtectionTriggered)
					ShowWarning (StringFormatter (LangString["DAMAGE_TO_HIDDEN_VOLUME_PREVENTED"], wstring (volume->Path)));

				if (Preferences.Verbose)
				{
					if (!message.IsEmpty())
						message += L'\n';
					message += StringFormatter (_("Volume \"{0}\" has been dismounted."), wstring (volume->Path));
				}
			}

			if (twoPassMode && firstPass)
			{
				volumes = volumesLeft;

				if (volumesInUse && interactive)
				{
					if (AskYesNo (LangString["UNMOUNTALL_LOCK_FAILED"], true, true))
						ignoreOpenFiles = true;
					else
						throw UserAbort (SRC_POS);
				}
			}
			else
				break;

			firstPass = false;
		}

		if (Preferences.Verbose && !message.IsEmpty())
			ShowInfo (message);
	}
	
	wxString UserInterface::ExceptionToMessage (const exception &ex) const
	{
		wxString message;
		
		const Exception *e = dynamic_cast <const Exception *> (&ex);
		if (e)
		{
			message = ExceptionToString (*e);

			// System exception
			const SystemException *sysEx = dynamic_cast <const SystemException *> (&ex);
			if (sysEx)
			{
				if (!message.IsEmpty())
				{
					message += L"\n\n";
				}

				message += wxString (sysEx->SystemText()).Trim (true);
			}

			if (!message.IsEmpty())
			{
				// Subject
				if (!e->GetSubject().empty())
				{
					message = message.Trim (true);

					if (message.EndsWith (L"."))
						message.Truncate (message.size() - 1);

					if (!message.EndsWith (L":"))
						message << L":\n";
					else
						message << L"\n";

					message << e->GetSubject();
				}
#ifdef DEBUG
				if (sysEx && sysEx->what())
					message << L"\n\n" << StringConverter::ToWide (sysEx->what());
#endif
				return message;
			}
		}

		// bad_alloc
		const bad_alloc *outOfMemory = dynamic_cast <const bad_alloc *> (&ex);
		if (outOfMemory)
			return _("Out of memory.");

		// Unresolved exceptions
		string typeName (StringConverter::GetTypeName (typeid (ex)));
		size_t pos = typeName.find ("TrueCrypt::");
		if (pos != string::npos)
		{
			return StringConverter::ToWide (typeName.substr (pos + string ("TrueCrypt::").size()))
				+ L" at " + StringConverter::ToWide (ex.what());
		}

		return StringConverter::ToWide (typeName) + L" at " + StringConverter::ToWide (ex.what());
	}

	wxString UserInterface::ExceptionToString (const Exception &ex) const
	{
		// Error messages
		const ErrorMessage *errMsgEx = dynamic_cast <const ErrorMessage *> (&ex);
		if (errMsgEx)
			return wstring (*errMsgEx).c_str();

		// ExecutedProcessFailed
		const ExecutedProcessFailed *execEx = dynamic_cast <const ExecutedProcessFailed *> (&ex);
		if (execEx)
		{
			wstring errOutput;

			// ElevationFailed
			if (dynamic_cast <const ElevationFailed*> (&ex))
				errOutput += wxString (_("Failed to obtain administrator privileges")) + (StringConverter::Trim (execEx->GetErrorOutput()).empty() ? L". " : L": ");

			errOutput += StringConverter::ToWide (execEx->GetErrorOutput());

			if (errOutput.empty())
				return errOutput + StringFormatter (_("Command \"{0}\" returned error {1}."), execEx->GetCommand(), execEx->GetExitCode());

			return wxString (errOutput).Trim (true);
		}

		// PasswordIncorrect 
		if (dynamic_cast <const PasswordIncorrect *> (&ex))
		{
			wxString message = ExceptionTypeToString (typeid (ex));

			if (wxGetKeyState (WXK_CAPITAL))
				message += wxString (L"\n\n") + LangString["CAPSLOCK_ON"];

			return message;
		}

		// Other library exceptions
		return ExceptionTypeToString (typeid (ex));
	}

	wxString UserInterface::ExceptionTypeToString (const std::type_info &ex) const
	{
#define EX2MSG(exception, message) do { if (ex == typeid (exception)) return (message); } while (false)
		EX2MSG (DriveLetterUnavailable,				LangString["DRIVE_LETTER_UNAVAILABLE"]);
		EX2MSG (ExternalException,					LangString["EXCEPTION_OCCURRED"]);
		EX2MSG (InsufficientData,					_("Not enough data available."));
		EX2MSG (HigherVersionRequired,				LangString["NEW_VERSION_REQUIRED"]);
		EX2MSG (MissingArgument,					_("A required argument is missing."));
		EX2MSG (MissingVolumeData,					_("Volume data missing."));
		EX2MSG (MountPointRequired,					_("Mount point required."));
		EX2MSG (MountPointUnavailable,				_("Mount point is already in use."));
		EX2MSG (NoDriveLetterAvailable,				LangString["NO_FREE_DRIVES"]);
		EX2MSG (NoLoopbackDeviceAvailable,			_("No loopback device available."));
		EX2MSG (PasswordEmpty,						_("No password or keyfile specified."));
		EX2MSG (PasswordIncorrect,					LangString["PASSWORD_WRONG"]);
		EX2MSG (PasswordKeyfilesIncorrect,			LangString["PASSWORD_OR_KEYFILE_WRONG"]);
		EX2MSG (PasswordTooLong,					StringFormatter (_("Password is longer than {0} characters."), VolumePassword::MaxSize));
		EX2MSG (ProtectionPasswordIncorrect,		_("Incorrect keyfile(s) and/or password to the protected hidden volume or the hidden volume does not exist."));
		EX2MSG (ProtectionPasswordKeyfilesIncorrect,	_("Incorrect password to the protected hidden volume or the hidden volume does not exist."));
		EX2MSG (RootDeviceUnavailable,				LangString["NODRIVER"]);
		EX2MSG (StringConversionFailed,				_("Invalid characters encountered."));
		EX2MSG (StringFormatterException,			_("Error while parsing formatted string."));
		EX2MSG (UnportablePassword,					LangString["UNSUPPORTED_CHARS_IN_PWD"]);
		EX2MSG (UnsupportedSectorSize,					LangString["LARGE_SECTOR_UNSUPPORTED"]);
		EX2MSG (VolumeAlreadyMounted,				LangString["VOL_ALREADY_MOUNTED"]);
		EX2MSG (VolumeHostInUse,					_("The host file/device is already in use."));
		EX2MSG (VolumeSlotUnavailable,				_("Volume slot unavailable."));

#undef EX2MSG
		return L"";
	}

	void UserInterface::Init ()
	{
		SetAppName (Application::GetName());
		SetClassName (Application::GetName());

		LangString.Init();
		Core->Init();

		wxCmdLineParser parser;
		parser.SetCmdLine (argc, argv);
		CmdLine.reset (new CommandLineInterface (parser, InterfaceType));
		SetPreferences (CmdLine->Preferences);

		Core->SetApplicationExecutablePath (Application::GetExecutablePath());

		if (!Preferences.NonInteractive)
		{
			Core->SetAdminPasswordCallback (GetAdminPasswordRequestHandler());
		}
		else
		{
			struct AdminPasswordRequestHandler : public GetStringFunctor
			{
				virtual string operator() ()
				{
					throw ElevationFailed (SRC_POS, "sudo", 1, "");
				}
			};

			Core->SetAdminPasswordCallback (shared_ptr <GetStringFunctor> (new AdminPasswordRequestHandler));
		}

		Core->WarningEvent.Connect (EventConnector <UserInterface> (this, &UserInterface::OnWarning));
		Core->VolumeMountedEvent.Connect (EventConnector <UserInterface> (this, &UserInterface::OnVolumeMounted));
	}
	
	void UserInterface::ListMountedVolumes (const VolumeInfoList &volumes) const
	{
		if (volumes.size() < 1)
			throw_err (LangString["NO_VOLUMES_MOUNTED"]);

		wxString message;

		foreach_ref (const VolumeInfo &volume, volumes)
		{
			message << volume.SlotNumber << L": " << StringConverter::QuoteSpaces (volume.Path);

			if (!volume.VirtualDevice.IsEmpty())
				message << L' ' << wstring (volume.VirtualDevice);
			else
				message << L" - ";

			if (!volume.MountPoint.IsEmpty())
				message << L' ' << StringConverter::QuoteSpaces (volume.MountPoint);
			else
				message << L" - ";

			if (Preferences.Verbose)
				message << L' ' << SizeToString (volume.Size);

			message << L'\n';
		}

		ShowString (message);
	}
	
	VolumeInfoList UserInterface::MountAllDeviceHostedVolumes (MountOptions &options) const
	{
		BusyScope busy (this);

		VolumeInfoList newMountedVolumes;

		if (!options.MountPoint)
			options.MountPoint.reset (new DirectoryPath);

		Core->CoalesceSlotNumberAndMountPoint (options);

		bool sharedAccessAllowed = options.SharedAccessAllowed;
		bool someVolumesShared = false;

		HostDeviceList devices;
		foreach (shared_ptr <HostDevice> device, Core->GetHostDevices (true))
		{
			devices.push_back (device);

			foreach (shared_ptr <HostDevice> partition, device->Partitions)
				devices.push_back (partition);
		}

		set <wstring> mountedVolumes;
		foreach_ref (const VolumeInfo &v, Core->GetMountedVolumes())
			mountedVolumes.insert (v.Path);

		bool protectedVolumeMounted = false;
		bool legacyVolumeMounted = false;

		foreach_ref (const HostDevice &device, devices)
		{
			if (mountedVolumes.find (wstring (device.Path)) != mountedVolumes.end())
				continue;

			Yield();
			options.SlotNumber = Core->GetFirstFreeSlotNumber (options.SlotNumber);
			options.MountPoint.reset (new DirectoryPath);
			options.Path.reset (new VolumePath (device.Path));

			try
			{
				try
				{
					options.SharedAccessAllowed = sharedAccessAllowed;
					newMountedVolumes.push_back (Core->MountVolume (options));
				}
				catch (VolumeHostInUse&)
				{
					if (!sharedAccessAllowed)
					{
						try
						{
							options.SharedAccessAllowed = true;
							newMountedVolumes.push_back (Core->MountVolume (options));
							someVolumesShared = true;
						}
						catch (VolumeHostInUse&)
						{
							continue;
						}
					}
					else
						continue;
				}

				if (newMountedVolumes.back()->Protection == VolumeProtection::HiddenVolumeReadOnly)
					protectedVolumeMounted = true;

				if (newMountedVolumes.back()->EncryptionAlgorithmMinBlockSize == 8)
					legacyVolumeMounted = true;
			}
			catch (DriverError&) { }
			catch (MissingVolumeData&) { }
			catch (PasswordException&) { }
			catch (SystemException&) { }
		}

		if (newMountedVolumes.empty())
		{
			ShowWarning (LangString [options.Keyfiles && !options.Keyfiles->empty() ? "PASSWORD_OR_KEYFILE_WRONG_AUTOMOUNT" : "PASSWORD_WRONG_AUTOMOUNT"]);
		}
		else
		{
			if (someVolumesShared)
				ShowWarning ("DEVICE_IN_USE_INFO");

			if (legacyVolumeMounted)
				ShowWarning ("WARN_64_BIT_BLOCK_CIPHER");

			if (protectedVolumeMounted)
				ShowInfo (LangString[newMountedVolumes.size() > 1 ? "HIDVOL_PROT_WARN_AFTER_MOUNT_PLURAL" : "HIDVOL_PROT_WARN_AFTER_MOUNT"]);
		}

		return newMountedVolumes;
	}

	VolumeInfoList UserInterface::MountAllFavoriteVolumes (MountOptions &options) const
	{
		BusyScope busy (this);
		
		VolumeInfoList newMountedVolumes;
		foreach_ref (const FavoriteVolume &favorite, FavoriteVolume::LoadList())
		{
			shared_ptr <VolumeInfo> mountedVolume = Core->GetMountedVolume (favorite.Path);
			if (mountedVolume)
			{
				if (mountedVolume->MountPoint != favorite.MountPoint)
					ShowInfo (StringFormatter (LangString["VOLUME_ALREADY_MOUNTED"], wstring (favorite.Path)));
				continue;
			}

			favorite.ToMountOptions (options);

			if (Preferences.NonInteractive)
			{
				BusyScope busy (this);
				newMountedVolumes.push_back (Core->MountVolume (options));
			}
			else
			{
				try
				{
					BusyScope busy (this);
					newMountedVolumes.push_back (Core->MountVolume (options));
				}
				catch (...)
				{
					shared_ptr <VolumeInfo> volume = MountVolume (options);
					if (!volume)
						break;
					newMountedVolumes.push_back (volume);
				}
			}
		}

		return newMountedVolumes;
	}

	shared_ptr <VolumeInfo> UserInterface::MountVolume (MountOptions &options) const
	{
		shared_ptr <VolumeInfo> volume;

		try
		{
			volume = Core->MountVolume (options);
		}
		catch (VolumeHostInUse&)
		{
			if (options.SharedAccessAllowed)
				throw_err (LangString["FILE_IN_USE_FAILED"]);

			if (!AskYesNo (StringFormatter (LangString["VOLUME_HOST_IN_USE"], wstring (*options.Path)), false, true))
				throw UserAbort (SRC_POS);

			try
			{
				options.SharedAccessAllowed = true;
				volume = Core->MountVolume (options);
			}
			catch (VolumeHostInUse&)
			{
				throw_err (LangString["FILE_IN_USE_FAILED"]);
			}
		}

		if (volume->EncryptionAlgorithmMinBlockSize == 8)
			ShowWarning ("WARN_64_BIT_BLOCK_CIPHER");

		if (VolumeHasUnrecommendedExtension (*options.Path))
			ShowWarning ("EXE_FILE_EXTENSION_MOUNT_WARNING");

		if (options.Protection == VolumeProtection::HiddenVolumeReadOnly)
			ShowInfo ("HIDVOL_PROT_WARN_AFTER_MOUNT");

		return volume;
	}

	void UserInterface::OnUnhandledException ()
	{
		try
		{
			throw;
		}
		catch (UserAbort&)
		{
		}
		catch (exception &e)
		{
			ShowError (e);
		}
		catch (...)
		{
			ShowError (_("Unknown exception occurred."));
		}

		Yield();
		Application::SetExitCode (1);
	}

	void UserInterface::OnVolumeMounted (EventArgs &args)
	{
		shared_ptr <VolumeInfo> mountedVolume = (dynamic_cast <VolumeEventArgs &> (args)).mVolume;

		if (Preferences.OpenExplorerWindowAfterMount && !mountedVolume->MountPoint.IsEmpty())
			OpenExplorerWindow (mountedVolume->MountPoint);
	}
	
	void UserInterface::OnWarning (EventArgs &args)
	{
		ExceptionEventArgs &e = dynamic_cast <ExceptionEventArgs &> (args);
		ShowWarning (e.mException);
	}

	void UserInterface::OpenExplorerWindow (const DirectoryPath &path)
	{
		if (path.IsEmpty())
			return;

		list <string> args;

#ifdef TC_WINDOWS

		wstring p (Directory::AppendSeparator (path));
		SHFILEINFO fInfo;
		SHGetFileInfo (p.c_str(), 0, &fInfo, sizeof (fInfo), 0); // Force explorer to discover the drive
		ShellExecute (GetTopWindow() ? static_cast <HWND> (GetTopWindow()->GetHandle()) : nullptr, L"open", p.c_str(), nullptr, nullptr, SW_SHOWNORMAL);

#elif defined (TC_MACOSX)

		args.push_back (string (path));
		Process::Execute ("open", args);

#else
		// MIME handler for directory seems to be unavailable through wxWidgets
		if (GetTraits()->GetDesktopEnvironment() == L"GNOME")
		{
			args.push_back ("--no-default-window");
			args.push_back ("--no-desktop");
			args.push_back (string (path));
			try
			{
				Process::Execute ("nautilus", args, 2000);
			}
			catch (TimeOut&) { }
		}
		else if (GetTraits()->GetDesktopEnvironment() == L"KDE")
		{
			args.push_back ("openURL");
			args.push_back (string (path));
			try
			{
				Process::Execute ("kfmclient", args, 2000);
			}
			catch (TimeOut&) { }
		}
#endif
	}

	bool UserInterface::ProcessCommandLine ()
	{
		CommandLineInterface &cmdLine = *CmdLine;

		switch (cmdLine.ArgCommand)
		{
		case CommandId::None:
			return false;

		case CommandId::AutoMountDevices:
		case CommandId::AutoMountFavorites:
		case CommandId::AutoMountDevicesFavorites:
		case CommandId::MountVolume:
			{
				cmdLine.ArgMountOptions.Path = cmdLine.ArgVolumePath;
				cmdLine.ArgMountOptions.MountPoint = cmdLine.ArgMountPoint;
				cmdLine.ArgMountOptions.Password = cmdLine.ArgPassword;
				cmdLine.ArgMountOptions.Keyfiles = cmdLine.ArgKeyfiles;
				cmdLine.ArgMountOptions.SharedAccessAllowed = cmdLine.ArgForce;

				VolumeInfoList mountedVolumes;
				switch (cmdLine.ArgCommand)
				{
				case CommandId::AutoMountDevices:
				case CommandId::AutoMountFavorites:
				case CommandId::AutoMountDevicesFavorites:
					{
						if (cmdLine.ArgCommand == CommandId::AutoMountDevices || cmdLine.ArgCommand == CommandId::AutoMountDevicesFavorites)
						{
							if (Preferences.NonInteractive)
								mountedVolumes = UserInterface::MountAllDeviceHostedVolumes (cmdLine.ArgMountOptions);
							else
								mountedVolumes = MountAllDeviceHostedVolumes (cmdLine.ArgMountOptions);
						}

						if (cmdLine.ArgCommand == CommandId::AutoMountFavorites || cmdLine.ArgCommand == CommandId::AutoMountDevicesFavorites)
						{
							foreach (shared_ptr <VolumeInfo> v, MountAllFavoriteVolumes(cmdLine.ArgMountOptions))
								mountedVolumes.push_back (v);
						}
					}
					break;


					break;

				case CommandId::MountVolume:
					if (Preferences.OpenExplorerWindowAfterMount)
					{
						// Open explorer window for an already mounted volume
						shared_ptr <VolumeInfo> mountedVolume = Core->GetMountedVolume (*cmdLine.ArgMountOptions.Path);
						if (mountedVolume && !mountedVolume->MountPoint.IsEmpty())
						{
							OpenExplorerWindow (mountedVolume->MountPoint);
							break;
						}
					}

					if (Preferences.NonInteractive)
					{
						// Volume path
						if (!cmdLine.ArgMountOptions.Path)
							throw MissingArgument (SRC_POS);

						mountedVolumes.push_back (Core->MountVolume (cmdLine.ArgMountOptions));
					}
					else
					{
						shared_ptr <VolumeInfo> volume = MountVolume (cmdLine.ArgMountOptions);
						if (!volume)
						{
							Application::SetExitCode (1);
							throw UserAbort (SRC_POS);
						}
						mountedVolumes.push_back (volume);
					}
					break;

				default:
					throw ParameterIncorrect (SRC_POS);
				}

				if (Preferences.Verbose && !mountedVolumes.empty())
				{
					wxString message;
					foreach_ref (const VolumeInfo &volume, mountedVolumes)
					{
						if (!message.IsEmpty())
							message += L'\n';
						message += StringFormatter (_("Volume \"{0}\" has been mounted."), wstring (volume.Path));
					}
					ShowInfo (message);
				}
			}
			return true;

		case CommandId::ChangePassword:
			ChangePassword (cmdLine.ArgVolumePath, cmdLine.ArgPassword, cmdLine.ArgKeyfiles, cmdLine.ArgNewPassword, cmdLine.ArgNewKeyfiles);
			return true;

		case CommandId::DismountVolumes:
			DismountVolumes (cmdLine.ArgVolumes, cmdLine.ArgForce, !Preferences.NonInteractive);
			return true;

		case CommandId::DisplayVersion:
			ShowString (Application::GetName() + L" " + StringConverter::ToWide (Version::String()) + L"\n");
			return true;

		case CommandId::Help:
			ShowString (L"\nExamples:\n\nMount a volume:\ntruecrypt volume.tc /media/truecrypt1\n\n"
					L"Dismount a volume:\ntruecrypt -d volume.tc\n\n"
					L"Dismount all mounted volumes:\ntruecrypt -d\n\n"
					);
			return true;

		case CommandId::ListVolumes:
			ListMountedVolumes (cmdLine.ArgVolumes);
			return true;

		case CommandId::Test:
			Test();
			return true;

		default:
			throw ParameterIncorrect (SRC_POS);
		}

		return false;
	}

	void UserInterface::SetPreferences (const UserPreferences &preferences)
	{
		Preferences = preferences;
		PreferencesUpdatedEvent.Raise();
	}

	void UserInterface::ShowError (const exception &ex) const
	{
		if (!dynamic_cast <const UserAbort*> (&ex))
			DoShowError (ExceptionToMessage (ex));
	}

	wxString UserInterface::SizeToString (uint64 size) const
	{
		wstringstream s;
		if (size > 1024ULL*1024*1024*1024*1024*99)
			s << size/1024/1024/1024/1024/1024 << L" " << LangString["PB"].c_str();
		else if (size > 1024ULL*1024*1024*1024*1024)
			return wxString::Format (L"%.1f %s", (double)(size/1024.0/1024/1024/1024/1024), LangString["PB"].c_str());
		else if (size > 1024ULL*1024*1024*1024*99)
			s << size/1024/1024/1024/1024 << L" " << LangString["TB"].c_str();
		else if (size > 1024ULL*1024*1024*1024)
			return wxString::Format (L"%.1f %s", (double)(size/1024.0/1024/1024/1024), LangString["TB"].c_str());
		else if (size > 1024ULL*1024*1024*99)
			s << size/1024/1024/1024 << L" " << LangString["GB"].c_str();
		else if (size > 1024ULL*1024*1024)
			return wxString::Format (L"%.1f %s", (double)(size/1024.0/1024/1024), LangString["GB"].c_str());
		else if (size > 1024ULL*1024*99)
			s << size/1024/1024 << L" " << LangString["MB"].c_str();
		else if (size > 1024ULL*1024)
			return wxString::Format (L"%.1f %s", (double)(size/1024.0/1024), LangString["MB"].c_str());
		else if (size > 1024ULL)
			s << size/1024 << L" " << LangString["KB"].c_str();
		else
			s << size << L" " << LangString["BYTE"].c_str();

		return s.str();
	}

	wxString UserInterface::SpeedToString (uint64 speed) const
	{
		wstringstream s;

		if (speed > 1024ULL*1024*1024*1024*1024*99)
			s << speed/1024/1024/1024/1024/1024 << L" " << LangString["PB_PER_SEC"].c_str();
		else if (speed > 1024ULL*1024*1024*1024*1024)
			return wxString::Format (L"%.1f %s", (double)(speed/1024.0/1024/1024/1024/1024), LangString["PB_PER_SEC"].c_str());
		else if (speed > 1024ULL*1024*1024*1024*99)
			s << speed/1024/1024/1024/1024 << L" " << LangString["TB"].c_str();
		else if (speed > 1024ULL*1024*1024*1024)
			return wxString::Format (L"%.1f %s", (double)(speed/1024.0/1024/1024/1024), LangString["TB_PER_SEC"].c_str());
		else if (speed > 1024ULL*1024*1024*99)
			s << speed/1024/1024/1024 << L" " << LangString["GB"].c_str();
		else if (speed > 1024ULL*1024*1024)
			return wxString::Format (L"%.1f %s", (double)(speed/1024.0/1024/1024), LangString["GB_PER_SEC"].c_str());
		else if (speed > 1024ULL*1024*99)
			s << speed/1024/1024 << L" " << LangString["MB"].c_str();
		else if (speed > 1024ULL*1024)
			return wxString::Format (L"%.1f %s", (double)(speed/1024.0/1024), LangString["MB_PER_SEC"].c_str());
		else if (speed > 1024ULL)
			s << speed/1024 << L" " << LangString["KB_PER_SEC"].c_str();
		else
			s << speed << L" " << LangString["B_PER_SEC"].c_str();

		return s.str();
	}

	void UserInterface::Test () const
	{
		if (!PlatformTest::TestAll())
			throw TestFailed (SRC_POS);

		EncryptionTest::TestAll();

		// StringFormatter
		if (StringFormatter (L"{9} {8} {7} {6} {5} {4} {3} {2} {1} {0} {{0}}", "1", L"2", '3', L'4', 5, 6, 7, 8, 9, 10) != L"10 9 8 7 6 5 4 3 2 1 {0}")
			throw TestFailed (SRC_POS);
		try
		{
			StringFormatter (L"{0} {1}", 1);
			throw TestFailed (SRC_POS);
		}
		catch (StringFormatterException&) { }

		try
		{
			StringFormatter (L"{0} {1} {1}", 1, 2, 3);
			throw TestFailed (SRC_POS);
		}
		catch (StringFormatterException&) { }

		try
		{
			StringFormatter (L"{0} 1}", 1, 2);
			throw TestFailed (SRC_POS);
		}
		catch (StringFormatterException&) { }

		try
		{
			StringFormatter (L"{0} {1", 1, 2);
			throw TestFailed (SRC_POS);
		}
		catch (StringFormatterException&) { }

		ShowInfo ("TESTS_PASSED");
	}
	
	bool UserInterface::VolumeHasUnrecommendedExtension (const VolumePath &path) const
	{
		wxString ext = wxFileName (wxString (wstring (path)).Lower()).GetExt();
		return ext.IsSameAs (L"exe") || ext.IsSameAs (L"sys") || ext.IsSameAs (L"dll");
	}

	wxString UserInterface::VolumeTimeToString (VolumeTime volumeTime) const
	{
		wxString dateStr = VolumeTimeToDateTime (volumeTime).Format();

#ifdef TC_WINDOWS

		FILETIME ft;
		*(unsigned __int64 *)(&ft) = volumeTime;
		SYSTEMTIME st;
		FileTimeToSystemTime (&ft, &st);

		wchar_t wstr[1024];
		if (GetDateFormat (LOCALE_USER_DEFAULT, 0, &st, 0, wstr, array_capacity (wstr)) != 0)
		{
			dateStr = wstr;
			GetTimeFormat (LOCALE_USER_DEFAULT, 0, &st, 0, wstr, array_capacity (wstr));
			dateStr += wxString (L" ") + wstr;
		}
#endif
		return dateStr;
	}

	wxString UserInterface::VolumeTypeToString (VolumeType::Enum type, VolumeProtection::Enum protection) const
	{
		switch (type)
		{
		case VolumeType::Normal:
			return LangString[protection == VolumeProtection::HiddenVolumeReadOnly ? "OUTER" : "NORMAL"];

		case VolumeType::Hidden:
			return LangString["HIDDEN"];

		default:
			return L"?";
		}
	}
}
