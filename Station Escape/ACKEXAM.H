// Header file for example program

BOOL Register(HINSTANCE hInst);
BOOL CreateAll(HINSTANCE hInst);
int MessageLoop(void);
LRESULT	 CALLBACK _export AckWndProc( HWND hWnd, UINT iMessage, WPARAM wParam,
				 LPARAM lParam );

int LoadBackDrop(void);
int AckProcessInfoFile(void);

