/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.4 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#ifndef TC_HEADER_Encryption_EncryptionModeLRW
#define TC_HEADER_Encryption_EncryptionModeLRW

#include "Platform/Platform.h"
#include "EncryptionMode.h"

namespace TrueCrypt
{
	class EncryptionModeLRW : public EncryptionMode
	{
	public:
		EncryptionModeLRW () { }
		virtual ~EncryptionModeLRW () { }

		virtual void Decrypt (byte *data, uint64 length) const;
		virtual void DecryptSectors (byte *data, uint64 sectorIndex, uint64 sectorCount, size_t sectorSize) const;
		virtual void Encrypt (byte *data, uint64 length) const;
		virtual void EncryptSectors (byte *data, uint64 sectorIndex, uint64 sectorCount, size_t sectorSize) const;
		virtual size_t GetKeySize () const { return 16; };
		virtual wstring GetName () const { return L"LRW"; };
		virtual shared_ptr <EncryptionMode> GetNew () const { return shared_ptr <EncryptionMode> (new EncryptionModeLRW); }
		virtual void SetKey (const ConstBufferPtr &key);

	protected:
		void DecryptBuffer (byte *plainText, uint64 length, uint64 blockIndex) const;
		void EncryptBuffer (byte *plainText, uint64 length, uint64 blockIndex) const;
		void IncrementBlockIndex (byte *index) const;
		uint64 SectorToBlockIndex (uint64 sectorIndex) const;
		void Xor64 (uint64 *a, const uint64 *b) const;
		void Xor128 (uint64 *a, const uint64 *b) const;

		SecureBuffer GfContext;

	private:
		EncryptionModeLRW (const EncryptionModeLRW &);
		EncryptionModeLRW &operator= (const EncryptionModeLRW &);
	};
}

#endif // TC_HEADER_Encryption_EncryptionModeLRW
