
// Deskam.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Deskam.h"
#include "DeskamDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDeskamApp

BEGIN_MESSAGE_MAP(CDeskamApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CDeskamApp construction

CDeskamApp::CDeskamApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}


// The one and only CDeskamApp object

CDeskamApp theApp;


// CDeskamApp initialization

BOOL CDeskamApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	SetRegistryKey(_T("Deskam"));

	if((!mMemory.open(L"Local\\DeskamConfig", sizeof(DeskamGlobalConfig), true)  && 
		mMemory.open(L"Local\\DeskamConfig", sizeof(DeskamGlobalConfig), false))
		|| (!mMemory.open(L"Local\\DeskamConfig", sizeof(DeskamGlobalConfig), true)  && 
		mMemory.open(L"Local\\DeskamConfig", sizeof(DeskamGlobalConfig), false)))
	{
		if(DeskamGlobalConfig *cfg = config())
		{
			cfg->x = GetProfileInt(L"", L"X", 0);
			cfg->y = GetProfileInt(L"", L"Y", 0);
			cfg->width = GetProfileInt(L"", L"Width", GetSystemMetrics(SM_CYSCREEN));
			cfg->height = GetProfileInt(L"", L"Height", GetSystemMetrics(SM_CXSCREEN));
			cfg->outWidth = GetProfileInt(L"", L"OutputWidth", 1280);
			cfg->outHeight = GetProfileInt(L"", L"OutputHeight", 720);
			cfg->sleep = GetProfileInt(L"", L"SleepTime", 400); // ~2fps;
		}
	}

	CDeskamDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

int CDeskamApp::ExitInstance()
{
	if(DeskamGlobalConfig *cfg = config())
	{
		WriteProfileInt(L"", L"X", cfg->x);
		WriteProfileInt(L"", L"Y", cfg->y);
		WriteProfileInt(L"", L"Width", cfg->width);
		WriteProfileInt(L"", L"Height", cfg->height);
		WriteProfileInt(L"", L"OutputWidth", cfg->outWidth);
		WriteProfileInt(L"", L"OutputHeight", cfg->outHeight);
		WriteProfileInt(L"", L"SleepTime", cfg->sleep);
	}

	return CWinApp::ExitInstance();
}

DeskamGlobalConfig *CDeskamApp::config()
{
	if(!mMemory)
		return NULL;

	return reinterpret_cast<DeskamGlobalConfig*>(mMemory.ptr());
}
