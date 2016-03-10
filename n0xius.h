#pragma once

#include "Includes.h"

class C_Offsets;
class C_NetVars;
class CEntity;
class CDraw;
class CUtilities;
class CVars;
class CESP;
class CConVars;
class CMouse;
class CMenu;

class Interfaces
{
public:
	Interfaces::Interfaces();
	IBaseClientDLL*			m_pClient;
	CInput*					m_pInput;
	IVEngineClient*			m_pEngine;
	CGlobalVarsBase*		m_pGlobals;
	vgui::ISurface*			m_pMatSurface;
	vgui::IPanel*			m_pPanel;
	IClientEntityList*		m_pEntlist;
	IEngineTrace*			m_pEnginetrace;
	IVModelInfo*			m_pModelinfo;
	ICvar*					m_pCvar;
	IVModelRender*			m_pModelRender;
	IMaterialSystem*		m_pMaterialSystem;
	IVRenderView*			m_pRender;
	IGameEventManager2*		m_pGameEventManager;
	CPrediction*			m_pPrediction;
	CGameMovement*			m_pGameMovement;
	IMoveHelper*			m_pMoveHelper;
	IPhysicsSurfaceProps*	m_pPhysicAPI; 
public:
	C_VMT*					m_pClientVMT;
	C_VMT*					m_pPanelVMT;
	C_VMT*					m_pModelRenderVMT;
	C_VMT*					m_pPredictionVMT;
	C_VMT*					m_pMDLCacheVMT;
	vgui::HFont				m_pFont;
public:
	C_NetVars*				m_pNetVars;
	C_Offsets*				m_pOffsets;
	CEntity*				m_pEntity;
	CDraw*					m_pDraw;
	CUtilities*				m_pUtilities;
	CESP*					m_pESP;
	CConVars*				m_pConvars;
	CMouse*					m_pMouse;
	CMenu*					m_pMenu;
};

extern Interfaces Kardoffl;