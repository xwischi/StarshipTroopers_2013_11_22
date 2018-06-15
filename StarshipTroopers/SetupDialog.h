#pragma once
#include "afxwin.h"


// SetupDialog dialog

class SetupDialog : public CDialog
{
	#define MINFALLINGOBJECTS 4
	#define MAXFALLINGOBJECTS 15
	#define MAXRISINGOBJECTS 10
	#define MAXRUNTIME 1000000
	DECLARE_DYNAMIC(SetupDialog)

public:
	SetupDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~SetupDialog();
	char vpid[256];

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int fallingObjectsTypes;
	int replay;
	int fallingObjs, risingObjs,	avoidEachOther;
	CButton standAloneButton, avoidEachOtherButton;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio3();
};
