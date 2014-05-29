/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#include "Common/Tcdefs.h"
#include "VolumeInfo.h"
#include "Platform/SerializerFactory.h"

namespace TrueCrypt
{
	void VolumeInfo::Deserialize (shared_ptr <Stream> stream)
	{
		Serializer sr (stream);

		sr.Deserialize ("ProgramVersion", ProgramVersion);
		AuxMountPoint = sr.DeserializeWString ("AuxMountPoint");
		sr.Deserialize ("EncryptionAlgorithmBlockSize", EncryptionAlgorithmBlockSize);
		sr.Deserialize ("EncryptionAlgorithmKeySize", EncryptionAlgorithmKeySize);
		sr.Deserialize ("EncryptionAlgorithmMinBlockSize", EncryptionAlgorithmMinBlockSize);
		EncryptionAlgorithmName = sr.DeserializeWString ("EncryptionAlgorithmName");
		EncryptionModeName = sr.DeserializeWString ("EncryptionModeName");
		sr.Deserialize ("HeaderCreationTime", HeaderCreationTime);
		sr.Deserialize ("HiddenVolumeProtectionTriggered", HiddenVolumeProtectionTriggered);
		LoopDevice = sr.DeserializeWString ("LoopDevice");
		MountPoint = sr.DeserializeWString ("MountPoint");
		Path = sr.DeserializeWString ("Path");
		sr.Deserialize ("Pkcs5IterationCount", Pkcs5IterationCount);
		Pkcs5PrfName = sr.DeserializeWString ("Pkcs5PrfName");
		Protection = static_cast <VolumeProtection::Enum> (sr.DeserializeInt32 ("Protection"));
		sr.Deserialize ("SerialInstanceNumber", SerialInstanceNumber);
		sr.Deserialize ("Size", Size);
		sr.Deserialize ("SlotNumber", SlotNumber);
		sr.Deserialize ("TotalDataRead", TotalDataRead);
		sr.Deserialize ("TotalDataWritten", TotalDataWritten);
		Type = static_cast <VolumeType::Enum> (sr.DeserializeInt32 ("Type"));
		VirtualDevice = sr.DeserializeWString ("VirtualDevice");
		sr.Deserialize ("VolumeCreationTime", VolumeCreationTime);
	}

	void VolumeInfo::Serialize (shared_ptr <Stream> stream) const
	{
		Serializable::Serialize (stream);
		Serializer sr (stream);

		const uint32 version = VERSION_NUM;
		sr.Serialize ("ProgramVersion", version);
		sr.Serialize ("AuxMountPoint", wstring (AuxMountPoint));
		sr.Serialize ("EncryptionAlgorithmBlockSize", EncryptionAlgorithmBlockSize);
		sr.Serialize ("EncryptionAlgorithmKeySize", EncryptionAlgorithmKeySize);
		sr.Serialize ("EncryptionAlgorithmMinBlockSize", EncryptionAlgorithmMinBlockSize);
		sr.Serialize ("EncryptionAlgorithmName", EncryptionAlgorithmName);
		sr.Serialize ("EncryptionModeName", EncryptionModeName);
		sr.Serialize ("HeaderCreationTime", HeaderCreationTime);
		sr.Serialize ("HiddenVolumeProtectionTriggered", HiddenVolumeProtectionTriggered);
		sr.Serialize ("LoopDevice", wstring (LoopDevice));
		sr.Serialize ("MountPoint", wstring (MountPoint));
		sr.Serialize ("Path", wstring (Path));
		sr.Serialize ("Pkcs5IterationCount", Pkcs5IterationCount);
		sr.Serialize ("Pkcs5PrfName", Pkcs5PrfName);
		sr.Serialize ("Protection", static_cast <uint32> (Protection));
		sr.Serialize ("SerialInstanceNumber", SerialInstanceNumber);
		sr.Serialize ("Size", Size);
		sr.Serialize ("SlotNumber", SlotNumber);
		sr.Serialize ("TotalDataRead", TotalDataRead);
		sr.Serialize ("TotalDataWritten", TotalDataWritten);
		sr.Serialize ("Type", static_cast <uint32> (Type));
		sr.Serialize ("VirtualDevice", wstring (VirtualDevice));
		sr.Serialize ("VolumeCreationTime", VolumeCreationTime);
	}

	void VolumeInfo::Set (const Volume &volume)
	{
		EncryptionAlgorithmBlockSize = static_cast <uint32> (volume.GetEncryptionAlgorithm()->GetMaxBlockSize());
		EncryptionAlgorithmKeySize = static_cast <uint32> (volume.GetEncryptionAlgorithm()->GetKeySize());
		EncryptionAlgorithmMinBlockSize = static_cast <uint32> (volume.GetEncryptionAlgorithm()->GetMinBlockSize());
		EncryptionAlgorithmName = volume.GetEncryptionAlgorithm()->GetName();
		EncryptionModeName = volume.GetEncryptionMode()->GetName();
		HeaderCreationTime = volume.GetHeaderCreationTime();
		VolumeCreationTime = volume.GetVolumeCreationTime();
		HiddenVolumeProtectionTriggered = volume.IsHiddenVolumeProtectionTriggered();
		Path = volume.GetPath();
		Pkcs5IterationCount = volume.GetPkcs5Kdf()->GetIterationCount();
		Pkcs5PrfName = volume.GetPkcs5Kdf()->GetName();
		Protection = volume.GetProtectionType();
		Size = volume.GetSize();
		Type = volume.GetType();
		TotalDataRead = volume.GetTotalDataRead();
		TotalDataWritten = volume.GetTotalDataWritten();
	}

	TC_SERIALIZER_FACTORY_ADD_CLASS (VolumeInfo);
}
