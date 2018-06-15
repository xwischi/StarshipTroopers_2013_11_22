#pragma once

#include <atlimage.h>
#include <dinput.h>

// Dialog1 dialog

class Dialog1 : public CDialog
{
	DECLARE_DYNAMIC(Dialog1)

public:
	Dialog1(CWnd* pParent = NULL);   // standard constructor
	virtual ~Dialog1();
	char runNumStr[256];

// Dialog Data
	enum { IDD = IDD_DIALOG2 };
	BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
