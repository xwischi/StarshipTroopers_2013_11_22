// StarshipTroopers.h : Hauptheaderdatei f�r die StarshipTroopers-Anwendung
//
#pragma once

#ifndef __AFXWIN_H__
	#error "\"stdafx.h\" vor dieser Datei f�r PCH einschlie�en"
#endif

#include "resource.h"       // Hauptsymbole


// CStarshipTroopersApp:
// Siehe StarshipTroopers.cpp f�r die Implementierung dieser Klasse
//

class CStarshipTroopersApp : public CWinApp
{
public:
	CStarshipTroopersApp();


// �berschreibungen
public:
	virtual BOOL InitInstance();

// Implementierung
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CStarshipTroopersApp theApp;