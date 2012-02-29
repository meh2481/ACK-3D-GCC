// ack.h : main header file for the ACK application
//

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CAckApp:
// See ack.cpp for the implementation of this class
//

class CAckApp : public CWinApp
{
public:
	CAckApp();

// Overrides
	virtual BOOL InitInstance();

// Implementation

	//{{AFX_MSG(CAckApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// VB-Event extern declarations

//{{AFX_VBX_REGISTER()
//}}AFX_VBX_REGISTER

/////////////////////////////////////////////////////////////////////////////
