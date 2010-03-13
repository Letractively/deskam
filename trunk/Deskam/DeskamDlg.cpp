
// DeskamDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Deskam.h"
#include "DeskamDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CDeskamDlg dialog




CDeskamDlg::CDeskamDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDeskamDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDeskamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDeskamDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_APPLY, &CDeskamDlg::OnApply)
END_MESSAGE_MAP()


// CDeskamDlg message handlers

BOOL CDeskamDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Load box values from global config! -- Ricky26
	if(DeskamGlobalConfig *cfg = theApp.config())
	{
		SetDlgItemInt(IDC_X_TXT, cfg->x);
		SetDlgItemInt(IDC_Y_TXT, cfg->y);
		SetDlgItemInt(IDC_WIDTH_TXT, cfg->width);
		SetDlgItemInt(IDC_HEIGHT_TXT, cfg->height);
		SetDlgItemInt(IDC_OUTWIDTH_TXT, cfg->outWidth);
		SetDlgItemInt(IDC_OUTHEIGHT_TXT, cfg->outHeight);
		SetDlgItemInt(IDC_SLEEP_TXT, cfg->sleep);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDeskamDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDeskamDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDeskamDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

inline void setDlgInt(CDeskamDlg *self, int iid, int &dest)
{
	BOOL ok = FALSE;
	int val = self->GetDlgItemInt(iid, &ok);
	if(ok)
		dest = val;
}

void CDeskamDlg::OnApply()
{
	if(DeskamGlobalConfig *cfg = theApp.config())
	{
		setDlgInt(this, IDC_X_TXT, cfg->x);
		setDlgInt(this, IDC_Y_TXT, cfg->y);
		setDlgInt(this, IDC_WIDTH_TXT, cfg->width);
		setDlgInt(this, IDC_HEIGHT_TXT, cfg->height);
		setDlgInt(this, IDC_OUTWIDTH_TXT, cfg->outWidth);
		setDlgInt(this, IDC_OUTHEIGHT_TXT, cfg->outHeight);
		setDlgInt(this, IDC_SLEEP_TXT, cfg->sleep);
	}
}
