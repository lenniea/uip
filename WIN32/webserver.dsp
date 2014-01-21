# Microsoft Developer Studio Project File - Name="webserver" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=webserver - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "webserver.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "webserver.mak" CFG="webserver - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "webserver - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "webserver - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "webserver - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Zp1 /MT /W3 /GX /O2 /I "..\uip" /I "..\apps\webserver" /I "." /I "..\..\WpdPack\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "_WEBSERVER" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib ws2_32.lib packet.lib wpcap.lib /nologo /subsystem:console /map /machine:I386 /libpath:"..\..\WpdPack\lib"

!ELSEIF  "$(CFG)" == "webserver - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /Zp1 /MTd /W3 /Gm /GX /Zi /Od /I "..\uip" /I "..\apps\webserver" /I "." /I "..\..\WpdPack\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "_WEBSERVER" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib ws2_32.lib packet.lib wpcap.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"..\..\WpdPack\lib"

!ENDIF 

# Begin Target

# Name "webserver - Win32 Release"
# Name "webserver - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\uip\psock.c
# End Source File
# Begin Source File

SOURCE=..\uip\timer.c
# End Source File
# Begin Source File

SOURCE="..\uip\uip-fw.c"
# End Source File
# Begin Source File

SOURCE="..\uip\uip-neighbor.c"
# End Source File
# Begin Source File

SOURCE=..\uip\uip.c
# End Source File
# Begin Source File

SOURCE=..\uip\uip_arp.c
# End Source File
# Begin Source File

SOURCE=..\uip\uiplib.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\uip\clock.h
# End Source File
# Begin Source File

SOURCE="..\uip\gcc-lc-addrlabels.h"
# End Source File
# Begin Source File

SOURCE="..\uip\lc-switch.h"
# End Source File
# Begin Source File

SOURCE=..\uip\lc.h
# End Source File
# Begin Source File

SOURCE=..\uip\psock.h
# End Source File
# Begin Source File

SOURCE=..\uip\pt.h
# End Source File
# Begin Source File

SOURCE=..\uip\timer.h
# End Source File
# Begin Source File

SOURCE="..\uip\uip-fw.h"
# End Source File
# Begin Source File

SOURCE="..\uip\uip-neighbor.h"
# End Source File
# Begin Source File

SOURCE="..\uip\uip-split.h"
# End Source File
# Begin Source File

SOURCE=..\uip\uip.h
# End Source File
# Begin Source File

SOURCE=..\uip\uip_arch.h
# End Source File
# Begin Source File

SOURCE=..\uip\uip_arp.h
# End Source File
# Begin Source File

SOURCE=..\uip\uiplib.h
# End Source File
# Begin Source File

SOURCE=..\uip\uipopt.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "App Files"

# PROP Default_Filter "*.c;*.h"
# Begin Source File

SOURCE="clock-arch.c"
# End Source File
# Begin Source File

SOURCE="clock-arch.h"
# End Source File
# Begin Source File

SOURCE="..\apps\webserver\http-strings.c"
# End Source File
# Begin Source File

SOURCE="..\apps\webserver\http-strings.h"
# End Source File
# Begin Source File

SOURCE="..\apps\webserver\httpd-cgi.c"
# End Source File
# Begin Source File

SOURCE="..\apps\webserver\httpd-cgi.h"
# End Source File
# Begin Source File

SOURCE="..\apps\webserver\httpd-fs.c"
# End Source File
# Begin Source File

SOURCE="..\apps\webserver\httpd-fs.h"
# End Source File
# Begin Source File

SOURCE="..\apps\webserver\httpd-fsdata.h"
# End Source File
# Begin Source File

SOURCE=..\apps\webserver\httpd.c
# End Source File
# Begin Source File

SOURCE=..\apps\webserver\httpd.h
# End Source File
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=.\tapdev.c
# End Source File
# Begin Source File

SOURCE=.\tapdev.h
# End Source File
# Begin Source File

SOURCE=".\uip-conf.h"
# End Source File
# Begin Source File

SOURCE=..\apps\webserver\webserver.h
# End Source File
# End Group
# End Target
# End Project
