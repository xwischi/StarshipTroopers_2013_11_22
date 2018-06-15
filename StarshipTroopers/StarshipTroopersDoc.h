// StarshipTroopersDoc.h : Schnittstelle der Klasse CStarshipTroopersDoc
//


#pragma once


class CStarshipTroopersDoc : public CDocument
{
protected: // Nur aus Serialisierung erstellen
	CStarshipTroopersDoc();
	DECLARE_DYNCREATE(CStarshipTroopersDoc)

// Attribute
public:

// Vorgänge
public:

// Überschreibungen
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementierung
public:
	virtual ~CStarshipTroopersDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generierte Funktionen für die Meldungstabellen
protected:
	DECLARE_MESSAGE_MAP()
};


