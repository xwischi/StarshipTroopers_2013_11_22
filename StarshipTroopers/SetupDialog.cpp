// SetupDialog.cpp : implementation file
//

#include "stdafx.h"
#include "StarshipTroopers.h"
#include "SetupDialog.h"


// SetupDialog dialog

IMPLEMENT_DYNAMIC(SetupDialog, CDialog)

SetupDialog::SetupDialog(CWnd* pParent /*=NULL*/)
	: CDialog(SetupDialog::IDD, pParent)
	, fallingObjectsTypes(0)
	, fallingObjs(10)
{
	replay = 0;
	fallingObjectsTypes = 0;
	fallingObjs = 0;
	risingObjs = 0;
	avoidEachOther = 0;
}

SetupDialog::~SetupDialog()
{
}


void SetupDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
//	DDX_Control(pDX, IDC_STAND_ALONE, standAloneButton);
	DDX_Control(pDX, IDC_AVOIDEACHOTHER, avoidEachOtherButton);
}


BEGIN_MESSAGE_MAP(SetupDialog, CDialog)
	ON_BN_CLICKED(IDOK, &SetupDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RADIO1, &SetupDialog::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &SetupDialog::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &SetupDialog::OnBnClickedRadio3)
END_MESSAGE_MAP()


// SetupDialog message handlers
void SetupDialog::OnBnClickedOk()
{
	LPTSTR bla[256];
	char tempstr[256];
	bool done = true;

	GetDlgItemText(IDC_EDIT1, (LPTSTR)&bla, 256);
	size_t origsize = wcslen((wchar_t *)bla) + 1;
	size_t convertedChars = 0;
	wcstombs_s(&convertedChars, vpid, origsize, (wchar_t *)&bla, _TRUNCATE);
	if( strlen(vpid) < 4)
	{
		MessageBox(L"Bitte geben Sie die VP-ID korrekt ein!", NULL, MB_OK);
		done = false;
		return;
	}

//	replay = standAloneButton.GetCheck();
	avoidEachOther = avoidEachOtherButton.GetCheck();

	if(fallingObjectsTypes == 0)
	{
		MessageBox(L"W‰hlen Sie bitte ein Experiment aus.", NULL, MB_OK);
		done = false;
		return;
	}
	if(! done)
		return;

	int tempInt;
	if(fallingObjectsTypes > 1)
	{
		GetDlgItemText(IDC_NUMOBJS, (LPTSTR)&bla, 256);
		origsize = wcslen((wchar_t *)bla) + 1;
		wcstombs_s(&convertedChars, tempstr, origsize, (wchar_t *)&bla, _TRUNCATE);
		if( strlen(tempstr) < 1)
		{
			MessageBox(L"Bitte geben Sie die Anzahl der fallenden Objekte ein!", NULL, MB_OK);
			done = false;
			return;
		}
		tempInt = (int)atoi(tempstr);
		if( (tempInt < MINFALLINGOBJECTS) || (tempInt > MAXFALLINGOBJECTS) )
		{
			MessageBox(L"Anzahl der fallenden Objekte muﬂ zwischen 4 und 15 liegen!", NULL, MB_OK);
			done = false;
			return;
		}
		else
			fallingObjs = tempInt;
		if(! done)
			return;

		GetDlgItemText(IDC_NUMOBJS2, (LPTSTR)&bla, 256);
		origsize = wcslen((wchar_t *)bla) + 1;
		wcstombs_s(&convertedChars, tempstr, origsize, (wchar_t *)&bla, _TRUNCATE);
		if( strlen(tempstr) < 1)
		{
			MessageBox(L"Bitte geben Sie die Anzahl der steigenden Objekte ein!", NULL, MB_OK);
			done = false;
			return;
		}
		else
		{
			tempInt = (int)atoi(tempstr);
			if( (tempInt < 0) || (tempInt > MAXRISINGOBJECTS) )
			{
				MessageBox(L"Anzahl der steigenden Objekte muﬂ zwischen 0 und 10 liegen!", NULL, MB_OK);
				done = false;
				return;
			}
			else
				risingObjs = tempInt;
		}
		if(! done)
			return;
}


	if(done)
		OnOK();
}


void SetupDialog::OnBnClickedRadio1()
{
	fallingObjectsTypes = 1;
}


void SetupDialog::OnBnClickedRadio2()
{
	fallingObjectsTypes = 2;
}


void SetupDialog::OnBnClickedRadio3()
{
	fallingObjectsTypes = 3;
}
