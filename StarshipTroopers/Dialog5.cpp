// Dialog5.cpp : implementation file
//

#include "stdafx.h"
#include "StarshipTroopers.h"
#include "Dialog5.h"


// Dialog5 dialog

IMPLEMENT_DYNAMIC(Dialog5, CDialog)

Dialog5::Dialog5(CWnd* pParent /*=NULL*/)
	: CDialog(Dialog5::IDD, pParent)
{

}

Dialog5::~Dialog5()
{
}

void Dialog5::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(Dialog5, CDialog)
END_MESSAGE_MAP()


// Dialog5 message handlers
