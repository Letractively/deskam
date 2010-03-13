#include "DeskamFilter.h"

#include <Windows.h>
#include "dllsetup.h"

#ifndef NO_AUTOLINK
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "vfw32.lib")
#endif

static const wchar_t *global_memory_name = L"Local\\DeskamConfig";
static const wchar_t *memory_name = L"Local\\DeskamConfig";
static const size_t memory_size = sizeof(DeskamGlobalConfig);

static const AMOVIESETUP_MEDIATYPE pinTypes =
{
    &MEDIATYPE_Video,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
};

static const AMOVIESETUP_PIN outputPin =
{
    L"Output",              // Pin string name
    FALSE,                  // Is it rendered
    TRUE,                   // Is it an output
    FALSE,                  // Can we have none
    FALSE,                  // Can we have many
    &CLSID_NULL,            // Connects to filter
    NULL,                   // Connects to pin
    1,                      // Number of types
    &pinTypes
};

const AMOVIESETUP_FILTER g_DeskamFilter =
{
    &CLSID_DeskamFilter,		// Filter CLSID
    L"Deskam",				// String name
    MERIT_DO_NOT_USE,		// Filter merit
    1,						// Number pins
    &outputPin				// Pin details
};

CFactoryTemplate g_Templates[] =
{
	{
		L"Deskam",
		&CLSID_DeskamFilter,
		DeskamFilter::CreateInstance,
		NULL,
		&g_DeskamFilter,
	},

	{
		L"Deskam Configuration Page",
		&CLSID_DeskamConfigPage,
		DeskamConfigPage::CreateInstance,
		NULL,
		NULL,
	},
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

/**
 * Helper function to retreive active configuration.
 */
inline DeskamGlobalConfig *config(SharedMemory &mem)
{
	return reinterpret_cast<DeskamGlobalConfig*>(mem.ptr());
}

// =========================== //
// DeskamStream Implementation //
// =========================== //
DeskamStream::DeskamStream(HRESULT *phr, CSource *pms, LPCWSTR pName):
	CSourceStream(L"Deskam Stream", phr, pms, pName),
	mMem(global_memory_name, memory_size, true)
{
	mX = 0;
	mY = 0;
	mWidth = GetSystemMetrics(SM_CXSCREEN);
	mHeight = GetSystemMetrics(SM_CYSCREEN);
	mOutWidth = 400;
	mOutHeight = 300;
	mSleepTime = 400; // ~2fps. :3

	if(!mMem)
		mMem.open(memory_name, memory_size, true);

	if(DeskamGlobalConfig *cfg = config(mMem))
	{
		mX = cfg->x;
		mY = cfg->y;
		mWidth = cfg->width;
		mHeight = cfg->height;
		mOutWidth = cfg->outWidth;
		mOutHeight = cfg->outHeight;
		mSleepTime = cfg->sleep;
	}

	GetMediaType(0, &m_mt);
	SetFormat(&m_mt);
	
	mClock = NULL;
	IMediaFilter *flt = NULL;
	HRESULT hr = pms->GetSyncSource(&mClock);
	if(FAILED(hr) || !mClock)
	{
		hr = QueryInterface(IID_IMediaFilter, (void**)&flt);
		if(SUCCEEDED(hr))
			hr = flt->GetSyncSource(&mClock);
	}

	if(!mClock)
	{
		if(FAILED(CoCreateInstance(CLSID_SystemClock, NULL, CLSCTX_INPROC_SERVER, IID_IReferenceClock, (void**)&mClock)))
			mClock = NULL;
	}
}

DeskamStream::~DeskamStream()
{
}

STDMETHODIMP DeskamStream::NonDelegatingQueryInterface(REFIID riid, __deref_out void ** ppv)
{
	if(riid == IID_IKsPropertySet)
		return GetInterface((IKsPropertySet*)this, ppv);
	else if(riid == IID_IAMStreamConfig)
		return GetInterface((IAMStreamConfig*)this, ppv);
	else if(riid == IID_IDeskamConfig)
		return GetInterface((IDeskamConfig*)this, ppv);

	return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT STDMETHODCALLTYPE DeskamStream::Set(REFGUID guidPropSet, DWORD dwPropID,
	__in_bcount(cbInstanceData) LPVOID pInstanceData, DWORD cbInstanceData,
	__in_bcount(cbPropData) LPVOID pPropData, DWORD cbPropData)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE DeskamStream::Get(REFGUID guidPropSet, DWORD dwPropID,
	__in_bcount(cbInstanceData) LPVOID pInstanceData, DWORD cbInstanceData,
	__out_bcount_part(cbPropData, *pcbReturned) LPVOID pPropData, DWORD cbPropData,
	__out  DWORD *pcbReturned)
{
	if (guidPropSet != AMPROPSETID_Pin)
		return E_PROP_SET_UNSUPPORTED;

	if (dwPropID != AMPROPERTY_PIN_CATEGORY)
		return E_PROP_ID_UNSUPPORTED;

	if (pPropData == NULL && pcbReturned == NULL)
		return E_POINTER;

	if (pcbReturned)
		*pcbReturned = sizeof(GUID);

	if (pPropData == NULL)  // Caller just wants to know the size.
		return S_OK;

	if (cbPropData < sizeof(GUID)) // The buffer is too small.
		return E_UNEXPECTED;

	*(GUID *)pPropData = PIN_CATEGORY_CAPTURE;
	return S_OK;
}
    
HRESULT STDMETHODCALLTYPE DeskamStream::QuerySupported( 
	REFGUID guidPropSet,
	DWORD dwPropID,
	__out  DWORD *pTypeSupport)
{
	if (guidPropSet != AMPROPSETID_Pin)
		return E_PROP_SET_UNSUPPORTED;

	if (dwPropID != AMPROPERTY_PIN_CATEGORY)
		return E_PROP_ID_UNSUPPORTED;
	
	// We support getting this property, but not setting it.
	if (pTypeSupport)
		*pTypeSupport = KSPROPERTY_SUPPORT_GET; 

	return S_OK;
}

STDMETHODIMP DeskamStream::SetFormat(AM_MEDIA_TYPE *pmt)
{
	if(!pmt || pmt->formattype != FORMAT_VideoInfo)
		return E_INVALIDARG;

	VIDEOINFO *vi = (VIDEOINFO*)pmt->pbFormat;
	int x = vi->bmiHeader.biWidth;
	int y = vi->bmiHeader.biHeight;

	if(pmt != &m_mt)
		return S_OK;

	mOutWidth = x;
	mOutHeight = y;
	m_mt = *pmt;

    IPin* pin; 
    ConnectedTo(&pin);
    if(pin)
    {
		DeskamFilter *flt = dynamic_cast<DeskamFilter*>(m_pFilter);
		if(flt)
		{
	        IFilterGraph *pGraph = flt->GetGraph();
	        pGraph->Reconnect(this);
		}
    }

	return S_OK;
}

STDMETHODIMP DeskamStream::GetFormat(AM_MEDIA_TYPE **ppmt)
{
	if(ppmt)
		*ppmt = CreateMediaType(&m_mt);

	return S_OK;
}

STDMETHODIMP DeskamStream::GetNumberOfCapabilities(int *piCount, int *piSize)
{
	if(piCount)
		*piCount = 1;

	if(piSize)
		*piSize = sizeof(VIDEO_STREAM_CONFIG_CAPS);

	return S_OK;
}

STDMETHODIMP DeskamStream::GetStreamCaps(int iIndex, AM_MEDIA_TYPE **ppmt, BYTE *pSCC)
{
	if(iIndex < 0)
		return E_INVALIDARG;

	if(ppmt)
	{
		CMediaType mt;
		HRESULT hr = GetMediaType(0, &mt);
		if(!SUCCEEDED(hr))
			return hr;

		*ppmt = CreateMediaType(&mt);
	}

	if(pSCC)
	{
		VIDEO_STREAM_CONFIG_CAPS *caps = (VIDEO_STREAM_CONFIG_CAPS*)pSCC;
		ZeroMemory(caps, sizeof(VIDEO_STREAM_CONFIG_CAPS));

		caps->guid = FORMAT_VideoInfo;
		caps->VideoStandard = AnalogVideo_None;
		caps->InputSize.cx = GetSystemMetrics(SM_CXSCREEN);
		caps->InputSize.cy = GetSystemMetrics(SM_CYSCREEN);
		caps->MinCroppingSize.cx = 16;
		caps->MinCroppingSize.cy = 9;
		caps->MaxCroppingSize.cx = GetSystemMetrics(SM_CXSCREEN);
		caps->MaxCroppingSize.cy = GetSystemMetrics(SM_CYSCREEN);
		caps->CropGranularityX = 16;
		caps->CropGranularityY = 9;
		caps->CropAlignX = 1;
		caps->CropAlignY = 1;
		
		caps->MinOutputSize.cx = 160;
		caps->MinOutputSize.cy = 90;
		caps->MaxOutputSize.cx = GetSystemMetrics(SM_CXSCREEN);
		caps->MaxOutputSize.cy = GetSystemMetrics(SM_CYSCREEN);
		caps->MinFrameInterval = 2000000;   //500 fps (0.02 x 10^7)
		caps->MaxFrameInterval = 500000000; // 0.02 fps
		caps->MinBitsPerSecond = (160 * 90 * 4 * 8) / 5;
		caps->MaxBitsPerSecond = GetSystemMetrics(SM_CXSCREEN) * GetSystemMetrics(SM_CYSCREEN) * 4 * 8 * 50;

		caps->OutputGranularityX = 1;
		caps->OutputGranularityY = 1;
		caps->StretchTapsX = 1;
		caps->StretchTapsY = 1;
		caps->ShrinkTapsX = 1;
		caps->ShrinkTapsY = 1;
	}

	return S_OK;
}

STDMETHODIMP DeskamStream::Notify(IBaseFilter * pSender, Quality q)
{
	return S_OK;
}

HRESULT DeskamStream::CheckMediaType(const CMediaType *pMediaType)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    if ((*(pMediaType->Type()) != MEDIATYPE_Video)	// we only output video!
	    || !(pMediaType->IsFixedSize()) ) {		// ...in fixed size samples
                return E_INVALIDARG;
    }

    // Check for the subtypes we support
    const GUID *SubType = pMediaType->Subtype();
    if ((*SubType != MEDIASUBTYPE_RGB8)
            //&& (*SubType != MEDIASUBTYPE_RGB565)
		    //&& (*SubType != MEDIASUBTYPE_RGB555)
	 	    //&& (*SubType != MEDIASUBTYPE_RGB24)
		    && (*SubType != MEDIASUBTYPE_RGB32)) {
                return E_INVALIDARG;
    }

    VIDEOINFO *pvi = (VIDEOINFO *) pMediaType->Format();

    if (pvi == NULL)
		return E_INVALIDARG;

    if ((pvi->bmiHeader.biWidth < 20) || (pvi->bmiHeader.biHeight < 20) ) {
		return E_INVALIDARG;
    }

	return S_OK;
}

HRESULT DeskamStream::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    if (iPosition < 0) {
        return E_INVALIDARG;
    }
	
    if (iPosition > 20) {
        return VFW_S_NO_MORE_ITEMS;
    }

    VIDEOINFO *pvi = (VIDEOINFO *) pMediaType->AllocFormatBuffer(sizeof(VIDEOINFO));
    if (pvi == NULL)
		return(E_OUTOFMEMORY);

    ZeroMemory(pvi, sizeof(VIDEOINFO));

	pvi->bmiHeader.biCompression = BI_RGB;
	pvi->bmiHeader.biBitCount = 32;
	pvi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pvi->bmiHeader.biWidth = mOutWidth;
	pvi->bmiHeader.biHeight = mOutHeight;
	pvi->bmiHeader.biPlanes = 1;
	pvi->bmiHeader.biSizeImage = GetBitmapSize(&pvi->bmiHeader);
	pvi->bmiHeader.biClrImportant = 0;

    pvi->AvgTimePerFrame = 1000000;

	SetRectEmpty(&pvi->rcSource);
	SetRectEmpty(&pvi->rcTarget);

	pMediaType->SetType(&MEDIATYPE_Video);
	pMediaType->SetSubtype(&MEDIASUBTYPE_RGB32);
	pMediaType->SetFormatType(&FORMAT_VideoInfo);
	pMediaType->SetTemporalCompression(FALSE);

    const GUID SubTypeGUID = GetBitmapSubtype(&pvi->bmiHeader);
    pMediaType->SetSubtype(&SubTypeGUID);
    pMediaType->SetSampleSize(pvi->bmiHeader.biSizeImage);

    return S_OK;
}

HRESULT DeskamStream::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties)
{
	CAutoLock cAutoLock(m_pFilter->pStateLock());
	ASSERT(pAlloc);
	ASSERT(pProperties);
	HRESULT hr = NOERROR;

	VIDEOINFO *pvi = (VIDEOINFO *) m_mt.Format();
	pProperties->cBuffers = 1;
	pProperties->cbBuffer = pvi->bmiHeader.biSizeImage;

	ASSERT(pProperties->cbBuffer);

	// Ask the allocator to reserve us some sample memory, NOTE the function
	// can succeed (that is return NOERROR) but still not have allocated the
	// memory that we requested, so we must check we got whatever we wanted

	ALLOCATOR_PROPERTIES Actual;
	hr = pAlloc->SetProperties(pProperties,&Actual);
	if (FAILED(hr)) {
		return hr;
	}

	// Is this allocator unsuitable

	if (Actual.cbBuffer < pProperties->cbBuffer) {
		return E_FAIL;
	}

	// Make sure that we have only 1 buffer (we erase the ball in the
	// old buffer to save having to zero a 200k+ buffer every time
	// we draw a frame)

	ASSERT(Actual.cBuffers == 1);
	return S_OK;
}

HRESULT DeskamStream::FillBuffer(IMediaSample *pms)
{
	if(DeskamGlobalConfig *cfg = config(mMem))
	{
		mX = cfg->x;
		mY = cfg->y;
		mWidth = cfg->width;
		mHeight = cfg->height;
		mSleepTime = cfg->sleep;

		if(cfg->outWidth != mOutWidth)
		{
			SetOutputWidth(cfg->outWidth);
		}

		if(cfg->outHeight != mOutHeight)
		{
			SetOutputHeight(cfg->outHeight);
		}
	}

	Sleep(mSleepTime);
	{
		REFERENCE_TIME rtNow = mLastTime;
		if(mClock)
		{
			mClock->GetTime(&rtNow);
			rtNow -= mStartTime;
		}

		pms->SetTime(&mLastTime, &rtNow);
		pms->SetSyncPoint(TRUE);
		mLastTime = rtNow;

		BYTE *buf = 0;
		HRESULT hr = pms->GetPointer(&buf);
		if(!SUCCEEDED(hr))
			return hr;

		HDC hScreenDC = ::GetDC(NULL);
		HDC hMemDC = ::CreateCompatibleDC(hScreenDC);
		HBITMAP hbm = ::CreateCompatibleBitmap(hScreenDC, mOutWidth, mOutHeight);

		SelectObject(hMemDC, hbm);
		if(mWidth == mOutWidth && mHeight == mOutHeight)
		{
			if(!BitBlt(hMemDC, 0, 0, mOutWidth, mOutHeight, hScreenDC, mX, mY, SRCCOPY))
				return E_FAIL;
		}
		else
		{
			if(!StretchBlt(hMemDC, 0, 0, mOutWidth, mOutHeight, hScreenDC, mX, mY, mWidth, mHeight, SRCCOPY))
				return E_FAIL;
		}

		BITMAPINFO bmi;
		bmi.bmiHeader.biBitCount=0;
		bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);

		BITMAP bm;
		if(GetObject(hbm, sizeof(BITMAP), &bm) != 0)
		{
			if(GetDIBits(hMemDC, hbm, 0, 0, NULL, &bmi, DIB_RGB_COLORS) == 0)
				return E_FAIL;

			bmi.bmiHeader.biCompression=BI_RGB;

			if(GetDIBits(hMemDC, hbm, 0, bmi.bmiHeader.biHeight, buf, &bmi, DIB_RGB_COLORS) == 0)
				return E_FAIL;
		}   

		DeleteObject(hbm);
		DeleteDC(hMemDC);
		ReleaseDC(NULL, hScreenDC);
		return S_OK;
	}
}

HRESULT DeskamStream::OnThreadCreate()
{
	mLastTime = 0;
	mStartTime = 0;
	if(mClock)
		mClock->GetTime(&mStartTime);

	return CSourceStream::OnThreadCreate();
}

STDMETHODIMP DeskamStream::GetX(int *_x)
{
	if(_x != NULL)
		*_x = mX;

	return S_OK;
}

STDMETHODIMP DeskamStream::SetX(int _x)
{
	if(_x < 0 || _x >= GetSystemMetrics(SM_CXSCREEN))
		return E_INVALIDARG;

	mX = _x;

	if(DeskamGlobalConfig *cfg = config(mMem))
		cfg->x = _x;

	return S_OK;
}

STDMETHODIMP DeskamStream::GetY(int *_y)
{
	if(_y != NULL)
		*_y = mY;

	return S_OK;
}

STDMETHODIMP DeskamStream::SetY(int _y)
{
	if(_y < 0 || _y >= GetSystemMetrics(SM_CYSCREEN))
		return E_INVALIDARG;

	mY = _y;

	if(DeskamGlobalConfig *cfg = config(mMem))
		cfg->y = _y;

	return S_OK;
}

STDMETHODIMP DeskamStream::GetWidth(int *_wid)
{
	if(_wid != NULL)
		*_wid = mWidth;

	return S_OK;
}

STDMETHODIMP DeskamStream::SetWidth(int _wid)
{
	if(_wid <= 0 || _wid >= GetSystemMetrics(SM_CXSCREEN))
		return E_INVALIDARG;

	mWidth = _wid;

	if(DeskamGlobalConfig *cfg = config(mMem))
		cfg->width = _wid;

	return S_OK;
}

STDMETHODIMP DeskamStream::GetHeight(int *_hei)
{
	if(_hei != NULL)
		*_hei = mHeight;

	return S_OK;
}

STDMETHODIMP DeskamStream::SetHeight(int _hei)
{
	if(_hei <= 0 || _hei >= GetSystemMetrics(SM_CXSCREEN))
		return E_INVALIDARG;

	mHeight = _hei;

	if(DeskamGlobalConfig *cfg = config(mMem))
		cfg->height = _hei;

	return S_OK;
}

STDMETHODIMP DeskamStream::GetOutputWidth(int *_wid)
{
	if(_wid != NULL)
		*_wid = mOutWidth;

	return S_OK;
}

STDMETHODIMP DeskamStream::SetOutputWidth(int _wid)
{
	if(_wid <= 0)
		return E_INVALIDARG;

	mOutWidth = _wid;

	if(DeskamGlobalConfig *cfg = config(mMem))
		cfg->outWidth = _wid;

    /*IPin* pin; 
    ConnectedTo(&pin);
    if(pin)
    {
		DeskamFilter *flt = dynamic_cast<DeskamFilter*>(m_pFilter);
		if(flt)
		{
	        IFilterGraph *pGraph = flt->GetGraph();
	        pGraph->Reconnect(this);
		}
    }*/
	return S_OK;
}

STDMETHODIMP DeskamStream::GetOutputHeight(int *_hei)
{
	if(_hei != NULL)
		*_hei = mOutHeight;

	return S_OK;
}

STDMETHODIMP DeskamStream::SetOutputHeight(int _hei)
{
	if(_hei <= 0)
		return E_INVALIDARG;

	mOutHeight = _hei;

	if(DeskamGlobalConfig *cfg = config(mMem))
		cfg->outHeight = _hei;


    /*IPin* pin; 
    ConnectedTo(&pin);
    if(pin)
    {
		DeskamFilter *flt = dynamic_cast<DeskamFilter*>(m_pFilter);
		if(flt)
		{
	        IFilterGraph *pGraph = flt->GetGraph();
	        pGraph->Reconnect(this);
		}
    }*/
	return S_OK;
}

STDMETHODIMP DeskamStream::GetSleepTime(int *_slp)
{
	if(_slp)
		*_slp = mSleepTime;

	return S_OK;
}

STDMETHODIMP DeskamStream::SetSleepTime(int _slp)
{
	if(_slp < 0 || _slp > 5000)
		return E_INVALIDARG;

	mSleepTime = _slp;

	if(DeskamGlobalConfig *cfg = config(mMem))
		cfg->sleep = _slp;

	return S_OK;
}

// =========================== //
// DeskamFilter Implementation //
// =========================== //

DeskamFilter::DeskamFilter(LPUNKNOWN pUnk, HRESULT *phr):
	CSource(L"Deskam", pUnk, CLSID_DeskamFilter)
{
    CAutoLock cAutoLock(pStateLock());
	mStrm = new DeskamStream(phr, this, L"~Capture");
	if(!mStrm)
	{
		*phr = E_OUTOFMEMORY;
		return;
	}
}

DeskamFilter::~DeskamFilter(void)
{
	if(mStrm)
	{
		// Dealt with by base classes, but we should nullify our
		// reference, just in case...
		mStrm = NULL;
	}
}

STDMETHODIMP DeskamFilter::NonDelegatingQueryInterface(REFIID riid, __deref_out void ** ppv)
{
	if(riid == IID_IAMStreamConfig
		|| riid == IID_IKsPropertySet
		|| riid == IID_IDeskamConfig)
		return mStrm->NonDelegatingQueryInterface(riid, ppv);
	else if(riid == IID_ISpecifyPropertyPages)
		return GetInterface((ISpecifyPropertyPages*)this, ppv);

	return CSource::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP DeskamFilter::GetPages(CAUUID *pPages)
{
	if(pPages == NULL)
		return E_POINTER;

	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID));
	pPages->pElems[0] = CLSID_DeskamConfigPage;

	return S_OK;
}

CUnknown *DeskamFilter::CreateInstance(IUnknown *pUnk, HRESULT *phr)
{
	DeskamFilter *ret = new DeskamFilter(pUnk, phr);

	if(!ret)
		*phr = E_OUTOFMEMORY;

	return ret;
}

// ================= //
// AX Implementation //
// ================= //

#define CreateComObject(clsid, iid, var) CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, iid, (void **)&var);

STDAPI AMovieSetupRegisterServer(CLSID clsServer, LPCWSTR szDescription, LPCWSTR szFileName,
	LPCWSTR szThreadingModel = L"Both", LPCWSTR szServerType = L"InprocServer32");
STDAPI AMovieSetupUnregisterServer(CLSID clsServer);

STDAPI RegisterFilters( BOOL bRegister )
{
    HRESULT hr = NOERROR;
    WCHAR achFileName[MAX_PATH];
    char achTemp[MAX_PATH];
    ASSERT(g_hInst != 0);

    if( 0 == GetModuleFileNameA(g_hInst, achTemp, sizeof(achTemp))) 
        return AmHresultFromWin32(GetLastError());

    MultiByteToWideChar(CP_ACP, 0L, achTemp, lstrlenA(achTemp) + 1, 
                       achFileName, NUMELMS(achFileName));
  
    hr = CoInitialize(0);
    if(bRegister)
    {
        hr = AMovieSetupRegisterServer(CLSID_DeskamFilter, L"Deskam", achFileName, L"Both", L"InprocServer32");
		if(SUCCEEDED(hr))
			hr = AMovieSetupRegisterServer(CLSID_DeskamConfigPage, L"Deskam Configuration Page", achFileName, L"Both", L"InprocServer32");
    }

    if(SUCCEEDED(hr))
    {
        IFilterMapper2 *fm = 0;
        hr = CreateComObject(CLSID_FilterMapper2, IID_IFilterMapper2, fm);
        if(SUCCEEDED(hr))
        {
            if(bRegister)
            {
                IMoniker *pMoniker = 0;
                REGFILTER2 rf2;
                rf2.dwVersion = 1;
                rf2.dwMerit = MERIT_DO_NOT_USE;
                rf2.cPins = 1;
                rf2.rgPins = &outputPin;
                hr = fm->RegisterFilter(CLSID_DeskamFilter, L"Deskam", &pMoniker, &CLSID_VideoInputDeviceCategory, NULL, &rf2);
            }
            else
            {
                hr = fm->UnregisterFilter(&CLSID_VideoInputDeviceCategory, 0, CLSID_DeskamFilter);
            }
        }

      // release interface
      //
      if(fm)
          fm->Release();
    }

    if( SUCCEEDED(hr) && !bRegister )
	{
        hr = AMovieSetupUnregisterServer(CLSID_DeskamFilter);
		hr = AMovieSetupUnregisterServer(CLSID_DeskamConfigPage);
	}

    CoFreeUnusedLibraries();
    CoUninitialize();
    return hr;
}

STDAPI DllRegisterServer()
{
    return RegisterFilters(TRUE);
}

STDAPI DllUnregisterServer()
{
    return RegisterFilters(FALSE);
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

SharedMemory shared_memory;
BOOL APIENTRY DllMain(HANDLE hModule, DWORD  dwReason, LPVOID lpReserved)
{
	if(dwReason == DLL_PROCESS_ATTACH)
	{
		if(!shared_memory.open(memory_name, memory_size, true) && shared_memory.open(memory_name, memory_size, false))
		{
			if(DeskamGlobalConfig *cfg = reinterpret_cast<DeskamGlobalConfig*>(shared_memory.ptr()))
			{
				cfg->x = 0;
				cfg->y = 0;
				cfg->width = GetSystemMetrics(SM_CXSCREEN);
				cfg->height = GetSystemMetrics(SM_CYSCREEN);
				cfg->outWidth = 1280;
				cfg->outHeight = 720;
				cfg->sleep = 400; // ~2fps
			}
		}
	}
	else if(dwReason == DLL_PROCESS_DETACH)
		shared_memory.close();

	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}