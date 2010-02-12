; General notes.

; This is a script for NSIS (http://nsis.sourceforge.net). Wiki contains many useful snippets.
; You may want to use HM NIS Edit (http://hmne.sourceforge.net/) for editing this file.

; You may use static or dynamic linking with libs.
; See comments before sections.

; Useful links about deployment:
; http://doc.qt.nokia.com/4.6-snapshot/deployment.html (also see Windows-specific page)
; http://msdn.microsoft.com/en-us/library/zebw5zk9.aspx
; http://www.dependencywalker.com/

!include "smart_uninstall_before_sections.nsh"


!define PRODUCT_NAME "Jabbin"
!define PRODUCT_VERSION "3.0"
!define PRODUCT_PUBLISHER "Jabbin"
!define PRODUCT_WEB_SITE "http://www.jabbin.com"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\jabbin.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

SetCompressor lzma

!include "${NSISDIR}\Contrib\Modern UI\System.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "jabbin_installer.ico"
!define MUI_UNICON "uninstall-cross.ico"

; MUI Settings / Header
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_LEFT
!define MUI_HEADERIMAGE_BITMAP "install-header.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "uninstall-header.bmp"
 
; MUI Settings / Wizard
!define MUI_WELCOMEFINISHPAGE_BITMAP "install-left-bar.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "uninstall-left-bar.bmp"

; Language Selection Dialog Settings
!define MUI_LANGDLL_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_LANGDLL_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "NSIS:Language"

; Welcome page
!insertmacro MUI_PAGE_WELCOME

; License page
!insertmacro MUI_PAGE_LICENSE "..\..\COPYING"

; Directory page
!insertmacro MUI_PAGE_DIRECTORY

; Instfiles page
!insertmacro MUI_PAGE_INSTFILES

; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\jabbin.exe"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Italian"

; Reserve files
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "setup.exe"
InstallDir "$PROGRAMFILES\${PRODUCT_NAME}"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Function .onInit
  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

Section "Main program file"
  ${SetOutPath} "$INSTDIR"
  SetOverwrite ifnewer
  ${File} "" "jabbin.exe"
  ${File} "" "jabbin.url"
  ${CreateDirectory} "$SMPROGRAMS\${PRODUCT_NAME}"
  ${CreateShortCut} "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\jabbin.exe"
  ${CreateShortCut} "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\jabbin.exe"
SectionEnd

Section "Sounds"
  ${SetOutPath} "$INSTDIR\sound"
  SetOverwrite ifnewer
  ${File} "..\..\sound\" "*.wav"
  ${File} "..\..\sound\" "*.mp3"
SectionEnd

Section "Iconsets"
  SetOverwrite ifnewer
  ${SetOutPath} "$INSTDIR\iconsets"

  ${SetOutPath} "$INSTDIR\iconsets\chatwindow"
  ${File} "..\..\iconsets\chatwindow\" "*.png"

  ${SetOutPath} "$INSTDIR\iconsets\emoticons"

  ${SetOutPath} "$INSTDIR\iconsets\emoticons\yaemo"
  ${File} "..\..\iconsets\emoticons\yaemo\" "*.png"
  ${File} "..\..\iconsets\emoticons\yaemo\" "*.xml"
  
  ${SetOutPath} "$INSTDIR\iconsets\emoticons\yaemomidget"
  ${File} "..\..\iconsets\emoticons\yaemomidget\" "*.png"
  ${File} "..\..\iconsets\emoticons\yaemomidget\" "*.xml"

  ${SetOutPath} "$INSTDIR\iconsets\roster"
  ${File} "..\..\iconsets\roster\" "*.png"

  ${SetOutPath} "$INSTDIR\iconsets\roster\aim"
  ${File} "..\..\iconsets\roster\aim\" "*.png"

  ${SetOutPath} "$INSTDIR\iconsets\roster\crystal-aim"
  ${File} "..\..\iconsets\roster\crystal-aim\" "*.png"

  ${SetOutPath} "$INSTDIR\iconsets\roster\crystal-gadugadu"
  ${File} "..\..\iconsets\roster\crystal-gadugadu\" "*.png"

  ${SetOutPath} "$INSTDIR\iconsets\roster\crystal-icq"
  ${File} "..\..\iconsets\roster\crystal-icq\" "*.png"

  ${SetOutPath} "$INSTDIR\iconsets\roster\crystal-jabber"
  ${File} "..\..\iconsets\roster\crystal-jabber\" "*.png"

  ${SetOutPath} "$INSTDIR\iconsets\roster\crystal-msn"
  ${File} "..\..\iconsets\roster\crystal-msn\" "*.png"

  ${SetOutPath} "$INSTDIR\iconsets\roster\crystal-yahoo"
  ${File} "..\..\iconsets\roster\crystal-yahoo\" "*.png"

  ${SetOutPath} "$INSTDIR\iconsets\roster\default"
  ${File} "..\..\iconsets\roster\default\" "*.png"
  ${File} "..\..\iconsets\roster\default\" "*.xml"

  ${SetOutPath} "$INSTDIR\iconsets\roster\facebook"
  ${File} "..\..\iconsets\roster\facebook\" "*.png"

  ${SetOutPath} "$INSTDIR\iconsets\roster\gadugadu"
  ${File} "..\..\iconsets\roster\gadugadu\" "*.png"

  ${SetOutPath} "$INSTDIR\iconsets\roster\gtalk"
  ${File} "..\..\iconsets\roster\gtalk\" "*.png"

  ${SetOutPath} "$INSTDIR\iconsets\roster\icq"
  ${File} "..\..\iconsets\roster\icq\" "*.png"

  ${SetOutPath} "$INSTDIR\iconsets\roster\jabber"
  ${File} "..\..\iconsets\roster\jabber\" "*.png"

  ${SetOutPath} "$INSTDIR\iconsets\roster\jabbin"
  ${File} "..\..\iconsets\roster\jabbin\" "*.png"

  ${SetOutPath} "$INSTDIR\iconsets\roster\msn"
  ${File} "..\..\iconsets\roster\msn\" "*.png"

  ${SetOutPath} "$INSTDIR\iconsets\roster\myspace"
  ${File} "..\..\iconsets\roster\myspace\" "*.png"

  ${SetOutPath} "$INSTDIR\iconsets\roster\qq"
  ${File} "..\..\iconsets\roster\qq\" "*.png"

  ${SetOutPath} "$INSTDIR\iconsets\roster\roster-big"
  ${File} "..\..\iconsets\roster\roster-big\" "*.png"
  ${File} "..\..\iconsets\roster\roster-big\" "*.xml"

  ${SetOutPath} "$INSTDIR\iconsets\roster\roster-tabs"
  ${File} "..\..\iconsets\roster\roster-tabs\" "*.png"

  ${SetOutPath} "$INSTDIR\iconsets\roster\yahoo"
  ${File} "..\..\iconsets\roster\yahoo\" "*.png"

SectionEnd

; Comment whole section if you use static linking
Section "Qt libs"
  ${SetOutPath} "$SYSDIR"
  SetOverwrite ifnewer
  ${File} "Qt\" "QtXml4.dll"
  ${File} "Qt\" "QtWebKit4.dll"
  ${File} "Qt\" "QtSql4.dll"
  ${File} "Qt\" "QtScript4.dll"
  ${File} "Qt\" "QtNetwork4.dll"
  ${File} "Qt\" "QtGui4.dll"
  ${File} "Qt\" "QtCore4.dll"
  ${File} "Qt\" "Qt3Support4.dll"
  ${File} "Qt\" "phonon4.dll"
SectionEnd

Section "OpenSSL libs"
  ${SetOutPath} "$SYSDIR"
  SetOverwrite ifnewer
  ${File} "openssl\" "ssleay32.dll"
  ${File} "openssl\" "libeay32.dll"
SectionEnd

Section "Expat lib"
  ${SetOutPath} "$SYSDIR"
  SetOverwrite ifnewer
  ${File} "expat\" "libexpat.dll"
SectionEnd

; Visual Studio Redistributable Package
; Make sure you use version for your compiler, it's very important!
; VS 2008    : http://www.microsoft.com/downloads/details.aspx?familyid=9B2DA534-3E03-4391-8A4D-074B9F2BC1BF
; VS 2008 SP1: http://www.microsoft.com/downloads/details.aspx?familyid=A5C84275-3B97-4AB7-A40D-3802B2AF5FC2
; Also it's possible to build app that will run without this package... You may try if you want, see links above.
Section "Visual Studio Redistributable Package"
  ${SetOutPath} "$TEMP"
  ${File} "" "vcredist_x86.exe"
  ExecWait '"$TEMP\vcredist_x86.exe" /q'
SectionEnd

Section -AdditionalIcons
  ${SetOutPath} $INSTDIR
  ${CreateShortCut} "$SMPROGRAMS\${PRODUCT_NAME}\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  ${CreateShortCut} "$SMPROGRAMS\${PRODUCT_NAME}\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  ${WriteUninstaller} "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\jabbin.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\jabbin.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd


!include "smart_uninstall_after_sections.nsh"
