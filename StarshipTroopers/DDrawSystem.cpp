// DDrawSystem.cpp: implementation of the CDDrawSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Winuser.h>
#include <windows.h>
#include <mmsystem.h>
#include <dinput.h>
#include "conio.h"
#include "stdio.h"
#include "DDrawSystem.h"
#include <dinput.h>
#include <afxwin.h>
#include "Resource.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif




#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 1024

LPDIRECTINPUT8 di;
LPDIRECTINPUTDEVICE8 joystick1, joystick2;
int joycounter;


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


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CDDrawSystem::CDDrawSystem()
{
	m_pDD = NULL;
	m_pddsFrontBuffer = NULL;
	m_pddsStoreBuffer = NULL;
	pcClipper = NULL;

}


CDDrawSystem::~CDDrawSystem()
{
	blueBallBm.DeleteObject();
	redBallBm.DeleteObject();
	m_blueBall.Empty();
	m_redBall.Empty();

	if (joystick1)
	{ 
    joystick1->Unacquire();
	}
	if (joystick2)
	{ 
    joystick2->Unacquire();
	}
	Terminate();
}


// old DirectDraw Initialization stuff. Set a window mode DirectDraw Display.
BOOL CDDrawSystem::Init(HWND hWnd)
{
	HRESULT hRet;

	this->hWnd = hWnd;

	blueBall.Load(_T("images\\blaueKugel.png"));
	blueBallBm.Attach(blueBall.Detach());
	redBall.Load(_T("images\\roteKugel.png"));
	redBallBm.Attach(redBall.Detach());

	m_blueBall.Load(IDB_BLUEBALL, _T("PNG"));
	m_redBall.Load(IDB_REDBALL, _T("PNG"));

//	ShowCursor(FALSE);
	GetClientRect(hWnd, rcClient);
	centerP.x = rcClient.Width()/2;
	centerP.y  = rcClient.Height()/2;

	hRet = DirectDrawCreateEx(NULL, (VOID**)&m_pDD, IID_IDirectDraw7, NULL);
	if(hRet != DD_OK)
	{
		AfxMessageBox(L"Failed to create directdraw object.");
		return FALSE;
	}

	hRet = m_pDD->SetCooperativeLevel(hWnd, DDSCL_NORMAL);
	if(hRet != DD_OK)
	{
		AfxMessageBox(L"Failed to set directdraw display behavior.");
		return FALSE;
	}
	HRESULT hr;

	DDSURFACEDESC2 ddsd;
  ZeroMemory( &ddsd, sizeof( ddsd ) );
  ddsd.dwSize         = sizeof( ddsd );
  ddsd.dwFlags        = DDSD_CAPS;
  ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	if(FAILED(hr = m_pDD->CreateSurface(&ddsd, &m_pddsFrontBuffer, NULL)))
	{
		AfxMessageBox(L"Failed to create primary surface.");
		return FALSE;		
	}


  // Create the backbuffer surface
  ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;    
  ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
  ddsd.dwWidth        = SCREEN_WIDTH;
  ddsd.dwHeight       = SCREEN_HEIGHT;

	if(FAILED(hr = m_pDD->CreateSurface(&ddsd, &m_pddsStoreBuffer, NULL)))
	{
		AfxMessageBox(L"Failed to create back buffer surface.");
		return FALSE;		
	}

	if(FAILED(hr = m_pDD->CreateClipper(0, &pcClipper, NULL)))
	{
		AfxMessageBox(L"Failed to create clipper.");
		return FALSE;		
	}

	if(FAILED(hr = pcClipper->SetHWnd(0, hWnd)))
	{
		pcClipper->Release();
		AfxMessageBox(L"Failed to create primary surface.");
		return FALSE;		
	}

	if(FAILED(hr = m_pddsFrontBuffer->SetClipper(pcClipper)))
	{
		pcClipper->Release();
		AfxMessageBox(L"Failed to create primary surface.");
		return FALSE;
	}


	// #################     Create a DirectInput device#################
	if (FAILED(hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&di, NULL)))
	{
		AfxMessageBox(L"ERRRRROR: DirectX not found!\nProgramm wird beendet!");
		exit(0);
	}

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
	if (FAILED(hr = joystick1->SetDataFormat(&c_dfDIJoystick2)))
	{
		AfxMessageBox(L"ERRRRROR: Joystick2 SetDataFormat!\nProgramm wird beendet!");
		exit(0);
	}
/*
	if( FAILED( hr = joystick2->SetCooperativeLevel(m_hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND ) ) )
	{
		MessageBox(L"ERRRRROR: Joystick2 SetCooperativeLevel!\nProgramm wird beendet!", NULL, MB_OK);
		exit(0);
	}
*/
	if (FAILED(hr = joystick2->GetCapabilities(&capabilities2))) {
		AfxMessageBox(L"ERRRRROR: Joystick2 GetCapabilities!\nProgramm wird beendet!");
		exit(0);
	}
	if (FAILED(hr = joystick2->SetDataFormat(&c_dfDIJoystick2)))
	{
		AfxMessageBox(L"ERRRRROR: Joystick2 SetDataFormat!\nProgramm wird beendet!");
		exit(0);
	}


	if( FAILED( hr = joystick1->EnumObjects( EnumObjectsCallback, (VOID*)hWnd, DIDFT_ALL ) ) )
	{
		AfxMessageBox(L"ERRRRROR: Joystick1 EnumObjectsCallback!\nProgramm wird beendet!");
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

	return TRUE;
}


// make sure all stuff are terminated. and set to NULL when application ends.
void CDDrawSystem::Terminate()
{
	if (m_pDD != NULL)
	{
		if (m_pddsFrontBuffer != NULL)
		{
			if (m_pddsStoreBuffer != NULL)
			{
				m_pddsStoreBuffer->Release();
				m_pddsStoreBuffer = NULL;
			}

			if (pcClipper != NULL)
			{
				pcClipper->Release();
				pcClipper = NULL;
			}

			m_pddsFrontBuffer->Release();
			m_pddsFrontBuffer = NULL;
		}
		m_pDD->Release();
		m_pDD = NULL;
	}
}


// clear both off csreen buffer and primary buffer.
void CDDrawSystem::Clear()
{
	HRESULT hRet;
	DDBLTFX fx;
	fx.dwSize = sizeof(fx);
	fx.dwFillColor = 0x000000;
	fx.dwFillColor = 0xDDDDDD;

	while (1)
	{
		hRet = m_pddsFrontBuffer->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &fx);
		if (hRet == DD_OK)
			break;
		else if (hRet == DDERR_SURFACELOST)
		{
			m_pddsFrontBuffer->Restore();
		}
		else if (hRet != DDERR_WASSTILLDRAWING)
			break;
	}

	while (1)
	{
		hRet = m_pddsStoreBuffer->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &fx);
		if (hRet == DD_OK)
			break;
		else if (hRet == DDERR_SURFACELOST)
		{
			m_pddsStoreBuffer->Restore();
		}
		else if (hRet != DDERR_WASSTILLDRAWING)
			break;
	}
}

// Load images from offscteen buffer to primary buffer and for display.
void CDDrawSystem::Display()
{
	HRESULT hRet;

	RECT rt;
	POINT p = {0, 0};
	ClientToScreen(hWnd, &p);
	rt.left = 0 + p.x; rt.top = 0 + p.y; rt.right = 800 + p.x; rt.bottom = 600 + p.y;

	m_pDD->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
	while( 1 )
	{
		hRet = m_pddsFrontBuffer->Blt(&rt, m_pddsStoreBuffer, NULL, DDBLT_WAIT, NULL);
		if (hRet == DD_OK)
		break;
		else if(hRet == DDERR_SURFACELOST)
		{
			m_pddsFrontBuffer->Restore();
			m_pddsStoreBuffer->Restore();
		}
		else if(hRet != DDERR_WASSTILLDRAWING)
			return;
	}
}


// a test:
// The conclusion is: Under no circumstance, draw directly to primary Surface!
// doesn't work that way.
// ...
// ...
// This is just a simple test function. It has shit use in this project.
void CDDrawSystem::TestDraw(void)
{
	HRESULT hRet;
/*  #################     original     #################
	HRESULT hRet;
	HDC dc;
	hRet = m_pddsStoreBuffer->GetDC(&dc);
	if (hRet != DD_OK)
		return;

	POINT p = {0 + x, 0 + y};
	ClientToScreen(hWnd, &p);

	SetTextColor(dc, RGB(255, 0, 0));
	TextOut(dc, 20, 20, "This is a stinky App", lstrlen("This is a stinky App"));

	Ellipse(dc, x-50, y-50, x+50,y+50);

	m_pddsStoreBuffer->ReleaseDC(dc);
*/
	HDC dc;
	hRet = m_pddsStoreBuffer->GetDC(&dc);
	if (hRet != DD_OK)
		return;
	CDC* pDC = CDC::FromHandle(dc);

	POINT p = {0 + 222, 0 + 222};
	CDC dcmem;
	dcmem.CreateCompatibleDC(NULL);
	CBitmap* pbmOld;
	pbmOld = dcmem.SelectObject(&blueBallBm);
	ClientToScreen(hWnd, &p);
	pDC->BitBlt(blueBallP.x, blueBallP.y, 100, 100, &dcmem, 0, 0, SRCCOPY);
//	SetTextColor(dc, RGB(255, 0, 0));
//	TextOut(dc, 20, 20, L"This is a stinky App", lstrlen(L"This is a stinky App"));

	//Ellipse(dc, 222-50, 222-50, 222+50,222+50);

	m_pddsStoreBuffer->ReleaseDC(dc);
}



void CDDrawSystem::RunExperiment()
{
	HRESULT hr;

	Clear();
	blueBallP.x = centerP.x - 200;
	blueBallP.y = centerP.y - 50;
	redBallP.x = centerP.x + 200;
	redBallP.y = centerP.y - 50;

	hr = joystick1->Poll(); 
  hr = joystick2->Poll(); 

	
	FILE *outf = NULL;
	fopen_s(&outf, "mist.txt", "w");
	fprintf(outf, "ms\t\t1_lX\t1_lY\t1_lZ\t2_lX\t2_lY\t2_lZ\t\t1_lRx\t1_lRy\t1_lRz\t2_lRx\t2_lRy\t2_lRz\t\t1_lVx\t1_lVY\t1_lVZ\t2_lVX\t2_lVY\t2_lVZ\t\t1_lVRx\t1_lVRy\t1_lVRz\t2_lVRx\t2_lVRy\t2_lVRz\t\t1_lAX\t1_lAY\t1_lAZ\t2_lAX\t2_llAY\t2_lAZ\t\t1_lARx\t1_lARy\t1_lARz\t2_lARx\t2_lARy\t2_lARz\n");

	DIJOYSTATE2 js1, js2;
	unsigned long startmillis, currmillis, stopmillis;
	currmillis = startmillis = stopmillis = timeGetTime();
	while(stopmillis < startmillis + 4000)
	{
		stopmillis = timeGetTime();

		hr = joystick1->Poll(); 
		hr = joystick1->Acquire();
		if (FAILED(hr = joystick1->GetDeviceState(sizeof(DIJOYSTATE2), &js1)))
		{
			AfxMessageBox(L"ERRRRROR: Something wrong with Joystick1!\nProgramm wird beendet!");
			exit(0);
		}

		hr = joystick2->Poll(); 
		hr = joystick2->Acquire();
		if (FAILED(hr = joystick2->GetDeviceState(sizeof(DIJOYSTATE2), &js2)))
		{
			AfxMessageBox(L"ERRRRROR: Something wrong with Joystick2!\nProgramm wird beendet!");
			exit(0);
		}

		if(currmillis != stopmillis)
		{
			//Clear();
			TestDraw();
			Display();
			blueBallP.x += 1;
			blueBallP.y -= 1;

			fprintf(outf, "%d\t\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\t\t%ld\t%ld\t%ld\t%ld\t%ld\t%ld\n", 
				stopmillis - startmillis, 
				js1.lX, js1.lY, js1.lZ, js2.lX, js2.lY, js2.lZ,
				js1.lVRx, js1.lVRy, js1.lVRz, js2.lVRx, js2.lVRy, js2.lVRz,

				js1.lVX, js1.lVY, js1.lVZ, js2.lVX, js2.lVY, js2.lVZ,
				js1.lVRx, js1.lVRy, js1.lVRz, js2.lVRx, js2.lVRy, js2.lVRz,

				js1.lAX, js1.lAY, js1.lAZ, js2.lAX, js2.lAY, js2.lAZ,
				js1.lARx, js1.lARy, js1.lARz, js2.lARx, js2.lARy, js2.lARz
				);
			currmillis = stopmillis;
		}
	}
	fclose(outf);
//	exit(0);
}



void CDDrawSystem::DrawBall(int whichBall)
{
}


BOOL CDDrawSystem::ChangeDisplayMode(BOOL bFullScreen, int nWidth, int nHeight)
{
	return FALSE;
}
