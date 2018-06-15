// Dialog4.cpp : implementation file
//

#include "stdafx.h"
#include "StarshipTroopers.h"
#include "Dialog4.h"


// Dialog4 dialog

IMPLEMENT_DYNAMIC(Dialog4, CDialog)

Dialog4::Dialog4(CWnd* pParent /*=NULL*/)
	: CDialog(Dialog4::IDD, pParent)
{

}

Dialog4::~Dialog4()
{
}

void Dialog4::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(Dialog4, CDialog)
END_MESSAGE_MAP()


// Dialog4 message handlers
