Download and build AFsp Audio File I/O package
====================================================
The AFsp Audio File I/O Package
-------------------------------

The AFsp Audio File I/O Package
-------------------------------

current version:  AFsp-v9r0.tar.gz
download site:    http://www-mmsp.ece.mcgill.ca/documents/Downloads/AFsp/


Build the libtsplite library
----------------------------
Unpack the downloaded file and follow the instructions below:

*) Linux/Mac:
   - change to unpacked folder and start make
   - 64bit builds only:
     rename the resulting library libtsplite.a to libtsplite_x86_64.a 
     for proper linking on 64bit platforms
	 
*) Win32:
   - use the MSVC.sln project file in AFsp-v9r0\MSVC and select libtsplite

Finally the following files have to be copied to the following locations:

modules_phase2/mps/import/AFsp/include/libtsp.h
modules_phase2/mps/import/AFsp/include/libtsp/UTpar.h
modules_phase2/mps/import/AFsp/include/libtsp/AFpar.h
modules_phase2/mps/import/AFsp/lib/libtsp.[a|lib]
scripts_phase2/bin/ResampAudio.exe (from /AFsp-v9r0/MSVC/bin/ResampAudio.exe)

Note:  In newer AFsp versions the libtsp library was named libtsplite. 
       Rename the library to libtsp to avoid linker errors.
		
