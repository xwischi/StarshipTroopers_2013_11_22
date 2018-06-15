// StarshipTroopersView.cpp : Implementierung der Klasse CStarshipTroopersView


#include "stdafx.h"
#define _USE_MATH_DEFINES
#include "StarshipTroopers.h"
#include <Winuser.h>
#include <windows.h>
#include <mmsystem.h>
#include <dinput.h>
#include "conio.h"
#include "stdio.h"
#include "math.h"

#include "Monitors.h"
#include "MultiMonitor.h"
#include "MonitorDC.h"

#include "StarshipTroopersDoc.h"
#include "StarshipTroopersView.h"
#include "SetupDialog.h"
#include "Dialog1.h"

#include "FullScreenHandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 1024
#define BALLSIZE 100
#define STARSIZE 50
#define SQUARESIZE 100
#define OBJECTTYPES 4
#define SPEEDTYPES 3
#define CIRCLEREDUCED 12
#define STEP 1
#define PARALLELPORTDURATION 10
#define RATINGDELAYMILLIS 300

enum{blueBall, redBall};
enum{js1Left=1, js1Right, js1Center, js2Left, js2Right, js2Center};
#define yellowStar 0
enum{lilaSquare=1, redSquare, greenSquare, collisionSquare};
enum{speed1=2, speed2=3, speed3=4};
enum{playTogether=1, allPerson1, allPerson2, alone, replayMaster, allSwitched, allSameold};

LPDIRECTINPUT8 di;
LPDIRECTINPUTDEVICE8 joystick1, joystick2;
int joycounter;


short _stdcall Inp32(short PortAddress);
void _stdcall Out32(short PortAddress, short data);

/*

	Out32(0x378, 0);
	Out32(0x378, runningtrigger);
*/

void ListDisplayDevices()
{
	int index=0;
	DISPLAY_DEVICE dd;
	dd.cb = sizeof(DISPLAY_DEVICE);

	while (EnumDisplayDevices(NULL, index++, &dd, 0))
	{
		if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
			printf("* ");
		printf("%s, %s\n", dd.DeviceName, dd.DeviceString);
	}
}
 
DISPLAY_DEVICE GetPrimaryDevice()
{
	int index=0;
	DISPLAY_DEVICE dd;
	dd.cb = sizeof(DISPLAY_DEVICE);

	while (EnumDisplayDevices(NULL, index++, &dd, 0))
	{
		if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
			return dd;
	}
	return dd;
}


void ListDisplaySettings(int index)
{
	DISPLAY_DEVICE dd;
	dd.cb = sizeof(DISPLAY_DEVICE);
	if (!EnumDisplayDevices(NULL, index, &dd, 0))
	{
		printf("EnumDisplayDevices failed:%d\n", GetLastError());
		return;
	}

	DISPLAY_DEVICE monitor;
	monitor.cb = sizeof(DISPLAY_DEVICE);
	if (!EnumDisplayDevices(dd.DeviceName, index, &monitor, 0))
	{
		printf("EnumDisplayDevices failed:%d\n", GetLastError());
		return;
	}

	DEVMODE dm;
	dm.dmSize = sizeof(DEVMODE);

	if (!EnumDisplaySettings(dd.DeviceName, ENUM_CURRENT_SETTINGS, &dm))
	{
		printf("EnumDisplaySettings failed:%d\n", GetLastError());
		return;
	}

	printf("Device name: %s\n", dd.DeviceName);
	printf("Monitor name: %s\n", monitor.DeviceID);
	printf("Refresh rate, in hertz: %d\n", dm.dmDisplayFrequency);
	printf("Color depth: %d\n", dm.dmBitsPerPel);
	printf("Screen resolution, in pixels: %d x %d\n", dm.dmPelsWidth, dm.dmPelsHeight);
}




BOOL CALLBACK	enumCallback(const DIDEVICEINSTANCE* instance, VOID* context)
{
	HRESULT hr;

  // Obtain an interface to the enumerated joystick.
	if(joycounter == 0)
    hr = di->CreateDevice(instance->guidInstance, &joystick1, NULL);
	else if(joycounter == 1)
    hr = di->CreateDevice(instance->guidInstance, &joystick2, NULL);
	joycounter++;

  // If it failed, then we can't use this joystick. (Maybe the user unplugged
  // it while we were in the middle of enumerating it.)
  if (FAILED(hr))
	{ 
      return DIENUM_CONTINUE;
  }

  // Stop enumeration. Note: we're just taking the first joystick we get. You
  // could store all the enumerated joysticks and let the user pick.
	//   return DIENUM_STOP;
	return DIENUM_CONTINUE;
}


//-----------------------------------------------------------------------------
// Name: EnumObjectsCallback()
// Desc: Callback function for enumerating objects (axes, buttons, POVs) on a 
//       joystick. This function enables user interface elements for objects
//       that are found to exist, and scales axes min/max values.
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext )
{
	HWND hDlg = (HWND)pContext;

	static int nSliderCount = 0;  // Number of returned slider controls
	static int nPOVCount = 0;     // Number of returned POV controls

	// For axes that are returned, set the DIPROP_RANGE property for the
	// enumerated axis in order to scale min/max values.
	if( pdidoi->dwType & DIDFT_AXIS )
	{
		DIPROPRANGE diprg; 
		diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
		diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
		diprg.diph.dwHow        = DIPH_BYID; 
		diprg.diph.dwObj        = pdidoi->dwType; // Specify the enumerated axis
		diprg.lMin              = -1000; 
		diprg.lMax              = +1000; 

		// Set the range for the axis
		if( FAILED( joystick1->SetProperty( DIPROP_RANGE, &diprg.diph ) ) ) 
			return DIENUM_STOP;
		if( FAILED( joystick2->SetProperty( DIPROP_RANGE, &diprg.diph ) ) ) 
			return DIENUM_STOP;
	}

/*
    // Set the UI to reflect what objects the joystick supports
    if (pdidoi->guidType == GUID_XAxis)
    {
        EnableWindow( GetDlgItem( hDlg, IDC_X_AXIS ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_X_AXIS_TEXT ), TRUE );
    }
    if (pdidoi->guidType == GUID_YAxis)
    {
        EnableWindow( GetDlgItem( hDlg, IDC_Y_AXIS ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_Y_AXIS_TEXT ), TRUE );
    }
    if (pdidoi->guidType == GUID_ZAxis)
    {
        EnableWindow( GetDlgItem( hDlg, IDC_Z_AXIS ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_Z_AXIS_TEXT ), TRUE );
    }
    if (pdidoi->guidType == GUID_RxAxis)
    {
        EnableWindow( GetDlgItem( hDlg, IDC_X_ROT ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_X_ROT_TEXT ), TRUE );
    }
    if (pdidoi->guidType == GUID_RyAxis)
    {
        EnableWindow( GetDlgItem( hDlg, IDC_Y_ROT ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_Y_ROT_TEXT ), TRUE );
    }
    if (pdidoi->guidType == GUID_RzAxis)
    {
        EnableWindow( GetDlgItem( hDlg, IDC_Z_ROT ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_Z_ROT_TEXT ), TRUE );
    }
    if (pdidoi->guidType == GUID_Slider)
    {
        switch( nSliderCount++ )
        {
            case 0 :
                EnableWindow( GetDlgItem( hDlg, IDC_SLIDER0 ), TRUE );
                EnableWindow( GetDlgItem( hDlg, IDC_SLIDER0_TEXT ), TRUE );
                break;

            case 1 :
                EnableWindow( GetDlgItem( hDlg, IDC_SLIDER1 ), TRUE );
                EnableWindow( GetDlgItem( hDlg, IDC_SLIDER1_TEXT ), TRUE );
                break;
        }
    }
    if (pdidoi->guidType == GUID_POV)
    {
        switch( nPOVCount++ )
        {
            case 0 :
                EnableWindow( GetDlgItem( hDlg, IDC_POV0 ), TRUE );
                EnableWindow( GetDlgItem( hDlg, IDC_POV0_TEXT ), TRUE );
                break;

            case 1 :
                EnableWindow( GetDlgItem( hDlg, IDC_POV1 ), TRUE );
                EnableWindow( GetDlgItem( hDlg, IDC_POV1_TEXT ), TRUE );
                break;

            case 2 :
                EnableWindow( GetDlgItem( hDlg, IDC_POV2 ), TRUE );
                EnableWindow( GetDlgItem( hDlg, IDC_POV2_TEXT ), TRUE );
                break;

            case 3 :
                EnableWindow( GetDlgItem( hDlg, IDC_POV3 ), TRUE );
                EnableWindow( GetDlgItem( hDlg, IDC_POV3_TEXT ), TRUE );
                break;
        }
    }
*/
	return DIENUM_CONTINUE;
}



// CStarshipTroopersView
IMPLEMENT_DYNCREATE(CStarshipTroopersView, CView)

BEGIN_MESSAGE_MAP(CStarshipTroopersView, CView)
	ON_COMMAND(ID_EXP_SETUP, &CStarshipTroopersView::OnSetupexperiment)
	ON_COMMAND(ID_EXP_RUN, &CStarshipTroopersView::OnStartexperiment)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


CStarshipTroopersView::CStarshipTroopersView()
{
	canrun = false;
	avoidEachOther = avoidEachOtherRun = 0;
	fallingObjs = 0;
	risingObjs = 0;

	HRESULT hr;
	lpDD = NULL;
	hr = DirectDrawCreate(NULL, &lpDD, NULL); 
  if(hr != DD_OK)
  {
		ShowCursor(TRUE);
		AfxMessageBox(L"ERRRRROR: Failed to DirectDrawCreate !\nProgramm wird beendet!");
		exit(0);
    return;
  }
	// #################     Create a DirectInput device#################
	if (FAILED(hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&di, NULL)))
	{
		AfxMessageBox(L"ERRRRROR: DirectX not found!\nProgramm wird beendet!");
		exit(0);
	}

	int num_monitors = ::GetSystemMetrics(SM_CMONITORS);
	if(num_monitors < 2)
		return;
  monitorptr1.cb = sizeof(DISPLAY_DEVICE);
  monitorptr2.cb = sizeof(DISPLAY_DEVICE);
  
	BOOL result = EnumDisplayDevices(NULL, 0, &monitorptr1, 0);
	result = EnumDisplayDevices(NULL, 1, &monitorptr2, 0);
	modemon1.dmSize = sizeof(DEVMODE);
	modemon2.dmSize = sizeof(DEVMODE);
	modemon1old.dmSize = sizeof(DEVMODE);
	modemon2old.dmSize = sizeof(DEVMODE);
	if(! EnumDisplaySettingsEx(monitorptr1.DeviceName, ENUM_CURRENT_SETTINGS, &modemon1, NULL))
	{
		return;
	}
	if(! EnumDisplaySettingsEx(monitorptr2.DeviceName, ENUM_CURRENT_SETTINGS, &modemon2, NULL))
	{
		return;
	}
	EnumDisplaySettingsEx(monitorptr1.DeviceName, ENUM_CURRENT_SETTINGS, &modemon1old, NULL);
	EnumDisplaySettingsEx(monitorptr2.DeviceName, ENUM_CURRENT_SETTINGS, &modemon2old, NULL);


	blueBallImg.Load(_T("images\\blauerRing.png"));
	blueBallBm.Attach(blueBallImg.Detach());
	redBallImg.Load(_T("images\\roterRing.png"));
	redBallBm.Attach(redBallImg.Detach());
	yellowStarImg.Load(_T("images\\stern.png"));
	yellowStarBm.Attach(yellowStarImg.Detach());
	lilaSquareImg.Load(_T("images\\lilaQuadrat.png"));
	lilaSquareBm.Attach(lilaSquareImg.Detach());
	redSquareImg.Load(_T("images\\rotesQuadrat.png"));
	redSquareBm.Attach(redSquareImg.Detach());
	greenSquareImg.Load(_T("images\\gruenesQuadrat.png"));
	greenSquareBm.Attach(greenSquareImg.Detach());
	collisionSquareImg.Load(_T("images\\collisionQuadrat.png"));
	collisionSquareBm.Attach(collisionSquareImg.Detach());

	BITMAP bmpInfo;
	CSize bitSize;
	blueBallBm.GetBitmap(&bmpInfo);
	bitSize = CSize(bmpInfo.bmWidth, bmpInfo.bmHeight);
	m_imageList.Create(bitSize.cx, bitSize.cy, ILC_COLOR32 |ILC_MASK, 0, 0);
	m_imageList.Add(&blueBallBm, RGB(0,0,0));

	redBallBm.GetBitmap(&bmpInfo);
	bitSize = CSize(bmpInfo.bmWidth, bmpInfo.bmHeight);
	m_imageList.Add(&redBallBm, RGB(0,0,0));

	lilaSquareBm.GetBitmap(&bmpInfo);
	bitSize = CSize(bmpInfo.bmWidth, bmpInfo.bmHeight);
	square_imageList.Create(bitSize.cx, bitSize.cy, ILC_COLOR32 |ILC_MASK, 0, 0);
	square_imageList.Add(&lilaSquareBm, RGB(0,0,0));

	redSquareBm.GetBitmap(&bmpInfo);
	bitSize = CSize(bmpInfo.bmWidth, bmpInfo.bmHeight);
	square_imageList.Add(&redSquareBm, RGB(0,0,0));

	greenSquareBm.GetBitmap(&bmpInfo);
	bitSize = CSize(bmpInfo.bmWidth, bmpInfo.bmHeight);
	square_imageList.Add(&greenSquareBm, RGB(0,0,0));

	collisionSquareBm.GetBitmap(&bmpInfo);
	bitSize = CSize(bmpInfo.bmWidth, bmpInfo.bmHeight);
	square_imageList.Add(&collisionSquareBm, RGB(0,0,0));

	yellowStarBm.GetBitmap(&bmpInfo);
	bitSize = CSize(bmpInfo.bmWidth, bmpInfo.bmHeight);
	star_imageList.Create(bitSize.cx, bitSize.cy, ILC_COLOR32 |ILC_MASK, 0, 0);
	star_imageList.Add(&yellowStarBm, RGB(0,0,0));

	joycounter = 0;
	joystick1 = NULL;
	joystick2 = NULL;
	di->EnumDevices(DI8DEVCLASS_GAMECTRL, enumCallback, NULL, DIEDFL_ATTACHEDONLY);
	if(joystick1 == NULL)
	{
		AfxMessageBox(L"ERRRRROR: Joystick1 not found!\nProgramm wird beendet!");
		exit(0);
	}
	if(joystick2 == NULL)
	{
		AfxMessageBox(L"ERRRRROR: Joystick2 not found!\nProgramm wird beendet!");
		exit(0);
	}


	DIDEVCAPS capabilities1, capabilities2;
	if (FAILED(hr = joystick1->SetDataFormat(&c_dfDIJoystick2)))
	{
		AfxMessageBox(L"ERRRRROR: Joystick1 SetDataFormat!\nProgramm wird beendet!");
		exit(0);
	}

	capabilities1.dwSize = sizeof(DIDEVCAPS);
	capabilities2.dwSize = sizeof(DIDEVCAPS);
	if (FAILED(hr = joystick1->GetCapabilities(&capabilities1))) {
		AfxMessageBox(L"ERRRRROR: Joystick1 GetCapabilities!\nProgramm wird beendet!");
		exit(0);
	}
	if( FAILED( hr = joystick1->EnumObjects( EnumObjectsCallback, (VOID*)m_hWnd, DIDFT_ALL ) ) )
	{
		AfxMessageBox(L"ERRRRROR: Joystick1 EnumObjectsCallback!\nProgramm wird beendet!");
		exit(0);
	}
	if (FAILED(hr = joystick2->SetDataFormat(&c_dfDIJoystick2)))
	{
		AfxMessageBox(L"ERRRRROR: Joystick2 SetDataFormat!\nProgramm wird beendet!");
		exit(0);
	}

	if(joystick1 == NULL)
	{
		AfxMessageBox(L"ERRRRROR: Joystick1 not found!\nProgramm wird beendet!");
		exit(0);
	}
	if(joystick2 == NULL)
	{
		AfxMessageBox(L"ERRRRROR: Joystick2 not found!\nProgramm wird beendet!");
		exit(0);
	}

	// Poll the device to read the current state
  hr = joystick1->Poll(); 
  if (FAILED(hr))
	{
		// DInput is telling us that the input stream has been
		// interrupted. We aren't tracking any state between polls, so
		// we don't have any special reset that needs to be done. We
		// just re-acquire and try again.
		hr = joystick1->Acquire();
		while (hr == DIERR_INPUTLOST)
		{
			hr = joystick1->Acquire();
		}

    // If we encounter a fatal error, return failure.
    if ((hr == DIERR_INVALIDPARAM) || (hr == DIERR_NOTINITIALIZED))
		{
       //return E_FAIL;
    }

    // If another application has control of this device, return successfully.
    // We'll just have to wait our turn to use the joystick.
    if (hr == DIERR_OTHERAPPHASPRIO)
		{
       //return S_OK;
    }
  }


	seed = (unsigned long)time(NULL);
	ResetGraphicObjects();

	for(int i = 0; i < MAXTRIALS; i++)
	{
		trials[i].type = 0;
		trials[i].seed = 0L;
		for(int j = 0; j < 64; j++)
			trials[i].typeStr[j] = 0;
		for(int j = 0; j < 1024; j++)
			trials[i].replayFileName[j] = 0;
	}
	HDC hDC = ::GetDC( NULL );
	CDC *pDC = CDC::FromHandle(hDC);
	CMonitors::GetVirtualDesktopRect( &rect);
	pDC->FillRect( &rect, &brWhite );
}


CStarshipTroopersView::~CStarshipTroopersView()
{
	blueBallBm.DeleteObject();
	redBallBm.DeleteObject();
	yellowStarBm.DeleteObject();
	lilaSquareBm.DeleteObject();
	redSquareBm.DeleteObject();
	greenSquareBm.DeleteObject();
	m_imageList.DeleteImageList();
	star_imageList.DeleteImageList();
	square_imageList.DeleteImageList();

	if (joystick1)
	{ 
    joystick1->Unacquire();
	}
	if (joystick2)
	{ 
    joystick2->Unacquire();
	}
}


BOOL CStarshipTroopersView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Ändern Sie hier die Fensterklasse oder die Darstellung, indem Sie
	//  CREATESTRUCT cs modifizieren.

	return CView::PreCreateWindow(cs);
}


// CStarshipTroopersView-Zeichnung
void CStarshipTroopersView::OnDraw(CDC *pdc)
{
	int x;
	x = loop;
//	if(isRunning)
//	{
//		HDC hDC = ::GetDC( NULL );
//		CDC *pDCx = CDC::FromHandle(hDC);
////		CRect bounds;
////		CRect lBounds;
////		GetClientRect(&bounds);
////		pdc->Rectangle(lBounds);
//////			GetBoundsRect(&bounds, DCB_SET);
//////			GetClientRect(&bounds);
////		lBounds = bounds;
////
//		CMemDC pDC(pDCx);
//
//		if(runtype == alone)
//		{
//			m_imageList.Draw(pDC, blueBall, blueBallP, ILD_TRANSPARENT);
//			m_imageList.Draw(pDC, redBall, redBallPS2, ILD_TRANSPARENT);
//		}
//		else
//		{
//			m_imageList.Draw(pDC, blueBall, blueBallP, ILD_TRANSPARENT);
//			m_imageList.Draw(pDC, redBall, redBallP, ILD_TRANSPARENT);
//			m_imageList.Draw(pDC, blueBall, blueBallPS2, ILD_TRANSPARENT);
//			m_imageList.Draw(pDC, redBall, redBallPS2, ILD_TRANSPARENT);
//		}
//		pDC.BlitBack();
//	}
}


BOOL CStarshipTroopersView::OnEraseBkgnd(CDC* pDC)
{
	return false;
}


// CStarshipTroopersView-Diagnose
#ifdef _DEBUG
void CStarshipTroopersView::AssertValid() const
{
	CView::AssertValid();
}


void CStarshipTroopersView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CStarshipTroopersDoc* CStarshipTroopersView::GetDocument() const // Nicht-Debugversion ist inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CStarshipTroopersDoc)));
	return (CStarshipTroopersDoc*)m_pDocument;
}
#endif //_DEBUG


void CStarshipTroopersView::InitializeOneFallingObject(int which, int currtime)
{
	int rand1;
	bool done = false;
	int loops = 0;
	while(! done)
	{
		int idxSlot = rand() % MAXSLOTS;
		fallingObjects[which].left = slots[idxSlot].left;
		fallingObjects[which].slot = idxSlot + 1;
		for(int i = 0; i < fallingObjs; i++)
		{
			if( (fallingObjects[which].left == fallingObjects[i].left) || (fallingObjects[which].left == fallingObjects[i].left - STARSIZE / 2) )
			{
				if(loops < 50)
				{
					if(fallingObjects[i].top > SCREEN_HEIGHT / 4)
					{
						done = true;
					}
					else
					{
						done = false;
						break;
					}
				}
				else
					done = true;
			}
		}
		loops++;
	}
	if(fallingObjects[which].objectNum == 0)
	{
		fallingObjects[which].left += STARSIZE / 2;
		fallingObjects[which].width = STARSIZE;
		fallingObjects[which].center.x = fallingObjects[which].left + (STARSIZE / 2);
		fallingObjects[which].center.y = STARSIZE / 2;
	}
	else
	{
		fallingObjects[which].width = SQUARESIZE;
		fallingObjects[which].center.x = fallingObjects[which].left + (SQUARESIZE / 2);
		fallingObjects[which].center.y = TOPBARHEIGHT + (SQUARESIZE / 2);
	}
	switch(fallingObjects[which].objectNum)
	{
		case yellowStar: sprintf_s(fallingObjects[which].objName, "yellowStar"); break;
		case lilaSquare: sprintf_s(fallingObjects[which].objName, "lilaSquare"); break;
		case redSquare: sprintf_s(fallingObjects[which].objName, "redSquare"); break;
		case greenSquare: sprintf_s(fallingObjects[which].objName, "greenSquare"); break;
	}
	rand1 = 1 + rand() % SPEEDTYPES;
	switch(rand1)
	{
		case 1: fallingObjects[which].speed = speed1; break;
		case 2: fallingObjects[which].speed = speed2; break;
		case 3: fallingObjects[which].speed = speed3; break;
	}
	fallingObjects[which].delay = currtime + 1000;
	fallingObjects[which].top = TOPBARHEIGHT;
	fallingObjects[which].blockRect.top = fallingObjects[which].top;
	if(fallingObjects[which].objectNum == yellowStar)
	{
		fallingObjects[which].blockRect.bottom = fallingObjects[which].blockRect.top + STARSIZE;
		fallingObjects[which].left -= (SQUARESIZE - STARSIZE) / 2;
		fallingObjects[which].blockRect.left = fallingObjects[which].left;
		fallingObjects[which].blockRect.right = fallingObjects[which].left + STARSIZE;
	}
	else
	{
		fallingObjects[which].blockRect.bottom = fallingObjects[which].blockRect.top + SQUARESIZE;
		fallingObjects[which].blockRect.left = fallingObjects[which].left;
		fallingObjects[which].blockRect.right = fallingObjects[which].left + SQUARESIZE;
	}
	fallingObjects[which].isOn = 0;
	fallingObjects[which].collisionTime = 0;
	fallingObjects[which].starWasHit = 0;

	fallingObjectsS2[which].width = fallingObjects[which].width;
	fallingObjectsS2[which].objectNum = fallingObjects[which].objectNum;
	fallingObjectsS2[which].top = fallingObjects[which].top;
	fallingObjectsS2[which].left = fallingObjects[which].left + SCREEN_WIDTH;
	fallingObjectsS2[which].slot = fallingObjects[which].slot;
	fallingObjectsS2[which].center.x = fallingObjects[which].center.x + SCREEN_WIDTH;
	fallingObjectsS2[which].center.y = fallingObjects[which].center.y;
	fallingObjectsS2[which].speed = fallingObjects[which].speed;
	fallingObjectsS2[which].delay = fallingObjects[which].delay;
	strcpy_s(fallingObjectsS2[which].objName, fallingObjects[which].objName);
	fallingObjectsS2[which].blockRect.top = fallingObjects[which].blockRect.top;
	fallingObjectsS2[which].blockRect.left = fallingObjects[which].blockRect.left + SCREEN_WIDTH;
	fallingObjectsS2[which].blockRect.right = fallingObjects[which].blockRect.right + SCREEN_WIDTH;
	fallingObjectsS2[which].blockRect.bottom = fallingObjects[which].blockRect.bottom;
	fallingObjectsS2[which].isOn = 0;
	fallingObjectsS2[which].collisionTime = 0;
	fallingObjectsS2[which].starWasHit = 0;
}


void CStarshipTroopersView::InitializeOneRisingObject(int which, int currtime)
{
	int rand1;
	bool done = false;
	int loops = 0;
	while(! done)
	{
		int idxSlot = rand() % MAXSLOTS;
		risingObjects[which].left = slots[idxSlot].left;
		risingObjects[which].slot = idxSlot + 1;
		for(int i = 0; i < risingObjs; i++)
		{
			if( (risingObjects[which].left == risingObjects[i].left) || (risingObjects[which].left == risingObjects[i].left - STARSIZE / 2) )
			{
				if(loops < 50)
				{
					if(risingObjects[i].top < SCREEN_HEIGHT - (SCREEN_HEIGHT / 4) )
					{
						done = true;
					}
					else
					{
						done = false;
						break;
					}
				}
				else
					done = true;
			}
		}
		loops++;
	}

	risingObjects[which].width = SQUARESIZE;
	risingObjects[which].center.x = risingObjects[which].left + (SQUARESIZE / 2);
	risingObjects[which].center.y = SCREEN_HEIGHT - (SQUARESIZE / 2);
	
	switch(risingObjects[which].objectNum)
	{
		case lilaSquare: sprintf_s(risingObjects[which].objName, "lilaSquare"); break;
		case redSquare: sprintf_s(risingObjects[which].objName, "redSquare"); break;
		case greenSquare: sprintf_s(risingObjects[which].objName, "greenSquare"); break;
	}
	rand1 = 1 + rand() % SPEEDTYPES;
	switch(rand1)
	{
		case 1: risingObjects[which].speed = speed1; break;
		case 2: risingObjects[which].speed = speed2; break;
		case 3: risingObjects[which].speed = speed3; break;
	}
	risingObjects[which].delay = currtime + 1000;
	risingObjects[which].top = SCREEN_HEIGHT - SQUARESIZE;
	risingObjects[which].blockRect.top = risingObjects[which].top;
	risingObjects[which].blockRect.bottom = SCREEN_HEIGHT;
	risingObjects[which].blockRect.left = risingObjects[which].left;
	risingObjects[which].blockRect.right = risingObjects[which].left + SQUARESIZE;
	risingObjects[which].isOn = 0;
	risingObjects[which].collisionTime = 0;
	risingObjects[which].starWasHit = 0;

	risingObjectsS2[which].width = risingObjects[which].width;
	risingObjectsS2[which].objectNum = risingObjects[which].objectNum;
	risingObjectsS2[which].top = risingObjects[which].top;
	risingObjectsS2[which].left = risingObjects[which].left + SCREEN_WIDTH;
	risingObjectsS2[which].slot = risingObjects[which].slot;
	risingObjectsS2[which].center.x = risingObjects[which].center.x + SCREEN_WIDTH;
	risingObjectsS2[which].center.y = risingObjects[which].center.y;
	risingObjectsS2[which].speed = risingObjects[which].speed;
	risingObjectsS2[which].delay = risingObjects[which].delay;
	strcpy_s(risingObjectsS2[which].objName, risingObjects[which].objName);
	risingObjectsS2[which].blockRect.top = risingObjects[which].blockRect.top;
	risingObjectsS2[which].blockRect.left = risingObjects[which].blockRect.left + SCREEN_WIDTH;
	risingObjectsS2[which].blockRect.right = risingObjects[which].blockRect.right + SCREEN_WIDTH;
	risingObjectsS2[which].blockRect.bottom = risingObjects[which].blockRect.bottom;
	risingObjectsS2[which].isOn = 0;
	risingObjectsS2[which].collisionTime = 0;
	risingObjectsS2[which].starWasHit = 0;
}


void CStarshipTroopersView::InitializeFallingObjects(void)
{
/*
	if(! avoidEachOther)
	{
		switch(run)
		{
			case 0:
				seed = (unsigned long)time(NULL);
				break;
			case 1:
				seed = (unsigned long)time(NULL);
				trials[4].seed = seed;
				break;
			case 2:
				seed = (unsigned long)time(NULL);
				trials[6].seed = seed;
				break;
			case 3:
				seed = trials[run].seed;
				break;
			case 4:
				seed = trials[4].seed;
				break;
			case 5:
				seed = (unsigned long)time(NULL);
				trials[10].seed = seed;
				break;
			case 6:
				seed = trials[run].seed;
				break;
			case 7:
				seed = (unsigned long)time(NULL);
				seed = trials[11].seed = seed;
				break;
			case 8:
				seed = trials[run].seed = seed;
				break;
			case 9:
				seed = (unsigned long)time(NULL);
				seed = trials[13].seed = seed;
				break;
			case 10:
				seed = (unsigned long)time(NULL);
				seed = trials[15].seed = seed;
				break;
			case 11:
				seed = trials[run].seed = seed;
				break;
			case 12:
				seed = (unsigned long)time(NULL);
				break;
			case 13:
				seed = trials[run].seed = seed;
				break;
			case 14:
				seed = (unsigned long)time(NULL);
				break;
			case 15:
				seed = trials[run].seed = seed;
				break;
			case 16:
				seed = (unsigned long)time(NULL);
				break;
		}
	}
*/
	seed = (unsigned long)time(NULL);
	srand(seed);
	
	int startleft = 41;
	for(int i = 0; i < MAXSLOTS; i++)
	{
		slots[i].left = startleft;
		slots[i].right = startleft + 99;
		startleft += 100;
	}

	// set object types
	int objnum[MAXFALLINGOBJECTS];
	for(int i = 0; i < MAXFALLINGOBJECTS; i++)
	{
		if(fallingObjectsTypes == 2)
		{
			switch(i % 4)
			{
				case 0: objnum[i] = yellowStar; break;
				case 1: objnum[i] = lilaSquare; break;
				case 2: objnum[i] = redSquare; break;
				case 3: objnum[i] = greenSquare; break;
			}
		}
		else if(fallingObjectsTypes == 3)
		{
			switch(i % 3)
			{
				case 0: objnum[i] = lilaSquare; break;
				case 1: objnum[i] = redSquare; break;
				case 2: objnum[i] = greenSquare; break;
			}
		}
	}
	// randomize them
	int temp;
	for(int i = 0; i < MAXFALLINGOBJECTS * 100; i++)
	{
		int rand1, rand2;
		rand1 = rand() % MAXFALLINGOBJECTS;
		rand2 = rand() % MAXFALLINGOBJECTS;
		if(rand1 != rand2)
		{
			temp = objnum[rand1];
			objnum[rand1] = objnum[rand2];
			objnum[rand2] = temp;
		}
	}

	// set the slot on the screen
	for(int i = 0; i < MAXFALLINGOBJECTS; i++)
	{
		int rand1 = rand() % MAXSLOTS;

		fallingObjects[i].left = slots[rand1].left;
		fallingObjects[i].slot = rand1 + 1;
	}

	for(int i = 0; i < MAXFALLINGOBJECTS * 100; i++)
	{
		int rand1, rand2;
		rand1 = rand() % MAXFALLINGOBJECTS;
		rand2 = rand() % MAXFALLINGOBJECTS;
		if(rand1 != rand2)
		{
			temp = fallingObjects[rand1].left;
			fallingObjects[rand1].left = fallingObjects[rand2].left;
			fallingObjects[rand2].left = temp;
		}
	}

	for(int i = 0; i < MAXFALLINGOBJECTS; i++)
	{
		fallingObjects[i].objectNum = objnum[i];
		switch(fallingObjects[i].objectNum)
		{
			case yellowStar: sprintf_s(fallingObjects[i].objName, "yellowStar"); break;
			case lilaSquare: sprintf_s(fallingObjects[i].objName, "lilaSquare"); break;
			case redSquare: sprintf_s(fallingObjects[i].objName, "redSquare"); break;
			case greenSquare: sprintf_s(fallingObjects[i].objName, "greenSquare"); break;
		}
		if(fallingObjects[i].objectNum == yellowStar)
		{
			fallingObjects[i].left += STARSIZE / 2;
			fallingObjects[i].width = STARSIZE;
			fallingObjects[i].center.x = fallingObjects[i].left + (STARSIZE / 2);
			fallingObjects[i].center.y = STARSIZE / 2;
		}
		else
		{
			fallingObjects[i].center.x = fallingObjects[i].left + (SQUARESIZE / 2);
			fallingObjects[i].center.y = SQUARESIZE / 2;
			fallingObjects[i].width = SQUARESIZE;
		}
		fallingObjects[i].top = TOPBARHEIGHT;
		fallingObjects[i].blockRect.top = fallingObjects[i].top;
		if(fallingObjects[i].objectNum == yellowStar)
		{
			fallingObjects[i].blockRect.bottom = fallingObjects[i].blockRect.top + STARSIZE;
			fallingObjects[i].left -= (SQUARESIZE - STARSIZE) / 2;
			fallingObjects[i].blockRect.left = fallingObjects[i].left;
			fallingObjects[i].blockRect.right = fallingObjects[i].left + STARSIZE;
		}
		else
		{
			fallingObjects[i].blockRect.bottom = fallingObjects[i].blockRect.top + SQUARESIZE;
			fallingObjects[i].blockRect.left = fallingObjects[i].left;
			fallingObjects[i].blockRect.right = fallingObjects[i].left + SQUARESIZE;
		}

		if(i % 3 == 0)
			fallingObjects[i].speed = speed1;
		else if(i % 3 == 1)
			fallingObjects[i].speed = speed2;
		else if(i % 3 == 2)
			fallingObjects[i].speed = speed3;
		
		fallingObjects[i].delay = 1000 + i * 1000;
		fallingObjects[i].isOn = 0;
		fallingObjects[i].collisionTime = 0;
		fallingObjects[i].starWasHit = 0;

		fallingObjectsS2[i].width = fallingObjects[i].width;
		fallingObjectsS2[i].objectNum = fallingObjects[i].objectNum;
		fallingObjectsS2[i].top = fallingObjects[i].top;
		fallingObjectsS2[i].left = fallingObjects[i].left + SCREEN_WIDTH;
		fallingObjectsS2[i].slot = fallingObjects[i].slot;
		fallingObjectsS2[i].center.x = fallingObjects[i].center.x + SCREEN_WIDTH;
		fallingObjectsS2[i].center.y = fallingObjects[i].center.y;
		fallingObjectsS2[i].speed = fallingObjects[i].speed;
		fallingObjectsS2[i].delay = fallingObjects[i].delay;
		strcpy_s(fallingObjectsS2[i].objName, fallingObjects[i].objName);
		fallingObjectsS2[i].blockRect.top = fallingObjects[i].blockRect.top;
		fallingObjectsS2[i].blockRect.left = fallingObjects[i].blockRect.left + SCREEN_WIDTH;
		fallingObjectsS2[i].blockRect.right = fallingObjects[i].blockRect.right + SCREEN_WIDTH;
		fallingObjectsS2[i].blockRect.bottom = fallingObjects[i].blockRect.bottom;
		fallingObjectsS2[i].isOn = 0;
		fallingObjectsS2[i].collisionTime = 0;
		fallingObjectsS2[i].starWasHit = 0;
	}

	// set object types
	for(int i = 0; i < MAXRISINGOBJECTS; i++)
	{
		switch(i % 3)
		{
			case 0: objnum[i] = lilaSquare; break;
			case 1: objnum[i] = redSquare; break;
			case 2: objnum[i] = greenSquare; break;
		}
	}
	// randomize them
	for(int i = 0; i < MAXRISINGOBJECTS * 100; i++)
	{
		int rand1, rand2;
		rand1 = rand() % MAXRISINGOBJECTS;
		rand2 = rand() % MAXRISINGOBJECTS;
		if(rand1 != rand2)
		{
			temp = objnum[rand1];
			objnum[rand1] = objnum[rand2];
			objnum[rand2] = temp;
		}
	}

	// set the slot on the screen
	for(int i = 0; i < MAXRISINGOBJECTS; i++)
	{
		int rand1 = rand() % MAXSLOTS;

		risingObjects[i].left = slots[rand1].left;
		risingObjects[i].slot = rand1 + 1;
	}

	for(int i = 0; i < MAXRISINGOBJECTS * 100; i++)
	{
		int rand1, rand2;
		rand1 = rand() % MAXRISINGOBJECTS;
		rand2 = rand() % MAXRISINGOBJECTS;
		if(rand1 != rand2)
		{
			temp = risingObjects[rand1].left;
			risingObjects[rand1].left = risingObjects[rand2].left;
			risingObjects[rand2].left = temp;
		}
	}

	for(int i = 0; i < MAXRISINGOBJECTS; i++)
	{
		risingObjects[i].objectNum = objnum[i];
		switch(risingObjects[i].objectNum)
		{
			case lilaSquare: sprintf_s(risingObjects[i].objName, "lilaSquare"); break;
			case redSquare: sprintf_s(risingObjects[i].objName, "redSquare"); break;
			case greenSquare: sprintf_s(risingObjects[i].objName, "greenSquare"); break;
		}
		
		risingObjects[i].center.x = risingObjects[i].left + (SQUARESIZE / 2);
		risingObjects[i].center.y = SCREEN_HEIGHT - (SQUARESIZE / 2);
		risingObjects[i].width = SQUARESIZE;
		
		risingObjects[i].top = SCREEN_HEIGHT - SQUARESIZE;
		risingObjects[i].blockRect.top = risingObjects[i].top;
		risingObjects[i].blockRect.bottom = risingObjects[i].blockRect.top + SQUARESIZE;
		risingObjects[i].blockRect.left = risingObjects[i].left;
		risingObjects[i].blockRect.right = risingObjects[i].left + SQUARESIZE;

		if(i % 3 == 0)
			risingObjects[i].speed = speed1;
		else if(i % 3 == 1)
			risingObjects[i].speed = speed2;
		else if(i % 3 == 2)
			risingObjects[i].speed = speed3;
		
		risingObjects[i].delay = 1000 + i * 1000;
		risingObjects[i].isOn = 0;
		risingObjects[i].collisionTime = 0;
		risingObjects[i].starWasHit = 0;

		risingObjectsS2[i].width = risingObjects[i].width;
		risingObjectsS2[i].objectNum = risingObjects[i].objectNum;
		risingObjectsS2[i].top = risingObjects[i].top;
		risingObjectsS2[i].left = risingObjects[i].left + SCREEN_WIDTH;
		risingObjectsS2[i].slot = risingObjects[i].slot;
		risingObjectsS2[i].center.x = risingObjects[i].center.x + SCREEN_WIDTH;
		risingObjectsS2[i].center.y = risingObjects[i].center.y;
		risingObjectsS2[i].speed = risingObjects[i].speed;
		risingObjectsS2[i].delay = risingObjects[i].delay;
		strcpy_s(risingObjectsS2[i].objName, risingObjects[i].objName);
		risingObjectsS2[i].blockRect.top = risingObjects[i].blockRect.top;
		risingObjectsS2[i].blockRect.left = risingObjects[i].blockRect.left + SCREEN_WIDTH;
		risingObjectsS2[i].blockRect.right = risingObjects[i].blockRect.right + SCREEN_WIDTH;
		risingObjectsS2[i].blockRect.bottom = risingObjects[i].blockRect.bottom;
		risingObjectsS2[i].isOn = 0;
		risingObjectsS2[i].collisionTime = 0;
		risingObjectsS2[i].starWasHit = 0;
	}

	ResetGraphicObjects();
}


void CStarshipTroopersView::UpdateFallingObjects(int currtime)
{
	if(! replay)
	{
		for(int i = 0; i < fallingObjs; i++)
		{
			if(fallingObjects[i].isOn)
			{
				fallingObjects[i].top += fallingObjects[i].speed;
				fallingObjects[i].blockRect.top = fallingObjects[i].top;
				fallingObjects[i].blockRect.bottom += fallingObjects[i].speed;
				fallingObjects[i].center.y += fallingObjects[i].speed;
				if(fallingObjects[i].blockRect.bottom > SCREEN_HEIGHT)
				{
					InitializeOneFallingObject(i, currtime);
					fallingObjects[i].blockRect.bottom = SCREEN_HEIGHT;
				}
			}
			else
			{
				if(fallingObjects[i].starWasHit == 1)
				{
					fallingObjects[i].top += fallingObjects[i].speed;
					fallingObjects[i].blockRect.top = fallingObjects[i].top;
					if(fallingObjects[i].objectNum == yellowStar)
						fallingObjects[i].blockRect.bottom = fallingObjects[i].blockRect.top + STARSIZE;
					else
						fallingObjects[i].blockRect.bottom = fallingObjects[i].blockRect.top + SQUARESIZE;
					fallingObjects[i].center.y += fallingObjects[i].speed;
					if(fallingObjects[i].blockRect.bottom > SCREEN_HEIGHT)
					{
						InitializeOneFallingObject(i, currtime);
						fallingObjects[i].blockRect.bottom = SCREEN_HEIGHT;
					}
				}
				if(fallingObjects[i].delay < currtime)
				{
					if(fallingObjects[i].objectNum == yellowStar)
					{
						if(fallingObjects[i].starWasHit == 1)
						{
							fallingObjects[i].isOn = 0;
						}
						else
						{
							fallingObjects[i].isOn = 1;
							fallingObjects[i].blockRect.bottom = fallingObjects[i].blockRect.top + STARSIZE;
						}
					}
					else
					{
						fallingObjects[i].isOn = 1;
						fallingObjects[i].blockRect.bottom = fallingObjects[i].blockRect.top + SQUARESIZE;
					}
				}
			}

			fallingObjectsS2[i].top = fallingObjects[i].top;
			fallingObjectsS2[i].center.x = fallingObjects[i].center.x + SCREEN_WIDTH;
			fallingObjectsS2[i].center.y = fallingObjects[i].center.y;
			fallingObjectsS2[i].speed = fallingObjects[i].speed;
			fallingObjectsS2[i].blockRect.top = fallingObjects[i].blockRect.top;
			fallingObjectsS2[i].blockRect.left = fallingObjects[i].blockRect.left + SCREEN_WIDTH;
			fallingObjectsS2[i].blockRect.right = fallingObjects[i].blockRect.right + SCREEN_WIDTH;
			fallingObjectsS2[i].blockRect.bottom = fallingObjects[i].blockRect.bottom;
			fallingObjectsS2[i].isOn = fallingObjects[i].isOn;
			fallingObjectsS2[i].starWasHit = fallingObjects[i].starWasHit;
		}

		for(int i = 0; i < risingObjs; i++)
		{
			if(risingObjects[i].isOn)
			{
				risingObjects[i].top -= risingObjects[i].speed;
				risingObjects[i].blockRect.top = risingObjects[i].top;
				risingObjects[i].blockRect.bottom = risingObjects[i].blockRect.top + SQUARESIZE;
				risingObjects[i].center.y -= risingObjects[i].speed;
				if(risingObjects[i].blockRect.top < 0)
				{
					InitializeOneRisingObject(i, currtime);
					risingObjects[i].blockRect.top = 0;
				}
			}
			else
			{
				if(risingObjects[i].delay < currtime)
				{
					risingObjects[i].isOn = 1;
					risingObjects[i].blockRect.bottom = risingObjects[i].blockRect.top + SQUARESIZE;
				}
			}

			risingObjectsS2[i].top = risingObjects[i].top;
			risingObjectsS2[i].slot = risingObjects[i].slot;
			risingObjectsS2[i].center.x = risingObjects[i].center.x + SCREEN_WIDTH;
			risingObjectsS2[i].center.y = risingObjects[i].center.y;
			risingObjectsS2[i].blockRect.top = risingObjects[i].blockRect.top;
			risingObjectsS2[i].blockRect.left = risingObjects[i].blockRect.left + SCREEN_WIDTH;
			risingObjectsS2[i].blockRect.right = risingObjects[i].blockRect.right + SCREEN_WIDTH;
			risingObjectsS2[i].blockRect.bottom = risingObjects[i].blockRect.bottom;
			risingObjectsS2[i].isOn = risingObjects[i].isOn;
			risingObjectsS2[i].starWasHit = 0;
		}
	}
	else
	{
		for(int i = 0; i < fallingObjs; i++)
		{
			fallingObjects[i].top = alonePoints[idx].blocks[i].y;
			fallingObjects[i].left = alonePoints[idx].blocks[i].x;
			fallingObjects[i].blockRect.left = fallingObjects[i].left;
			fallingObjects[i].blockRect.top = fallingObjects[i].top;
			fallingObjects[i].objectNum = alonePoints[idx].blockType[i];
			if(fallingObjects[i].objectNum == yellowStar)
			{
				fallingObjects[i].blockRect.right = fallingObjects[i].blockRect.left + STARSIZE;
				fallingObjects[i].blockRect.bottom = fallingObjects[i].blockRect.top + STARSIZE;
				fallingObjects[i].center.y = fallingObjects[i].top + (STARSIZE / 2);
				fallingObjects[i].center.x = fallingObjects[i].left + (STARSIZE / 2);
				fallingObjects[i].width = STARSIZE;
			}
			else
			{
				fallingObjects[i].blockRect.right = fallingObjects[i].blockRect.left + SQUARESIZE;
				fallingObjects[i].blockRect.bottom = fallingObjects[i].blockRect.top + SQUARESIZE;
				fallingObjects[i].center.y = fallingObjects[i].top + (SQUARESIZE / 2);
				fallingObjects[i].center.x = fallingObjects[i].left + (SQUARESIZE / 2);
				fallingObjects[i].width = SQUARESIZE;
			}

			fallingObjects[i].slot = alonePoints[idx].blockSlot[i];
			if(alonePoints[idx].blocks[i].x > 0)
				fallingObjects[i].isOn = 1;
			else
				fallingObjects[i].isOn = 0;
			if(fallingObjects[i].top + SQUARESIZE > SCREEN_HEIGHT)
			{
				fallingObjects[i].isOn = 0;
			}

			fallingObjectsS2[i].top = fallingObjects[i].top;
			fallingObjectsS2[i].objectNum = fallingObjects[i].objectNum;
			fallingObjectsS2[i].left = fallingObjects[i].left + SCREEN_WIDTH;
			fallingObjectsS2[i].center.x = fallingObjects[i].center.x + SCREEN_WIDTH;
			fallingObjectsS2[i].center.y = fallingObjects[i].center.y;
			fallingObjectsS2[i].blockRect.top = fallingObjects[i].blockRect.top;
			fallingObjectsS2[i].blockRect.left = fallingObjects[i].blockRect.left + SCREEN_WIDTH;
			fallingObjectsS2[i].blockRect.right = fallingObjects[i].blockRect.right + SCREEN_WIDTH;
			fallingObjectsS2[i].blockRect.bottom = fallingObjects[i].blockRect.bottom;
			fallingObjectsS2[i].isOn = fallingObjects[i].isOn;
		}

		for(int i = 0; i < risingObjs; i++)
		{
			risingObjects[i].width = SQUARESIZE;
			risingObjects[i].top = alonePoints[idx].blocksRising[i].y;
			risingObjects[i].left = alonePoints[idx].blocksRising[i].x;
			risingObjects[i].blockRect.left = risingObjects[i].left;
			risingObjects[i].blockRect.top = risingObjects[i].top;
			risingObjects[i].objectNum = alonePoints[idx].blockTypeRising[i];
			risingObjects[i].blockRect.right = risingObjects[i].blockRect.left + SQUARESIZE;
			risingObjects[i].blockRect.bottom = risingObjects[i].blockRect.top + SQUARESIZE;
			risingObjects[i].center.y = risingObjects[i].top + (SQUARESIZE / 2);
			risingObjects[i].center.x = risingObjects[i].left + (SQUARESIZE / 2);
			risingObjects[i].slot = alonePoints[idx].blockSlot[i];
			if(alonePoints[idx].blocksRising[i].x > 0)
				risingObjects[i].isOn = 1;
			else
				risingObjects[i].isOn = 0;
			if(risingObjects[i].top < TOPBARHEIGHT)
			{
				risingObjects[i].isOn = 0;
			}

			risingObjectsS2[i].width = risingObjects[i].width;
			risingObjectsS2[i].objectNum = risingObjects[i].objectNum;
			risingObjectsS2[i].top = risingObjects[i].top;
			risingObjectsS2[i].left = risingObjects[i].left + SCREEN_WIDTH;
			risingObjectsS2[i].slot = risingObjects[i].slot;
			risingObjectsS2[i].center.x = risingObjects[i].center.x + SCREEN_WIDTH;
			risingObjectsS2[i].center.y = risingObjects[i].center.y;
			risingObjectsS2[i].blockRect.top = risingObjects[i].blockRect.top;
			risingObjectsS2[i].blockRect.left = risingObjects[i].blockRect.left + SCREEN_WIDTH;
			risingObjectsS2[i].blockRect.right = risingObjects[i].blockRect.right + SCREEN_WIDTH;
			risingObjectsS2[i].blockRect.bottom = risingObjects[i].blockRect.bottom;
			risingObjectsS2[i].isOn = risingObjects[i].isOn;
			risingObjectsS2[i].starWasHit = 0;
		}
	}
}

  
void CStarshipTroopersView::OnSetupexperiment()
{
	canrun = false;
	strcpy_s(vpid, "");
	SetupDialog setupDlg;
	if(setupDlg.DoModal() == IDOK)
	{
		strcpy_s(vpid, setupDlg.vpid);
		canrun = true;
		fallingObjectsTypes = setupDlg.fallingObjectsTypes;
		fallingObjs = setupDlg.fallingObjs;
		risingObjs = setupDlg.risingObjs;
		avoidEachOther = avoidEachOtherRun = setupDlg.avoidEachOther;
	}
}


bool CStarshipTroopersView::ReadStandAloneData(char *filename, int type)
{
	FILE *input = NULL;
	fopen_s(&input, filename, "r");
	if(! input)
		return false;

	char linestr[4096];
	if(fgets(linestr, 4096, input) == NULL)
	{
		return false;
	}
	idx = 0;
	maxidx = 0;
	bool done = false;
	while(! done)
	{
		if(fgets(linestr, 4096, input) == NULL)
			done = true;
		else
		{
			char *nextToken = NULL;
			char *partstr = strtok_s(linestr, "\t", &nextToken);
			alonePoints[idx].millis = atoi(partstr);

			partstr = strtok_s(NULL, "\t", &nextToken);	// run
			partstr = strtok_s(NULL, "\t", &nextToken);	// runtype
			partstr = strtok_s(NULL, "\t", &nextToken);	// runtypeString
			partstr = strtok_s(NULL, "\t", &nextToken);	// avoidEachOther
			partstr = strtok_s(NULL, "\t", &nextToken);	// falling objs
			partstr = strtok_s(NULL, "\t", &nextToken);	// rising objs
			partstr = strtok_s(NULL, "\t", &nextToken);	// minutes
			if(type != replayMaster)
			{
				partstr = strtok_s(NULL, "\t", &nextToken);	// blueRating1
				partstr = strtok_s(NULL, "\t", &nextToken);	// redRating1
				partstr = strtok_s(NULL, "\t", &nextToken);	// blueRating2
				partstr = strtok_s(NULL, "\t", &nextToken);	// redRating2
				partstr = strtok_s(NULL, "\t", &nextToken);	// trigger
			}
			partstr = strtok_s(NULL, "\t", &nextToken);	// together
			partstr = strtok_s(NULL, "\t", &nextToken);	// stars
			partstr = strtok_s(NULL, "\t", &nextToken);	// collisions
			partstr = strtok_s(NULL, "\t", &nextToken);	// together S2
			partstr = strtok_s(NULL, "\t", &nextToken);	// stars S2
			partstr = strtok_s(NULL, "\t", &nextToken);	// collisions S2
			partstr = strtok_s(NULL, "\t", &nextToken);	// js1ButtonPressed
			partstr = strtok_s(NULL, "\t", &nextToken);	// js2ButtonPressed

			partstr = strtok_s(NULL, "\t", &nextToken);
			alonePoints[idx].pointBlue.x = atoi(partstr);
			partstr = strtok_s(NULL, "\t", &nextToken);
			alonePoints[idx].pointBlue.y = atoi(partstr);
			partstr = strtok_s(NULL, "\t", &nextToken);
			alonePoints[idx].pointRed.x = atoi(partstr);
			partstr = strtok_s(NULL, "\t", &nextToken);
			alonePoints[idx].pointRed.y = atoi(partstr);
			partstr = strtok_s(NULL, "\t", &nextToken);		// distance red-blue

			partstr = strtok_s(NULL, "\t", &nextToken);
			alonePoints[idx].pointBlueS2.x = atoi(partstr);
			partstr = strtok_s(NULL, "\t", &nextToken);
			alonePoints[idx].pointBlueS2.y = atoi(partstr);
			partstr = strtok_s(NULL, "\t", &nextToken);
			alonePoints[idx].pointRedS2.x = atoi(partstr);
			partstr = strtok_s(NULL, "\t", &nextToken);
			alonePoints[idx].pointRedS2.y = atoi(partstr);
			partstr = strtok_s(NULL, "\t", &nextToken);		// distance red-blue S2

			for(int i = 0; i < MAXFALLINGOBJECTS; i++)
			{
				partstr = strtok_s(NULL, "\t", &nextToken);	// fallingObject[i].x
				alonePoints[idx].blocks[i].x  = atoi(partstr);
				partstr = strtok_s(NULL, "\t", &nextToken);	// fallingObject[i].y
				alonePoints[idx].blocks[i].y  = atoi(partstr);
				partstr = strtok_s(NULL, "\t", &nextToken);	// fallingObject[i].blockType
				alonePoints[idx].blockType[i] = atoi(partstr) - 1;
				partstr = strtok_s(NULL, "\t", &nextToken);	// blueDistance
				partstr = strtok_s(NULL, "\t", &nextToken);	// redDistance
				partstr = strtok_s(NULL, "\t", &nextToken);	// fallingObjectS2[i].x
				alonePoints[idx].blocksS2[i].x  = atoi(partstr);
				partstr = strtok_s(NULL, "\t", &nextToken);	// fallingObjectS2[i].y
				alonePoints[idx].blocksS2[i].y  = atoi(partstr);
				partstr = strtok_s(NULL, "\t", &nextToken);	// fallingObjectS2[i].blockType
				partstr = strtok_s(NULL, "\t", &nextToken);	// blueDistanceS2
				partstr = strtok_s(NULL, "\t", &nextToken);	// redDistanceS2

				if(alonePoints[idx].blockType[i] == yellowStar)
				{
					if(alonePoints[idx].blocks[i].x > 0)
					{
						alonePoints[idx].blocks[i].x = alonePoints[idx].blocks[i].x - STARSIZE / 2;
						alonePoints[idx].blocks[i].y = alonePoints[idx].blocks[i].y - STARSIZE / 2;
						alonePoints[idx].blocksS2[i].x = alonePoints[idx].blocksS2[i].x - STARSIZE / 2;
						alonePoints[idx].blocksS2[i].y = alonePoints[idx].blocksS2[i].y - STARSIZE / 2;
					}
				}
				else
				{
					if(alonePoints[idx].blocks[i].x > 0)
					{
						alonePoints[idx].blocks[i].x = alonePoints[idx].blocks[i].x - SQUARESIZE / 2;
						alonePoints[idx].blocks[i].y = alonePoints[idx].blocks[i].y - SQUARESIZE / 2;
						alonePoints[idx].blocksS2[i].x = alonePoints[idx].blocksS2[i].x - SQUARESIZE / 2;
						alonePoints[idx].blocksS2[i].y = alonePoints[idx].blocksS2[i].y - SQUARESIZE / 2;
					}
				}

				switch(alonePoints[idx].blocks[i].x)
				{
					case 91: alonePoints[idx].blockSlot[i] = 1; break;
					case 191: alonePoints[idx].blockSlot[i] = 2; break;
					case 291: alonePoints[idx].blockSlot[i] = 3; break;
					case 391: alonePoints[idx].blockSlot[i] = 4; break;
					case 491: alonePoints[idx].blockSlot[i] = 5; break;
					case 591: alonePoints[idx].blockSlot[i] = 6; break;
					case 691: alonePoints[idx].blockSlot[i] = 7; break;
					case 791: alonePoints[idx].blockSlot[i] = 8; break;
					case 891: alonePoints[idx].blockSlot[i] = 9; break;
					case 991: alonePoints[idx].blockSlot[i] = 10; break;
					case 1091: alonePoints[idx].blockSlot[i] = 11; break;
					case 1191: alonePoints[idx].blockSlot[i] = 12; break;
					default: alonePoints[idx].blockSlot[i] = 0;	break;
				}
			}

			for(int i = 0; i < MAXRISINGOBJECTS; i++)
			{
				partstr = strtok_s(NULL, "\t", &nextToken);	// risingObject[i].x
				alonePoints[idx].blocksRising[i].x  = atoi(partstr);
				partstr = strtok_s(NULL, "\t", &nextToken);	// risingObject[i].y
				alonePoints[idx].blocksRising[i].y  = atoi(partstr);
				partstr = strtok_s(NULL, "\t", &nextToken);	// risingObject[i].blockType
				alonePoints[idx].blockTypeRising[i] = atoi(partstr) - 1;
				partstr = strtok_s(NULL, "\t", &nextToken);	// blueDistance
				partstr = strtok_s(NULL, "\t", &nextToken);	// redDistance

				partstr = strtok_s(NULL, "\t", &nextToken);	// risingObjectS2[i].x
				alonePoints[idx].blocksRisingS2[i].x  = atoi(partstr);
				partstr = strtok_s(NULL, "\t", &nextToken);	// risingObjectS2[i].y
				alonePoints[idx].blocksRisingS2[i].y  = atoi(partstr);
				partstr = strtok_s(NULL, "\t", &nextToken);	// blockTypeRising
				partstr = strtok_s(NULL, "\t", &nextToken);	// blueDistanceRisingS2
				partstr = strtok_s(NULL, "\t", &nextToken);	// redDistanceRisingS2

				if(alonePoints[idx].blocksRising[i].x > 0)
				{
					if(alonePoints[idx].blocksRising[i].x > 0)
					{
						alonePoints[idx].blocksRising[i].x = alonePoints[idx].blocksRising[i].x - SQUARESIZE / 2;
						alonePoints[idx].blocksRising[i].y = alonePoints[idx].blocksRising[i].y - SQUARESIZE / 2;
						alonePoints[idx].blocksRisingS2[i].x = alonePoints[idx].blocksRisingS2[i].x - SQUARESIZE / 2;
						alonePoints[idx].blocksRisingS2[i].y = alonePoints[idx].blocksRisingS2[i].y - SQUARESIZE / 2;
					}
				}
				switch(alonePoints[idx].blocksRising[i].x)
				{
					case 41: alonePoints[idx].blockSlotRising[i] = 1; break;
					case 141: alonePoints[idx].blockSlotRising[i] = 2; break;
					case 241: alonePoints[idx].blockSlotRising[i] = 3; break;
					case 341: alonePoints[idx].blockSlotRising[i] = 4; break;
					case 441: alonePoints[idx].blockSlotRising[i] = 5; break;
					case 541: alonePoints[idx].blockSlotRising[i] = 6; break;
					case 641: alonePoints[idx].blockSlotRising[i] = 7; break;
					case 741: alonePoints[idx].blockSlotRising[i] = 8; break;
					case 841: alonePoints[idx].blockSlotRising[i] = 9; break;
					case 941: alonePoints[idx].blockSlotRising[i] = 10; break;
					case 1041: alonePoints[idx].blockSlotRising[i] = 11; break;
					case 1141: alonePoints[idx].blockSlotRising[i] = 12; break;
					default: 
						alonePoints[idx].blockSlotRising[i] = 0;
					break;
				}
			}

			idx++;
		}
	}
	fclose(input);

	maxidx = idx - 1;
	idx = 0;
	return true;
}


void CStarshipTroopersView::InitializeBalls(void)
{
// enum{playTogether=1, allPerson1, allPerson2, alone, replayMaster, allSwitched, allSameold};
	if( (runtype == playTogether) || (runtype == allSwitched) || (runtype == allSameold) )
	{
		blueBallP.x = centerP.x - 200;
		blueBallP.y = centerP.y - 50;
		redBallP.x = centerP.x + 200;
		redBallP.y = centerP.y - 50;

		blueBallPS2.x = blueBallP.x + SCREEN_WIDTH;
		blueBallPS2.y = blueBallP.y;
		redBallPS2.x = redBallP.x + SCREEN_WIDTH;
		redBallPS2.y = redBallP.y;

		blueRect.left = blueBallP.x;
		blueRect.right = blueRect.left + BALLSIZE;
		blueRect.top = blueBallP.y;
		blueRect.bottom = blueRect.top + BALLSIZE;
		oldBlueRect.CopyRect(blueRect);

		blueRectS2.left = blueBallPS2.x;
		blueRectS2.right = blueRectS2.left + BALLSIZE;
		blueRectS2.top = blueBallPS2.y;
		blueRectS2.bottom = blueRectS2.top + BALLSIZE;
		oldBlueRectS2.CopyRect(blueRectS2);

		redRect.left = redBallP.x;
		redRect.right = redRect.left + BALLSIZE;
		redRect.top = blueBallP.y;
		redRect.bottom = redRect.top + BALLSIZE;
		oldRedRect.CopyRect(redRect);

		redRectS2.left = redBallPS2.x;
		redRectS2.right = redRectS2.left + BALLSIZE;
		redRectS2.top = blueBallPS2.y;
		redRectS2.bottom = redRectS2.top + BALLSIZE;
		oldRedRectS2.CopyRect(redRectS2);
	}
	else if(runtype == allPerson1) 
	{
		redBallP.x = centerP.x - 200;
		redBallP.y = centerP.y - 50;
		blueBallP.x = centerP.x + 200;
		blueBallP.y = centerP.y - 50;

		blueBallPS2.x = redBallP.x + SCREEN_WIDTH;
		blueBallPS2.y = redBallP.y;
		redBallPS2.x = blueBallP.x + SCREEN_WIDTH;
		redBallPS2.y = blueBallP.y;

		blueRect.left = blueBallP.x;
		blueRect.right = blueRect.left + BALLSIZE;
		blueRect.top = blueBallP.y;
		blueRect.bottom = blueRect.top + BALLSIZE;
		oldBlueRect.CopyRect(blueRect);

		blueRectS2.left = blueBallPS2.x;
		blueRectS2.right = blueRectS2.left + BALLSIZE;
		blueRectS2.top = blueBallPS2.y;
		blueRectS2.bottom = blueRectS2.top + BALLSIZE;
		oldBlueRectS2.CopyRect(blueRectS2);

		redRect.left = redBallP.x;
		redRect.right = redRect.left + BALLSIZE;
		redRect.top = blueBallP.y;
		redRect.bottom = redRect.top + BALLSIZE;
		oldRedRect.CopyRect(redRect);

		redRectS2.left = redBallPS2.x;
		redRectS2.right = redRectS2.left + BALLSIZE;
		redRectS2.top = blueBallPS2.y;
		redRectS2.bottom = redRectS2.top + BALLSIZE;
		oldRedRectS2.CopyRect(redRectS2);
	}
	else if( (runtype == allPerson2) || (runtype == replayMaster) )
	{
		blueBallP.x = centerP.x - 200;
		blueBallP.y = centerP.y - 50;
		redBallP.x = centerP.x + 200;
		redBallP.y = centerP.y - 50;

		redBallPS2.x = blueBallP.x + SCREEN_WIDTH;
		redBallPS2.y = blueBallP.y;
		blueBallPS2.x = redBallP.x + SCREEN_WIDTH;
		blueBallPS2.y = redBallP.y;

		blueRect.left = blueBallP.x;
		blueRect.right = blueRect.left + BALLSIZE;
		blueRect.top = blueBallP.y;
		blueRect.bottom = blueRect.top + BALLSIZE;
		oldBlueRect.CopyRect(blueRect);

		blueRectS2.left = blueBallPS2.x;
		blueRectS2.right = blueRectS2.left + BALLSIZE;
		blueRectS2.top = blueBallPS2.y;
		blueRectS2.bottom = blueRectS2.top + BALLSIZE;
		oldBlueRectS2.CopyRect(blueRectS2);

		redRect.left = redBallP.x;
		redRect.right = redRect.left + BALLSIZE;
		redRect.top = blueBallP.y;
		redRect.bottom = redRect.top + BALLSIZE;
		oldRedRect.CopyRect(redRect);

		redRectS2.left = redBallPS2.x;
		redRectS2.right = redRectS2.left + BALLSIZE;
		redRectS2.top = blueBallPS2.y;
		redRectS2.bottom = redRectS2.top + BALLSIZE;
		oldRedRectS2.CopyRect(redRectS2);
	}
	else if(runtype == alone)
	{
		blueBallP.x = centerP.x - 50;
		blueBallP.y = centerP.y - 50;
		redBallP.x = -1;
		redBallP.y = -1;

		blueBallPS2.x = -1;
		blueBallPS2.y = -1;
		redBallPS2.x = centerP.x - 50 + SCREEN_WIDTH;
		redBallPS2.y = blueBallP.y;

		blueRect.left = blueBallP.x;
		blueRect.right = blueRect.left + BALLSIZE;
		blueRect.top = blueBallP.y;
		blueRect.bottom = blueRect.top + BALLSIZE;
		oldBlueRect.CopyRect(blueRect);

		blueRectS2.left = -1;
		blueRectS2.right = -1;
		blueRectS2.top = -1;
		blueRectS2.bottom = -1;
		oldBlueRectS2.CopyRect(blueRectS2);

		redRect.left = -1;
		redRect.right = -1;
		redRect.top = -1;
		redRect.bottom = -1;
		oldRedRect.CopyRect(redRect);

		redRectS2.left = redBallPS2.x;
		redRectS2.right = redRectS2.left + BALLSIZE;
		redRectS2.top = redBallPS2.y;
		redRectS2.bottom = redRectS2.top + BALLSIZE;
		oldRedRectS2.CopyRect(redRectS2);
	}
}


void CStarshipTroopersView::ResetAlonePoints()
{
	for(int i = 0; i < MAXGRAPHICOBJECTS; i++)
	{
		alonePoints[i].pointBlue.x = alonePoints[i].pointBlue.y = alonePoints[i].pointRed.x = alonePoints[i].pointRed.y = 0;
		alonePoints[i].pointBlueS2.x = alonePoints[i].pointBlueS2.y = alonePoints[i].pointRedS2.x = alonePoints[i].pointRedS2.y = 0;
		for(int j = 0; j < MAXFALLINGOBJECTS; j++)
		{
			alonePoints[i].blocks[j].x = alonePoints[i].blocks[j].x = 0;
			alonePoints[i].blocksS2[j].x = alonePoints[i].blocksS2[j].x = 0;
			alonePoints[i].blockType[j] = 0;
			alonePoints[i].blockSlot[j] = 0;
		}
		for(int j = 0; j < MAXRISINGOBJECTS; j++)
		{
			alonePoints[i].blocksRising[j].x = alonePoints[i].blocksRising[j].x = 0;
			alonePoints[i].blocksRisingS2[j].x = alonePoints[i].blocksRisingS2[j].x = 0;
			alonePoints[i].blockTypeRising[j] = 0;
			alonePoints[i].blockSlotRising[j] = 0;
		}
		alonePoints[i].millis = 0L;
	}
}


void CStarshipTroopersView::ResetGraphicObjects()
{
	for(int i = 0; i < MAXGRAPHICOBJECTS; i++)
	{
		for(int j = 0; j < MAXFALLINGOBJECTS; j++)
		{
			graphicObjects[i].distanceBlue[j] = graphicObjects[i].distanceRed[j] = 0;
			graphicObjects[i].typeObj[j] = 0;
			graphicObjects[i].centerP[j].x = graphicObjects[i].centerP[j].y = 0;
			graphicObjects[i].distanceBlueS2[j] = graphicObjects[i].distanceRedS2[j] = 0;
			graphicObjects[i].centerPS2[j].x = graphicObjects[i].centerPS2[j].y = 0;
		}
		for(int j = 0; j < MAXRISINGOBJECTS; j++)
		{
			graphicObjects[i].distanceBlueRising[j] = graphicObjects[i].distanceRedRising[j] = 0;
			graphicObjects[i].typeObjRising[j] = 0;
			graphicObjects[i].centerPRising[j].x = graphicObjects[i].centerPRising[j].y = 0;
			graphicObjects[i].distanceBlueRisingS2[j] = graphicObjects[i].distanceRedRisingS2[j] = 0;
			graphicObjects[i].centerPRisingS2[j].x = graphicObjects[i].centerPRisingS2[j].y = 0;
		}
		graphicObjects[i].centerRed.x = graphicObjects[i].centerRed.y = 0;
		graphicObjects[i].centerRedS2.x = graphicObjects[i].centerRedS2.y = 0;
		graphicObjects[i].centerBlue.x = graphicObjects[i].centerBlue.y = 0;
		graphicObjects[i].centerBlueS2.x = graphicObjects[i].centerBlueS2.y = 0;
		graphicObjects[i].blueRedDistance = 0;
		graphicObjects[i].blueRedDistanceS2 = 0;
		graphicObjects[i].js1ButtonPressed = graphicObjects[i].js2ButtonPressed = 0;
		graphicObjects[i].trigger = 0;
		graphicObjects[i].millitime = 0L;
	}
}


void CStarshipTroopersView::ReadBlueJoystick(void)
{
	if(runtype == playTogether)
	{
		if(js1.lX > 0)
		{
			blueBallP.x += STEP;
			if(js1.lX > 250)
				blueBallP.x += STEP;
			if(js1.lX > 500)
				blueBallP.x += STEP;
			if(js1.lX > 750)
				blueBallP.x += STEP;

			if(blueBallP.x > SCREEN_WIDTH - BALLSIZE)
				blueBallP.x = SCREEN_WIDTH - BALLSIZE;
		}
		else if(js1.lX < 0)
		{
			blueBallP.x -= STEP;
			if(js1.lX < -250)
				blueBallP.x -= STEP;
			if(js1.lX < -500)
				blueBallP.x -= STEP;
			if(js1.lX < -750)
				blueBallP.x -= STEP;

			if(blueBallP.x < 0)
				blueBallP.x = 0;
		}
		if(js1.lY > 0)
		{
			blueBallP.y += STEP;
			if(js1.lY > 250)
				blueBallP.y += STEP;
			if(js1.lY > 500)
				blueBallP.y += STEP;
			if(js1.lY > 750)
				blueBallP.y += STEP;

			if(blueBallP.y > SCREEN_HEIGHT - BALLSIZE)
				blueBallP.y = SCREEN_HEIGHT - BALLSIZE;
		}
		else if(js1.lY < 0)
		{
			blueBallP.y -= STEP;
			if(js1.lY < -250)
				blueBallP.y -= STEP;
			if(js1.lY < -500)
				blueBallP.y -= STEP;
			if(js1.lY < -750)
				blueBallP.y -= STEP;

			if(blueBallP.y < 0)
				blueBallP.y = 0;
		}

		if( (js1.lX > 0) || (js1.lX < 0) || (js1.lY > 0) || (js1.lY < 0) )
		{
			oldBlueRect.CopyRect(blueRect);
			blueRect.left = blueBallP.x;
			blueRect.right = blueRect.left + BALLSIZE;
			blueRect.top = blueBallP.y;
			blueRect.bottom = blueRect.top + BALLSIZE;
		}
		oldBlueRectS2.CopyRect(blueRectS2);
		blueBallPS2.y = blueBallP.y;
		blueBallPS2.x = blueBallP.x + SCREEN_WIDTH;
		blueRectS2.left = blueBallPS2.x;
		blueRectS2.right = blueRectS2.left + BALLSIZE;
		blueRectS2.top = blueRect.top;
		blueRectS2.bottom = blueRectS2.top + BALLSIZE;
	}
	else if(runtype == allSwitched)
	{
		if(js1.lX > 0)
		{
			blueBallP.x += STEP;
			if(js1.lX > 250)
				blueBallP.x += STEP;
			if(js1.lX > 500)
				blueBallP.x += STEP;
			if(js1.lX > 750)
				blueBallP.x += STEP;

			if(blueBallP.x > SCREEN_WIDTH - BALLSIZE)
				blueBallP.x = SCREEN_WIDTH - BALLSIZE;
		}
		else if(js1.lX < 0)
		{
			blueBallP.x -= STEP;
			if(js1.lX < -250)
				blueBallP.x -= STEP;
			if(js1.lX < -500)
				blueBallP.x -= STEP;
			if(js1.lX < -750)
				blueBallP.x -= STEP;

			if(blueBallP.x < 0)
				blueBallP.x = 0;
		}
		if(js1.lY > 0)
		{
			blueBallP.y += STEP;
			if(js1.lY > 250)
				blueBallP.y += STEP;
			if(js1.lY > 500)
				blueBallP.y += STEP;
			if(js1.lY > 750)
				blueBallP.y += STEP;

			if(blueBallP.y > SCREEN_HEIGHT - BALLSIZE)
				blueBallP.y = SCREEN_HEIGHT - BALLSIZE;
		}
		else if(js1.lY < 0)
		{
			blueBallP.y -= STEP;
			if(js1.lY < -250)
				blueBallP.y -= STEP;
			if(js1.lY < -500)
				blueBallP.y -= STEP;
			if(js1.lY < -750)
				blueBallP.y -= STEP;

			if(blueBallP.y < 0)
				blueBallP.y = 0;
		}

		if( (js1.lX > 0) || (js1.lX < 0) || (js1.lY > 0) || (js1.lY < 0) )
		{
			oldBlueRect.CopyRect(blueRect);
			blueRect.left = blueBallP.x;
			blueRect.right = blueRect.left + BALLSIZE;
			blueRect.top = blueBallP.y;
			blueRect.bottom = blueRect.top + BALLSIZE;
		}
	}
	else if(runtype == allSameold)
	{
		if(js1.lX > 0)
		{
			redBallP.x += STEP;
			if(js1.lX > 250)
				redBallP.x += STEP;
			if(js1.lX > 500)
				redBallP.x += STEP;
			if(js1.lX > 750)
				redBallP.x += STEP;

			if(redBallP.x > SCREEN_WIDTH - BALLSIZE)
				redBallP.x = SCREEN_WIDTH - BALLSIZE;
		}
		else if(js1.lX < 0)
		{
			redBallP.x -= STEP;
			if(js1.lX < -250)
				redBallP.x -= STEP;
			if(js1.lX < -500)
				redBallP.x -= STEP;
			if(js1.lX < -750)
				redBallP.x -= STEP;

			if(redBallP.x < 0)
				redBallP.x = 0;
		}
		if(js1.lY > 0)
		{
			redBallP.y += STEP;
			if(js1.lY > 250)
				redBallP.y += STEP;
			if(js1.lY > 500)
				redBallP.y += STEP;
			if(js1.lY > 750)
				redBallP.y += STEP;

			if(redBallP.y > SCREEN_HEIGHT - BALLSIZE)
				redBallP.y = SCREEN_HEIGHT - BALLSIZE;
		}
		else if(js1.lY < 0)
		{
			redBallP.y -= STEP;
			if(js1.lY < -250)
				redBallP.y -= STEP;
			if(js1.lY < -500)
				redBallP.y -= STEP;
			if(js1.lY < -750)
				redBallP.y -= STEP;

			if(redBallP.y < 0)
				redBallP.y = 0;
		}

		if( (js1.lX > 0) || (js1.lX < 0) || (js1.lY > 0) || (js1.lY < 0) )
		{
			oldRedRect.CopyRect(redRect);
			redRect.left = redBallP.x;
			redRect.right = redRect.left + BALLSIZE;
			redRect.top = redBallP.y;
			redRect.bottom = redRect.top + BALLSIZE;
		}
	}
	else if( (runtype == allPerson1) || (runtype == allPerson2)  || (runtype == replayMaster) || (runtype == alone) )
	{
		if(js1.lX > 0)
		{
			blueBallP.x += STEP;
			if(js1.lX > 250)
				blueBallP.x += STEP;
			if(js1.lX > 500)
				blueBallP.x += STEP;
			if(js1.lX > 750)
				blueBallP.x += STEP;

			if(blueBallP.x > SCREEN_WIDTH - BALLSIZE)
				blueBallP.x = SCREEN_WIDTH - BALLSIZE;
		}
		else if(js1.lX < 0)
		{
			blueBallP.x -= STEP;
			if(js1.lX < -250)
				blueBallP.x -= STEP;
			if(js1.lX < -500)
				blueBallP.x -= STEP;
			if(js1.lX < -750)
				blueBallP.x -= STEP;

			if(blueBallP.x < 0)
				blueBallP.x = 0;
		}
		if(js1.lY > 0)
		{
			blueBallP.y += STEP;
			if(js1.lY > 250)
				blueBallP.y += STEP;
			if(js1.lY > 500)
				blueBallP.y += STEP;
			if(js1.lY > 750)
				blueBallP.y += STEP;

			if(blueBallP.y > SCREEN_HEIGHT - BALLSIZE)
				blueBallP.y = SCREEN_HEIGHT - BALLSIZE;
		}
		else if(js1.lY < 0)
		{
			blueBallP.y -= STEP;
			if(js1.lY < -250)
				blueBallP.y -= STEP;
			if(js1.lY < -500)
				blueBallP.y -= STEP;
			if(js1.lY < -750)
				blueBallP.y -= STEP;

			if(blueBallP.y < 0)
				blueBallP.y = 0;
		}

		if( (js1.lX > 0) || (js1.lX < 0) || (js1.lY > 0) || (js1.lY < 0) )
		{
			oldBlueRect.CopyRect(blueRect);
			blueRect.left = blueBallP.x;
			blueRect.right = blueRect.left + BALLSIZE;
			blueRect.top = blueBallP.y;
			blueRect.bottom = blueRect.top + BALLSIZE;
		}
	}
}


void CStarshipTroopersView::ReadRedJoystick(void)
{
// enum{playTogether=1, allSwitched, allSameold, allPerson1, allPerson2, alone};
	if(runtype == playTogether)
	{
		if(js2.lX > 0)
		{
			redBallP.x += STEP;
			if(js2.lX > 250)
				redBallP.x += STEP;
			if(js2.lX > 500)
				redBallP.x += STEP;
			if(js2.lX > 750)
				redBallP.x += STEP;
			if(redBallP.x > SCREEN_WIDTH - BALLSIZE)
				redBallP.x = SCREEN_WIDTH - BALLSIZE;
		}
		else if(js2.lX < 0)
		{
			redBallP.x -= STEP;
			if(js2.lX < -250)
				redBallP.x -= STEP;
			if(js2.lX < -500)
				redBallP.x -= STEP;
			if(js2.lX < -750)
				redBallP.x -= STEP;

			if(redBallP.x < 0)
				redBallP.x = 0;
		}
		if(js2.lY > 0)
		{
			redBallP.y += STEP;
			if(js2.lY > 250)
				redBallP.y += STEP;
			if(js2.lY > 500)
				redBallP.y += STEP;
			if(js2.lY > 750)
				redBallP.y += STEP;

			if(redBallP.y > SCREEN_HEIGHT - BALLSIZE)
				redBallP.y = SCREEN_HEIGHT - BALLSIZE;
		}
		else if(js2.lY < 0)
		{
			redBallP.y -= STEP;
			if(js2.lY < -250)
				redBallP.y -= STEP;
			if(js2.lY < -500)
				redBallP.y -= STEP;
			if(js2.lY < -750)
				redBallP.y -= STEP;

			if(redBallP.y < 0)
				redBallP.y = 0;
		}

		if( (js2.lX > 0) || (js2.lX < 0) || (js2.lY > 0) || (js2.lY < 0) )
		{
			oldRedRect.CopyRect(redRect);
			redRect.left = redBallP.x;
			redRect.right = redRect.left + BALLSIZE;
			redRect.top = redBallP.y;
			redRect.bottom = redRect.top + BALLSIZE;
		}
		oldRedRectS2.CopyRect(redRectS2);
		redBallPS2.y = redBallP.y;
		redBallPS2.x = redBallP.x + SCREEN_WIDTH;
		redRectS2.left = redBallPS2.x;
		redRectS2.right = redRectS2.left + BALLSIZE;
		redRectS2.top = redRect.top;
		redRectS2.bottom = redRectS2.top + BALLSIZE;
	}
	else if(runtype == allSwitched)
	{
		if(js2.lX > 0)
		{
			redBallPS2.x += STEP;
			if(js2.lX > 250)
				redBallPS2.x += STEP;
			if(js2.lX > 500)
				redBallPS2.x += STEP;
			if(js2.lX > 750)
				redBallPS2.x += STEP;
			if(redBallPS2.x > SCREEN_WIDTH + SCREEN_WIDTH - BALLSIZE)
				redBallPS2.x = SCREEN_WIDTH + SCREEN_WIDTH - BALLSIZE;
		}
		else if(js2.lX < 0)
		{
			redBallPS2.x -= STEP;
			if(js2.lX < -250)
				redBallPS2.x -= STEP;
			if(js2.lX < -500)
				redBallPS2.x -= STEP;
			if(js2.lX < -750)
				redBallPS2.x -= STEP;

			if(redBallPS2.x < SCREEN_WIDTH)
				redBallPS2.x = SCREEN_WIDTH;
		}
		if(js2.lY > 0)
		{
			redBallPS2.y += STEP;
			if(js2.lY > 250)
				redBallPS2.y += STEP;
			if(js2.lY > 500)
				redBallPS2.y += STEP;
			if(js2.lY > 750)
				redBallPS2.y += STEP;

			if(redBallPS2.y > SCREEN_HEIGHT - BALLSIZE)
				redBallPS2.y = SCREEN_HEIGHT - BALLSIZE;
		}
		else if(js2.lY < 0)
		{
			redBallPS2.y -= STEP;
			if(js2.lY < -250)
				redBallPS2.y -= STEP;
			if(js2.lY < -500)
				redBallPS2.y -= STEP;
			if(js2.lY < -750)
				redBallPS2.y -= STEP;

			if(redBallPS2.y < 0)
				redBallPS2.y = 0;
		}

		if( (js2.lX > 0) || (js2.lX < 0) || (js2.lY > 0) || (js2.lY < 0) )
		{
			oldRedRectS2.CopyRect(redRectS2);
			redRectS2.left = redBallPS2.x;
			redRectS2.right = redRectS2.left + BALLSIZE;
			redRectS2.top = redBallPS2.y;
			redRectS2.bottom = redRectS2.top + BALLSIZE;
		}
	}
	else if(runtype == allSameold)
	{
		if(js2.lX > 0)
		{
			blueBallPS2.x += STEP;
			if(js2.lX > 250)
				blueBallPS2.x += STEP;
			if(js2.lX > 500)
				blueBallPS2.x += STEP;
			if(js2.lX > 750)
				blueBallPS2.x += STEP;
			if(blueBallPS2.x > SCREEN_WIDTH + SCREEN_WIDTH - BALLSIZE)
				blueBallPS2.x = SCREEN_WIDTH + SCREEN_WIDTH - BALLSIZE;
		}
		else if(js2.lX < 0)
		{
			blueBallPS2.x -= STEP;
			if(js2.lX < -250)
				blueBallPS2.x -= STEP;
			if(js2.lX < -500)
				blueBallPS2.x -= STEP;
			if(js2.lX < -750)
				blueBallPS2.x -= STEP;

			if(blueBallPS2.x < SCREEN_WIDTH)
				blueBallPS2.x = SCREEN_WIDTH;
		}
		if(js2.lY > 0)
		{
			blueBallPS2.y += STEP;
			if(js2.lY > 250)
				blueBallPS2.y += STEP;
			if(js2.lY > 500)
				blueBallPS2.y += STEP;
			if(js2.lY > 750)
				blueBallPS2.y += STEP;

			if(blueBallPS2.y > SCREEN_HEIGHT - BALLSIZE)
				blueBallPS2.y = SCREEN_HEIGHT - BALLSIZE;
		}
		else if(js2.lY < 0)
		{
			blueBallPS2.y -= STEP;
			if(js2.lY < -250)
				blueBallPS2.y -= STEP;
			if(js2.lY < -500)
				blueBallPS2.y -= STEP;
			if(js2.lY < -750)
				blueBallPS2.y -= STEP;

			if(blueBallPS2.y < 0)
				blueBallPS2.y = 0;
		}

		if( (js2.lX > 0) || (js2.lX < 0) || (js2.lY > 0) || (js2.lY < 0) )
		{
			oldBlueRectS2.CopyRect(blueRectS2);
			blueRectS2.left = blueBallPS2.x;
			blueRectS2.right = blueRectS2.left + BALLSIZE;
			blueRectS2.top = blueBallPS2.y;
			blueRectS2.bottom = blueRectS2.top + BALLSIZE;
		}
	}
	else if( (runtype == allPerson1) || (runtype == allPerson2) || (runtype == replayMaster) || (runtype == alone) )
	{
		if(js2.lX > 0)
		{
			redBallPS2.x += STEP;
			if(js2.lX > 250)
				redBallPS2.x += STEP;
			if(js2.lX > 500)
				redBallPS2.x += STEP;
			if(js2.lX > 750)
				redBallPS2.x += STEP;
			if(redBallPS2.x > SCREEN_WIDTH + SCREEN_WIDTH - BALLSIZE)
				redBallPS2.x = SCREEN_WIDTH + SCREEN_WIDTH - BALLSIZE;
		}
		else if(js2.lX < 0)
		{
			redBallPS2.x -= STEP;
			if(js2.lX < -250)
				redBallPS2.x -= STEP;
			if(js2.lX < -500)
				redBallPS2.x -= STEP;
			if(js2.lX < -750)
				redBallPS2.x -= STEP;

			if(redBallPS2.x < SCREEN_WIDTH)
				redBallPS2.x = SCREEN_WIDTH;
		}
		if(js2.lY > 0)
		{
			redBallPS2.y += STEP;
			if(js2.lY > 250)
				redBallPS2.y += STEP;
			if(js2.lY > 500)
				redBallPS2.y += STEP;
			if(js2.lY > 750)
				redBallPS2.y += STEP;

			if(redBallPS2.y > SCREEN_HEIGHT - BALLSIZE)
				redBallPS2.y = SCREEN_HEIGHT - BALLSIZE;
		}
		else if(js2.lY < 0)
		{
			redBallPS2.y -= STEP;
			if(js2.lY < -250)
				redBallPS2.y -= STEP;
			if(js2.lY < -500)
				redBallPS2.y -= STEP;
			if(js2.lY < -750)
				redBallPS2.y -= STEP;

			if(redBallPS2.y < 0)
				redBallPS2.y = 0;
		}

		if( (js2.lX > 0) || (js2.lX < 0) || (js2.lY > 0) || (js2.lY < 0) )
		{
			oldRedRectS2.CopyRect(redRectS2);
			redRectS2.left = redBallPS2.x;
			redRectS2.right = redRectS2.left + BALLSIZE;
			redRectS2.top = redBallPS2.y;
			redRectS2.bottom = redRectS2.top + BALLSIZE;
		}
	}
}


void CStarshipTroopersView::ReadBlueJoystickRating(int which)
{
	if(currmillis - millis1 > RATINGDELAYMILLIS)
	{
		if(js1.lX > 250)
		{
			if(which == 1)
			{
				if(blueRating1 < 10)
					blueRating1++;
			}
			else if(which == 2)
			{
				if(blueRating2 < 9)
					blueRating2++;
			}
		}
		else if(js1.lX < -250)
		{
			if(which == 1)
			{
				if(blueRating1 > 1)
					blueRating1--;
			}
			else if(which == 2)
			{
				if(blueRating2 > 1)
					blueRating2--;
			}
		}
		millis1 = currmillis;
	}
}


void CStarshipTroopersView::ReadRedJoystickRating(int which)
{
	if(currmillis - millis2 > RATINGDELAYMILLIS)
	{
		if(js2.lX > 250)
		{
			if(which == 1)
			{
				if(redRating1 < 10)
					redRating1++;
			}
			else if(which == 2)
			{
				if(redRating2 < 9)
					redRating2++;
			}
		}
		else if(js2.lX < -250)
		{
			if(which == 1)
			{
				if(redRating1 > 1)
					redRating1--;
			}
			else if(which == 2)
			{
				if(redRating2 > 1)
					redRating2--;
			}
		}
		millis2 = currmillis;
	}
}



int CStarshipTroopersView::FindStandAlonePos(int which, int lastIdx, unsigned long currtime)
{
	bool done = false;
	while(! done)
	{
		if(alonePoints[lastIdx].millis >= currtime)
		{
//enum{playTogether=1, allSwitched, allSameold, allPerson1, allPerson2, alone};
			if(which == allSwitched)
			{
				oldRedRect.CopyRect(redRect);
				oldBlueRectS2.CopyRect(blueRectS2);
				redBallP.x = alonePoints[lastIdx].pointRed.x - (BALLSIZE / 2);
				redBallP.y = alonePoints[lastIdx].pointRed.y - (BALLSIZE / 2);
				redRect.left = redBallP.x;
				redRect.right = redRect.left + BALLSIZE;
				redRect.top = redBallP.y;
				redRect.bottom = redRect.top + BALLSIZE;

				blueBallPS2.x = alonePoints[lastIdx].pointBlueS2.x - (BALLSIZE / 2);
				blueBallPS2.y = alonePoints[lastIdx].pointBlueS2.y - (BALLSIZE / 2);
				blueRectS2.left = blueBallPS2.x;
				blueRectS2.right = blueBallPS2.x + BALLSIZE;
				blueRectS2.top = blueBallPS2.y;
				blueRectS2.bottom = blueRectS2.top + BALLSIZE;
			}
			else if(which == allSameold)
			{
				oldBlueRect.CopyRect(blueRect);
				oldRedRectS2.CopyRect(redRectS2);
				blueBallP.x = alonePoints[lastIdx].pointBlue.x - (BALLSIZE / 2);
				blueBallP.y = alonePoints[lastIdx].pointBlue.y - (BALLSIZE / 2);
				blueRect.left = blueBallP.x;
				blueRect.right = blueRect.left + BALLSIZE;
				blueRect.top = blueBallP.y;
				blueRect.bottom = blueRect.top + BALLSIZE;

				redBallPS2.x = alonePoints[lastIdx].pointRed.x - (BALLSIZE / 2) + SCREEN_WIDTH;
				redBallPS2.y = alonePoints[lastIdx].pointRed.y - (BALLSIZE / 2);
				redRectS2.left = redBallPS2.x;
				redRectS2.right = redBallPS2.x + BALLSIZE;
				redRectS2.top = redBallPS2.y;
				redRectS2.bottom = redRectS2.top + BALLSIZE;
			}
			else if(which == allPerson1)
			{
				oldRedRect.CopyRect(redRect);
				oldBlueRectS2.CopyRect(blueRectS2);
				redBallP.x = alonePoints[lastIdx].pointBlue.x - (BALLSIZE / 2);
				redBallP.y = alonePoints[lastIdx].pointBlue.y - (BALLSIZE / 2);
				redRect.left = redBallP.x;
				redRect.right = redRect.left + BALLSIZE;
				redRect.top = redBallP.y;
				redRect.bottom = redRect.top + BALLSIZE;

				blueBallPS2.x = alonePoints[lastIdx].pointBlue.x - (BALLSIZE / 2) + SCREEN_WIDTH;
				blueBallPS2.y = alonePoints[lastIdx].pointBlue.y - (BALLSIZE / 2);
				blueRectS2.left = blueBallPS2.x;
				blueRectS2.right = blueRectS2.left + BALLSIZE;
				blueRectS2.top = blueBallPS2.y;
				blueRectS2.bottom = blueRectS2.top + BALLSIZE;
			}
			else if( (which == allPerson2) || (which == replayMaster) )
			{
				oldRedRect.CopyRect(redRect);
				oldBlueRectS2.CopyRect(blueRectS2);
				redBallP.x = alonePoints[lastIdx].pointRedS2.x - (BALLSIZE / 2) - SCREEN_WIDTH;
				redBallP.y = alonePoints[lastIdx].pointRedS2.y - (BALLSIZE / 2);
				redRect.left = redBallP.x;
				redRect.right = redRect.left + BALLSIZE;
				redRect.top = redBallP.y;
				redRect.bottom = redRect.top + BALLSIZE;

				blueBallPS2.x = alonePoints[lastIdx].pointRedS2.x - (BALLSIZE / 2);
				blueBallPS2.y = alonePoints[lastIdx].pointRedS2.y - (BALLSIZE / 2);
				blueRectS2.left = blueBallPS2.x;
				blueRectS2.right = blueRectS2.left + BALLSIZE;
				blueRectS2.top = blueBallPS2.y;
				blueRectS2.bottom = blueRectS2.top + BALLSIZE;
			}
			done = true;
			return lastIdx;
		}
		lastIdx++;
		if(lastIdx >= maxidx)
		{
			lastIdx--;
			break;
		}
	}
	return lastIdx;
}


void CStarshipTroopersView::WhiteItemRects(CDC* pDC)
{
	CPen oldPen, myPen(PS_SOLID, 3, RGB(255, 255, 255));
	if(avoidEachOtherRun)
	{
		pDC->SelectObject(&myPen);
		pDC->MoveTo(avoid1P);
		pDC->LineTo(avoid2P);
		pDC->MoveTo(avoid1PS2);
		pDC->LineTo(avoid2PS2);
	}

	for(int i = 0; i < fallingObjs; i++)
	{
		if(fallingObjects[i].isOn)
		{
			pDC->FillRect( &fallingObjects[i].blockRect, &brWhite );
			pDC->FillRect( &fallingObjectsS2[i].blockRect, &brWhite );
		}
	}

	for(int i = 0; i < risingObjs; i++)
	{
		if(risingObjects[i].isOn)
		{
			pDC->FillRect( &risingObjects[i].blockRect, &brWhite );
			pDC->FillRect( &risingObjectsS2[i].blockRect, &brWhite );
		}
	}

	oldBlueRect.InflateRect(3,3);
	oldRedRect.InflateRect(3,3);
	oldBlueRectS2.InflateRect(3,3);
	oldRedRectS2.InflateRect(3,3);
	pDC->FillRect( &oldBlueRect, &brWhite );
	pDC->FillRect( &oldRedRect, &brWhite );
	pDC->FillRect( &oldBlueRectS2, &brWhite );
	pDC->FillRect( &oldRedRectS2, &brWhite );
}


void CStarshipTroopersView::OnStartexperiment()
{
	if(! canrun)
	{
		MessageBox(L"ERRRRROR: Cannot run experiment without Setup!", NULL, MB_OK);
		return;
	}

	CMonitors::GetVirtualDesktopRect( &rect);
	HDC hDC = ::GetDC( NULL );
	CDC *pDC = CDC::FromHandle(hDC);
	CMonitor monitor;
	CMonitors monitors;
	brWhite.CreateSolidBrush( RGB(255,255,255) );
	brRed.CreateSolidBrush( RGB(255,120,120) );
	brBlack.CreateSolidBrush( RGB(0,0,0) );
	CBrush *oldBrush = pDC->SelectObject(&brWhite);
	::Rectangle( hDC, rect.left, rect.top, rect.right, rect.bottom );

	ShowCursor(FALSE);
	exprunning = true;
	FullScreenHandler.Maximize(this->GetParentFrame(), this);
	HRESULT hr;
// pDC = GetDC();
	CRect rcClient;
	GetClientRect( rcClient );
	newFont.CreatePointFont(200, L"Arial", pDC);
	oldfont = pDC->SelectObject(&newFont);

	CString iuDateStr, iuTimeStr;
	CTime t = CTime::GetCurrentTime();
	char fname[256], daystr[256], monthstr[256];
	int day = t.GetDay(), month = t.GetMonth(), year = t.GetYear();
	int hour = t.GetHour(), minute = t.GetMinute();
	if(day < 10)
		sprintf_s(daystr, "0%d", day);
	else
		sprintf_s(daystr, "%d", day);

	if(month < 10)
		sprintf_s(monthstr, "0%d", month);
	else
		sprintf_s(monthstr, "%d", month);

	centerP.x = rcClient.Width()/2;
	centerP.y  = centerPS2.y  = rcClient.Height()/2;
	centerPS2.x = rcClient.Width()/2 + SCREEN_WIDTH;

	if(! avoidEachOther)
	{
		trials[0].type = alone;
		strcpy_s(trials[0].typeStr, "alone");

		trials[1].type = playTogether;
		strcpy_s(trials[1].typeStr, "together");

		trials[2].type = replayMaster;
		strcpy_s(trials[2].typeStr, "replayMaster");
		strcpy_s(trials[2].replayFileName, ".\\Daten\\Master1.txt");

		trials[3].type = playTogether;
		strcpy_s(trials[3].typeStr, "together");

		trials[4].type = allPerson1;
		strcpy_s(trials[4].typeStr, "allPerson1");

		trials[5].type = playTogether;
		strcpy_s(trials[5].typeStr, "together");

		trials[6].type = allPerson2;
		strcpy_s(trials[6].typeStr, "allPerson2");

		trials[7].type = alone;
		strcpy_s(trials[7].typeStr, "alone");

		trials[8].type = playTogether;
		strcpy_s(trials[8].typeStr, "together");

		trials[9].type = allPerson1;
		strcpy_s(trials[9].typeStr, "allPerson1");

		trials[10].type = playTogether;
		strcpy_s(trials[10].typeStr, "together");

		trials[11].type = replayMaster;
		strcpy_s(trials[11].typeStr, "replayMaster");
		strcpy_s(trials[11].replayFileName, ".\\Daten\\Master2.txt");

		trials[12].type = playTogether;
		strcpy_s(trials[12].typeStr, "together");

		trials[13].type = allPerson2;
		strcpy_s(trials[13].typeStr, "allPerson2");

		trials[14].type = alone;
		strcpy_s(trials[14].typeStr, "alone");

		trials[15].type = playTogether;
		strcpy_s(trials[15].typeStr, "together");

		trials[16].type = allPerson1;
		strcpy_s(trials[16].typeStr, "allPerson1");

		trials[17].type = playTogether;
		strcpy_s(trials[17].typeStr, "together");

		trials[18].type = allPerson2;
		strcpy_s(trials[18].typeStr, "allPerson2");

		trials[19].type = playTogether;
		strcpy_s(trials[19].typeStr, "together");

		trials[20].type = replayMaster;
		strcpy_s(trials[20].typeStr, "replayMaster");
		strcpy_s(trials[20].replayFileName, ".\\Daten\\Master2.txt");
	}
	else
	{
		trials[0].type = alone;
		strcpy_s(trials[0].typeStr, "alone");

		trials[1].type = playTogether;
		strcpy_s(trials[1].typeStr, "together");

		trials[2].type = replayMaster;
		strcpy_s(trials[2].typeStr, "replayMaster");
		strcpy_s(trials[2].replayFileName, ".\\Daten\\Master1.txt");

		trials[3].type = playTogether;
		strcpy_s(trials[3].typeStr, "together");

		trials[4].type = allPerson1;
		strcpy_s(trials[4].typeStr, "allPerson1");

		trials[5].type = playTogether;
		strcpy_s(trials[5].typeStr, "together");

		trials[6].type = allPerson2;
		strcpy_s(trials[6].typeStr, "allPerson2");

		trials[7].type = alone;
		strcpy_s(trials[7].typeStr, "alone");

		trials[8].type = playTogether;
		strcpy_s(trials[8].typeStr, "together");

		trials[9].type = allPerson1;
		strcpy_s(trials[9].typeStr, "allPerson1");

		trials[10].type = playTogether;
		strcpy_s(trials[10].typeStr, "together");

		trials[11].type = replayMaster;
		strcpy_s(trials[11].typeStr, "replayMaster");
		strcpy_s(trials[11].replayFileName, ".\\Daten\\Master2.txt");

		trials[12].type = playTogether;
		strcpy_s(trials[12].typeStr, "together");

		trials[13].type = allPerson2;
		strcpy_s(trials[13].typeStr, "allPerson2");

		trials[14].type = alone;
		strcpy_s(trials[14].typeStr, "alone");

		trials[15].type = playTogether;
		strcpy_s(trials[15].typeStr, "together");

		trials[16].type = allPerson1;
		strcpy_s(trials[16].typeStr, "allPerson1");

		trials[17].type = playTogether;
		strcpy_s(trials[17].typeStr, "together");

		trials[18].type = allPerson2;
		strcpy_s(trials[18].typeStr, "allPerson2");

		trials[19].type = playTogether;
		strcpy_s(trials[19].typeStr, "together");

		trials[20].type = replayMaster;
		strcpy_s(trials[20].typeStr, "replayMaster");
		strcpy_s(trials[20].replayFileName, ".\\Daten\\Master2.txt");

		trials[21].type = allPerson1;
		strcpy_s(trials[21].typeStr, "allPerson1");

		trials[22].type = allPerson2;
		strcpy_s(trials[22].typeStr, "allPerson2");

		trials[23].type = allPerson1;
		strcpy_s(trials[23].typeStr, "allPerson1");
	}
//enum{playTogether=1, allPerson1, allPerson2, replayMaster, alone, allSwitched, allSameold};

	Dialog1 dialog1;

	int maxRuns;
	if(! avoidEachOther)
		maxRuns = CURRTRIALS;
	else
		maxRuns = CURRTRIALSAVOID;
	for(run = 0; run < maxRuns; run++)
	{
		if(avoidEachOther)
		{
			if(run >= TRIALSAVOID)
				avoidEachOtherRun = 0;
		}
		runtype = trials[run].type;
		switch(trials[run].type)
		{
			case playTogether:
				player1StickColor = blueBall;
				player2StickColor = redBall;
				joy1Startpos = js1Left;
				joy2Startpos = js2Right;
				break;
			case allSwitched:
				player1StickColor = blueBall;
				player2StickColor = redBall;
				joy1Startpos = js1Left;
				joy2Startpos = js2Right;
				break;
			case allSameold:
				player1StickColor = redBall;
				player2StickColor = blueBall;
				joy1Startpos = js1Right;
				joy2Startpos = js2Left;
				break;
			case allPerson1:
				player1StickColor = blueBall;
				player2StickColor = redBall;
				joy1Startpos = js1Right;
				joy2Startpos = js2Right;
				break;
			case allPerson2:
			case replayMaster:
				player1StickColor = blueBall;
				player2StickColor = redBall;
				joy1Startpos = js1Left;
				joy2Startpos = js2Left;
				break;
			case alone:
				player1StickColor = blueBall;
				player2StickColor = redBall;
				joy1Startpos = js1Center;
				joy2Startpos = js2Center;
				break;
		}

		pDC->FillRect( &rect, &brWhite );
		ShowCursor(TRUE);

		sprintf_s(dialog1.runNumStr, "Durchgang %d beginnt!", run + 1);
		if(dialog1.DoModal() != IDOK)
		{
			exit(0);
		}

		ShowCursor(FALSE);
 
		sprintf_s(fname, ".\\Daten\\%s_%d_%d_%d_%s_%s.txt", vpid, run + 1, avoidEachOther, year, monthstr, daystr);

		if(! avoidEachOther)
		{
			switch(run)
			{
				case 1: strcpy_s(trials[4].replayFileName, fname); break;
				case 3: strcpy_s(trials[6].replayFileName, fname); break;
				case 5: strcpy_s(trials[9].replayFileName, fname); break;
				case 8: strcpy_s(trials[13].replayFileName, fname); break;
				case 10: strcpy_s(trials[16].replayFileName, fname); break;
				case 12: strcpy_s(trials[18].replayFileName, fname); break;
			}
		}
		else
		{
			switch(run)
			{
				case 1: strcpy_s(trials[4].replayFileName, fname); break;
				case 3: strcpy_s(trials[6].replayFileName, fname); break;
				case 5: strcpy_s(trials[9].replayFileName, fname); break;
				case 8: strcpy_s(trials[13].replayFileName, fname); break;
				case 10: strcpy_s(trials[16].replayFileName, fname); break;
				case 12: strcpy_s(trials[18].replayFileName, fname); break;
				case 15: strcpy_s(trials[21].replayFileName, fname); break;
				case 17: strcpy_s(trials[22].replayFileName, fname); break;
				case 19: strcpy_s(trials[23].replayFileName, fname); break;
			}
		}

		if( (runtype == playTogether) || (runtype == alone) )
			replay = 0;
		else
			replay = 1;

		if(fallingObjectsTypes > 1)
			InitializeFallingObjects();

		if(replay == 1)
		{			
			if(! ReadStandAloneData(trials[run].replayFileName, trials[run].type))
			{
				ShowCursor(TRUE);
				AfxMessageBox(L"ERRRRROR: Cannot read coordinate of last try!");
				exit(0);
			}
		}

		InitializeBalls();
		
		pDC->FillRect( &rect, &brWhite );
		
		hr = joystick1->Poll(); 
		hr = joystick1->Acquire();
		if (FAILED(hr = joystick1->GetDeviceState(sizeof(DIJOYSTATE2), &js1)))
		{
			ShowCursor(TRUE);
			AfxMessageBox(L"ERRRRROR: Something wrong with Joystick1!\nProgramm wird beendet!");
			exit(0);
		}

		hr = joystick2->Poll(); 
		hr = joystick2->Acquire();
		if (FAILED(hr = joystick2->GetDeviceState(sizeof(DIJOYSTATE2), &js2)))
		{
			ShowCursor(TRUE);
			AfxMessageBox(L"ERRRRROR: Something wrong with Joystick2!\nProgramm wird beendet!");
			exit(0);
		}

		if(runtype == alone)
		{
			m_imageList.Draw(pDC, blueBall, blueBallP, ILD_TRANSPARENT);
			m_imageList.Draw(pDC, redBall, redBallPS2, ILD_TRANSPARENT);
		}
		else
		{
			m_imageList.Draw(pDC, blueBall, blueBallP, ILD_TRANSPARENT);
			m_imageList.Draw(pDC, redBall, redBallP, ILD_TRANSPARENT);
			m_imageList.Draw(pDC, blueBall, blueBallPS2, ILD_TRANSPARENT);
			m_imageList.Draw(pDC, redBall, redBallPS2, ILD_TRANSPARENT);
		}


		currmillis = startmillis = stopmillis = timeGetTime();
		counter = 0;
		collisions = 0, collisionsS2 = 0;
		stars = 0, starsS2 = 0;
		together = 0, togetherS2 = 0;
		idx = 0;

		touchTime1 = touchTime2 = 0L;
		touchDistance1 = touchDistance2 = 9999;
		if(avoidEachOtherRun)
			touchDistance1 = touchDistance2 = 0;

		for(int i = 0; i < MAXFALLINGOBJECTS; i++)
		{
			bluecollision[i] = bluecollisionS2[i] = redcollision[i] = redcollisionS2[i] = 0;
		}

		Out32(0x378, 0); //reset parallel port
		while(stopmillis < startmillis + 20) // wait 20 ms
			stopmillis = timeGetTime();
		if(! avoidEachOther) // send trigger out
			Out32(0x378, run + 101);
		else
			Out32(0x378, run + 201);
		currmillis = startmillis = stopmillis = timeGetTime();
		while(stopmillis < startmillis + 20) // wait 20 ms
			stopmillis = timeGetTime();
		Out32(0x378, 0); //reset parallel port

		HDC hDC = ::GetDC( NULL );
		CDC *pDCx = CDC::FromHandle(hDC);

		currmillis = startmillis = stopmillis = millis10 = timeGetTime();
		int second10Count = 1;
		boolean parPortHigh = false;
		while(stopmillis < startmillis + 30000)
//		while(stopmillis < startmillis + (runMinutes * 60 * 1000))
//		while(stopmillis < startmillis + (100 * 1000))
		{
			stopmillis = timeGetTime();
			currtime = stopmillis - startmillis;

			if(currmillis - millis10 > 10000)
			{
				Out32(0x378, second10Count);
				millis10 = currmillis;
				parPortHigh = true;
				graphicObjects[counter].trigger = second10Count;
				second10Count++;
			}
			else if(parPortHigh)
			{
				if(currmillis - millis10 > 20)
				{
					Out32(0x378, 0);
					parPortHigh = false;
				}
			}
			hr = joystick1->Poll(); 
			hr = joystick1->Acquire();
			if (FAILED(hr = joystick1->GetDeviceState(sizeof(DIJOYSTATE2), &js1)))
			{
				ShowCursor(TRUE);
				AfxMessageBox(L"ERRRRROR: Something wrong with Joystick1!\nProgramm wird beendet!");
				exit(0);
			}

			hr = joystick2->Poll(); 
			hr = joystick2->Acquire();
			if (FAILED(hr = joystick2->GetDeviceState(sizeof(DIJOYSTATE2), &js2)))
			{
				ShowCursor(TRUE);
				AfxMessageBox(L"ERRRRROR: Something wrong with Joystick2!\nProgramm wird beendet!");
				exit(0);
			}

			isOverlapping1 = isOverlapping2 = FALSE;
			
			ReadBlueJoystick();
			ReadRedJoystick();

			if(replay)
			{
				idx = FindStandAlonePos(runtype, idx, currtime);
			}

			isOverlapping1 = FALSE;
			isOverlapping2 = FALSE;
			if(! avoidEachOtherRun)
			{
				if(overlappingRect.IntersectRect(blueRect, redRect))
				{
					overlappingRect.UnionRect(blueRect, redRect);
					isOverlapping1 = TRUE;

					touchDistance1 += (int)sqrt(pow(abs((blueRect.left - redRect.left)), 2.0) + pow(abs((blueRect.top - redRect.top)), 2.0));
					if(currmillis - touchTime1 >= 1000)
					{
						int distance = touchDistance1;
						if(distance <= 80)
						{
							int winPoints = 0;
							winPoints = 80 - distance;
							together += winPoints;
							touchTime1 = currmillis;
						}
						touchDistance1 = 0;
					}
				}

				if(overlappingRectS2.IntersectRect(blueRectS2, redRectS2))
				{
					overlappingRectS2.UnionRect(blueRectS2, redRectS2);
					isOverlapping2 = TRUE;

					touchDistance2 += (int)sqrt(pow(abs((blueRectS2.left - redRectS2.left)), 2.0) + pow(abs((blueRectS2.top - redRectS2.top)), 2.0));
					if(currmillis - touchTime2 >= 1000)
					{
						int distance = touchDistance2;
						if(distance <= 80)
						{
							int winPoints = 0;
							winPoints = 80 - distance;
							togetherS2 += winPoints;
							touchTime2 = currmillis;
						}
						touchDistance2 = 0;
					}
				}
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////
//			lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);

			CMemDC pDCOff(pDCx);
			
			//pDC->FillRect( &fallingObjects[i].blockRect, &brWhite );
			//WhiteItemRects(pDCOff);

			bool squareOverlapping = false, squareOverlapping2 = false;
			CRect blueRectReduced, redRectReduced;
			CRect blueRectReducedS2, redRectReducedS2;

			if(fallingObjectsTypes > 1)
			{
				UpdateFallingObjects(currtime);
			}

			if(fallingObjectsTypes > 1)
			{
				for(int i = 0; i < fallingObjs; i++)
				{
					bluecollision[i] = 0;
					redcollision[i] = 0;
					bluecollisionS2[i] = 0;
					redcollisionS2[i] = 0;
				}

				blueRectReduced.CopyRect(blueRect);
				redRectReduced.CopyRect(redRect);
				blueRectReduced.DeflateRect(CIRCLEREDUCED, CIRCLEREDUCED);
				redRectReduced.DeflateRect(CIRCLEREDUCED, CIRCLEREDUCED);
				blueRectReducedS2.CopyRect(blueRectS2);
				redRectReducedS2.CopyRect(redRectS2);
				blueRectReducedS2.DeflateRect(CIRCLEREDUCED, CIRCLEREDUCED);
				redRectReducedS2.DeflateRect(CIRCLEREDUCED, CIRCLEREDUCED);

				boolean collisiondetectedFalling = false, collisiondetectedRising = false;
				boolean collisiondetectedS2Falling = false, collisiondetectedS2Rising = false;
				for(int i = 0; i < fallingObjs; i++)
				{
					squareOverlapping = false;
					if(fallingObjects[i].isOn)
					{
						if(overlappingRect.IntersectRect(blueRectReduced, fallingObjects[i].blockRect))
						{
							bluecollision[i] = 1;
							squareOverlapping = true;
						}
						if(overlappingRect.IntersectRect(redRectReduced, fallingObjects[i].blockRect))
						{
							redcollision[i] = 1;
							squareOverlapping = true;
						}


						p.x = fallingObjects[i].left;
						p.y = fallingObjects[i].top;
						if(fallingObjects[i].objectNum == yellowStar)
						{
							if(! (redcollision[i] && bluecollision[i]))
							{
								star_imageList.Draw(pDCOff, 0, p, ILD_TRANSPARENT);
							}
							else
							{
								if(fallingObjects[i].collisionTime == 0)
								{
									fallingObjects[i].starWasHit = 1;
									fallingObjects[i].isOn = 0;
									stars++;
									fallingObjects[i].collisionTime = currtime;
									fallingObjects[i].top = SCREENHEIGHT;
									InitializeOneFallingObject(i, currtime);
								}
								else if(fallingObjects[i].collisionTime < currtime - 3000)
								{
									fallingObjects[i].starWasHit = 1;
									fallingObjects[i].isOn = 0;
									stars++;
									starsS2++;
									fallingObjects[i].collisionTime = currtime;
									fallingObjects[i].top = SCREENHEIGHT;
								}
							}
						}
						else
						{
							if(! squareOverlapping)
							{
								square_imageList.Draw(pDCOff, fallingObjects[i].objectNum-1, p, ILD_TRANSPARENT);
							}
							else
							{
								square_imageList.Draw(pDCOff, collisionSquare-1, p, ILD_TRANSPARENT);

								if(fallingObjects[i].objectNum > 0)
								{
									if(fallingObjects[i].collisionTime == 0)
									{
										if(collisiondetectedFalling == false)
										{
											collisions++;
											fallingObjects[i].collisionTime = currtime;
											collisiondetectedFalling = true;
										}
									}
									else if(fallingObjects[i].collisionTime < currtime - 3000)
									{
										if(collisiondetectedFalling == false)
										{
											collisions++;
											fallingObjects[i].collisionTime = currtime;
											collisiondetectedFalling = true;
										}
									}
								}
							}
						}
					}
					if(currtime - fallingObjects[i].collisionTime > 3000)
						fallingObjects[i].collisionTime = 0;
				}

				for(int i = 0; i < fallingObjs; i++)
				{
					squareOverlapping = false;
					if(fallingObjects[i].isOn)
					{
						if(overlappingRect.IntersectRect(blueRectReducedS2, fallingObjectsS2[i].blockRect))
						{
							bluecollisionS2[i] = 1;
							squareOverlapping = true;
						}
						if(overlappingRect.IntersectRect(redRectReducedS2, fallingObjectsS2[i].blockRect))
						{
							redcollisionS2[i] = 1;
							squareOverlapping = true;
						}

						p.x = fallingObjectsS2[i].left;
						p.y = fallingObjectsS2[i].top;
						if(fallingObjectsS2[i].objectNum == yellowStar)
						{
							if(! (redcollisionS2[i] && bluecollisionS2[i]))
								if(! fallingObjectsS2[i].starWasHit)
									star_imageList.Draw(pDCOff, 0, p, ILD_TRANSPARENT);
						}
						else
						{
							if(! squareOverlapping)
							{
								square_imageList.Draw(pDCOff, fallingObjectsS2[i].objectNum-1, p, ILD_TRANSPARENT);
							}
							else
							{
								square_imageList.Draw(pDCOff, collisionSquare-1, p, ILD_TRANSPARENT);

								if(fallingObjectsS2[i].objectNum > 0)
								{
									if(fallingObjectsS2[i].collisionTime == 0)
									{
										if(collisiondetectedS2Falling == false)
										{
											collisionsS2++;
											fallingObjectsS2[i].collisionTime = currtime;
											collisiondetectedS2Falling = true;
										}
									}
									else if(fallingObjectsS2[i].collisionTime < currtime - 3000)
									{
										if(collisiondetectedS2Falling == false)
										{
											collisionsS2++;
											fallingObjectsS2[i].collisionTime = currtime;
											collisiondetectedS2Falling = true;
										}
									}
								}
							}
						}
					}
					if(currtime - fallingObjectsS2[i].collisionTime > 3000)
						fallingObjectsS2[i].collisionTime = 0;
				}

				for(int i = 0; i < risingObjs; i++)
				{
					squareOverlapping = false;
					if(risingObjects[i].isOn)
					{
						if(overlappingRect.IntersectRect(blueRectReduced, risingObjects[i].blockRect))
						{
							bluecollision[i] = 1;
							squareOverlapping = true;
						}
						if(overlappingRect.IntersectRect(redRectReduced, risingObjects[i].blockRect))
						{
							redcollision[i] = 1;
							squareOverlapping = true;
						}

						p.x = risingObjects[i].left;
						p.y = risingObjects[i].top;
						if(! squareOverlapping)
						{
							square_imageList.Draw(pDCOff, risingObjects[i].objectNum-1, p, ILD_TRANSPARENT);
						}
						else
						{
							square_imageList.Draw(pDCOff, collisionSquare-1, p, ILD_TRANSPARENT);

							if(risingObjects[i].objectNum > 0)
							{
								if(risingObjects[i].collisionTime == 0)
								{
									if(collisiondetectedRising == false)
									{
										collisions++;
										risingObjects[i].collisionTime = currtime;
										collisiondetectedRising = true;
									}
								}
								else if(risingObjects[i].collisionTime < currtime - 3000)
								{
									if(collisiondetectedRising == false)
									{
										collisions++;
										risingObjects[i].collisionTime = currtime;
										collisiondetectedRising = true;
									}
								}
							}
						}
						if(currtime - risingObjects[i].collisionTime > 3000)
							risingObjects[i].collisionTime = 0;
					}
				}

				for(int i = 0; i < risingObjs; i++)
				{
					squareOverlapping = false;
					if(risingObjectsS2[i].isOn)
					{
						if(overlappingRect.IntersectRect(blueRectReducedS2, risingObjectsS2[i].blockRect))
						{
							bluecollisionS2[i] = 1;
							squareOverlapping = true;
						}
						if(overlappingRect.IntersectRect(redRectReducedS2, risingObjectsS2[i].blockRect))
						{
							redcollisionS2[i] = 1;
							squareOverlapping = true;
						}

						p.x = risingObjectsS2[i].left;
						p.y = risingObjectsS2[i].top;
						if(! squareOverlapping)
						{
							square_imageList.Draw(pDCOff, risingObjectsS2[i].objectNum-1, p, ILD_TRANSPARENT);
						}
						else
						{
							square_imageList.Draw(pDCOff, collisionSquare-1, p, ILD_TRANSPARENT);

							if(risingObjectsS2[i].objectNum > 0)
							{
								if(risingObjectsS2[i].collisionTime == 0)
								{
									if(collisiondetectedS2Rising == false)
									{
										collisionsS2++;
										risingObjectsS2[i].collisionTime = currtime;
										collisiondetectedS2Rising = true;
									}
								}
								else if(risingObjectsS2[i].collisionTime < currtime - 3000)
								{
									if(collisiondetectedS2Rising == false)
									{
										collisionsS2++;
										risingObjectsS2[i].collisionTime = currtime;
										collisiondetectedS2Rising = true;
									}
								}
							}
						}
						if(currtime - risingObjectsS2[i].collisionTime > 3000)
							risingObjectsS2[i].collisionTime = 0;
					}
				}

				if( (avoidEachOtherRun == 1) && (runtype != alone) )
				{
					aeoP.x = (blueBallP.x + BALLSIZE / 2) + ((redBallP.x + BALLSIZE / 2) - (blueBallP.x + BALLSIZE / 2)) / 2;
					aeoP.y = (blueBallP.y + BALLSIZE / 2) + ((redBallP.y+ BALLSIZE / 2) - (blueBallP.y + BALLSIZE / 2)) / 2;
					aeoPS2.x = (blueBallPS2.x + BALLSIZE / 2) + ((redBallPS2.x + BALLSIZE / 2) - (blueBallPS2.x + BALLSIZE / 2)) / 2;
					aeoPS2.y = (blueBallPS2.y + BALLSIZE / 2) + ((redBallPS2.y+ BALLSIZE / 2) - (blueBallPS2.y + BALLSIZE / 2)) / 2;
					
					steigungsWinkel = atan(double(blueBallP.y - redBallP.y) / double(blueBallP.x - redBallP.x));
					avoid1P.x = aeoP.x - (int)(cos(steigungsWinkel) * double(AVOIDLEN / 2));
					avoid1P.y = aeoP.y - (int)(sin(steigungsWinkel) * double(AVOIDLEN / 2));
					avoid2P.x = aeoP.x + (int)(cos(steigungsWinkel) * double(AVOIDLEN / 2));
					avoid2P.y = aeoP.y + (int)(sin(steigungsWinkel) * double(AVOIDLEN / 2));

					steigungsWinkel = atan(double(blueBallPS2.y - redBallPS2.y) / double(blueBallPS2.x - redBallPS2.x));
					avoid1PS2.x = aeoPS2.x - (int)(cos(steigungsWinkel) * double(AVOIDLEN / 2));
					if(avoid1PS2.x < SCREEN_WIDTH)
					{
						int dx = SCREEN_WIDTH - avoid1PS2.x;
						int dy = (int)tan(steigungsWinkel) * dx;
						avoid1PS2.x = SCREEN_WIDTH;
						avoid1PS2.y = aeoPS2.y - (int)(sin(steigungsWinkel) * double(AVOIDLEN / 2)) + dy;
					}
					else
						avoid1PS2.y = aeoPS2.y - (int)(sin(steigungsWinkel) * double(AVOIDLEN / 2));
					avoid2PS2.x = aeoPS2.x + (int)(cos(steigungsWinkel) * double(AVOIDLEN / 2));
					if(avoid2PS2.x < SCREEN_WIDTH)
					{
						//int dx = SCREEN_WIDTH - avoid1PS2.x;
						//int dy = (int)tan(steigungsWinkel) * dx;
						avoid2PS2.x = SCREEN_WIDTH;
						avoid2PS2.y = aeoPS2.y + (int)(sin(steigungsWinkel) * (double)(AVOIDLEN / 2));
//						avoid2PS2.y = aeoPS2.y - (int)(sin(steigungsWinkel) * double(AVOIDLEN / 2)) + dy;
					}
					else
						avoid2PS2.y = aeoPS2.y + (int)(sin(steigungsWinkel) * (double)(AVOIDLEN / 2));
					CPen oldPen, myPen(PS_SOLID, 3, RGB(0, 110, 50));
					pDCOff->SelectObject(&myPen);

					if(runtype == playTogether)
					{
						if(blueRect.PtInRect(avoid1P))
						{
							touchDistance1 += (int)sqrt(pow(abs((blueBallP.x + (BALLSIZE / 2) - avoid1P.x)), 2.0) + pow(abs((blueBallP.y + (BALLSIZE / 2) - avoid1P.y)), 2.0));
							if(currmillis - touchTime1 >= 1000)
							{
								int distance = touchDistance1;
								if(distance <= 80)
								{
									int winPoints = 0;
									winPoints = 80 - distance;
									together += winPoints;
									touchTime1 = currmillis;
								}
								touchDistance1 = 0;
							}
						}
						if(redRect.PtInRect(avoid2P))
						{
							touchDistance2 += (int)sqrt(pow(abs((redBallP.x + (BALLSIZE / 2) - avoid2P.x)), 2.0) + pow(abs((redBallP.y + (BALLSIZE / 2) - avoid2P.y)), 2.0));
							if(currmillis - touchTime2 >= 1000)
							{
								int distance = touchDistance2;
								if(distance <= 80)
								{
									int winPoints = 0;
									winPoints = 80 - distance;
									togetherS2 += winPoints;
									touchTime2 = currmillis;
								}
								touchDistance2 = 0;
							}
						}
					}
					else if(runtype == allPerson1)
					{
						if(blueRect.PtInRect(avoid2P))
						{
							touchDistance1 += (int)sqrt(pow(abs((blueBallP.x + (BALLSIZE / 2) - avoid2P.x)), 2.0) + pow(abs((blueBallP.y + (BALLSIZE / 2) - avoid2P.y)), 2.0));
							if(currmillis - touchTime1 >= 1000)
							{
								int distance = touchDistance1;
								if(distance <= 80)
								{
									int winPoints = 0;
									winPoints = 80 - distance;
									together += winPoints;
									touchTime1 = currmillis;
								}
								touchDistance1 = 0;
							}
						}
						if(redRectS2.PtInRect(avoid2PS2))
						{
							touchDistance2 += (int)sqrt(pow(abs((redBallPS2.x + (BALLSIZE / 2) - avoid2PS2.x)), 2.0) + pow(abs((redBallPS2.y + (BALLSIZE / 2) - avoid2PS2.y)), 2.0));
							if(currmillis - touchTime2 >= 1000)
							{
								int distance = touchDistance2;
								if(distance <= 80)
								{
									int winPoints = 0;
									winPoints = 80 - distance;
									togetherS2 += winPoints;
									touchTime2 = currmillis;
								}
								touchDistance2 = 0;
							}
						}
					}
					else if( (runtype == allPerson2) || (runtype == replayMaster) )
					{
						if(blueRect.PtInRect(avoid1P))
						{
							touchDistance1 += (int)sqrt(pow(abs((blueBallP.x + (BALLSIZE / 2) - avoid1P.x)), 2.0) + pow(abs((blueBallP.y + (BALLSIZE / 2) - avoid1P.y)), 2.0));
							if(currmillis - touchTime1 >= 1000)
							{
								int distance = touchDistance1;
								if(distance <= 80)
								{
									int winPoints = 0;
									winPoints = 80 - distance;
									together += winPoints;
									touchTime1 = currmillis;
								}
								touchDistance1 = 0;
							}
						}
						if(redRectS2.PtInRect(avoid1PS2))
						{
							touchDistance2 += (int)sqrt(pow(abs((redBallPS2.x + (BALLSIZE / 2) - avoid1PS2.x)), 2.0) + pow(abs((redBallPS2.y + (BALLSIZE / 2) - avoid1PS2.y)), 2.0));
							if(currmillis - touchTime2 >= 1000)
							{
								int distance = touchDistance2;
								if(distance <= 80)
								{
									int winPoints = 0;
									winPoints = 80 - distance;
									togetherS2 += winPoints;
									touchTime2 = currmillis;
								}
								touchDistance2 = 0;
							}
						}
					}

					pDCOff->MoveTo(avoid1P);
					pDCOff->LineTo(avoid2P);
					pDCOff->MoveTo(avoid1PS2);
					pDCOff->LineTo(avoid2PS2);
				}
			}

			if( js1.rgbButtons[0] & 0x80 )
			{
				graphicObjects[counter].js1ButtonPressed = 1;
			}
			if( js2.rgbButtons[0] & 0x80 )
			{
				graphicObjects[counter].js2ButtonPressed = 1;
			}

			graphicObjects[counter].centerBlue.x = blueBallP.x + BALLSIZE / 2;
			graphicObjects[counter].centerBlue.y = blueBallP.y + BALLSIZE / 2;
			graphicObjects[counter].centerRed.x = redBallP.x + BALLSIZE / 2;
			graphicObjects[counter].centerRed.y = redBallP.y + BALLSIZE / 2;

			graphicObjects[counter].centerBlueS2.x = blueBallPS2.x + BALLSIZE / 2;
			graphicObjects[counter].centerBlueS2.y = blueBallPS2.y + BALLSIZE / 2;
			graphicObjects[counter].centerRedS2.x = redBallPS2.x + BALLSIZE / 2;
			graphicObjects[counter].centerRedS2.y = redBallPS2.y + BALLSIZE / 2;

			if(fallingObjs)
			{
				for(int i = 0; i < fallingObjs; i++)
				{
					if(fallingObjects[i].isOn)
					{
						graphicObjects[counter].centerP[i].x = fallingObjects[i].center.x;
						graphicObjects[counter].centerP[i].y = fallingObjects[i].center.y;
						graphicObjects[counter].typeObj[i] = fallingObjects[i].objectNum + 1;
						graphicObjects[counter].centerPS2[i].x = fallingObjectsS2[i].center.x;
						graphicObjects[counter].centerPS2[i].y = fallingObjectsS2[i].center.y;
					}
					else
					{
						graphicObjects[counter].centerP[i].x = 0;
						graphicObjects[counter].centerP[i].y = 0;
						graphicObjects[counter].typeObj[i] = 0;
						graphicObjects[counter].centerPS2[i].x = 0;
						graphicObjects[counter].centerPS2[i].y = 0;
					}
				}
			}

			if(risingObjs)
			{
				for(int i = 0; i < risingObjs; i++)
				{
					if(risingObjects[i].isOn)
					{
						graphicObjects[counter].centerPRising[i].x = risingObjects[i].center.x;
						graphicObjects[counter].centerPRising[i].y = risingObjects[i].center.y;
						graphicObjects[counter].typeObjRising[i] = risingObjects[i].objectNum + 1;
						graphicObjects[counter].centerPRisingS2[i].x = risingObjectsS2[i].center.x;
						graphicObjects[counter].centerPRisingS2[i].y = risingObjectsS2[i].center.y;
					}
					else
					{
						graphicObjects[counter].centerPRising[i].x = 0;
						graphicObjects[counter].centerPRising[i].y = 0;
						graphicObjects[counter].typeObjRising[i] = 0;
						graphicObjects[counter].centerPRisingS2[i].x = 0;
						graphicObjects[counter].centerPRisingS2[i].y = 0;
					}
				}
			}
			stopmillis = timeGetTime();
			graphicObjects[counter].millitime = stopmillis - startmillis;

			if(runtype == alone)
			{
				m_imageList.Draw(pDCOff, blueBall, blueBallP, ILD_TRANSPARENT);
				m_imageList.Draw(pDCOff, redBall, redBallPS2, ILD_TRANSPARENT);
			}
			else
			{
				m_imageList.Draw(pDCOff, blueBall, blueBallP, ILD_TRANSPARENT);
				m_imageList.Draw(pDCOff, redBall, redBallP, ILD_TRANSPARENT);
				m_imageList.Draw(pDCOff, blueBall, blueBallPS2, ILD_TRANSPARENT);
				m_imageList.Draw(pDCOff, redBall, redBallPS2, ILD_TRANSPARENT);
			}
		lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
			pDCOff.BlitBack();
			counter++;
			currmillis = stopmillis;
		}

		DoRating(1);

		lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
		lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
		lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
		lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
		lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
		DoRating(2);
		lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
		pDC->FillRect( &rect, &brWhite );

		TCHAR uc_str[256];
		pDC->SetTextColor(RGB(0, 0, 200));
		_stprintf_s(uc_str, L"%d Kollisionen", collisions);
		pDC->TextOutW(300, 100, (LPCTSTR)uc_str, lstrlen((LPCTSTR)uc_str));
		_stprintf_s(uc_str, L"%d Kollisionen", collisionsS2);
		pDC->TextOutW(300 + SCREEN_WIDTH, 100, (LPCTSTR)uc_str, lstrlen((LPCTSTR)uc_str));

		if(fallingObjectsTypes == 2)
		{
//			pDC->SetTextColor(RGB(0, 0, 200));
			_stprintf_s(uc_str, L"%d Sterne", stars);
			pDC->TextOutW(300, 150, (LPCTSTR)uc_str, lstrlen((LPCTSTR)uc_str));
			_stprintf_s(uc_str, L"%d Sterne", starsS2);
			pDC->TextOutW(300 + SCREEN_WIDTH, 150, (LPCTSTR)uc_str, lstrlen((LPCTSTR)uc_str));
		}

//		pDC->SetTextColor(RGB(0, 0, 200));
		_stprintf_s(uc_str, L"%d gemeinsam", together);
		pDC->TextOutW(300, 200, (LPCTSTR)uc_str, lstrlen((LPCTSTR)uc_str));
		_stprintf_s(uc_str, L"%d gemeinsam", togetherS2);
		pDC->TextOutW(300 + SCREEN_WIDTH, 200, (LPCTSTR)uc_str, lstrlen((LPCTSTR)uc_str));
		currmillis = startmillis = timeGetTime();
		lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);

		while(currmillis < startmillis + 2000)
			currmillis = timeGetTime();
		lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
		pDC->FillRect( &rect, &brWhite );

		// write results
		FILE *output = NULL;
		fopen_s(&output, fname, "r");
		if(output)
		{
			fclose(output);
			output = NULL;
			fopen_s(&output, fname, "a");
			fprintf_s(output, "\n");
		}
		else
		{
			fopen_s(&output, fname, "w");
		}
		if(! output)
		{
			AfxMessageBox(L"ERRRROR: Fehler beim schreiben der Experimentdaten!!! Experimentdaten sind verloren!!! Programm, wird beendet!!!");
			exit(0);
		}
		fprintf(output, "ms\trun\truntype\truntypeString\tavoidEachOther\tfallingObjects\trisingObjects\tminutes\tblueRating1\tredRating1\tblueRating2\tredRating2\ttrigger\ttogether\tstars\tcollisions\ttogetherS2\tstarsS2\tcollisionsS2\tjs1ButtonPressed\tjs2ButtonPressed\tblueBall_X\tblueBall_Y\tredBall_X\tredBall_Y\tdistanceRedBlue\tblueBallS2_X\tblueBallS2_Y\tredBallS2_X\tredBallS2_Y\tdistanceRedBlueS2");
		for(int i = 0; i < MAXFALLINGOBJECTS; i++)
		{
			fprintf(output, "\tf_obj%dX\tf_obj%dY\tf_obj%dType\tf_obj%dBlueDistance\tf_obj%dRedDistance\tf_obj%dXS2\tf_obj%dYS2\tf_obj%dTypeS2\tf_obj%dBlueDistanceS2\tf_obj%dRedDistanceS2", i+1, i+1, i+1, i+1, i+1, i+1, i+1, i+1, i+1, i+1);
		}
		for(int i = 0; i < MAXRISINGOBJECTS; i++)
		{
			fprintf(output, "\tr_obj%dX\tr_obj%dY\tr_obj%dType\tr_obj%dBlueDistance\tr_obj%dRedDistance\tr_obj%dXS2\tr_obj%dYS2\tr_obj%dTypeS2\tr_obj%dBlueDistanceS2\tr_obj%dRedDistanceS2", i+1, i+1, i+1, i+1, i+1, i+1, i+1, i+1, i+1, i+1);
		}
		fprintf(output, "\n");

		for(int i = 0; i < counter; i++)
		{
			graphicObjects[i].blueRedDistance = (int)sqrt(pow((graphicObjects[i].centerBlue.x - graphicObjects[i].centerRed.x), 2.0) + pow((graphicObjects[i].centerBlue.y - graphicObjects[i].centerRed.y), 2.0));
			graphicObjects[i].blueRedDistanceS2 = (int)sqrt(pow((graphicObjects[i].centerBlueS2.x - graphicObjects[i].centerRedS2.x), 2.0) + pow((graphicObjects[i].centerBlueS2.y - graphicObjects[i].centerRedS2.y), 2.0));
			for(int j = 0; j < fallingObjs; j++)
			{
				if(graphicObjects[i].centerP[j].x == 0)
				{
					graphicObjects[i].distanceBlue[j] = 0;
					graphicObjects[i].distanceRed[j] = 0;
					graphicObjects[i].distanceBlueS2[j] = 0;
					graphicObjects[i].distanceRedS2[j] = 0;
				}
				else
				{
					graphicObjects[i].distanceBlue[j] = (int)sqrt(pow((graphicObjects[i].centerBlue.x - graphicObjects[i].centerP[j].x), 2.0) + pow((graphicObjects[i].centerBlue.y - graphicObjects[i].centerP[j].y), 2.0));
					graphicObjects[i].distanceRed[j] = (int)sqrt(pow((graphicObjects[i].centerRed.x - graphicObjects[i].centerP[j].x), 2.0) + pow((graphicObjects[i].centerBlue.y - graphicObjects[i].centerP[j].y), 2.0));
					graphicObjects[i].distanceBlueS2[j] = (int)sqrt(pow((graphicObjects[i].centerBlueS2.x - graphicObjects[i].centerPS2[j].x), 2.0) + pow((graphicObjects[i].centerBlueS2.y - graphicObjects[i].centerPS2[j].y), 2.0));
					graphicObjects[i].distanceRedS2[j] = (int)sqrt(pow((graphicObjects[i].centerRedS2.x - graphicObjects[i].centerPS2[j].x), 2.0) + pow((graphicObjects[i].centerBlueS2.y - graphicObjects[i].centerPS2[j].y), 2.0));
				}
			}
			for(int j = 0; j < risingObjs; j++)
			{
				if(graphicObjects[i].centerPRising[j].x == 0)
				{
					graphicObjects[i].distanceBlueRising[j] = 0;
					graphicObjects[i].distanceBlueRising[j] = 0;
					graphicObjects[i].distanceBlueRisingS2[j] = 0;
					graphicObjects[i].distanceRedRisingS2[j] = 0;
				}
				else
				{
					graphicObjects[i].distanceBlueRising[j] = (int)sqrt(pow((graphicObjects[i].centerBlue.x - graphicObjects[i].centerPRising[j].x), 2.0) + pow((graphicObjects[i].centerBlue.y - graphicObjects[i].centerPRising[j].y), 2.0));
					graphicObjects[i].distanceBlueRising[j] = (int)sqrt(pow((graphicObjects[i].centerRed.x - graphicObjects[i].centerPRising[j].x), 2.0) + pow((graphicObjects[i].centerBlue.y - graphicObjects[i].centerPRising[j].y), 2.0));
					graphicObjects[i].distanceBlueRisingS2[j] = (int)sqrt(pow((graphicObjects[i].centerBlueS2.x - graphicObjects[i].centerPRisingS2[j].x), 2.0) + pow((graphicObjects[i].centerBlueS2.y - graphicObjects[i].centerPRisingS2[j].y), 2.0));
					graphicObjects[i].distanceRedRisingS2[j] = (int)sqrt(pow((graphicObjects[i].centerRedS2.x - graphicObjects[i].centerPRisingS2[j].x), 2.0) + pow((graphicObjects[i].centerBlueS2.y - graphicObjects[i].centerPRisingS2[j].y), 2.0));
				}
			}
			fprintf(output, "%ld\t%d\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d", 
				graphicObjects[i].millitime, run, runtype, trials[run].typeStr, avoidEachOtherRun, fallingObjs, risingObjs, 100, blueRating1, redRating1, blueRating2, redRating2, 
				graphicObjects[i].trigger, together, stars, collisions, togetherS2, starsS2, collisionsS2,
				graphicObjects[i].js1ButtonPressed, graphicObjects[i].js2ButtonPressed,
				graphicObjects[i].centerBlue.x, graphicObjects[i].centerBlue.y, graphicObjects[i].centerRed.x, graphicObjects[i].centerRed.y, graphicObjects[i].blueRedDistance,
				graphicObjects[i].centerBlueS2.x, graphicObjects[i].centerBlueS2.y, graphicObjects[i].centerRedS2.x, graphicObjects[i].centerRedS2.y, graphicObjects[i].blueRedDistanceS2);
			for(int j = 0; j < MAXFALLINGOBJECTS; j++)
			{
				fprintf(output, "\t%d\t%d\t%d\t%d\t%d",
					graphicObjects[i].centerP[j].x, graphicObjects[i].centerP[j].y, graphicObjects[i].typeObj[j], graphicObjects[i].distanceBlue[j], graphicObjects[i].distanceRed[j]);
				fprintf(output, "\t%d\t%d\t%d\t%d\t%d",
					graphicObjects[i].centerPS2[j].x, graphicObjects[i].centerPS2[j].y, graphicObjects[i].typeObj[j], graphicObjects[i].distanceBlueS2[j], graphicObjects[i].distanceRedS2[j]);
			}
			for(int j = 0; j < MAXRISINGOBJECTS; j++)
			{
				fprintf(output, "\t%d\t%d\t%d\t%d\t%d",
					graphicObjects[i].centerPRising[j].x, graphicObjects[i].centerPRising[j].y, graphicObjects[i].typeObjRising[j], graphicObjects[i].distanceBlueRising[j], graphicObjects[i].distanceRedRising[j]);
				fprintf(output, "\t%d\t%d\t%d\t%d\t%d",
					graphicObjects[i].centerPRisingS2[j].x, graphicObjects[i].centerPRisingS2[j].y, graphicObjects[i].typeObjRising[j], graphicObjects[i].distanceBlueRisingS2[j], graphicObjects[i].distanceRedRisingS2[j]);
			}

			fprintf(output, "\n");
		}
		fclose(output);
		output = NULL;
	}

	ShowCursor(TRUE);
	sprintf_s(dialog1.runNumStr, "Das Expriment ist jetzt beendet.");
	if(dialog1.DoModal() != IDOK)
	{
		exit(0);
	}

	FullScreenHandler.Restore(this->GetParentFrame());
	::ReleaseDC( NULL, hDC );
	exit(0);
}


void CStarshipTroopersView::DoRating(int which)
{
	HRESULT hr;
	int leftRatingRects[10];
	if(which == 1)
	{
		for(int i = 0; i < 10; i++)
			leftRatingRects[i] = centerP.x - 400 - 4 + (int)((float)i * 800.0 / 9.0);
	}
	else if(which == 2)
	{
		for(int i = 0; i < 9; i++)
			leftRatingRects[i] = centerP.x - 400 - 4 + (int)((float)i * 800.0 / 8.0);
	}

	TCHAR uc_str[256], uc_str2[256], uc_str3[256], uc_str4[256];
	bool done = false, done1 = false, done2 = false;
	if(runtype != alone)
	{
		blueRating1 = redRating1 = blueRating2 = redRating2 = 1;
		millis1 = 0L, millis2 = 0L;
		currmillis = startmillis = timeGetTime();
		loop = 0;
		CPen oldPen, myPen(PS_SOLID, 3, RGB(0, 20, 110));

		HDC hDC2 = NULL;
		hDC2 =	::GetDC( NULL );
		CDC *pDCx2 = NULL;
		pDCx2 = CDC::FromHandle(hDC2);
		
		while(! done)
		{
			currmillis = timeGetTime();

			hr = joystick1->Poll(); 
			hr = joystick1->Acquire();
			if (FAILED(hr = joystick1->GetDeviceState(sizeof(DIJOYSTATE2), &js1)))
			{
				ShowCursor(TRUE);
				AfxMessageBox(L"ERRRRROR: Something wrong with Joystick1!\nProgramm wird beendet!");
				exit(0);
			}

			hr = joystick2->Poll(); 
			hr = joystick2->Acquire();
			if (FAILED(hr = joystick2->GetDeviceState(sizeof(DIJOYSTATE2), &js2)))
			{
				ShowCursor(TRUE);
				AfxMessageBox(L"ERRRRROR: Something wrong with Joystick2!\nProgramm wird beendet!");
				exit(0);
			}

			if( js1.rgbButtons[0] & 0x80 )
			{
				done1 = true;
			}
			if( js2.rgbButtons[0] & 0x80 )
			{
				done2 = true;
			}
			ReadBlueJoystickRating(which);
			ReadRedJoystickRating(which);
			if( js1.rgbButtons[0] & 0x80 )
			{
				done1 = true;
			}
			if( js2.rgbButtons[0] & 0x80 )
			{
				done2 = true;
			}

			if(loop > 9916)
				TRACE("loop: %d\n", loop);

			CMemDC pDCOff2(pDCx2);
			pDCOff2->FillRect( &rect, &brWhite );

			CPoint penPos1, penPos2, penPos1S2, penPos2S2;
			pDCOff2->SelectObject(&myPen);
			oldfont = pDCOff2->SelectObject(&newFont);

			if(which == 1)
			{
				_stprintf_s(uc_str, L"1. Wie hat das Zusammenspiel funktioniert?");
				_stprintf_s(uc_str2, L"sehr schlecht");
				_stprintf_s(uc_str3, L"sehr gut");
			}
			else
			{
				_stprintf_s(uc_str, L"2. Wer hat den Spielverlauf überwiegend gestaltet?");
				_stprintf_s(uc_str2, L"ich");
				_stprintf_s(uc_str3, L"mein Mitspieler");
				_stprintf_s(uc_str4, L"gemeinsam");
			}

			if(! done1)
			{

				penPos1.x = centerP.x - 400;
				penPos1.y = penPos2.y = centerP.y + 20;
				penPos2.x = centerP.x + 400;

				if(which == 1)
				{
					pDCOff2->TextOutW(centerP.x - 250, centerP.y - 200, (LPCTSTR)uc_str, lstrlen((LPCTSTR)uc_str));
					pDCOff2->TextOutW(penPos1.x - 90, penPos1.y + 40, (LPCTSTR)uc_str2, lstrlen((LPCTSTR)uc_str2));
					pDCOff2->TextOutW(penPos2.x - 44, penPos2.y + 40, (LPCTSTR)uc_str3, lstrlen((LPCTSTR)uc_str3));
				}
				else if(which == 2)
				{
					pDCOff2->TextOutW(centerP.x - 306, centerP.y - 200, (LPCTSTR)uc_str, lstrlen((LPCTSTR)uc_str));
					pDCOff2->TextOutW(penPos1.x - 20, penPos1.y + 40, (LPCTSTR)uc_str2, lstrlen((LPCTSTR)uc_str2));
					pDCOff2->TextOutW(penPos2.x - 90, penPos2.y + 40, (LPCTSTR)uc_str3, lstrlen((LPCTSTR)uc_str3));
					pDCOff2->TextOutW(centerP.x - 66, penPos2.y + 40, (LPCTSTR)uc_str4, lstrlen((LPCTSTR)uc_str4));
				}

				pDCOff2->MoveTo(penPos1);
				pDCOff2->LineTo(penPos2);

				penPos2.x = penPos1.x;
				penPos1.y = penPos1.y - 12;
				penPos2.y = penPos2.y + 12;
				if(which == 1)
				{
					for(int i = 0; i < 11; i++)
					{
						pDCOff2->MoveTo(penPos1);
						pDCOff2->LineTo(penPos2);
						penPos1.x = centerP.x - 400 + (int)((float)i * 800.0 / 9.0);
						penPos2.x = penPos1.x;
					}
				}
				else if(which == 2)
				{
					for(int i = 0; i < 10; i++)
					{
						pDCOff2->MoveTo(penPos1);
						pDCOff2->LineTo(penPos2);
						penPos1.x = centerP.x - 400 + (int)((float)i * 800.0 / 8.0);
						penPos2.x = penPos1.x;
					}
				}
				CRect blueRatingRect;
				if(which == 1)
					blueRatingRect.left = leftRatingRects[blueRating1-1];
				else if(which == 2)
					blueRatingRect.left = leftRatingRects[blueRating2-1];
				blueRatingRect.right = blueRatingRect.left + 8;
				blueRatingRect.top = penPos1.y - 8;
				blueRatingRect.bottom = penPos2.y + 8;
				pDCOff2->FillRect(blueRatingRect, &brRed);
				pDCOff2->FrameRect(blueRatingRect, &brBlack);
			}

			if(! done2)
			{
				penPos1S2.x = centerPS2.x - 400;
				penPos1S2.y = penPos2S2.y = centerPS2.y + 20;
				penPos2S2.x = centerPS2.x + 400;

				if(which == 1)
				{
					pDCOff2->TextOutW(centerPS2.x - 250, centerPS2.y - 200, (LPCTSTR)uc_str, lstrlen((LPCTSTR)uc_str));
					pDCOff2->TextOutW(penPos1S2.x - 90, penPos1S2.y + 40, (LPCTSTR)uc_str2, lstrlen((LPCTSTR)uc_str2));
					pDCOff2->TextOutW(penPos2S2.x - 44, penPos2S2.y + 40, (LPCTSTR)uc_str3, lstrlen((LPCTSTR)uc_str3));
				}
				else if(which == 2)
				{
					pDCOff2->TextOutW(centerPS2.x - 306, centerPS2.y - 200, (LPCTSTR)uc_str, lstrlen((LPCTSTR)uc_str));
					pDCOff2->TextOutW(penPos1S2.x - 20, penPos1S2.y + 40, (LPCTSTR)uc_str2, lstrlen((LPCTSTR)uc_str2));
					pDCOff2->TextOutW(penPos2S2.x - 90, penPos2S2.y + 40, (LPCTSTR)uc_str3, lstrlen((LPCTSTR)uc_str3));
					pDCOff2->TextOutW(centerPS2.x - 66, penPos2S2.y + 40, (LPCTSTR)uc_str4, lstrlen((LPCTSTR)uc_str4));
				}

				pDCOff2->MoveTo(penPos1S2);
				pDCOff2->LineTo(penPos2S2);

				penPos2S2.x = penPos1S2.x;
				penPos1S2.y = penPos1S2.y - 12;
				penPos2S2.y = penPos2S2.y + 12;
				if(which == 1)
				{
					for(int i = 0; i < 11; i++)
					{
						pDCOff2->MoveTo(penPos1S2);
						pDCOff2->LineTo(penPos2S2);
						penPos1S2.x = centerPS2.x - 400 + (int)((float)i * 800.0 / 9.0);
						penPos2S2.x = penPos1S2.x;
					}
				}
				else if(which == 2)
				{
					for(int i = 0; i < 10; i++)
					{
						pDCOff2->MoveTo(penPos1S2);
						pDCOff2->LineTo(penPos2S2);
						penPos1S2.x = centerPS2.x - 400 + (int)((float)i * 800.0 / 8.0);
						penPos2S2.x = penPos1S2.x;
					}
				}
				CRect redRatingRect;
				if(which == 1)
					redRatingRect.left = leftRatingRects[redRating1-1] + SCREEN_WIDTH;
				else if(which == 2)
					redRatingRect.left = leftRatingRects[redRating2-1] + SCREEN_WIDTH;
				redRatingRect.right = redRatingRect.left + 8;
				redRatingRect.top = penPos1S2.y - 8;
				redRatingRect.bottom = penPos2S2.y + 8;
				pDCOff2->FillRect(redRatingRect, &brRed);
				pDCOff2->FrameRect(redRatingRect, &brBlack);
			}

			lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
//				pDCOff2.BlitBack();
			if(done1 && done2)
				break;
			loop++;
		}
	}

	if(which == 1)
	{
		lpDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
		HDC hDC2 = NULL;
		hDC2 =	::GetDC( NULL );
		CDC *pDCx2 = NULL;
		pDCx2 = CDC::FromHandle(hDC2);
		CMemDC pDCOff2(pDCx2);
		pDCOff2->FillRect( &rect, &brWhite );
	}

	millis1 = 0L, millis2 = 0L;
	done1 = false; done2 = false;
	while(1)
	{
		currmillis = timeGetTime();

		hr = joystick1->Poll(); 
		hr = joystick1->Acquire();
		if (FAILED(hr = joystick1->GetDeviceState(sizeof(DIJOYSTATE2), &js1)))
		{
			ShowCursor(TRUE);
			AfxMessageBox(L"ERRRRROR: Something wrong with Joystick1!\nProgramm wird beendet!");
			exit(0);
		}

		hr = joystick2->Poll(); 
		hr = joystick2->Acquire();
		if (FAILED(hr = joystick2->GetDeviceState(sizeof(DIJOYSTATE2), &js2)))
		{
			ShowCursor(TRUE);
			AfxMessageBox(L"ERRRRROR: Something wrong with Joystick2!\nProgramm wird beendet!");
			exit(0);
		}
		ReadBlueJoystickRating(which);
		ReadRedJoystickRating(which);
		if(! (js1.rgbButtons[0] & 0x80) )
		{
			done1 = true;
		}
		if(! (js2.rgbButtons[0] & 0x80) )
		{
			done2 = true;
		}
		if(done1 && done2)
			break;
	}

// Wer hat den Spielverlauf überwiegend gestaltet? ich 1,  gemeinsam 5, mein Mitspieler 9
}


void CStarshipTroopersView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
}
