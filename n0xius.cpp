#include "n0xius.h"

Interfaces Kardoffl;

Interfaces::Interfaces()
{
	m_pOffsets = new C_Offsets();
	m_pNetVars = new C_NetVars();
	m_pEntity = new CEntity();
	m_pDraw = new CDraw();
	m_pUtilities = new CUtilities();
	m_pESP = new CESP();
	m_pConvars = new CConVars();
	m_pMouse = new CMouse();
	m_pMenu = new CMenu();
}