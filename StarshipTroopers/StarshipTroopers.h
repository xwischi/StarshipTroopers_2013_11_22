// StarshipTroopers.h : Hauptheaderdatei für die StarshipTroopers-Anwendung
//
#pragma once

#ifndef __AFXWIN_H__
	#error "\"stdafx.h\" vor dieser Datei für PCH einschließen"
#endif

#include "resource.h"       // Hauptsymbole


// CStarshipTroopersApp:
// Siehe StarshipTroopers.cpp für die Implementierung dieser Klasse
//

class CStarshipTroopersApp : public CWinApp
{
public:
	CStarshipTroopersApp();


// Überschreibungen
public:
	virtual BOOL InitInstance();

// Implementierung
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CStarshipTroopersApp theApp;