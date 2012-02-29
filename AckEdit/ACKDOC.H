// ackdoc.h : interface of the CAckDoc class
//
/////////////////////////////////////////////////////////////////////////////

class CAckDoc : public CDocument
{
protected: // create from serialization only
	CAckDoc();
	DECLARE_DYNCREATE(CAckDoc)

// Attributes
public:
// Operations
public:

// Implementation
public:
	virtual ~CAckDoc();
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual BOOL OnNewDocument();

// Generated message map functions
protected:
	//{{AFX_MSG(CAckDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
