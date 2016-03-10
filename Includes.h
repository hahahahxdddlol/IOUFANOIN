#pragma once

#define CLIENT_DLL
#define WIN32_LEAN_AND_MEAN

//Windows
#include <windows.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <Shlwapi.h>

//SourceSDK
#include "public/tier0/wchartypes.h"
#include "public/tier0/basetypes.h"
#include "public/cdll_int.h"
#include "public/iprediction.h"
#include "public/bone_setup.h"
#include "public/icliententitylist.h"
#include "public/ienginevgui.h"
#include "public/dlight.h"
#include "public/iefx.h" 
#include "public/igameevents.h"
#include "public/view_shared.h"
#include "public/inetchannelinfo.h"
#include "public/texture_group_names.h"
#include "game/client/imessagechars.h"
#include "game/client/cliententitylist.h"
#include "game/client/cdll_client_int.h"
#include "game/client/cbase.h"
#include "game/client/c_baseanimating.h"
#include "game/client/c_basecombatweapon.h"
#include "game/client/c_baseplayer.h"
#include "game/client/input.h"
#include "game/client/prediction.h"
#include "game/shared/gamemovement.h"
#include "game/shared/basecombatweapon_shared.h"
#include "game/shared/in_buttons.h"
#include "game/shared/decals.h"
#include "public/engine/ivmodelrender.h"
#include "public/engine/ivmodelinfo.h"
#include "public/engine/IEngineTrace.h"
#include "public/materialsystem/imaterialsystemstub.h"
#include "public/materialsystem/itexture.h"
#include "public/materialsystem/IMaterialVar.h"
#include "public/vgui/IPanel.h"
#include "public/vgui/ISurface.h"
#include "public/VGuiMatSurface/IMatSystemSurface.h"

struct ScreenSize_t
{
	int iWidth;
	int iHeight;
};

class CTraceFilterNoPlayers : public ITraceFilter
{
public:
	bool ShouldHitEntity(IHandleEntity* pEntityHandle, int contentsMask)
	{
		return !(pEntityHandle == pSkip);
	}

	virtual TraceType_t GetTraceType() const
	{
		return TRACE_EVERYTHING;
	}

	void* pSkip;
};

struct CCSWeaponInfo
{
	BYTE	__pad0[0x7C4];
	float	m_flPenetration;
	int		m_iDamage;
	float	m_flRange;
	float	m_flRangeModifier;
	int		m_iBullets;
};

//Includes
#include "vtable.h"
#include "n0xius.h"
#include "NetVars.h"
#include "draw.h"
#include "Util.h"
#include "esp.h"
#include "Cvars.h"
#include "Menu.h"

//externs
extern HMODULE hModul;
extern HMODULE hClient;
extern HMODULE hEngine;
extern BYTE bSendPacket;
extern ScreenSize_t sScreenSize;
extern DWORD dwClientState;
extern DWORD dwForceUpdate;

//Defines
extern void Print(const char *pszText, ...);
extern void AntiAntiAimXProxy(const CRecvProxyData *pData, void *pStruct, void *pOut);
extern void RecvProxy_Viewmodel(const CRecvProxyData* pData, void* pStruct, void* pOut);
extern void RecvProxy_Sequence(const CRecvProxyData* pData, void* pStruct, void* pOut);
extern bool GetVisible(Vector& vAbsStart, Vector& vAbsEnd, IClientEntity* pEntity);

typedef void(__stdcall* PaintTraverse_t)(vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce);
typedef int(__stdcall* IN_KeyEvent_t)(int eventcode, ButtonCode_t keynum, const char *pszCurrentBinding);
typedef void(__stdcall* CreateMove_t)(int, float, bool);
typedef void(__stdcall* DrawModelExecute_t)(IMatRenderContext* pRender, const DrawModelState_t &state, const ModelRenderInfo_t &pInfo, matrix3x4_t *pCustomBoneToWorld);
typedef void(__stdcall* FrameStageNotify_t)(ClientFrameStage_t);
typedef void(__stdcall* RunCommand_t)(C_BasePlayer*, CUserCmd*, IMoveHelper*);

extern void __stdcall new_CreateMove(int sequence_number);
extern void __stdcall new_PaintTraverse(vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce);
extern void __stdcall new_DrawModelExecute(IMatRenderContext* pRender, const DrawModelState_t &state, const ModelRenderInfo_t &pInfo, matrix3x4_t *pCustomBoneToWorld);
extern void __stdcall new_FrameStageNotify(ClientFrameStage_t curStage);