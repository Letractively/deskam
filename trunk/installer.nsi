; The name of the installer
Name "Deskam"

; The file to write
OutFile "deskam_setup.exe"

; The default installation directory
InstallDir $PROGRAMFILES\Deskam

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Deskam" "InstallDir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "Deskam (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "Debug/deskam.exe"
  File "Debug/deskam.ax"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\Deskam "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Deskam" "DisplayName" "Deskam"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Deskam" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Deskam" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Deskam" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Deskam"
  CreateShortCut "$SMPROGRAMS\Deskam\Deskam.lnk" "$INSTDIR\deskam.exe" "" "$INSTDIR\deskam.exe" 0
  CreateShortCut "$SMPROGRAMS\Deskam\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "un.Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Deskam"
  DeleteRegKey HKLM SOFTWARE\Deskam

  ; Remove files and uninstaller
  Delete $INSTDIR\deskam.exe
  Delete $INSTDIR\deskam.ax
  Delete $INSTDIR\uninstall.exe

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Deskam\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Deskam"
  RMDir "$INSTDIR"

SectionEnd
