# Microsoft Developer Studio Project File - Name="Prelude" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Prelude - Win32 autotest
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Prelude.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Prelude.mak" CFG="Prelude - Win32 autotest"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Prelude - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Prelude - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "Prelude - Win32 Logged" (based on "Win32 (x86) Application")
!MESSAGE "Prelude - Win32 autotest" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Prelude - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"C:\Windows\Desktop\ToTest\Prelude Release.exe"

!ELSEIF  "$(CFG)" == "Prelude - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fr /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"c:\ToTest\Prelude.exe"

!ELSEIF  "$(CFG)" == "Prelude - Win32 Logged"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Prelude___Win32_Logged"
# PROP BASE Intermediate_Dir "Prelude___Win32_Logged"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Prelude___Win32_Logged"
# PROP Intermediate_Dir "Prelude___Win32_Logged"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "SHOW_SCRIPT_DEBUG" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"c:\windows\desktop\ToTest\Prelude.exe" /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"c:\windows\desktop\ToTest\Prelude.exe" /pdbtype:sept

!ELSEIF  "$(CFG)" == "Prelude - Win32 autotest"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Prelude___Win32_autotest"
# PROP BASE Intermediate_Dir "Prelude___Win32_autotest"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Prelude___Win32_autotest"
# PROP Intermediate_Dir "Prelude___Win32_autotest"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "AUTOTEST" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"c:\windows\desktop\ToTest\Prelude.exe"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"c:\windows\desktop\ToTest\PreludeAuto.exe"

!ENDIF 

# Begin Target

# Name "Prelude - Win32 Release"
# Name "Prelude - Win32 Debug"
# Name "Prelude - Win32 Logged"
# Name "Prelude - Win32 autotest"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\Source\actions.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\area.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\attacks.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\cavewall.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\Chunks.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\combatdemomain.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\CombatManager.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\Corpse.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\creatures.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\EditRegion.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\entrance.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\equipobject.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\events.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\Flags.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\forest.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\gameitem.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\Generatworld.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\items.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\LoadObject.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\Locator.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\maplocator.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\modifiers.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\MovePointer.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\Objects.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\Party.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\path.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\Pickpocket.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\portals.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\regions.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\registration.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\script.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\scriptfuncs.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\spellbook.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\spellfuncs.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\Spells.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\texturemanager.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\things.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\updateworld.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\walls.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\water.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\wavread.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\World.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\Source\actions.h
# End Source File
# Begin Source File

SOURCE=..\Source\animrange.h
# End Source File
# Begin Source File

SOURCE=..\Source\area.h
# End Source File
# Begin Source File

SOURCE=..\Source\attacks.h
# End Source File
# Begin Source File

SOURCE=..\Source\cavewall.h
# End Source File
# Begin Source File

SOURCE=..\Source\Chunks.h
# End Source File
# Begin Source File

SOURCE=..\Source\CombatManager.h
# End Source File
# Begin Source File

SOURCE=..\Source\Corpse.h
# End Source File
# Begin Source File

SOURCE=..\Source\creatures.h
# End Source File
# Begin Source File

SOURCE=..\Source\EditRegion.h
# End Source File
# Begin Source File

SOURCE=..\Source\entrance.h
# End Source File
# Begin Source File

SOURCE=..\Source\equipobject.h
# End Source File
# Begin Source File

SOURCE=..\Source\events.h
# End Source File
# Begin Source File

SOURCE=..\Source\file_input.h
# End Source File
# Begin Source File

SOURCE=..\Source\Flags.h
# End Source File
# Begin Source File

SOURCE=..\Source\forest.h
# End Source File
# Begin Source File

SOURCE=..\Source\gameitem.h
# End Source File
# Begin Source File

SOURCE=..\Source\items.h
# End Source File
# Begin Source File

SOURCE=..\Source\Locator.h
# End Source File
# Begin Source File

SOURCE=..\Source\maplocator.h
# End Source File
# Begin Source File

SOURCE=..\Source\modifiers.h
# End Source File
# Begin Source File

SOURCE=..\Source\MovePointer.h
# End Source File
# Begin Source File

SOURCE=..\Source\Objects.h
# End Source File
# Begin Source File

SOURCE=..\Source\party.h
# End Source File
# Begin Source File

SOURCE=..\Source\path.h
# End Source File
# Begin Source File

SOURCE=..\Source\portals.h
# End Source File
# Begin Source File

SOURCE=..\Source\regions.h
# End Source File
# Begin Source File

SOURCE=..\Source\registration.h
# End Source File
# Begin Source File

SOURCE=..\Source\Resfile.h
# End Source File
# Begin Source File

SOURCE=..\Source\script.h
# End Source File
# Begin Source File

SOURCE=..\Source\scriptfuncs.h
# End Source File
# Begin Source File

SOURCE=..\Source\spellbook.h
# End Source File
# Begin Source File

SOURCE=..\Source\spellfuncs.h
# End Source File
# Begin Source File

SOURCE=..\Source\spells.h
# End Source File
# Begin Source File

SOURCE=..\Source\texturemanager.h
# End Source File
# Begin Source File

SOURCE=..\Source\things.h
# End Source File
# Begin Source File

SOURCE=..\Source\walls.h
# End Source File
# Begin Source File

SOURCE=..\Source\water.h
# End Source File
# Begin Source File

SOURCE=..\Source\wavread.h
# End Source File
# Begin Source File

SOURCE=..\Source\World.h
# End Source File
# End Group
# Begin Group "Engine"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Source\defs.h
# End Source File
# Begin Source File

SOURCE=..\Source\wave.c
# End Source File
# Begin Source File

SOURCE=..\Source\wave.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSEngine.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSFontEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSFontEngine.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSGraphics.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSGraphics.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSinput.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSinput.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSModel.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSModel.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSModelEx.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSmodelEx.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSsound.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSsound.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZStexture.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZStextures.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSutilities.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSutilities.h
# End Source File
# End Group
# Begin Group "FX"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Source\aura.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\aura.h
# End Source File
# Begin Source File

SOURCE=..\Source\blood.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\blood.h
# End Source File
# Begin Source File

SOURCE=..\Source\castaura.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\CastAura.h
# End Source File
# Begin Source File

SOURCE=..\Source\explosion.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\explosion.h
# End Source File
# Begin Source File

SOURCE=..\Source\fireball.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\fireball.h
# End Source File
# Begin Source File

SOURCE=..\Source\fountain.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\fountain.h
# End Source File
# Begin Source File

SOURCE=..\Source\healaura.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\healaura.h
# End Source File
# Begin Source File

SOURCE=..\Source\meshfx.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\meshfx.h
# End Source File
# Begin Source File

SOURCE=..\Source\missile.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\missile.h
# End Source File
# Begin Source File

SOURCE=..\Source\pattern.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\pattern.h
# End Source File
# Begin Source File

SOURCE=..\Source\zsfire.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\zsfire.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSparticle.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSparticle.h
# End Source File
# Begin Source File

SOURCE=..\Source\zsweapontrace.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\zsweapontrace.h
# End Source File
# End Group
# Begin Group "Windowing System"

# PROP Default_Filter ""
# Begin Group "Base Controls"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Source\ZSbutton.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\zsbutton.h
# End Source File
# Begin Source File

SOURCE=..\Source\zsconfirm.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\zsconfirm.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSDescribe.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\zsdescribe.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSEditWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSEditWindow.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSFloatSpin.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSFloatSpin.h
# End Source File
# Begin Source File

SOURCE=..\Source\zsgetnumber.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\zsgetnumber.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSGetText.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZsGetText.h
# End Source File
# Begin Source File

SOURCE=..\Source\zsIntSpin.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSIntSpin.h
# End Source File
# Begin Source File

SOURCE=..\Source\zsitemslot.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\zsitemslot.h
# End Source File
# Begin Source File

SOURCE=..\Source\zslistbox.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSListBox.h
# End Source File
# Begin Source File

SOURCE=..\Source\zsOldlistbox.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSOldListBox.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSProgress.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSProgress.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSShrinkWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSShrinkWindow.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSText.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSText.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSVerticalScroll.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSVerticalScroll.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSwindow.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSwindow.h
# End Source File
# End Group
# Begin Group "Dialogs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Source\actionmenuclass.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\actionmenuclass.h
# End Source File
# Begin Source File

SOURCE=..\Source\Barter.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\Barter.h
# End Source File
# Begin Source File

SOURCE=..\Source\CharacterWin.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\CharacterWin.h
# End Source File
# Begin Source File

SOURCE=..\Source\CreatePartyWin.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\CreatePartyWin.h
# End Source File
# Begin Source File

SOURCE=..\Source\deathwin.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\deathwin.h
# End Source File
# Begin Source File

SOURCE=..\Source\equipeditorwin.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\equipeditorwin.h
# End Source File
# Begin Source File

SOURCE=..\Source\equipwin.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\Equipwin.h
# End Source File
# Begin Source File

SOURCE=..\Source\inventorywin.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\inventorywin.h
# End Source File
# Begin Source File

SOURCE=..\Source\journal.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\journal.h
# End Source File
# Begin Source File

SOURCE=..\Source\MainWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\mainwindow.h
# End Source File
# Begin Source File

SOURCE=..\Source\mapwin.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\mapwin.h
# End Source File
# Begin Source File

SOURCE=..\Source\minimap.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\Minimap.h
# End Source File
# Begin Source File

SOURCE=..\Source\peopleedit.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\peopleedit.h
# End Source File
# Begin Source File

SOURCE=..\Source\pickpocket.h
# End Source File
# Begin Source File

SOURCE=..\Source\skillwin.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\skillwin.h
# End Source File
# Begin Source File

SOURCE=..\Source\StartScreen.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\StartScreen.h
# End Source File
# Begin Source File

SOURCE=..\Source\translucentwindow.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\TranslucentWindow.h
# End Source File
# Begin Source File

SOURCE=..\Source\worldedit.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\Worldedit.h
# End Source File
# Begin Source File

SOURCE=..\Source\zsactionwindow.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\zsactionwindow.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSaskwin.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSaskwin.h
# End Source File
# Begin Source File

SOURCE=..\Source\zsbrushwindow.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\zsbrushwindow.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSCaveEdit.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSCaveEdit.h
# End Source File
# Begin Source File

SOURCE=..\Source\zscontainer.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\zscontainer.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZScutscene.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSCutScene.h
# End Source File
# Begin Source File

SOURCE=..\Source\zsgettarget.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\zsgettarget.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSHelpWin.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSHelpWin.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSMainDescribe.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSMainDescribe.h
# End Source File
# Begin Source File

SOURCE=..\Source\zsmenubar.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\zsmenubar.h
# End Source File
# Begin Source File

SOURCE=..\Source\zsMessage.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\zsmessage.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSobjectwindow.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\zsobjectwindow.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSOptions.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSOptions.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSportrait.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSportrait.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSREGIONEDIT.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSREGIONEDIT.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSRest.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSRest.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSSaveLoad.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSSaveLoad.h
# End Source File
# Begin Source File

SOURCE=..\Source\zssaychar.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\zssaychar.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSScriptWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSScriptWindow.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSSpellWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSSpellWindow.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZSTalk.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\ZSTalk.h
# End Source File
# Begin Source File

SOURCE=..\Source\ZStoolwindow.cpp
# End Source File
# Begin Source File

SOURCE=..\Source\zstoolwindow.h
# End Source File
# End Group
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\icon1.ico
# End Source File
# Begin Source File

SOURCE=..\Script1.rc
# End Source File
# End Group
# Begin Group "Libraries"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\..\Program Files\Microsoft Visual Studio\Vc98\Lib\Winmm.lib"
# End Source File
# Begin Source File

SOURCE=..\Source\bass.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\dx7a\lib\dxguid.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\dx7a\lib\d3dim.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\dx7a\lib\d3dx.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\dx7a\lib\ddraw.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\dx7a\lib\dinput.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\dx7a\lib\dplayx.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\dx7a\lib\DSETUP.lib
# End Source File
# Begin Source File

SOURCE=..\..\..\dx7a\lib\dsound.lib
# End Source File
# End Group
# End Target
# End Project
