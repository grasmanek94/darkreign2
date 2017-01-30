; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CSplashScreen
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "WhiteBoard.h"
LastPage=0

ClassCount=12
Class1=CWhiteBoardApp
Class2=CWhiteBoardDoc
Class3=CWhiteBoardView
Class4=CMainFrame

ResourceCount=7
Resource1=IDR_MAINFRAME
Class5=CAboutDlg
Resource2=IDD_ABOUTBOX
Class6=CLoginDialog
Resource3=IDD_STROKESIZEDIALOG
Class7=CStrokeSizeDialog
Class8=CChatView
Class9=CInputView
Class10=CChatRoomsDialog
Resource4=IDD_LOGIN
Class11=CBoardsDialog
Resource5=IDD_BOARDS
Resource6=IDD_ROOMS
Class12=CSplashScreen
Resource7=IDD_SPLASHSCREEN

[CLS:CWhiteBoardApp]
Type=0
HeaderFile=WhiteBoard.h
ImplementationFile=WhiteBoard.cpp
Filter=N

[CLS:CWhiteBoardDoc]
Type=0
HeaderFile=WhiteBoardDoc.h
ImplementationFile=WhiteBoardDoc.cpp
Filter=N
LastObject=CWhiteBoardDoc
BaseClass=CDocument
VirtualFilter=DC

[CLS:CWhiteBoardView]
Type=0
HeaderFile=WhiteBoardView.h
ImplementationFile=WhiteBoardView.cpp
Filter=C
BaseClass=CScrollView
VirtualFilter=VWC
LastObject=ID_OPTIONS_STROKE_COLOR


[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
BaseClass=CFrameWnd
VirtualFilter=fWC
LastObject=CMainFrame




[CLS:CAboutDlg]
Type=0
HeaderFile=WhiteBoard.cpp
ImplementationFile=WhiteBoard.cpp
Filter=D
LastObject=CAboutDlg

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_APP_EXIT
Command2=ID_OPTIONS_LOGIN
Command3=ID_OPTIONS_ROOMS
Command4=ID_OPTIONS_BOARDS
Command5=ID_OPTIONS_STROKE_SIZE
Command6=ID_OPTIONS_STROKE_COLOR
Command7=ID_APP_ABOUT
CommandCount=7

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_UNDO
Command5=ID_EDIT_CUT
Command6=ID_EDIT_COPY
Command7=ID_EDIT_PASTE
Command8=ID_EDIT_UNDO
Command9=ID_EDIT_CUT
Command10=ID_EDIT_COPY
Command11=ID_EDIT_PASTE
Command12=ID_NEXT_PANE
Command13=ID_PREV_PANE
CommandCount=13

[CLS:CLoginDialog]
Type=0
HeaderFile=LoginDialog.h
ImplementationFile=LoginDialog.cpp
BaseClass=CDialog
Filter=D
LastObject=CLoginDialog
VirtualFilter=dWC

[DLG:IDD_LOGIN]
Type=1
Class=CLoginDialog
ControlCount=7
Control1=IDC_EDIT_LOGINNAME,edit,1350631552
Control2=IDC_EDIT_PASSWORD,edit,1350631584
Control3=IDOK,button,1342242817
Control4=IDCANCEL,button,1342242816
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATUS,static,1342308352

[CLS:CChatView]
Type=0
HeaderFile=ChatView.h
ImplementationFile=ChatView.cpp
BaseClass=CView
Filter=C
VirtualFilter=VWC
LastObject=CChatView

[CLS:CInputView]
Type=0
HeaderFile=InputView.h
ImplementationFile=InputView.cpp
BaseClass=CEditView
Filter=C
VirtualFilter=VWC
LastObject=CInputView

[DLG:IDD_BOARDS]
Type=1
Class=CChatRoomsDialog
ControlCount=4
Control1=IDC_EDIT_ROOMNAME,edit,1350631552
Control2=IDC_LIST_ROOMS,listbox,1352728835
Control3=IDOK,button,1342242817
Control4=IDCANCEL,button,1342242816

[CLS:CBoardsDialog]
Type=0
HeaderFile=BoardsDialog.h
ImplementationFile=BoardsDialog.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CBoardsDialog

[DLG:IDD_ROOMS]
Type=1
Class=CChatRoomsDialog
ControlCount=4
Control1=IDC_EDIT_ROOMNAME,edit,1350631552
Control2=IDC_LIST_ROOMS,listbox,1352728835
Control3=IDOK,button,1342242817
Control4=IDCANCEL,button,1342242816

[DLG:IDD_STROKESIZEDIALOG]
Type=1
Class=CStrokeSizeDialog
ControlCount=3
Control1=IDC_EDIT2,edit,1350639616
Control2=IDOK,button,1342242817
Control3=IDC_STATIC,static,1342308865

[CLS:CStrokeSizeDialog]
Type=0
HeaderFile=StrokeSizeDialog.h
ImplementationFile=StrokeSizeDialog.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CStrokeSizeDialog

[CLS:CChatRoomsDialog]
Type=0
HeaderFile=chatroomsdialog.h
ImplementationFile=chatroomsdialog.cpp
BaseClass=CDialog
Filter=D
LastObject=CChatRoomsDialog
VirtualFilter=dWC

[DLG:IDD_SPLASHSCREEN]
Type=1
Class=CSplashScreen
ControlCount=1
Control1=IDC_SPLASHSTATIC,static,1342308865

[CLS:CSplashScreen]
Type=0
HeaderFile=SplashScreen.h
ImplementationFile=SplashScreen.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_SPLASHSTATIC

