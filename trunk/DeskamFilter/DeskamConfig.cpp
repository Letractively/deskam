#include "DeskamConfig.h"
#include "resource.h"

#include <Windows.h>
#include <CommCtrl.h>

DeskamConfigPage::DeskamConfigPage(IUnknown *pUnk):
	CBasePropertyPage(L"Deskam Configuration Page", pUnk, IDD_DESKAM_CONFIG_DLG, IDS_DESKAM_CONFIG_TITLE)
{
};

DeskamConfigPage::~DeskamConfigPage()
{
	if(mConfig)
	{
		mConfig->Release();
		mConfig = NULL;
	}
}

void DeskamConfigPage::SetDirty()
{
	m_bDirty = TRUE;
	//if(m_pPageSite)
	//	m_pPageSite->OnStatusChange(PROPPAGESTATUS_DIRTY);
}

HRESULT DeskamConfigPage::OnConnect(IUnknown *pUnk)
{
	if(pUnk == NULL)
		return E_POINTER;

	HRESULT hr = pUnk->QueryInterface(IID_IDeskamConfig, reinterpret_cast<void**>(&mConfig));
	if(!SUCCEEDED(hr))
		return hr;

	return CBasePropertyPage::OnConnect(pUnk);
}

HRESULT DeskamConfigPage::OnActivate()
{
	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_BAR_CLASSES;
	
	if(!InitCommonControlsEx(&icc))
		return E_FAIL;

	if(mConfig == NULL)
		return E_POINTER;

	int x=0, y=0, width=0, height=0, outWidth=0, outHeight=0, sleep=0;
	HRESULT hr = mConfig->GetX(&x);
	if(SUCCEEDED(hr))
	{
		SetDlgItemInt(m_Dlg, IDC_X_TXT, x, TRUE);
	}

	hr = mConfig->GetY(&y);
	if(SUCCEEDED(hr))
	{
		SetDlgItemInt(m_Dlg, IDC_Y_TXT, y, TRUE);
	}

	hr = mConfig->GetWidth(&width);
	if(SUCCEEDED(hr))
	{
		SetDlgItemInt(m_Dlg, IDC_WIDTH_TXT, width, TRUE);
	}

	hr = mConfig->GetHeight(&height);
	if(SUCCEEDED(hr))
	{
		SetDlgItemInt(m_Dlg, IDC_HEIGHT_TXT, height, TRUE);
	}

	hr = mConfig->GetOutputWidth(&outWidth);
	if(SUCCEEDED(hr))
	{
		SetDlgItemInt(m_Dlg, IDC_OUTWIDTH_TXT, outWidth, TRUE);
	}

	hr = mConfig->GetOutputHeight(&outHeight);
	if(SUCCEEDED(hr))
	{
		SetDlgItemInt(m_Dlg, IDC_OUTHEIGHT_TXT, outHeight, TRUE);
	}

	hr = mConfig->GetSleepTime(&sleep);
	if(SUCCEEDED(hr))
	{
		SetDlgItemInt(m_Dlg, IDC_SLEEP_TXT, sleep, TRUE);
	}
	
	return CBasePropertyPage::OnActivate();
}

HRESULT DeskamConfigPage::OnApplyChanges()
{
	BOOL ok = FALSE;
	HRESULT hr = S_OK;
	int val = (int)GetDlgItemInt(m_Dlg, IDC_X_TXT, &ok, TRUE);
	if(ok)
	{
		hr = mConfig->SetX(val);
		if(FAILED(hr))
			return hr;
	}

	val = (int)GetDlgItemInt(m_Dlg, IDC_Y_TXT, &ok, TRUE);
	if(ok)
	{
		hr = mConfig->SetY(val);
		if(FAILED(hr))
			return hr;
	}

	val = (int)GetDlgItemInt(m_Dlg, IDC_WIDTH_TXT, &ok, TRUE);
	if(ok)
	{
		hr = mConfig->SetWidth(val);
		if(FAILED(hr))
			return hr;
	}

	val = (int)GetDlgItemInt(m_Dlg, IDC_HEIGHT_TXT, &ok, TRUE);
	if(ok)
	{
		hr = mConfig->SetHeight(val);
		if(FAILED(hr))
			return hr;
	}

	val = (int)GetDlgItemInt(m_Dlg, IDC_OUTWIDTH_TXT, &ok, TRUE);
	if(ok)
	{
		hr = mConfig->SetOutputWidth(val);
		if(FAILED(hr))
			return hr;
	}

	val = (int)GetDlgItemInt(m_Dlg, IDC_OUTHEIGHT_TXT, &ok, TRUE);
	if(ok)
	{
		hr = mConfig->SetOutputHeight(val);
		if(FAILED(hr))
			return hr;
	}

	val = (int)GetDlgItemInt(m_Dlg, IDC_SLEEP_TXT, &ok, TRUE);
	if(ok)
	{
		hr = mConfig->SetSleepTime(val);
		if(FAILED(hr))
			return hr;
	}

	return CBasePropertyPage::OnApplyChanges();
}

BOOL DeskamConfigPage::OnReceiveMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//if(uMsg == WM_SETTEXT)
		SetDirty();

	return CBasePropertyPage::OnReceiveMessage(hWnd, uMsg, wParam, lParam);
}

CUnknown *WINAPI DeskamConfigPage::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
	DeskamConfigPage *ret = new DeskamConfigPage(lpunk);
	if(phr)
	{
		if(!ret)
			*phr = E_OUTOFMEMORY;
	}

	return ret;
}
