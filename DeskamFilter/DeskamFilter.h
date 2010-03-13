#pragma once

#include <streams.h>
#include <olectl.h>
#include <initguid.h>
#include <Vfw.h>
#include <map>

#include "DeskamConfig.h"
#include "SharedMemory.h"

class DeskamStream: public CSourceStream, public IAMStreamConfig, public IKsPropertySet, public IDeskamConfig
{
public:
	DeskamStream(HRESULT *phr, CSource *pms, LPCWSTR pName);
	virtual ~DeskamStream();
	
	// IUnknown
	DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, __deref_out void ** ppv);

	// IKsPropertySet
	STDMETHODIMP Set(REFGUID guidPropSet, DWORD dwPropID,
		LPVOID pInstanceData, DWORD cbInstanceData,
		LPVOID pPropData, DWORD cbPropData);

	STDMETHODIMP Get(REFGUID guidPropSet, DWORD dwPropID,
		LPVOID pInstanceData, DWORD cbInstanceData,
		LPVOID pPropData, DWORD cbPropData,
		DWORD *pcbReturned);
        
	STDMETHODIMP QuerySupported( 
		REFGUID guidPropSet,
		DWORD dwPropID,
		DWORD *pTypeSupport);

	// IAMStreamConfig
	STDMETHODIMP SetFormat(AM_MEDIA_TYPE *pmt);
	STDMETHODIMP GetFormat(AM_MEDIA_TYPE **ppmt);
	STDMETHODIMP GetNumberOfCapabilities(int *piCount, int *piSize);
	STDMETHODIMP GetStreamCaps(int iIndex, AM_MEDIA_TYPE **ppmt, BYTE *pSCC);

	// IDeskamConfig
	STDMETHOD(GetX)(int *_x);
	STDMETHOD(SetX)(int _x);
	STDMETHOD(GetY)(int *_y);
	STDMETHOD(SetY)(int _y);
	STDMETHOD(GetWidth)(int *_wid);
	STDMETHOD(SetWidth)(int _wid);
	STDMETHOD(GetHeight)(int *_hei);
	STDMETHOD(SetHeight)(int _hei);
	
	STDMETHOD(GetOutputWidth)(int *_wid);
	STDMETHOD(SetOutputWidth)(int _wid);
	STDMETHOD(GetOutputHeight)(int *_hei);
	STDMETHOD(SetOutputHeight)(int _hei);

	STDMETHOD(GetSleepTime)(int *_slp);
	STDMETHOD(SetSleepTime)(int _slp);

	// CSourceStream
	STDMETHOD(Notify)(IBaseFilter * pSender, Quality q);
	HRESULT CheckMediaType(const CMediaType *pMediaType);
	HRESULT GetMediaType(int iPosition, CMediaType *pMediaType);
	HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties);
	HRESULT FillBuffer(IMediaSample *pms);

	HRESULT OnThreadCreate(void);

protected:
	int mX;
	int mY;
	int mWidth;
	int mHeight;
	int mOutWidth;
	int mOutHeight;
	int mSleepTime;
    REFERENCE_TIME mStartTime;
    REFERENCE_TIME mLastTime;

	IReferenceClock *mClock;

	SharedMemory mMem;
};

// {04BE74EB-763C-4906-B200-F2DCD9876D2E}
static const GUID CLSID_DeskamFilter = 
{ 0x4be74eb, 0x763c, 0x4906, { 0xb2, 0x0, 0xf2, 0xdc, 0xd9, 0x87, 0x6d, 0x2e } };

class DeskamFilter: public CSource, public ISpecifyPropertyPages
{
public:
	DeskamFilter(LPUNKNOWN pUnk, HRESULT *phr);
	virtual ~DeskamFilter();

	DECLARE_IUNKNOWN

    STDMETHOD(NonDelegatingQueryInterface)(REFIID riid, __deref_out void ** ppv);
	STDMETHOD(GetPages)(CAUUID *pPages);

    IFilterGraph *GetGraph() { return m_pGraph; }

	static CUnknown *WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

protected:
	DeskamStream *mStrm;
};