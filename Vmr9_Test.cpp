#include "stdafx.h"
#include "Vmr9_Test.h"

namespace {
	const LPCWSTR MOVIE_FILENAME= L"clock.avi";
}

class CMyVideoWindow :
	public CWindowImpl<CMyVideoWindow> 
{
	// DirectShow
	CComPtr<IGraphBuilder> m_Graph;
	CComPtr<IBaseFilter> m_Vmr;
	CComPtr<CComObject<CScaler> > m_VmrAlloc;
	
	// CWindowImpl
	BEGIN_MSG_MAP(CMyWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()
private:
	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
		BuildGraph(MOVIE_FILENAME);
		return 0;
	}

	LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&) {
		PostQuitMessage(0);
		return 0;
	}

	void BuildGraph(LPCWSTR filename) {
		m_Graph.CoCreateInstance(CLSID_FilterGraph);
		m_Vmr.CoCreateInstance(CLSID_VideoMixingRenderer9);
		CComQIPtr<IVMRFilterConfig9> vmr_config(m_Vmr);
		vmr_config->SetNumberOfStreams(1);
		vmr_config->SetRenderingMode(VMR9Mode_Renderless);
		
		CComObject<CScaler>::CreateInstance(&m_VmrAlloc);
		(*m_VmrAlloc).AddRef();
		m_VmrAlloc->SetWindow(m_hWnd);

		CComQIPtr<IVMRSurfaceAllocatorNotify9> vmr_notify(m_Vmr);
		vmr_notify->AdviseSurfaceAllocator(0xacdcacdc, m_VmrAlloc);
		m_VmrAlloc->AdviseNotify(vmr_notify);

		m_Graph->AddFilter(m_Vmr, L"VMR9");
		m_Graph->RenderFile(filename, NULL);
		CComQIPtr<IMediaControl> mc(m_Graph);
		mc->Run();
	}
};

class CVmrTestApp : public CAtlExeModuleT<CVmrTestApp> {
	CMyVideoWindow m_win;
public:
	bool ParseCommandLine(LPCTSTR lpCmdLine, HRESULT* pnRetCode) throw() {
		pnRetCode = S_OK;
		return true;
	}
	HRESULT PreMessageLoop(int nShowCmd) throw() {
		RECT win_rect={100, 100, 100+640*2, 100+480*2};
		AdjustWindowRect(&win_rect, WS_OVERLAPPEDWINDOW, FALSE);

		m_win.Create(NULL, _U_RECT(win_rect), L"VMR9 TEST",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE);
		m_win.ShowWindow(nShowCmd);
		m_win.UpdateWindow();
		return S_OK;
	}
};

CVmrTestApp _Module;

int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR    lpCmdLine,
					   int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	_Module.WinMain(nCmdShow);
}
