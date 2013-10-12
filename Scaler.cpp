// Scaler for Direct3D 9Ex

#include"stdafx.h"
#include"Vmr9_Test.h"

namespace {
	const D3DVERTEXELEMENT9 decl_scaler[]={
		{0,  0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};
}

HRESULT CScaler::FinalConstruct() {
	ATLTRACE("%s\n", __FUNCTION__);
	ATLENSURE(Direct3DCreate9Ex(D3D_SDK_VERSION, &m_D3D)==S_OK);
	return S_OK;
}

void CScaler::FinalRelease() {
	ATLTRACE("%s\n", __FUNCTION__);
}

void CScaler::SetWindow(HWND hwnd) {
	ATLENSURE(IsWindow(hwnd)!=FALSE);
	m_Window=hwnd;
	ATLENSURE(m_D3D);
	ATLENSURE(CreateDevice()==S_OK);
}

// IVMRSurfaceAllocator9
STDMETHODIMP CScaler::InitializeDevice(DWORD_PTR dwUserID
	, VMR9AllocationInfo *lpAllocInfo, DWORD *lpNumBuffers)
{
	ATLTRACE("%s\n", __FUNCTION__);
	HRESULT hr=NOERROR;
	ATLENSURE_RETURN_HR(lpAllocInfo, E_POINTER);
	ATLENSURE_RETURN_HR(lpNumBuffers, E_POINTER);
	ATLENSURE_RETURN_HR(m_D3D, E_UNEXPECTED);
	ATLENSURE_RETURN_HR(m_D3DDev, E_UNEXPECTED);
	// 入力画像を包含する2の累乗サイズを求める
	DWORD texture_width=1, texture_height=1;
	while(texture_width <=lpAllocInfo->dwWidth)  texture_width*=2;
	while(texture_height<=lpAllocInfo->dwHeight) texture_height*=2;
	// テクスチャを作成する
	m_D3DDev->CreateTexture(texture_width, texture_height
		, 0, D3DUSAGE_RENDERTARGET
		, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT
		, &m_MirroredTexture.p, NULL);
	// 頂点バッファを作成する
	m_D3DDev->CreateVertexDeclaration(decl_scaler, &m_VtxDecl.p);
	// テクスチャ全面にレンダリングする
	const float f_frame_width=(float)lpAllocInfo->dwWidth;
	const float f_frame_height=(float)lpAllocInfo->dwHeight;
	float ztu=0;
	float ztv=0;
	float tu=f_frame_width/texture_width;
	float tv=f_frame_height/texture_height;
	float lft =-1-  0.5f                 *2.0f/texture_width;
	float itmx=-1+(-0.5f+f_frame_width)  *2.0f/texture_width;
	float rgt =-1+(-0.5f+f_frame_width*2)*2.0f/texture_width;

	float top = 1+ 0.5f                  *2.0f/texture_height;
	float itmy= 1+(0.5f-f_frame_height  )*2.0f/texture_height;
	float btm = 1+(0.5f-f_frame_height*2)*2.0f/texture_height;
	SCALER_VTX vertices_mirror[]={
		// 1
		{lft , itmy, 0, 1, ztu, tv},
		{lft , top , 0, 1, ztv,ztv},
		{itmx, itmy, 0, 1,  tu, tv},
		{itmx, top , 0, 1,  tu,ztv},
		{rgt , itmy, 0, 1, ztu, tv},
		{rgt , top , 0, 1, ztu,ztv},
		// 2
		{lft , btm , 0, 1, ztu,ztv},
		{lft , itmy, 0, 1, ztu, tv},
		{itmx, btm , 0, 1,  tu,ztv},
		{itmx, itmy, 0, 1,  tu, tv},
		{rgt , btm , 0, 1, ztu,ztv},
		{rgt , itmy, 0, 1, ztu, tv},
	};
	// 0.5ピクセル左上にずらす
	D3DSURFACE_DESC sur_desc;
	m_RenderTarget->GetDesc(&sur_desc);
	lft=-1-0.5f*2.0f/sur_desc.Width;
	rgt= 1-0.5f*2.0f/sur_desc.Width;
	top= 1+0.5f*2.0f/sur_desc.Height;
	btm=-1+0.5f*2.0f/sur_desc.Height;
	/*
	// 参考:頂点座標ではなく,テクスチャ座標をずらす場合
	lft=btm=-1;
	rgt=top=1;
	float ztu=+0.5f/width;
	tu       =(wd+0.5f)/width;
	float ztv=   +0.5f /height;
	tv       =(hg+0.5f)/height;
	*/

	SCALER_VTX vertices_lanzcos[]={
		{lft, btm, 0, 1,ztu,tv},
		{lft, top, 0, 1,ztu,ztv},
		{rgt, btm, 0, 1,tu ,tv},
		{rgt, top, 0, 1,tu ,ztv},
	};

	void* pVertices;
	m_D3DDev->CreateVertexBuffer(sizeof(vertices_mirror)
		, 0, D3DFVF, D3DPOOL_DEFAULT, &m_VtxMirror.p, NULL);
	m_VtxMirror->Lock(0, sizeof(vertices_mirror), (void**)&pVertices, 0);
	memcpy(pVertices, vertices_mirror, sizeof(vertices_mirror));
	m_VtxMirror->Unlock();

	m_D3DDev->CreateVertexBuffer(sizeof(vertices_lanzcos)
		, 0, D3DFVF, D3DPOOL_DEFAULT, &m_VtxSrc.p, NULL);
	m_VtxSrc->Lock(0, sizeof(vertices_lanzcos), (void**)&pVertices, 0);
	memcpy(pVertices, vertices_lanzcos, sizeof(vertices_lanzcos));
	m_VtxSrc->Unlock();

	// エフェクトを読み込む
	CComPtr<ID3DXBuffer> err_buffer;
	hr=D3DXCreateEffectFromFile(m_D3DDev, L"filter1.fx", NULL, NULL
		, D3DXSHADER_DEBUG, NULL, &m_Effect.p, &err_buffer.p);
	if(FAILED(hr)) {
		return hr;
	}
	INT size_iarray[2]={texture_width, texture_height};
 	m_Effect->SetIntArray("__tex0_size", size_iarray, 2);

	// テクスチャを lpNumBuffers で指定された数だけ作成する
	lpAllocInfo->dwWidth =texture_width;
	lpAllocInfo->dwHeight=texture_height;
	lpAllocInfo->Format  =D3DFMT_X8R8G8B8;
	m_Surfaces.resize(*lpNumBuffers);
	lpAllocInfo->dwFlags|=VMR9AllocFlag_TextureSurface;
	hr=m_SurfAllocNotify->AllocateSurfaceHelper(lpAllocInfo
		, lpNumBuffers, &m_Surfaces[0]);
	return hr;
}

STDMETHODIMP CScaler::TerminateDevice(DWORD_PTR dwID) {
	ATLTRACE("%s\n", __FUNCTION__);
	ReleaseResources();
	return S_OK;
}

STDMETHODIMP CScaler::GetSurface(DWORD_PTR dwUserID
	, DWORD SurfaceIndex, DWORD SurfaceFlags
	, IDirect3DSurface9 **lplpSurface)
{
	ATLENSURE_RETURN_HR(lplpSurface, E_POINTER);
	ATLENSURE_RETURN_HR(SurfaceIndex<m_Surfaces.size(), E_INVALIDARG);
	CAutoLock my_lock(m_CritSec);
	*lplpSurface=m_Surfaces[SurfaceIndex];
	(*lplpSurface)->AddRef(); // 参照カウンタを増やす
	return S_OK;
}

STDMETHODIMP
CScaler::AdviseNotify(IVMRSurfaceAllocatorNotify9 *lpIVMRSurfAllocNotify) {
	CAutoLock my_lock(m_CritSec);
	ATLTRACE("%s\n", __FUNCTION__);
	ATLENSURE_RETURN_HR(m_D3D, E_UNEXPECTED);
	ATLENSURE_RETURN_HR(m_D3DDev, E_UNEXPECTED);
	ATLENSURE_RETURN_HR(lpIVMRSurfAllocNotify, E_POINTER);
	m_SurfAllocNotify=lpIVMRSurfAllocNotify;
	HMONITOR monitor=m_D3D->GetAdapterMonitor(D3DADAPTER_DEFAULT);
	return m_SurfAllocNotify->SetD3DDevice(m_D3DDev, monitor);
}

// IVMRImagePresenter9
STDMETHODIMP CScaler::StartPresenting(DWORD_PTR dwUserID) {
	CAutoLock my_lock(m_CritSec);
	ATLTRACE("%s\n", __FUNCTION__);
	ATLENSURE_RETURN_HR(m_D3D, E_UNEXPECTED);
	ATLENSURE_RETURN_HR(m_D3DDev, E_UNEXPECTED);
	return S_OK;
}

STDMETHODIMP CScaler::StopPresenting(DWORD_PTR dwUserID) {
	ATLTRACE("%s\n", __FUNCTION__);
	return S_OK;
}

STDMETHODIMP CScaler::PresentImage(DWORD_PTR dwUserID
	, VMR9PresentationInfo *lpPresInfo)
{
	HRESULT hr;
	UINT num_of_passes;
	CAutoLock my_lock(m_CritSec);

	// VMR9PresentationInfo::lpSurf からIDirect3DTexture9を取り出す
	CComPtr<IDirect3DTexture9> src_texture;
	lpPresInfo->lpSurf->GetContainer(
		IID_IDirect3DTexture9, (void**)&src_texture);
	// 領域外参照対策テクスチャからIDirect3DSurface9を取り出す
	CComPtr<IDirect3DSurface9> mirrored_surface;
	m_MirroredTexture->GetSurfaceLevel(0, &mirrored_surface);

	hr=m_D3DDev->BeginScene();
	m_D3DDev->SetVertexDeclaration(m_VtxDecl);

	// 領域外参照対策テクスチャにレンダリングする
	hr=m_D3DDev->SetRenderTarget(0, mirrored_surface);
	hr=m_D3DDev->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0);
	hr=m_Effect->SetTechnique("simple");
	hr=m_Effect->SetTexture("__tex0", src_texture);
	hr=m_D3DDev->SetStreamSource(0, m_VtxMirror, 0, sizeof(SCALER_VTX));
	m_Effect->Begin(&num_of_passes, 0);
	m_Effect->BeginPass(0);
	hr=m_D3DDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 4);
	hr=m_D3DDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 6, 4);
	m_Effect->EndPass();
	m_Effect->End();
	// バックバッファにレンダリングする
	hr=m_D3DDev->SetRenderTarget(0, m_RenderTarget);
	hr=m_Effect->SetTechnique("lanzcos");
	// hr=m_Effect->SetTechnique("simple");
	hr=m_Effect->SetTexture("__tex0", m_MirroredTexture);
	hr=m_D3DDev->SetStreamSource(0, m_VtxSrc, 0, sizeof(SCALER_VTX));
	m_Effect->Begin(&num_of_passes, 0);
	m_Effect->BeginPass(0);
	hr=m_D3DDev->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	m_Effect->EndPass();
	m_Effect->End();

	m_D3DDev->EndScene();

	hr=m_D3DDev->PresentEx(NULL, NULL, NULL, NULL, NULL);
	if(hr==S_PRESENT_MODE_CHANGED) {
		// 解像度が変更されたときにS_PRESENT_MODE_CHANGEDとなる
		// ResetEx を呼び出してデバイスをリセットする必要がある
		D3DPRESENT_PARAMETERS pp;
		CreatePresParam(pp);
		m_D3DDev->ResetEx(&pp, NULL);
		return S_OK;
	}
	// hr==S_PRESENT_OCCLUDED 問題なし
	if(hr==D3DERR_DEVICELOST) {
		// エラーが発生したら全リソースを解放し、再度、初期化する
		ReleaseResources();
		m_RenderTarget.Release();
		m_D3DDev.Release();
		CreateDevice();
		HMONITOR monitor=
			m_D3D->GetAdapterMonitor(D3DADAPTER_DEFAULT);
		hr=m_SurfAllocNotify->ChangeD3DDevice(m_D3DDev, monitor);
	}
	if(hr==D3DERR_DEVICEHUNG || hr==D3DERR_DEVICEREMOVED) {
		// デバイスがハングまたは取り外された場合、エラーを返す
		return E_FAIL;
	}
	return hr;
}

HRESULT CScaler::CreateDevice() {
	D3DPRESENT_PARAMETERS pp;
	CreatePresParam(pp);
	HRESULT hr;
	hr=m_D3D->CreateDeviceEx(D3DADAPTER_DEFAULT
		, D3DDEVTYPE_HAL, m_Window
		, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED
		, &pp, NULL, &m_D3DDev);
	hr=m_D3DDev->GetRenderTarget(0, &m_RenderTarget.p);
	return hr;
}

HRESULT CScaler::CreatePresParam(D3DPRESENT_PARAMETERS &pp) {
	D3DDISPLAYMODE dm;
	ATLENSURE_RETURN_HR(m_D3D, E_UNEXPECTED);
	HRESULT hr=m_D3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dm);
	if(FAILED(hr)) {
		return hr;
	}
	ZeroMemory(&pp, sizeof(D3DPRESENT_PARAMETERS));
	pp.Windowed=TRUE;
	pp.hDeviceWindow=m_Window;
	pp.SwapEffect=D3DSWAPEFFECT_COPY;
	pp.BackBufferFormat=dm.Format;
	return hr;
}

void CScaler::ReleaseResources() {
	CAutoLock my_lock(m_CritSec);
	for(size_t i=0;i<m_Surfaces.size();i++) {
		m_Surfaces[i].Release();
	}
	m_MirroredTexture.Release();
	m_VtxMirror.Release();
	m_VtxSrc.Release();
	m_VtxDecl.Release();
	m_Effect.Release();
}
