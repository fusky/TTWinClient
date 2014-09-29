;作者：快刀
; NSIS 编译器版本： 2.44

!define SOURCE_DIR "bin"

SetCompressor /SOLID  /FINAL lzma
SetCompressorDictSize 32

!include "Library.nsh"
!include "MUI.nsh"

/******************************
 *   安装程序初始定义常量  *
 ******************************/
;常用量
!define INSTALL_EXE_NAME					"teamtalk-1.0.exe"
!define MAIN_EXE_NAME      					"teamtalk.exe"
!define UPDATE_EXE_NAME						"teamtalkdownload.exe"
!define SPEEXDEC_EXE_NAME                   "speexdec.exe"
!define IOS 								"ioSpecial.ini"
!define INSTALL_BIN							"bin"

;产品信息
!define PRODUCT_NAME 						"TeamTalk"
!define INSTALL_DIR							"TeamTalk"
!define PRODUCT_DETAIL 						"TeamTalk安装程序"
!define PRODUCT_VERSION 					"1.0"
!define PRODUCT_VERSION_DETAIL				"1.0.0.1"
!define PRODUCT_PUBLISHER 					"蘑菇街"
!define PRODUCT_WEB_SITE 					"http://tt.mogu.io/"

;键
!define PRODUCT_UNINST_KEY 					"Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_DIR_REGKEY 					"Software\Microsoft\Windows\CurrentVersion\App Paths\${MAIN_EXE_NAME}"

;全局变量定义								
Var 	g_IsInstalled					;是否安装过的判断
Var		g_InstallPath					;安装路径

; ------ MUI 现代界面定义 (1.67 版本以上兼容) ------

; MUI 预定义常量
!define MUI_ABORTWARNING
!define MUI_WELCOMEFINISHPAGE_BITMAP "res\WelcomeImage.bmp"
!define MUI_ICON "res\installer.ico"
!define MUI_UNICON "res\uninstaller.ico"

; 欢迎页面
!insertmacro MUI_PAGE_WELCOME
; 安装目录选择页面
!insertmacro MUI_PAGE_DIRECTORY
; 安装过程页面
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\${PRODUCT_NAME}新特性.txt"
!define MUI_FINISHPAGE_SHOWREADME_TEXT "显示新特性"
;!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
!define MUI_PAGE_CUSTOMFUNCTION_PRE  FinishPagePre
!define MUI_PAGE_CUSTOMFUNCTION_SHOW  FinishPageShow
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE  FinishPageLeave
!insertmacro MUI_PAGE_FINISH

; 安装卸载过程页面
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; 安装界面包含的语言设置
!insertmacro MUI_LANGUAGE "SimpChinese"
; When using waskin.dll, XPStyle must be off.
; And we specify it after '!insertmacro MUI_LANGUAGE "English"'
; so we can override Modern UI default of "XPStyle on".
XPStyle on

; 安装预释放文件
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS
; ------ MUI 现代界面定义结束 ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile ${INSTALL_EXE_NAME}
InstallDir "$PROGRAMFILES\${INSTALL_DIR}"
ShowInstDetails show
ShowUnInstDetails show
BrandingText ${PRODUCT_NAME}
LicenseText "" "我同意(I)"
UninstallButtonText "解除安装(U)"

VIProductVersion ${PRODUCT_VERSION_DETAIL}
VIAddVersionKey  "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey "CompanyName" ${PRODUCT_PUBLISHER}
VIAddVersionKey "FileDescription" "${PRODUCT_NAME}"
VIAddVersionKey "ProductVersion" ${PRODUCT_VERSION_DETAIL}
VIAddVersionKey "FileVersion" ${PRODUCT_VERSION_DETAIL}
VIAddVersionKey "Comments" "请仔细阅读许可协议"
VIAddVersionKey "LegalCopyright" ${PRODUCT_PUBLISHER}

/******************************
*  安装程序的安装section部分  *
******************************/
Section "MainSection" SEC01
	SetOutPath "$INSTDIR"
	SetOverwrite on
  /*判断操作系统版本是否是Vista，是的话给其安装目录增加everyone权限*/
	GetDllVersion "$SYSDIR\user32.dll" $R0 $R1
	strcpy $1 $R0
	strcmp $1 "" Skip
	IntOp $R2 $R0 / 0x00010000
	IntOp $R3 $R0 & 0x0000FFFF
	StrCpy $0 "$R2$R3"
	IntCmp $0 60 cacl Skip cacl
cacl:
	ExecShell "open" "cacls" '"$INSTDIR" /e /t /g everyone:F' "SW_HIDE" 
	/*判断操作系统版本是否是Vista，是的话给其安装目录增加everyone权限*/
Skip:
	Delete "$INSTDIR\bin\CrashDumper.exe"
	File /r "${SOURCE_DIR}\*.*"
	/*强制删除数据库更新*/
    ;RMDir /r "$INSTDIR\users\download\"
    /*强制删除config.dat版本升级*/
    ;Delete "$INSTDIR\bin\config.dat"
	Delete "$INSTDIR\data\duoduo_water.wav"
    ;删除JS src文件
    RMDir /r "$INSTDIR\data\module\js\src"
    RMDir /r "$INSTDIR\gui\shading"
    RMDir /r "$INSTDIR\gui\preview"
    Delete "$INSTDIR\gui\skin_base.xml"
    Delete "$INSTDIR\bin\debugConfig.xml"
    
    ExecWait '"regsvr32.exe /s"  "$INSTDIR\${INSTALL_BIN}\GifSmiley.dll"'    
SectionEnd

Section -AdditionalIcons
	;删除原先的快捷方式
  Delete "$INSTDIR\${PRODUCT_NAME}.url"
  Delete "$DESKTOP\${PRODUCT_NAME}.lnk"
  Delete "$SENDTO\${PRODUCT_NAME}.lnk"
  Delete "$QUICKLAUNCH\${PRODUCT_NAME}.lnk"	
  RMDir /r "$SMPROGRAMS\${PRODUCT_NAME}"
  SetShellVarContext all
  Delete "$DESKTOP\${PRODUCT_NAME}.lnk"
  Delete "$SENDTO\${PRODUCT_NAME}.lnk"
  Delete "$QUICKLAUNCH\${PRODUCT_NAME}.lnk"	
  RMDir /r "$SMPROGRAMS\${PRODUCT_NAME}"
  ;添加快捷方式到all user 目录下
  SetShellVarContext all
  CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\bin\${MAIN_EXE_NAME}"
  CreateShortCut "$SENDTO\${PRODUCT_NAME}.lnk" "$INSTDIR\bin\${MAIN_EXE_NAME}" "/sendto"
  CreateDirectory "$SMPROGRAMS\${PRODUCT_NAME}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\bin\${MAIN_EXE_NAME}"
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\访问网站.lnk" "$INSTDIR\${PRODUCT_NAME}.url" "" "$SYSDIR\url.dll"
  CreateShortCut "$SMPROGRAMS\${PRODUCT_NAME}\卸载${PRODUCT_NAME}.lnk" "$INSTDIR\卸载${PRODUCT_NAME}.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\卸载${PRODUCT_NAME}.exe"

  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "Path" "$INSTDIR"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "Mark" "Installing"
    
;卸载设置
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\卸载${PRODUCT_NAME}.exe"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\bin\${MAIN_EXE_NAME}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr HKLM "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"

SectionEnd

/******************************
*  安装程序的卸载section部分  *
******************************/
Section Uninstall
Check:
	KillProcDLL::KillProc "${MAIN_EXE_NAME}"
	KillProcDLL::KillProc "${UPDATE_EXE_NAME}"
    KillProcDLL::KillProc "${SPEEXDEC_EXE_NAME}"
	sleep 200
	FindProcDll::FindProc "${MAIN_EXE_NAME}"
	IntCmp $R0 1 Check 0
	FindProcDll::FindProc "${UPDATE_EXE_NAME}"
	IntCmp $R0 1 Check 0
    FindProcDll::FindProc "${SPEEXDEC_EXE_NAME}"
	IntCmp $R0 1 Check GoOn
GoOn:


;快捷方式,开始菜单
	Delete "$INSTDIR\${PRODUCT_NAME}.url"
	Delete "$DESKTOP\${PRODUCT_NAME}.lnk"
	Delete "$SENDTO\${PRODUCT_NAME}.lnk"  
	Delete "$QUICKLAUNCH\${PRODUCT_NAME}.lnk"	
	RMDir /r "$SMPROGRAMS\${PRODUCT_NAME}"
	SetShellVarContext all
	Delete "$DESKTOP\${PRODUCT_NAME}.lnk"
	Delete "$SENDTO\${PRODUCT_NAME}.lnk"
	Delete "$QUICKLAUNCH\${PRODUCT_NAME}.lnk"	
	RMDir /r "$SMPROGRAMS\${PRODUCT_NAME}"
		
;文件列表
	Delete "$INSTDIR\${PRODUCT_NAME}"
	Delete "$INSTDIR\卸载${PRODUCT_NAME}.exe"
	Delete "$INSTDIR\${PRODUCT_NAME}新特性.txt"
	Delete "$INSTDIR\stringtable.ini"
	
;文件夹列表
	RMDir /r "$INSTDIR\gui"
	RMDir /r "$INSTDIR\bin"
	RMDir /r "$INSTDIR\data"
	RMDir /r "$INSTDIR\users"
	RMDir  "$INSTDIR"
	
	DeleteRegKey 	HKLM "${PRODUCT_UNINST_KEY}"
	WriteRegStr 	HKLM "${PRODUCT_DIR_REGKEY}" "Mark" "UnInstalling"
	DeleteRegValue  HKLM "${PRODUCT_DIR_REGKEY}" "SilentMark"
		
	SetAutoClose true
SectionEnd

#-- 根据 NSIS 脚本编辑规则，所有 Function 区段必须放置在 Section 区段之后编写，以避免安装程序出现未可预知的问题。--#

/******************************
 *  安装程序的安装函数部分  		*
 ******************************/
 Function .onInit
	InitPluginsDir
		
;检测相关程序是否在运行
	Call CheckRunning
  
	ReadRegStr $0 HKLM "${PRODUCT_DIR_REGKEY}" "Path"  	;使用这个键来寻找旧版本安装路径
	ReadRegStr $1 HKLM "${PRODUCT_DIR_REGKEY}" "Mark"  	;使用这个键来表示旧版本状态:installing or uninstall
	Strcmp $1 "Installing" installed	end				 ;这时候$1应该有2种状态 uninstall 或者是 空字符串
installed:
	StrCpy  $g_IsInstalled		"Installed"
	StrCpy  $g_InstallPath 		$0
	StrCpy $INSTDIR $g_InstallPath
	
end: 
FunctionEnd

Function  CheckRunning 
Check:
	FindProcDll::FindProc "${MAIN_EXE_NAME}"
	IntCmp $R0 1 Running 0
	FindProcDll::FindProc "${UPDATE_EXE_NAME}"
	IntCmp $R0 1 Running 0
	FindProcDll::FindProc "${SPEEXDEC_EXE_NAME}"
	IntCmp $R0 1 Running End    
Running:
	;如果是强制升级，不跳出提示直接杀死进程
	IfSilent kill prompt
prompt:
	MessageBox MB_ICONINFORMATION|MB_YESNO "系统检测到有$(^Name)或相关程序正在运行，点击$\r“是”将强制关闭并继续安装,点击“否 ”直接退出安装程序" IDYES +2
	Quit
kill:
	KillProcDLL::KillProc "${MAIN_EXE_NAME}"
	KillProcDLL::KillProc "${UPDATE_EXE_NAME}"
    KillProcDLL::KillProc "${SPEEXDEC_EXE_NAME}"
	sleep 200
	Goto Check
End:
FunctionEnd

Function  FinishPagePre
	WriteINIStr "$PLUGINSDIR\${IOS}" "Settings" "NumFields" "6" 
	WriteINIStr "$PLUGINSDIR\${IOS}" "Field 5" "Type" "CheckBox" 
	WriteINIStr "$PLUGINSDIR\${IOS}" "Field 5" "State" "1"
	WriteINIStr "$PLUGINSDIR\${IOS}" "Field 5" "Text" "运行$(^Name)"
	WriteINIStr "$PLUGINSDIR\${IOS}" "Field 5" "Left" "120"
	WriteINIStr "$PLUGINSDIR\${IOS}" "Field 5" "Right" "315" 
	WriteINIStr "$PLUGINSDIR\${IOS}" "Field 5" "Top" "108"
	WriteINIStr "$PLUGINSDIR\${IOS}" "Field 5" "Bottom" "119" 

	WriteINIStr "$PLUGINSDIR\${IOS}" "Field 6" "Type" "CheckBox" 
	WriteINIStr "$PLUGINSDIR\${IOS}" "Field 6" "State" "1"
	WriteINIStr "$PLUGINSDIR\${IOS}" "Field 6" "Text" "创建快速启动"
	WriteINIStr "$PLUGINSDIR\${IOS}" "Field 6" "Left" "120"
	WriteINIStr "$PLUGINSDIR\${IOS}" "Field 6" "Right" "315" 
	WriteINIStr "$PLUGINSDIR\${IOS}" "Field 6" "Top" "129"
	WriteINIStr "$PLUGINSDIR\${IOS}" "Field 6" "Bottom" "140" 
FunctionEnd

;用来皮肤不需要设置该控件的背景色
Function  FinishPageShow 
    ReadINIStr $0 "$PLUGINSDIR\${IOS}" "Field 5" "HWND"
    SetCtlColors $0 0x000000 0xFFFFFF 
    ReadINIStr $0 "$PLUGINSDIR\${IOS}" "Field 6" "HWND"
    SetCtlColors $0 0x000000 0xFFFFFF 
FunctionEnd

Function  FinishPageLeave 
	ReadINIStr $0 "$PLUGINSDIR\${IOS}" "Field 6" "State" 
    StrCmp $0 "1" +1 +2
    CreateShortCut "$QUICKLAUNCH\${PRODUCT_NAME}.lnk" "$INSTDIR\bin\${MAIN_EXE_NAME}"
	
	ReadINIStr $0 "$PLUGINSDIR\${IOS}" "Field 5" "State" 
	StrCmp $0 "1" +1 end
	Exec '"$INSTDIR\bin\${MAIN_EXE_NAME}"'
end:
FunctionEnd

/******************************
*  安装程序的卸载函数部分  	  *
******************************/
Function un.onInit
	Call un.CheckRunning
FunctionEnd

;安装时，;检测相关程序是否在运行
Function  un.CheckRunning 
Check:
	FindProcDll::FindProc "${MAIN_EXE_NAME}"
	IntCmp $R0 1 Running 0
	FindProcDll::FindProc "${UPDATE_EXE_NAME}"
	IntCmp $R0 1 Running 0
	FindProcDll::FindProc "${SPEEXDEC_EXE_NAME}"
	IntCmp $R0 1 Running End        
Running:
	MessageBox MB_ICONINFORMATION|MB_YESNO "系统检测到有$(^Name)或相关程序正在运行，点击$\r“是”将强制关闭并继续安装,点击“否 ”直接退出安装程序" IDYES +2
	Quit
	KillProcDLL::KillProc "${MAIN_EXE_NAME}"
	KillProcDLL::KillProc "${UPDATE_EXE_NAME}"
    KillProcDLL::KillProc "${SPEEXDEC_EXE_NAME}"
	sleep 200
	Goto Check
End:
FunctionEnd

Function un.onUninstSuccess
	MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) 已成功地从你的计算机移除。"
FunctionEnd
