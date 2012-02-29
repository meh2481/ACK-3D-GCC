// Header file for ACK Door class

typedef	unsigned char	UCHAR;

class CAckDoor
{

public:
	CAckDoor();
	~CAckDoor();

public:
	    short	m_mPos;
	    short	m_mPos1;
	    short	m_mCode;
	    short	m_mCode1;
	    UCHAR	m_ColOffset;
	    char	m_Speed;
	    char	m_Type;
	    UCHAR	m_Flags;

// Methods
public:
	short	GetPos() {return m_mPos;};
	void	SetPos(short nPos) {m_mPos = nPos;};
	short	GetPos1() {return m_mPos1;};
	void	SetPos1(short nPos) {m_mPos1 = nPos;};
	short	GetCode() {return m_mCode;};
	void	SetCode(short nCode) {m_mCode = nCode;};
	short	GetCode1() {return m_mCode1;};
	void	SetCode1(short nCode) {m_mCode1 = nCode;};
	UCHAR	GetColOffset() {return m_ColOffset;};
	void	SetColOffset(UCHAR nCO) {m_ColOffset = nCO;};
	char	GetSpeed() {return m_Speed;};
	void	SetSpeed(char nS) {m_Speed = nS;};
	char	GetType() {return m_Type;};
	void	SetType(char cT) {m_Type = cT;};
	UCHAR	GetFlags() {return m_Flags;};
	void	SetFlags(UCHAR nF) {m_Flags = nF;};

};
	