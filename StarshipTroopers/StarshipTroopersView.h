// StarshipTroopersView.h : Schnittstelle der Klasse CStarshipTroopersView


#include "DDrawSystem.h"
#include <dinput.h>
#include <atlimage.h>

#pragma once

class CMonitor;

class CStarshipTroopersView : public CView
{
#define TOPBARHEIGHT 0
#define MAXFALLINGOBJECTS 15
#define MAXRISINGOBJECTS 10
#define MAXSLOTS 12
#define AVOIDLEN 400
#define MAXGRAPHICOBJECTS 200000
#define MAXTRIALS 100
#define CURRTRIALS 20
#define CURRTRIALSAVOID 23
#define TRIALSAVOID 20

typedef struct
{
	CPoint centerRed, centerBlue;
	CPoint centerRedS2, centerBlueS2;
	CPoint centerP[MAXFALLINGOBJECTS];
	CPoint centerPS2[MAXFALLINGOBJECTS];
	int typeObj[MAXFALLINGOBJECTS];
	int distanceRed[MAXFALLINGOBJECTS], distanceBlue[MAXFALLINGOBJECTS];
	int distanceRedS2[MAXFALLINGOBJECTS], distanceBlueS2[MAXFALLINGOBJECTS];
	int trigger;
	CPoint centerPRising[MAXFALLINGOBJECTS];
	CPoint centerPRisingS2[MAXFALLINGOBJECTS];
	int typeObjRising[MAXFALLINGOBJECTS];
	int distanceRedRising[MAXFALLINGOBJECTS], distanceBlueRising[MAXFALLINGOBJECTS];
	int distanceRedRisingS2[MAXFALLINGOBJECTS], distanceBlueRisingS2[MAXFALLINGOBJECTS];

	int blueRedDistance;
	int blueRedDistanceS2;
	int js1ButtonPressed, js2ButtonPressed;
	unsigned long millitime;
} GraphicObjects;

typedef struct
{
	int width;
	int objectNum;
	int left, top;
	int slot;
	CPoint center;
	int speed;
	int delay; // start after delay ms
	int isOn;
	int starWasHit;
	char objName[256];
	CRect blockRect;
	unsigned int collisionTime;
} FallinStruct;

typedef struct
{
	int left;
	int right;
} Slot;

typedef struct
{
	CPoint pointBlue, pointRed;
	CPoint pointBlueS2, pointRedS2;

	CPoint blocks[MAXFALLINGOBJECTS];
	int blockType[MAXFALLINGOBJECTS];
	int blockSlot[MAXFALLINGOBJECTS];
	CPoint blocksS2[MAXFALLINGOBJECTS];

	CPoint blocksRising[MAXRISINGOBJECTS];
	int blockTypeRising[MAXRISINGOBJECTS];
	int blockSlotRising[MAXRISINGOBJECTS];
	CPoint blocksRisingS2[MAXRISINGOBJECTS];

	unsigned long millis;
} StandAloneStruct;

typedef struct
{
	int type;
	char typeStr[64];
	char replayFileName[1024];
	unsigned long seed;
} Trialstruct;

	protected: // Nur aus Serialisierung erstellen
		CStarshipTroopersView();
		DECLARE_DYNCREATE(CStarshipTroopersView)

	// Attribute
	public:
		CStarshipTroopersDoc* GetDocument() const;

	public:
	  DISPLAY_DEVICE monitorptr1, monitorptr2;
		DEVMODE modemon1, modemon2, modemon1old, modemon2old;
		LPDIRECTDRAW  lpDD; 
		CImageList m_imageList;
		CImageList square_imageList;
		CImageList star_imageList;
		CImage blueBallImg, redBallImg;
		CImage yellowStarImg, lilaSquareImg, redSquareImg, greenSquareImg, collisionSquareImg;
		CBitmap blueBallBm, redBallBm;
		CBitmap yellowStarBm, lilaSquareBm, redSquareBm, greenSquareBm, collisionSquareBm;
		CPoint centerP, blueBallP, redBallP, aeoP, aeoPS2, avoid1P, avoid2P;
		CPoint centerPS2, blueBallPS2, redBallPS2, avoid1PS2, avoid2PS2;
		CRect oldBlueRect, oldRedRect;
		CRect oldBlueRectS2, oldRedRectS2;
		BOOL isOverlapping1, isOverlapping2;
		CRect blueRect, redRect, overlappingRect;
		CRect blueRectS2, redRectS2, overlappingRectS2;
		CBrush  brWhite, brRed, brBlack;
		Trialstruct trials[MAXTRIALS];
		DIJOYSTATE2 js1, js2;


		StandAloneStruct alonePoints[MAXGRAPHICOBJECTS];
		unsigned long seed;
		bool canrun, exprunning;
		char vpid[256];
		char path[1000];
		char masterFilename[512];
		int replay;
		int fallingObjectsTypes;
		int fallingObjs, risingObjs, avoidEachOther, avoidEachOtherRun;
		double steigungsWinkel;
		int idx, maxidx;
		CString cpath;
		TCHAR tpath[1000];
		FallinStruct fallingObjects[MAXFALLINGOBJECTS], risingObjects[MAXRISINGOBJECTS];
		FallinStruct fallingObjectsS2[MAXFALLINGOBJECTS], risingObjectsS2[MAXRISINGOBJECTS];
		Slot slots[MAXSLOTS];
		GraphicObjects graphicObjects[MAXGRAPHICOBJECTS];
		unsigned long touchTime1, touchTime2;
		unsigned long currtime;
		unsigned long millis1, millis2;
		int touchDistance1, touchDistance2;
		int player1StickColor, player2StickColor;
		int run, runtype, joy1Startpos, joy2Startpos;
		int loop;

		CFont newFont, *oldfont;
		CRect rect;
		CPoint p;
		unsigned long startmillis, stopmillis, currmillis, millis10;
		int counter;
		int collisions, collisionsS2;
		int stars, starsS2;
		int together, togetherS2;
		int blueRating1, redRating1, blueRating2, redRating2;
		int bluecollision[MAXFALLINGOBJECTS];
		int bluecollisionS2[MAXFALLINGOBJECTS];
		int redcollision[MAXFALLINGOBJECTS];
		int redcollisionS2[MAXFALLINGOBJECTS];

	public:
		virtual void OnDraw(CDC* pdc);  // Überschrieben, um diese Ansicht darzustellen
		virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

		bool ReadStandAloneData(char *filename, int type);
		void InitializeFallingObjects(void);
		void InitializeBalls(void);
		void ReadBlueJoystick(void);
		void ReadRedJoystick(void);
		void ReadBlueJoystickRating(int which);
		void ReadRedJoystickRating(int which);
		void InitializeOneFallingObject(int which, int currtime);
		void InitializeOneRisingObject(int which, int currtime);
		int FindStandAlonePos(int which, int lastIdx, unsigned long currtime);
		void UpdateFallingObjects(int currtime);
		void OnInitialUpdate();
		void ResetGraphicObjects();
		void ResetAlonePoints();
		void WhiteItemRects(CDC* pDC);
		void DoRating(int which);

	protected:

	// Implementierung
	public:
		virtual ~CStarshipTroopersView();
	#ifdef _DEBUG
		virtual void AssertValid() const;
		virtual void Dump(CDumpContext& dc) const;
	#endif

	// Generierte Funktionen für die Meldungstabellen
	protected:
		//{{AFX_MSG(CDdraw_in_mfcwindView)
//		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		//}}AFX_MSG
		DECLARE_MESSAGE_MAP()
			afx_msg void OnSetupexperiment();
			afx_msg void OnStartexperiment();
			afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

#ifndef _DEBUG  // Debugversion in StarshipTroopersView.cpp
inline CStarshipTroopersDoc* CStarshipTroopersView::GetDocument() const
   { return reinterpret_cast<CStarshipTroopersDoc*>(m_pDocument); }
#endif

