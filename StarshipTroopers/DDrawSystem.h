// DDrawSystem.h: interface for the CDDrawSystem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DDRAWSYSTEM_H__1E152EB4_ED1D_4079_BDD4_773383DD98C8__INCLUDED_)
#define AFX_DDRAWSYSTEM_H__1E152EB4_ED1D_4079_BDD4_773383DD98C8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <atlimage.h>
#include <ddraw.h>
#include "CGdiPlusBitmap.h"


#define _CHARACTORBUILDER_
//#include "../GameLib/Image.h"

#define SCREENWIDTH 1208
#define SCREENHEIGHT 1024

/*
typedef struct
{
	RECT sRect;
	int x, y;
} SpriteStruct;
*/
class CDDrawSystem  
{
/*
	public:
		CImage blueBall, redBall;
		CGdiPlusBitmapResource m_blueBall, m_redBall;

		CBitmap blueBallBm, redBallBm;
		CRect rcClient;
		CPoint centerP, blueBallP, redBallP;
		CDC *pScreenDC;

		CDDrawSystem();
		virtual ~CDDrawSystem();

		BOOL Init(HWND hWnd);
		void Terminate();
		void Clear();
		void TestDraw();
		void Display();
		void RunExperiment();
		void DrawBall(int whichBall);
		BOOL ChangeDisplayMode(BOOL bFullScreen, int nWidth, int nHeight);
*/
	protected:
		LPDIRECTDRAW7 m_pDD;
		LPDIRECTDRAWSURFACE7 m_pddsFrontBuffer;
		LPDIRECTDRAWSURFACE7 m_pddsStoreBuffer;
		LPDIRECTDRAWCLIPPER pcClipper;

		HWND hWnd;
};

#endif // !defined(AFX_DDRAWSYSTEM_H__1E152EB4_ED1D_4079_BDD4_773383DD98C8__INCLUDED_)
