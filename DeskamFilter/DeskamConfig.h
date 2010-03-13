#pragma once

#include "streams.h"

struct DeskamGlobalConfig
{
	int x;
	int y;
	int width;
	int height;
	int outWidth;
	int outHeight;
	int sleep;
};

// {75AAA454-3C9E-4189-BA41-A634EF876EB4}
static const GUID IID_IDeskamConfig = 
{ 0x75aaa454, 0x3c9e, 0x4189, { 0xba, 0x41, 0xa6, 0x34, 0xef, 0x87, 0x6e, 0xb4 } };

class IDeskamConfig: public IUnknown
{
public:
	STDMETHOD(GetX)(int *_x) PURE;
	STDMETHOD(SetX)(int _x) PURE;
	STDMETHOD(GetY)(int *_y) PURE;
	STDMETHOD(SetY)(int _y) PURE;
	STDMETHOD(GetWidth)(int *_wid) PURE;
	STDMETHOD(SetWidth)(int _wid) PURE;
	STDMETHOD(GetHeight)(int *_hei) PURE;
	STDMETHOD(SetHeight)(int _hei) PURE;
	
	STDMETHOD(GetOutputWidth)(int *_wid) PURE;
	STDMETHOD(SetOutputWidth)(int _wid) PURE;
	STDMETHOD(GetOutputHeight)(int *_hei) PURE;
	STDMETHOD(SetOutputHeight)(int _hei) PURE;

	STDMETHOD(GetSleepTime)(int *_slp) PURE;
	STDMETHOD(SetSleepTime)(int _slp) PURE;
};

// {06E0C80D-C525-48BD-B47B-97D311EFB132}
static const GUID CLSID_DeskamConfigPage = 
{ 0x6e0c80d, 0xc525, 0x48bd, { 0xb4, 0x7b, 0x97, 0xd3, 0x11, 0xef, 0xb1, 0x32 } };

class DeskamConfigPage: public CBasePropertyPage
{
public:
	DeskamConfigPage(IUnknown *pUnk);
	virtual ~DeskamConfigPage();

	HRESULT OnConnect(IUnknown *pUnk);
	HRESULT OnActivate();
	HRESULT OnApplyChanges();
	BOOL OnReceiveMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static CUnknown *WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

protected:
	void SetDirty();

	IDeskamConfig *mConfig;
};
