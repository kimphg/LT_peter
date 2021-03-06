//=============================================================================
//	Ban quyen	: phong Phat trien He thong Thong tin - Cong ty AIC
//	Bat dau		: 05/02/2002
//	Cap nhat	: 12/03/2013;
//	Tac gia		: Dang Quang Hieu, hieudq@aic.com.vn
//
//	Module		: CPoint - Quan ly lop diem tren ban do dia ly
//=============================================================================

//#include "StdAfx.h"
#include "CPoints.h"

#include <math.h>


//=============================================================================
// Construction/Destruction
//=============================================================================
void C2_Point::setLat(long lat)
{
    m_Lat = lat;
    mLat_dec = m_Lat*180.0/bit23;
}

void C2_Point::setLon(long lon)
{
    m_Long = lon;
    mLon_dec = m_Long*180.0/bit23;
}

double C2_Point::getLat()
{

    return mLat_dec;
}

double C2_Point::getLon()
{
    return mLon_dec;
}

C2_Point::C2_Point()
{
	x	= 0;
	y	= 0;
}

C2_Point::~C2_Point()
{}

void C2_Point::Update(C2_Point *pPoint)
{	
	m_Lat	= pPoint->m_Lat ;
	m_Long	= pPoint->m_Long;
    mLat_dec = m_Lat/bit23*180.0f;
    mLon_dec	= m_Long/bit23*180.0f;
}

//-----------------------------------------------------------------------------
//	Convert: String to Lat
//-----------------------------------------------------------------------------
//bool C2_Point::ConvStrToLat(CString lpStr)
//{
//	// 1: Check input string
//	if (lpStr.GetLength() < 9)
//		return false;

//	if ((lpStr[8] != 'B') && (lpStr[8] != 'b') &&
//		(lpStr[8] != 'N') && (lpStr[8] != 'n'))
//		return false;

//	if ((lpStr[2] != ':') || (lpStr[5] != ':'))
//		return false;

//	for (int i = 0; i < 8; i++)
//	if ((i!=2) && (i!=5))
//	{
//		if ((lpStr[i] < 48) || (lpStr[i] > 57))		// '0'...'9'
//			return false;
//	}

//	// 2: Convert string to Lat
//	float	LSB = float ((1<<18)/(45*450.0));		// 360 = (1^24)
//	float	nLat;

//	nLat = float(((lpStr[0]-48)*10 + (lpStr[1]-48))*GG +
//				 ((lpStr[3]-48)*10 + (lpStr[4]-48))*gg +
//				 ((lpStr[6]-48)*10 + (lpStr[7]-48)))*LSB;

//	if ((lpStr[8] == 'N') || (lpStr[8] == 'n'))
//		nLat = nLat*(-1);

//	m_Lat = long(nLat);
//	return true ;
//}

//-----------------------------------------------------------------------------
//	Convert: String to Long
//-----------------------------------------------------------------------------
//bool C2_Point::ConvStrToLong(CString lpStr)
//{
//	// 1: Check input string
//	if (lpStr.GetLength() < 10)
//		return false;

//	if ((lpStr[9] != 'D') && (lpStr[9] != 'd') &&
//		(lpStr[9] != 'T') && (lpStr[9] != 't'))
//		return false;

//	if ((lpStr[3] != ':') || (lpStr[6] != ':'))
//		return false;

//	for (int i = 0; i < 9; i++)
//	if ((i!=3) && (i!=6))
//	{
//		if ((lpStr[i] < 48) || (lpStr[i] > 57))		// '0'...'9'
//			return false;
//	}

//	float	LSB = float ((1<<18)/(45*450.0));		// 360 = (1^24)
//	float	nLong;

//	nLong = float(((lpStr[0]-48)*100+ (lpStr[1]-48)*10 + (lpStr[2]-48))*GG +
//				  ((lpStr[4]-48)*10 + (lpStr[5]-48))*gg +
//				  ((lpStr[7]-48)*10 + (lpStr[8]-48)))*LSB;

//	if ((lpStr[9] == 'T') || (lpStr[9] == 't'))
//		nLong = nLong*(-1);

//	m_Long = long(nLong);
//	return true;
//}


//-----------------------------------------------------------------------------
//	Get Lat string
//-----------------------------------------------------------------------------
//CString C2_Point::GetStrLat()
//{
//	char	Head;
//	int		deg, min, sec;
//	float	nTmp;

//	nTmp = float(m_Lat * 45 * GG/(1<<21));	// [sec]

//	if (nTmp >= 0.0)
//		Head = 'B';
//	else
//	{
//		Head = 'N';
//		nTmp = nTmp * (-1);
//	}

//	deg   = int(nTmp / 3600);		// [deg]
//	nTmp -= deg * 3600;				// [sec]
//	min   = int(nTmp / 60);			// [min]
//	nTmp -= min * 60;				// [sec]
//	sec   = int((nTmp - long(nTmp) > 0.5) ? (nTmp+1) : nTmp );

//	if (sec > 59)
//	{
//		sec -= 60;
//		min ++;
//		if (min > 59)
//		{
//			min -= 60;
//			deg ++;
//		}
//	}

//	CString szStr;
//	szStr.Format(_T("%02d:%02d:%02d%c"), deg, min, sec, Head);
//	return szStr;
//}

////-----------------------------------------------------------------------------
////	Get Long string
////-----------------------------------------------------------------------------
//CString C2_Point::GetStrLong()
//{
//	char	Head;
//	int		deg, min, sec;
//	float	nTmp;

//	nTmp = float(m_Long * ((45*GG)/(1<<21)));	// [sec]

//	if (nTmp >= 0)
//		Head = 'D';
//	else
//	{
//		Head = 'T';
//		nTmp = nTmp * (-1);
//	}

//	deg   = int(nTmp / 3600);		// [deg]
//	nTmp -= deg * 3600;				// [sec]
//	min   = int(nTmp / 60);			// [min]
//	nTmp -= min * 60;				// [sec]
//	sec   = int((nTmp - long(nTmp) >= 0.5) ? (nTmp+1) : nTmp );

//	if (sec > 59)
//	{
//		sec -= 60;
//		min ++;
//		if (min > 59)
//		{
//			min -= 60;
//			deg ++;
//		}
//	}

//	CString szStr;
//	szStr.Format(_T("%03d:%02d:%02d%c"), deg, min, sec, Head);
//	return szStr;
//}

//-----------------------------------------------------------------------------
//	Convert WGS(Lat, Long) -> Screen(X, Y)
//-----------------------------------------------------------------------------
void C2_Point::ConvWGSToScr(C2_Point *pPoint, long nLat0, long nLong0, int nScale)
{
	float	rEarth;
	long	dX, dY;

	rEarth	= float(R_EARTH * cos(nLat0 * WGS_RAD));    		// [m]
	dX		= long((nLong0 - pPoint->m_Long) * WGS_RAD * (rEarth  / nScale));
	dY		= long((nLat0  - pPoint->m_Lat ) * WGS_RAD * (R_EARTH / nScale));

	x		= pPoint->x + dX;
	y		= pPoint->y - dY; 
}

void C2_Point::ConvWGSToScr(C2_Point *pPoint, int nScale)
{
	float	rEarth;
	long	dX, dY;

	rEarth	= float(R_EARTH * cos(m_Lat * WGS_RAD));    		// [m]
	dX		= long((m_Long - pPoint->m_Long) * WGS_RAD * (rEarth  / nScale));
	dY		= long((m_Lat  - pPoint->m_Lat ) * WGS_RAD * (R_EARTH / nScale));

	x		= pPoint->x + dX;
	y		= pPoint->y - dY; 
}

//-----------------------------------------------------------------------------
//	Convert: Screen(X, Y) -> WGS(Lat, Long)
//-----------------------------------------------------------------------------
void C2_Point::ConvScrToWGS(C2_Point *pPoint, int nScale)
{
	ConvScrToWGS(pPoint, x, y, nScale);
}

void C2_Point::ConvScrToWGS(C2_Point *pPoint, int nX0, int nY0, int nScale)
{
	long	dX, dY;				//  Do lech Long, Lat so voi diem chuan 
	float	dLong, dLat;		// 
	float	rEarth, nVido;

	dX		= (nX0 - pPoint->x) * nScale;			// [m]  Long
	dY		= (nY0 - pPoint->y) * nScale;			// [m]  Lat

	dLat	= float((dY / R_EARTH) * RAD_WGS);		// [LSBWGS]	
	nVido	= float(pPoint->m_Lat - dLat);			// [LSBWGS]
	rEarth	= float(R_EARTH * cos(nVido*WGS_RAD));	// [m]
	dLong	= float((dX / rEarth ) * RAD_WGS);		// [LSBWGS]


	m_Lat  = long(pPoint->m_Lat - dLat );
	m_Long = long(pPoint->m_Long+ dLong);

	if (dLat > 0.0)
		m_Lat  = pPoint->m_Lat  - long((dLat - long(dLat ) >  0.5) ? (dLat+1 ):dLat );
	else
		m_Lat  = pPoint->m_Lat  - long((long(dLat) - dLat  >  0.5) ? (dLat-1 ):dLat );

	if (dLong > 0.0)
		m_Long = pPoint->m_Long + long((dLong- long(dLong) >= 0.5) ? (dLong+1):dLong);
	else
		m_Long = pPoint->m_Long + long((long(dLong)- dLong >= 0.5) ? (dLong-1):dLong);
}


//-----------------------------------------------------------------------------
//	Convert WGS(Lat, Long) to Polar(Range, Azimuth) chuyển từ Lat/Long sang radian và độ
//-----------------------------------------------------------------------------
void C2_Point::ConvWGSToPol(C2_Point *pPoint, long &nRge, UINT &nAzi)
{
	float 	rEarth;             // Ban kinh trai dat o Lat
	float 	dX, dY;				// Chenh khoang cach [m]
	float 	dLat, dLong;        // Chenh goc Lat - Long
	float 	nCuly;
	UINT	nGoc ;

	dLat	= float(m_Lat  - pPoint->m_Lat);		// LSB = 180/ bit23
	dY		= float(dLat  * WGS_RAD * R_EARTH);		// [m]

	rEarth	= float(R_EARTH * cos(m_Lat*WGS_RAD));	// [m]	 Ban kinh vong tron o vi do Lat

	dLong	= float(m_Long - pPoint->m_Long);		// LSB = 180/ bit23
	dX		= float(dLong * WGS_RAD * rEarth);		// [m]

	// Vi tri muc tieu
	nCuly	= float(sqrt(dX*dX + dY*dY));
	if (nCuly > 0.0)
		nGoc = UINT((bit15/M_PI) * asin(dX/nCuly));	// LSB = bit16/360	
	else
		nGoc = 0;	

	if (dLong > 0.0)
		nGoc = (dLat > 0.0)? (nGoc		  ) : (bit15 - nGoc);
	else
		nGoc = (dLat > 0.0)? (bit16 + nGoc) : (bit15 - nGoc);
	
	nRge = long (nCuly);
	nAzi = UINT (nGoc % bit16);
}

//-----------------------------------------------------------------------------
//	Convert Polar(Range, Azimuth) to WGS(Lat, Long)
//-----------------------------------------------------------------------------
void C2_Point::ConvPolToWGS(C2_Point *pPoint, long nRge, UINT nAzi)
{
	float	rEarth;
	float	dX, dY;		   		// Chenh khoang cach
	long	dLat, dLong;      	// Chenh goc
	float	nVido, nGoc;

	// Vi do:
	nGoc	= float((nAzi*M_PI)/bit15);				// [radian]
	dY		= float(nRge * cos(nGoc));             	// [m] Chenh khoang cach theo truc dung

	dLat	= long (dY/R_EARTH * RAD_WGS);  		// LSB = 180/ 2 +23
	nVido	= float(m_Lat + dLat);            		// LSB = 180/ 2 +23

	// Kinh do 
	rEarth	= float(R_EARTH * cos(nVido*WGS_RAD));	// [m] Ban kinh vong tron o vi do Lat
	dX		= float(nRge * sin(nGoc)); 				// [m] Chenh khoang cach theo truc ngang
	dLong	= long (dX/rEarth  * RAD_WGS);			// LSB = 180/ 2 +23	

	pPoint->m_Lat  = m_Lat  + dLat ; 	
	pPoint->m_Long = m_Long + dLong;	
}


//-----------------------------------------------------------------------------
//	Function: Get Distance to point on screen
//-----------------------------------------------------------------------------
int C2_Point::GetDistPoint(C2_Point *pPoint)
{	
	int dX = abs(x - pPoint->x);
	int	dY = abs(y - pPoint->y);

	return int (sqrt(double(dX*dX) + double(dY*dY)));
}

//-----------------------------------------------------------------------------
//	Get Distance to line on screen
//-----------------------------------------------------------------------------
int C2_Point::GetDistLine(C2_Point *pPnt1, C2_Point *pPnt2)
{
	int		Dx, Dy, Dr;		//
	int		Min, Max;		//  Use same for X,Y	

	Dx = pPnt2->x - pPnt1->x;
	Dy = pPnt2->y - pPnt1->y;

	if (Dx == 0)	// Line: Dung thang
	{		
		Min = (pPnt1->y < pPnt2->y) ? pPnt1->y : pPnt2->y;
		Max = (pPnt1->y > pPnt2->y) ? pPnt1->y : pPnt2->y;
		if ((y >= Min) && (y <= Max))
			Dr = abs(x - pPnt1->x);
		else 
		if (y < Min)		
			Dr = int (sqrt(double((x - pPnt1->x)*(x - pPnt1->x)) + 
						   double((Min - y	   )*(Min - y	  ))));
		else		// Y > max
			Dr = int (sqrt(double((x - pPnt1->x)*(x - pPnt1->x)) +
						   double((y - Max	   )*(y - Max	  ))));
	}
	else 
	if (Dy == 0)	// Line: Nam ngang
	{
		Min = (pPnt1->x < pPnt2->x) ? pPnt1->x : pPnt2->x;
		Max = (pPnt1->x > pPnt2->x) ? pPnt1->x : pPnt2->x;
		if ((x >= Min) && (x<= Max))
			Dr = abs(y - pPnt1->y);
		else 
		if (x < Min)		
			Dr = int (sqrt(double((y - pPnt1->y)*(y - pPnt1->y)) + 
						   double((Min - x	   )*(Min -	x	  ))));
		else		// X > max
			Dr = int (sqrt(double((y - pPnt1->y)*(y - pPnt1->y)) + 
						   double((x - Max	   )*(x - Max	  ))));
	}
	else				// Line: Xien
	{
		double	a1, a2, b1, b2;	// He so line
		double	xx, yy;			// Giao diem

		// Line 1: y = a1.x + b1
		// Line 2: y = a2.x + b2
		a1 = ( 1.0*Dy)/(1.0*Dx);
		a2 = (-1.0*Dx)/(1.0*Dy);
		b1 = pPnt1->y - a1*pPnt1->x;
		b2 = y - a2*x;
		xx = (b1 - b2)/(a2 - a1);
		yy = a1*xx + b1;

		// Check 
		Min = (pPnt1->x < pPnt2->x) ? pPnt1->x : pPnt2->x;
		Max = (pPnt1->x > pPnt2->x) ? pPnt1->x : pPnt2->x;
		if ((xx >= Min) && (xx<= Max))	// In line
			Dr = int (sqrt( (xx-x)*(xx-x) + (yy-y)*(yy-y) ));			
		else	// Out Line
		{			
			xx = int (sqrt( double((x - pPnt1->x)*(x - pPnt1->x)) +
							double((y - pPnt1->y)*(y - pPnt1->y))));
			yy = int (sqrt( double((x - pPnt2->x)*(x - pPnt2->x)) + 
							double((y - pPnt2->y)*(y - pPnt2->y))));
			Dr = (xx > yy)? int(yy): int(xx);
		}
	}
	return Dr;
}


//=============================================================================
// Place class
//=============================================================================
C2_Place::C2_Place()
{
	C2_Point::C2_Point();

	m_Uid	= 0;
	m_Type	= 0;
    m_szName= ("");
	m_Color	= 0;

 	m_Lat0		= 02 *(1<<21)/45;
 	m_Long0		= 107*(1<<21)/45;
}

C2_Place::~C2_Place()
{}

void C2_Place::Update(C2_Place *pPlace)
{
	C2_Point::Update(pPlace);

	m_Uid	= pPlace->m_Uid  ;
	m_szName= pPlace->m_szName;
	m_Type	= pPlace->m_Type ;
	m_Color	= pPlace->m_Color;

	m_Lat0	= pPlace->m_Lat0 ;
	m_Long0	= pPlace->m_Long0;
}

//-----------------------------------------------------------------------------
//	Get Type-string of Place
//CString C2_Place::GetStrType()
//{
//	_TCHAR	*szType[7] = {_T("0"), _T("1"), _T("2"), _T("3"), _T("4"),
//						  _T("5"), _T("...")};

//	return (m_Type < 6)? szType[m_Type] : szType[6];
//}

//CString C2_Place::GetStrColor(int nId)
//{
//	_TCHAR	*szColor[9] = {
//		_T("Mầu trắng")	,	_T("Mầu xám nhạt"),	_T("Xanh da trời"),
//		_T("Xanh lá cây"), 	_T("Mầu đỏ")	,	_T("Mầu đỏ hồng"),
//		_T("Mầu da cam"),	_T("Mầu tím")	,	_T("Mầu vàng")	};

//	return (nId < 9)? szColor[nId] : _T("...");
//}

////-----------------------------------------------------------------------------
////	Get string of Range
////-----------------------------------------------------------------------------
//CString C2_Place::GetStrRange(long nRge)	//[m]
//{
//	long	nTmp = nRge/100;		// 0.1km

//	CString szStr;
//	szStr.Format(_T("%4d.%01d"), nTmp/10, nTmp%10);
//	return szStr;
//}

////-----------------------------------------------------------------------------
////	 Get string of Azi: (Deg)
////-----------------------------------------------------------------------------
//CString C2_Place::GetStrAzimu(UINT nAzi)	// (360/2^16)
//{
//	UINT	nDeg;
//	long	nTmp;

//	nTmp = (45*60*nAzi)/(1<<13);			// [minute]
//	nDeg = nTmp/60;

//	if ((nTmp%60) > 29)
//	{
//		nDeg ++;
//		if (nDeg > 359)
//			nDeg -= 360;
//	}

//	CString	szStr;
//	szStr.Format(_T("%03d"), nDeg);
//	return szStr;
//}

//-----------------------------------------------------------------------------
//	Get string of Azimu (Deg:Min)
//-----------------------------------------------------------------------------
//CString C2_Place::GetStrAzMin(UINT nAzi)	// (xxx'yy)
//{
//	int		nDeg, nMin, nSec;			// Sec;
//	long	nTmp;

//	nTmp  = (45*225*nAzi)/(1<< 9);		// [second]
//	nDeg  = nTmp/3600;
//	nTmp %= 3600;

//	nMin  = nTmp/60;
//	nSec  = nTmp%60;
//	if (nSec > 30)
//	{
//		nMin ++;
//		if (nMin > 59)
//		{
//			nMin -= 60;
//			nDeg ++;
//			if (nDeg > 359)
//				nDeg -= 360;
//		}
//	}

//	CString szStr;
//	szStr.Format(_T("%03d:%02d'"), nDeg, nMin);
//	return szStr;
//}

//-----------------------------------------------------------------------------
//	Get int of Azi: (Degree)
//-----------------------------------------------------------------------------
int C2_Place::GetIntAzimu(UINT nAzi)
{
	int		nDeg;
	long	nTmp;

	nTmp = (45*60*nAzi)/(1<<13);		// [minute]
	nDeg = nTmp/60;

	if ((nTmp%60) > 29)
	{
		nDeg ++;
		if (nDeg > 359)
			nDeg -= 360;
	}
	return nDeg;
}



//=============================================================================
// Place class container
//=============================================================================
CC_Place::CC_Place()
{}

CC_Place::~CC_Place()
{
	Clear();	
}

//-----------------------------------------------------------------------------
//	Clear/Count/Empty
//-----------------------------------------------------------------------------
void CC_Place::Clear()
{
//	POSITION nPos;

//	nPos = m_ListPlace.GetHeadPosition();
//	while (nPos)
//		delete m_ListPlace.GetNext(nPos);

    m_ListPlace.clear();
}

int  CC_Place::Count()
{
    return (int)m_ListPlace.size();
}

bool CC_Place::IsEmpty()
{	
    return (m_ListPlace.isEmpty() == 1);
}

//-----------------------------------------------------------------------------
//	Copy new list to CC_Place
//-----------------------------------------------------------------------------
void CC_Place::Copy(CC_Place *pCPlace)
{	
	C2_Place *	pPlace;
	C2_Place *	pPlNew;
//	POSITION	nPos;

	Clear();

//	nPos  = pCPlace->GetPosH();
//	while (nPos)
//	{
//		pPlNew  = pCPlace->GetNext(nPos);

//		pPlace	= new C2_Place();
//		if (pPlace)
//		{
//			pPlace->Update(pPlNew);
//			m_ListPlace.AddTail(pPlace);
//		}
//	}
}

//-----------------------------------------------------------------------------
//	Add / Get / Remove	
//-----------------------------------------------------------------------------
void CC_Place::AddHead(C2_Place *pPlace)
{
	if (pPlace)
        m_ListPlace.push_front(pPlace);
}

void CC_Place::AddTail(C2_Place *pPlace)
{
	if (pPlace)
        m_ListPlace.push_back(pPlace);
}

//C2_Place * CC_Place::GetHead()
//{
//    if (!m_ListPlace.isEmpty())
//        return m_ListPlace.at(0);
//	else
//		return NULL;
//}

//C2_Place * CC_Place::GetTail()
//{
//    if (!m_ListPlace.isEmpty())
//        return m_ListPlace.last();
//	else
//		return NULL;
//}

//C2_Place * CC_Place::GetAt (int nIndex)
//{
////	POSITION nPos = m_ListPlace.FindIndex(nIndex);
////	if (nPos)
////		return m_ListPlace.GetAt(nPos);
////	else
//        return NULL;
//}

void * CC_Place::RemvHead()
{
    if (!m_ListPlace.isEmpty())
         m_ListPlace.pop_front();
	else
		return NULL;
}

void * CC_Place::RemvTail()
{
    if (!m_ListPlace.isEmpty())
         m_ListPlace.pop_back();
	else
		return NULL;
}

void CC_Place::RemvAt (int nIndex)
{

    if (m_ListPlace.size()<nIndex)
        m_ListPlace.removeAt(nIndex);
}


//-----------------------------------------------------------------------------
//	POSITION: get, remove
//-----------------------------------------------------------------------------



//C2_Place * CC_Place::GetPos(POSITION nPos)
//{
//	if (nPos)
//		return m_ListPlace.GetAt(nPos);
//	else
//		return NULL;
//}

//void CC_Place::RemvPos(POSITION nPos)
//{
//	if (nPos)
//		m_ListPlace.RemoveAt(nPos);
//}

//-----------------------------------------------------------------------------
//	Get new ID for data
//-----------------------------------------------------------------------------
//BYTE CC_Place::GetUid()
//{
//	C2_Place *	pPlace;
//	POSITION	nPos;
//	BYTE		nUid = 0;
//	bool		bFind= false;

//	while (!bFind)
//	{
//		nUid  ++;
//		bFind = true;
//		nPos  = m_ListPlace.GetHeadPosition();
//		while (nPos && bFind)
//		{
//			pPlace = m_ListPlace.GetNext(nPos);
//			if (pPlace->m_Uid == nUid)
//				bFind = false;
//		}
//	}

//	return nUid;
//}

//-----------------------------------------------------------------------------
//	Find a Place with nTRN
//-----------------------------------------------------------------------------
//C2_Place * CC_Place::Find(BYTE nUid)
//{
//	int		nIndex;
//	return  Find(nUid, nIndex);
//}

// Over load
//C2_Place * CC_Place::Find(BYTE nUid, int &nIndex)
//{
//	C2_Place *	pPlace;
//	POSITION	nPos;

//	nPos   = m_ListPlace.GetHeadPosition();
//	nIndex = 0;
//	while (nPos)
//	{
//		pPlace = m_ListPlace.GetNext(nPos);
//		if (pPlace->m_Uid == nUid)
//			return pPlace;

//		nIndex ++;
//	}
//	return NULL;
//}

//POSITION CC_Place::FindPos(int nIndex)
//{
//	return m_ListPlace.FindIndex(nIndex);
//}

//-----------------------------------------------------------------------------
// Command of data to list
//-----------------------------------------------------------------------------
//BYTE CC_Place::Command(BYTE nComm, C2_Place *pNewPl)
//{
//	int		nIndex;
//	return	Command(nComm, pNewPl, nIndex);
//}
/*
BYTE CC_Place::Command(BYTE nComm, C2_Place *pNewPl, int &nIndex)
{
	C2_Place *	pPlace;

	if (nComm == COMM_NEW) 
	{		
		nIndex = Count();
		if (nIndex > MAX_NUMB_PLACE)
			return COMM_NON;

		pPlace = new C2_Place();
		if (!pPlace)
			return COMM_NON;

		pNewPl->m_Uid = GetUid();
		pPlace->Update(pNewPl);			// Copy
		m_ListPlace.AddTail(pPlace);
		return COMM_NEW;
	}
	else 
	if (nComm == COMM_UPD)
	{
		pPlace = Find(pNewPl->m_Uid, nIndex);
		if (!pPlace)
			return COMM_NON;

		pPlace->Update(pNewPl);
		return COMM_UPD;
	}
	else
	if (nComm == COMM_DEL)
	{
		pPlace = Find(pNewPl->m_Uid, nIndex);
		if (!pPlace)
			return COMM_NON;

		delete pPlace;
		RemvAt(nIndex);
		return COMM_DEL;
	}
	else
		return COMM_NON;
}
*/

//-----------------------------------------------------------------------------
// Update data to list
//-----------------------------------------------------------------------------
//BYTE CC_Place::Update(BYTE nComm, C2_Place *pNewPl)
//{
//	int		nIndex;
//	return	Update(nComm, pNewPl, nIndex);
//}

//BYTE CC_Place::Update(BYTE nComm, C2_Place *pNewPl, int &nIndex)
//{
//	C2_Place *	pPlace;

//	if((nComm == COMM_NEW) || (nComm == COMM_UPD))
//	{
//		pPlace = Find(pNewPl->m_Uid, nIndex);
//		if (!pPlace)
//		{
//			nIndex = Count();
//			if (nIndex > MAX_NUMB_PLACE)
//				return COMM_NON;

//			pPlace = new C2_Place();
//			if (!pPlace)
//				return COMM_NON;

//			pNewPl->m_Uid = GetUid();
//			pPlace->Update(pNewPl);			// Copy
//			m_ListPlace.AddTail(pPlace);
//			return COMM_NEW;
//		}
//		else
//		{
//			pPlace->Update(pNewPl);
//			return COMM_UPD;
//		}
//	}
//	else
//	if (nComm == COMM_DEL)
//	{
//		pPlace = Find(pNewPl->m_Uid, nIndex);
//		if (!pPlace)
//			return COMM_NON;
//		else
//		{
//			delete pPlace;
//			RemvAt(nIndex);
//			return COMM_DEL;
//		}
//	}
//	else
//		return COMM_NON;
//}
