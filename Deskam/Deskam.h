
// Deskam.h : main header file for the PROJECT_NAME application
//

#pragma once

#include "SharedMemory.h"
#include "DeskamConfig.h"

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CDeskamApp:
// See Deskam.cpp for the implementation of this class
//

class CDeskamApp : public CWinApp
{
public:
	CDeskamApp();
	
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	DeskamGlobalConfig *config();

protected:
	SharedMemory mMemory;
};

extern CDeskamApp theApp;