This archive contains the source code of TrueCrypt 5.1.


Important
=========

You may use the source code contained in this archive only if you accept and
agree to be bound by the license terms contained in the file 'License.txt',
which is included in this archive. Note that the license specifies, for
example, that a derived work must not be called 'TrueCrypt'.



Contents
========

I. Windows
	Requirements for Building TrueCrypt for Windows
	Instructions for Building TrueCrypt for Windows

II. Linux and Mac OS X
	Requirements for Building TrueCrypt for Linux and Mac OS X
	Instructions for Building TrueCrypt for Linux and Mac OS X

III. Third-Party Developers (Contributors)

IV. Further Information



I. Windows
==========

Requirements for Building TrueCrypt for Windows:
------------------------------------------------

- Microsoft Visual C++ 2005 SP1 (Professional Edition or compatible)
- Microsoft Visual C++ 1.52
- Microsoft Windows Driver Kit for Windows Vista (build 6000)
- Microsoft Windows SDK 6.0 (integrated with Visual Studio)
- NASM 0.99 assembler
- Yasm assembler
- gzip compressor

IMPORTANT:

The 64-bit editions of Windows Vista and in some cases (e.g. playback of HD DVD
content) also the 32-bit editions of Windows Vista do not allow the TrueCrypt
driver to run without an appropriate digital signature. Therefore, all .sys
files in official TrueCrypt binary packages are digitally signed with the
digital certificate of the TrueCrypt Foundation, which was issued by a
certification authority. At the end of each official .exe and .sys file,
there are embedded digital signatures and all related certificates (i.e. all
certificates in the relevant certification chain, such as the certification
authority certificates, CA-MS cross-certificate, and the TrueCrypt Foundation
certificate). Keep this in mind if you compile TrueCrypt and compare your
binaries with the official binaries. If your binaries are unsigned, the sizes
of the official binaries will usually be approximately 10 KB greater than sizes
of your binaries (if you use a different version of compiler or if you install
a different or no service pack for Visual Studio, there may be further
differences).


Instructions for Building TrueCrypt for Windows:
------------------------------------------------

1) Create an environment variable called 'MSVC16_ROOT' pointing to the
   installation directory of MS Visual C++ 1.52.

2) If you have installed the Windows Driver Development Kit in another
   directory than '%SYSTEMDRIVE%\WinDDK', create an environment variable called
   'WINDDK_ROOT' or 'WINDDK_6000_ROOT' pointing to the DDK installation
   directory.
   
3) Open the 'TrueCrypt.sln' solution in Microsoft Visual Studio 2005.

4) Select 'All' as the active solution configuration.

5) Build the solution.

6) If successful, there should be newly built TrueCrypt binaries in the
   'Release' folder.



II. Linux and Mac OS X
======================

Requirements for Building TrueCrypt for Linux and Mac OS X:
-----------------------------------------------------------

- GNU Make
- GNU C++ Compiler 4.0 or compatible
- Apple XCode SDK (Mac OS X only)
- pkg-config
- wxWidgets 2.8 library source code (available at http://www.wxwidgets.org)
- FUSE library (available at http://fuse.sourceforge.net and 
  http://code.google.com/p/macfuse)


Instructions for Building TrueCrypt for Linux and Mac OS X:
-----------------------------------------------------------

1) Change the current directory to the root of the TrueCrypt source code.

2) Run the following command to configure the wxWidgets library for TrueCrypt 
   and to build it: 

   $ make WX_ROOT=/usr/src/wxWidgets wxbuild

   The variable WX_ROOT must point to the location of the source code of the
   wxWidgets library. Output files will be placed in the './wxrelease/'
   directory.

3) To build TrueCrypt, run the following command:

   $ make

4) If successful, the TrueCrypt executable should be located in the directory
   'Main'.



III. Third-Party Developers (Contributors)
==========================================

If you intend to implement a feature, please contact us first to make sure:

1) That the feature has not been implemented (we may have already implemented
   it, but haven't released the code yet).
2) That the feature is acceptable.
3) Whether we need help of third-party developers with implementing the feature.

Information on how to contact us can be found at:
http://www.truecrypt.org/contact.php



IV. Further Information
=======================

http://www.truecrypt.org
