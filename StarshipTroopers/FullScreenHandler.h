////////////////////////////////////////////////////////////////
// MSDN Magazine -- December 2002
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
// Compiles with VC 6.0 or VS.NET on Windows XP. Tab size=3.
//
// Handle full-screen mode: adjust frame size to make
// view's client area fill the available screen.
//
class CFullScreenHandler {
public:
	CFullScreenHandler();
	~CFullScreenHandler();

	void Maximize(CFrameWnd* pFrame, CWnd* pView);
	void Restore(CFrameWnd* pFrame);
	BOOL InFullScreenMode() { return !m_rcRestore.IsRectEmpty(); }
	CSize GetMaxSize();

protected:
	CRect m_rcRestore;
};

// Global instance
extern CFullScreenHandler FullScreenHandler;
