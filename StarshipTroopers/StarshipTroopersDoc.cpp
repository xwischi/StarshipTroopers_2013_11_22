// StarshipTroopersDoc.cpp : Implementierung der Klasse CStarshipTroopersDoc
//

#include "stdafx.h"
#include "StarshipTroopers.h"

#include "StarshipTroopersDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CStarshipTroopersDoc

IMPLEMENT_DYNCREATE(CStarshipTroopersDoc, CDocument)

BEGIN_MESSAGE_MAP(CStarshipTroopersDoc, CDocument)
END_MESSAGE_MAP()


// CStarshipTroopersDoc-Erstellung/Zerst�rung

CStarshipTroopersDoc::CStarshipTroopersDoc()
{
	// TODO: Hier Code f�r One-Time-Konstruktion einf�gen

}

CStarshipTroopersDoc::~CStarshipTroopersDoc()
{
}

BOOL CStarshipTroopersDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: Hier Code zur Reinitialisierung einf�gen
	// (SDI-Dokumente verwenden dieses Dokument)

	return TRUE;
}




// CStarshipTroopersDoc-Serialisierung

void CStarshipTroopersDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: Hier Code zum Speichern einf�gen
	}
	else
	{
		// TODO: Hier Code zum Laden einf�gen
	}
}


// CStarshipTroopersDoc-Diagnose

#ifdef _DEBUG
void CStarshipTroopersDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CStarshipTroopersDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CStarshipTroopersDoc-Befehle
