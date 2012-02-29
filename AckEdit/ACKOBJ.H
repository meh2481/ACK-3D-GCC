// Header file for AckObject class

#define	MAX_OBJ_BITMAPS		255

typedef unsigned char	UCHAR;

class CAckObjSeq
{

public:
	CAckObjSeq();
	~CAckObjSeq();

public:
	UCHAR	m_flags;					//Flags for this sequence
	UCHAR	m_bitmaps[MAX_OBJ_BITMAPS];	//Bitmap numbers in this sequence
	short	m_bmSides;					//Number of views in sequence
	short	m_bmBitmapsPerView;			//Number of bitmaps in each view
	short	m_AngleFactor;
	UCHAR	m_MaxBitmaps;				//Max bitmaps in this sequence


//methods
public:
	UCHAR	GetFlags() {return m_flags;};
	void	SetFlags(UCHAR cFlags) {m_flags = cFlags;};
	UCHAR	GetBmNum(int index) {return m_bitmaps[index];};
	void	SetBmNum(int index,UCHAR cBM) {m_bitmaps[index] = cBM;};
	short	GetSides() {return m_bmSides;};
	void	SetSides(short sSides) {m_bmSides = sSides;};
	short	GetBPV() {return m_bmBitmapsPerView;};
	void	SetBPV(short nBPV) {m_bmBitmapsPerView = nBPV;};
	short	GetAngleFactor() {return m_AngleFactor;};
	void	SetAngleFactor(short sAF) {m_AngleFactor = sAF;};
	UCHAR	GetMaxBM() {return m_MaxBitmaps;};
	void	SetMaxBM(UCHAR cMax) {m_MaxBitmaps = cMax;};

};


class CAckObject
{

public:
	CAckObject();
	~CAckObject();


public:
	char	m_Active;			//0=Inactive, 1=Active
	UCHAR	m_Flags;			//Misc flags for this object
	char	m_Speed;			//Speed of obj (used by appl)
	short	m_Dir;				//Direction of obj (used by appl)
	short	m_x;				//Current location in grid
	short	m_y;
	short	m_mPos;				//Current map location in grid
	UCHAR	m_id;				//Object id
	short	m_CurrentType;		//Create,Destroy,etc is current
	UCHAR	*m_CurrentBitmaps;	//Current bitmap list
	short	m_Sides;			//Number of views
	short	m_aFactor;			//Angle factor
	short	m_BitmapsPerView;	//Number of bitmaps in each view
	UCHAR	m_CurrentBm;		//Current bitmap number
	UCHAR	m_Maxbm;			//Max bitmap number for this view
	CAckObjSeq	m_Create;
	CAckObjSeq	m_Destroy;
	CAckObjSeq	m_Walk;
	CAckObjSeq	m_Attack;
	CAckObjSeq	m_Interact;

// Methods
public:
	char	GetActive() {return m_Active;};
	void	SetActive(char cA) {m_Active = cA;};
	UCHAR	GetFlags() {return m_Flags;};
	void	SetFlags(UCHAR cF) {m_Flags = cF;};
	char	GetSpeed() {return m_Speed;};
	void	SetSpeed(char cS) {m_Speed = cS;};
	short	GetDir() {return m_Dir;};
	void	SetDir(short nDir) {m_Dir = nDir;};
	short	GetX() {return m_x;};
	void	SetX(short nX) {m_x = nX;};
	short	GetY() {return m_y;};
	void	SetY(short nY) {m_y = nY;};
	short	GetPos() {return m_mPos;};
	void	SetPos(short nPos) {m_mPos = nPos;};
	UCHAR	GetID() {return m_id;};
	void	SetID(UCHAR cID) {m_id = cID;};
	short	GetType() {return m_CurrentType;};
	void	SetType(short nType) {m_CurrentType = nType;};
	UCHAR	*GetbmTypes() {return m_CurrentBitmaps;};
	void	SetbmTypes(UCHAR *pbm) {m_CurrentBitmaps = pbm;};
	short	GetSides() {return m_Sides;};
	void	SetSides(short nSides) {m_Sides = nSides;};
	short	GetFactor() {return m_aFactor;};
	void	SetFactor(short nAF) {m_aFactor = nAF;};
	short	GetBPV() {return m_BitmapsPerView;};
	void	SetBPV(short nBPV) {m_BitmapsPerView = nBPV;};
	UCHAR	GetCurrentBM() {return m_CurrentBm;};
	void	SetCurrentBM(UCHAR cBM) {m_CurrentBm = cBM;};
	UCHAR	GetMaxBM() {return m_Maxbm;};
	void	SetMaxBM(UCHAR cBM) {m_Maxbm = cBM;};
	

};