// MainFrm.cpp : Implementierung der Klasse CMainFrame
//

#include "stdafx.h"
#include "StarshipTroopers.h"
#include "FullScreenHandler.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_GETMINMAXINFO()
	ON_COMMAND(ID_VIEW_FULLSCREEN, OnViewFullScreen)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FULLSCREEN, OnUpdateViewFullScreen)
END_MESSAGE_MAP()


// CMainFrame-Erstellung/Zerstörung

CMainFrame::CMainFrame()
{
	// TODO: Hier Code für die Memberinitialisierung einfügen
}

CMainFrame::~CMainFrame()
{
}


BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	cs.style = WS_MAXIMIZE | WS_CAPTION | FWS_ADDTOTITLE | WS_SYSMENU;
	cs.cy = ::GetSystemMetrics(SM_CYSCREEN);
	cs.cx = ::GetSystemMetrics(SM_CXSCREEN);

	return CFrameWnd::PreCreateWindow(cs);
}


// CMainFrame-Diagnose

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}


void CMainFrame::OnGetMinMaxInfo(MINMAXINFO* lpmmi)
{
	CSize sz = FullScreenHandler.GetMaxSize();
	lpmmi->ptMaxSize = CPoint(sz);
	lpmmi->ptMaxTrackSize = CPoint(sz);
}


void CMainFrame::OnViewFullScreen()
{
	if (FullScreenHandler.InFullScreenMode())
		FullScreenHandler.Restore(this);
	else 
		FullScreenHandler.Maximize(this, GetActiveView());
}


void CMainFrame::OnUpdateViewFullScreen(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(FullScreenHandler.InFullScreenMode());
}


