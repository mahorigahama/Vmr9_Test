#pragma once

#define D3DFVF (D3DFVF_XYZRHW | D3DFVF_TEX0)

struct SCALER_VTX {
    FLOAT x, y, z, w;
	FLOAT u, v;
};

class CAutoLock {
	CComMultiThreadModel::AutoCriticalSection &m_CritSec;
public:
	CAutoLock(CComMultiThreadModel::AutoCriticalSection &sec) :
		m_CritSec(sec)
	{
		m_CritSec.Lock();
	}
	~CAutoLock() {
		m_CritSec.Unlock();
	}
};

class ATL_NO_VTABLE CScaler :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IVMRSurfaceAllocator9,
	IVMRImagePresenter9
{
	BEGIN_COM_MAP(CScaler)
		COM_INTERFACE_ENTRY(IVMRSurfaceAllocator9)
		COM_INTERFACE_ENTRY(IVMRImagePresenter9)
	END_COM_MAP()
	DECLARE_PROTECT_FINAL_CONSTRUCT();
private:
	CComMultiThreadModel::AutoCriticalSection m_CritSec;

	HWND m_Window;
	CComPtr<IDirect3D9Ex> m_D3D;
	CComPtr<IDirect3DDevice9Ex> m_D3DDev;
	CComPtr<IDirect3DSurface9> m_RenderTarget;
	std::vector< CComPtr<IDirect3DSurface9> > m_Surfaces;
	CComPtr<IVMRSurfaceAllocatorNotify9> m_SurfAllocNotify;

	CComPtr<IDirect3DTexture9> m_MirroredTexture;

	CComPtr<IDirect3DVertexBuffer9> m_VtxMirror;
	CComPtr<IDirect3DVertexBuffer9> m_VtxSrc;
	CComPtr<IDirect3DVertexDeclaration9> m_VtxDecl;

	CComPtr<ID3DXEffect> m_Effect;

public:
	HRESULT FinalConstruct();
	void FinalRelease();
	void SetWindow(HWND hwnd);

	// IVMRSurfaceAllocator9
	STDMETHODIMP InitializeDevice( 
		/* [in] */ DWORD_PTR dwUserID,
		/* [in] */ VMR9AllocationInfo *lpAllocInfo,
		/* [out][in] */ DWORD *lpNumBuffers);
	STDMETHODIMP TerminateDevice( 
		/* [in] */ DWORD_PTR dwID);
	STDMETHODIMP GetSurface( 
		/* [in] */ DWORD_PTR dwUserID,
		/* [in] */ DWORD SurfaceIndex,
		/* [in] */ DWORD SurfaceFlags,
		/* [out] */ IDirect3DSurface9 **lplpSurface);
	STDMETHODIMP AdviseNotify( 
		/* [in] */ IVMRSurfaceAllocatorNotify9 *lpIVMRSurfAllocNotify);

	// IVMRImagePresenter9
	STDMETHODIMP StartPresenting( 
		/* [in] */ DWORD_PTR dwUserID);
	STDMETHODIMP StopPresenting( 
		/* [in] */ DWORD_PTR dwUserID);
	STDMETHODIMP PresentImage( 
		/* [in] */ DWORD_PTR dwUserID,
		/* [in] */ VMR9PresentationInfo *lpPresInfo);
private:
	HRESULT CreateDevice();
	HRESULT CreatePresParam(D3DPRESENT_PARAMETERS &pp);
	void ReleaseResources();
};
