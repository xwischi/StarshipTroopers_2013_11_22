// Dialog1.cpp : implementation file
//

#include "stdafx.h"
#include "StarshipTroopers.h"
#include "Dialog1.h"


// Dialog1 dialog

IMPLEMENT_DYNAMIC(Dialog1, CDialog)

Dialog1::Dialog1(CWnd* pParent /*=NULL*/)
	: CDialog(Dialog1::IDD, pParent)
{
}


Dialog1::~Dialog1()
{
}


BOOL Dialog1::OnInitDialog()
{
	wchar_t wtext[256];
	for(int i = 0; i < 256; i++)
		wtext[i] = 0;
	mbstowcs(wtext, runNumStr, strlen(runNumStr)+1);
	SetDlgItemText(IDC_RUNNUM, (LPCTSTR)&wtext);
	return true;
}


void Dialog1::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(Dialog1, CDialog)
END_MESSAGE_MAP()


// Dialog1 message handlers
