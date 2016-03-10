#include "Includes.h"

HMODULE hClient = NULL,
		hEngine = NULL,
		hModul = NULL;
DWORD	dwOrigCreateMove = NULL,
		dwClientState = NULL,
		dwForceUpdate = NULL;
BYTE	bSendPacket;
ScreenSize_t sScreenSize;

void CreateConsole()
{
	if (AllocConsole())
	{
		freopen("CONOUT$", "wt", stdout);
		SetConsoleTitleA("Debug Console");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
	}
}

void Print(const char *pszText, ...)
{
#ifdef DEBUG
	const char* In = pszText;
	char* Arg = (char*)&pszText + _INTSIZEOF(pszText);

	if (In == NULL || Arg == NULL)
		return;

	char Out[0x1000];
	int len = vsprintf(Out, In, Arg);

	Out[len + 0] = '\r';
	Out[len + 1] = '\n';
	Out[len + 2] = '\0';
	len = len + 2;

	printf(Out);
#endif
}

void* GetInterface(const char* pszModuleName, const char* pszInterfaceName)
{
	HMODULE hModule = GetModuleHandleA(pszModuleName);

	if (!hModule)
		return nullptr;

	CreateInterfaceFn pCreateInterface = (CreateInterfaceFn)GetProcAddress(hModule, "CreateInterface");

	if (!pCreateInterface)
		return nullptr;

	void* pReturn = nullptr;

	char szBuffer[1024];

	for (int i = 0; i < 1000; ++i)
	{
		if (i > 99)
			sprintf(szBuffer, "%s%i", pszInterfaceName, i);
		else if (i > 9)
			sprintf(szBuffer, "%s0%i", pszInterfaceName, i);
		else
			sprintf(szBuffer, "%s00%i", pszInterfaceName, i);

		pReturn = pCreateInterface(szBuffer, nullptr);

		if (pReturn)
		{
			#ifdef DEBUG
				Print("Found: [%s]", szBuffer);
			#endif
			break;
		}
	}

	return pReturn;
}

void UTIL_ClipTraceToPlayers(const Vector& vecAbsStart, const Vector& vecAbsEnd, unsigned int mask, ITraceFilter *filter, trace_t *tr)
{
	trace_t playerTrace;
	Ray_t ray;
	float smallestFraction = tr->fraction;
	const float maxRange = 60.0f;

	ray.Init(vecAbsStart, vecAbsEnd);

	for (int k = 1; k <= Kardoffl.m_pEntlist->GetHighestEntityIndex(); ++k)
	{
		C_BasePlayer *player = (C_BasePlayer*)Kardoffl.m_pEntlist->GetClientEntity(k);

		if (!player || !Kardoffl.m_pEntity->GetLifeState(player))
			continue;

		if (filter && !filter->ShouldHitEntity(player, mask))
			continue;

		float range = DistanceToRay(player->WorldSpaceCenter(), vecAbsStart, vecAbsEnd);
		if (range < 0.0f || range > maxRange)
			continue;

		Kardoffl.m_pEnginetrace->ClipRayToEntity(ray, mask | CONTENTS_HITBOX, player, &playerTrace);
		if (playerTrace.fraction < smallestFraction)
		{
			// we shortened the ray - save off the trace
			*tr = playerTrace;
			smallestFraction = playerTrace.fraction;
		}
	}
}

bool CGameTrace::DidHitWorld() const
{
	return m_pEnt == Kardoffl.m_pEntlist->GetClientEntity(0);
}

// Returns true if we hit something and it wasn't the world.
bool CGameTrace::DidHitNonWorldEntity() const
{
	return m_pEnt != NULL && !DidHitWorld();
}

// Gets the entity's network index if the trace has hit an entity.
// If not, returns -1.
int CGameTrace::GetEntityIndex() const
{
	if (m_pEnt)
		return m_pEnt->GetIClientEntity()->entindex();
	else
		return -1;
}

void __declspec(naked) __fastcall hkCreateMove(CInput* thisptr, void* _EAX, int sequence_number, float input_sample_frametime, bool active)
{
	__asm
	{
		mov     bSendPacket, bl
		push    ebp
		mov     ebp, esp
		sub     esp, 8
		pusha
		push	[ebp + 10h]
		push	[ebp + 0Ch]
		push	[ebp + 08h]
		call    dwOrigCreateMove
	}

	new_CreateMove(sequence_number);

	__asm
	{
		popa
		mov     bl, bSendPacket
		mov     esp, ebp
		pop     ebp
		retn    0Ch
	}
}

void __stdcall new_RunCommand(C_BasePlayer* pPlayer, CUserCmd* pCmd, IMoveHelper* pMoveHelper)
{
	Kardoffl.m_pPredictionVMT->Function<RunCommand_t>(19)(pPlayer, pCmd, pMoveHelper);

	if (Kardoffl.m_pMoveHelper == NULL && pMoveHelper != NULL)
		Kardoffl.m_pMoveHelper = pMoveHelper;
}

void GetInterfaces()
{
	//Client
	Kardoffl.m_pClient = reinterpret_cast<IBaseClientDLL*>(GetInterface("client.dll", "VClient"));
#ifdef DEBUG
	Print("-> Kardoffl.m_pClient: [0x%x]", (DWORD)Kardoffl.m_pClient);
#endif
	DWORD dwClient = (DWORD)Kardoffl.m_pClient;
	Kardoffl.m_pClientVMT = new C_VMT(&dwClient);

	DWORD dwInputPointer = Kardoffl.m_pUtilities->dwFindPattern(Kardoffl.m_pClientVMT->Function<DWORD>(21), 0x100, (BYTE*)"\x24\xB9", "xx");

	if (dwInputPointer != NULL)
	{
		dwInputPointer += 0x2;
		Kardoffl.m_pInput = *(CInput**)dwInputPointer;
	}
#ifdef DEBUG
	Print("-> Kardoffl.m_pInput: [0x%x]\n", (DWORD)Kardoffl.m_pInput);
#endif
	Kardoffl.m_pEntlist =	reinterpret_cast<IClientEntityList*>(GetInterface("client.dll", "VClientEntityList"));
#ifdef DEBUG
	Print("-> Kardoffl.m_pEntlist: [0x%x]\n", (DWORD)Kardoffl.m_pEntlist);
#endif
	Kardoffl.m_pPrediction = reinterpret_cast<CPrediction*>(GetInterface("client.dll", "VClientPrediction"));
#ifdef DEBUG
	Print("-> Kardoffl.m_pPrediction: [0x%x]\n", (DWORD)Kardoffl.m_pPrediction);
#endif
	DWORD dwPrediction = (DWORD)Kardoffl.m_pPrediction;
	Kardoffl.m_pPredictionVMT = new C_VMT(&dwPrediction);

	Kardoffl.m_pGameMovement = reinterpret_cast<CGameMovement*>(GetInterface("client.dll", "GameMovement"));
#ifdef DEBUG
	Print("-> Kardoffl.m_pGameMovement: [0x%x]\n", (DWORD)Kardoffl.m_pGameMovement);
#endif

	//Engine
	Kardoffl.m_pEngine = reinterpret_cast<IVEngineClient*>(GetInterface("engine.dll", "VEngineClient"));
#ifdef DEBUG
	Print("-> Kardoffl.m_pEngine: [0x%x]\n", (DWORD)Kardoffl.m_pEngine);
#endif
	Kardoffl.m_pEnginetrace = reinterpret_cast<IEngineTrace*>(GetInterface("engine.dll", "EngineTraceClient"));
#ifdef DEBUG
	Print("-> Kardoffl.m_pEnginetrace: [0x%x]\n", (DWORD)Kardoffl.m_pEnginetrace);
#endif
	Kardoffl.m_pModelinfo = reinterpret_cast<IVModelInfo*>(GetInterface("engine.dll", "VModelInfoClient"));
#ifdef DEBUG
	Print("-> Kardoffl.m_pModelinfo: [0x%x]\n", (DWORD)Kardoffl.m_pModelinfo);
#endif
	Kardoffl.m_pModelRender = reinterpret_cast<IVModelRender*>(GetInterface("engine.dll", "VEngineModel"));
#ifdef DEBUG
	Print("-> Kardoffl.m_pModelRender: [0x%x]\n", (DWORD)Kardoffl.m_pModelRender);
#endif
	DWORD dwModelRender = (DWORD)Kardoffl.m_pModelRender;
	Kardoffl.m_pModelRenderVMT = new C_VMT(&dwModelRender);

	Kardoffl.m_pRender = reinterpret_cast<IVRenderView*>(GetInterface("engine.dll", "VEngineRenderView"));
#ifdef DEBUG
	Print("-> Kardoffl.m_pRender: [0x%x]\n", (DWORD)Kardoffl.m_pRender);
#endif
	
	//Material
	Kardoffl.m_pMaterialSystem = reinterpret_cast<IMaterialSystem*>(GetInterface("MaterialSystem.dll", "VMaterialSystem"));
#ifdef DEBUG
	Print("-> Kardoffl.m_pMaterialSystem: [0x%x]\n", (DWORD)Kardoffl.m_pMaterialSystem);
#endif

	//vphysics
	Kardoffl.m_pPhysicAPI = reinterpret_cast<IPhysicsSurfaceProps*>(GetInterface("vphysics.dll", "VPhysicsSurfaceProps"));
#ifdef DEBUG
	Print("-> Kardoffl.m_pPhysicAPI: [0x%x]\n", (DWORD)Kardoffl.m_pPhysicAPI);
#endif

	//vguimatsurface
	Kardoffl.m_pMatSurface = reinterpret_cast<vgui::ISurface*>(GetInterface("vguimatsurface.dll", "VGUI_Surface"));
#ifdef DEBUG
	Print("-> Kardoffl.m_pMatSurface: [0x%x]\n", (DWORD)Kardoffl.m_pMatSurface);
#endif

	//vgui2
	Kardoffl.m_pPanel = reinterpret_cast<vgui::IPanel*>(GetInterface("vgui2.dll", "VGUI_Panel"));
#ifdef DEBUG
	Print("-> Kardoffl.m_pPanel: [0x%x]\n", (DWORD)Kardoffl.m_pPanel);
#endif
	DWORD dwPanel = (DWORD)Kardoffl.m_pPanel;
	Kardoffl.m_pPanelVMT = new C_VMT(&dwPanel);

	//vstdlib
	Kardoffl.m_pCvar = reinterpret_cast<ICvar*>(GetInterface("vstdlib.dll", "VEngineCvar"));
#ifdef DEBUG
	Print("-> Kardoffl.m_pCvar: [0x%x]\n", (DWORD)Kardoffl.m_pCvar);
#endif

	Kardoffl.m_pGlobals = (CGlobalVarsBase*)*(PDWORD)*(PDWORD)(Kardoffl.m_pUtilities->dwFindPattern((DWORD)GetModuleHandleA("client.dll"), 0x7CE000, (BYTE*)"\xA1\x00\x00\x00\x00\x8B\x40\x10\x89\x41\x04", "x????xxxxxx") + 0x1);
#ifdef DEBUG
	Print("Found Kardoffl.m_pGlobals: [0x%x]\n", (DWORD)Kardoffl.m_pGlobals);
#endif

	dwClientState = **(DWORD**)(Kardoffl.m_pUtilities->dwFindPattern((DWORD)GetModuleHandleA("engine.dll"), 0x438000, (BYTE*)"\x8B\x3D\x00\x00\x00\x00\x8A\xF9", "xx????xx") + 2);
#ifdef DEBUG
	Print("Found ClientState at 0x%x", dwClientState);
#endif
	dwForceUpdate = Kardoffl.m_pUtilities->dwFindPattern((DWORD)GetModuleHandleA("engine.dll"), 0x438000, (BYTE*)"\xE8\x00\x00\x00\x00\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\xC7\x87\x00\x00\x00\x00\x00\x00\x00\x00", "x????x????x????xx????????");
#ifdef DEBUG
	Print("Found ForceUpdate at 0x%x", dwForceUpdate);
#endif
}

void SetupHooks()
{
	for (; hClient == NULL; Sleep(100))
		hClient = (HMODULE)GetModuleHandleA("client.dll");

	for (; hEngine == NULL; Sleep(100))
		hEngine = (HMODULE)GetModuleHandleA("engine.dll");

	GetInterfaces();

	Kardoffl.m_pNetVars->GetOffsets();
	Kardoffl.m_pMenu->InitConfig(0);

	ClientClass *pClass = Kardoffl.m_pClient->GetAllClasses();
	while (pClass)
	{
		const char *pszName = pClass->m_pRecvTable->GetName();

		if (!strcmp(pszName, "DT_CSPlayer"))
		{
			for (int i = 0; i < pClass->m_pRecvTable->m_nProps; i++)
			{
				RecvProp *pProp = pClass->m_pRecvTable->GetProp(i);

				if (!strcmp(pProp->GetName(), "m_angEyeAngles[0]"))
					pProp->SetProxyFn(AntiAntiAimXProxy);
			}
		}
		if (!strcmp(pszName, "DT_BaseViewModel"))
		{
			for (int i = 0; i < pClass->m_pRecvTable->m_nProps; i++)
			{
				RecvProp *pProp = pClass->m_pRecvTable->GetProp(i);

				if (!strcmp(pProp->GetName(), "m_nModelIndex"))
					pProp->SetProxyFn(RecvProxy_Viewmodel);

				if (!strcmp(pProp->GetName(), "m_nSequence"))
					pProp->SetProxyFn(RecvProxy_Sequence);
			}
		}

		pClass = pClass->m_pNext;
	}

	Kardoffl.m_pPanelVMT->Hook((DWORD)new_PaintTraverse, 41);
	Kardoffl.m_pPredictionVMT->Hook((DWORD)new_RunCommand, 19);
	Kardoffl.m_pClientVMT->Hook((DWORD)hkCreateMove, 21);
	Kardoffl.m_pClientVMT->Hook((DWORD)new_FrameStageNotify, 36);
	dwOrigCreateMove = Kardoffl.m_pClientVMT->dwOldPointer(21);

	Kardoffl.m_pDraw->FontInit(Kardoffl.m_pFont, "Tahoma", 13, FONTFLAG_OUTLINE);
	Kardoffl.m_pEngine->GetScreenSize(sScreenSize.iWidth, sScreenSize.iWidth);

	ExitThread(1);
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpvReserved)
{
	DisableThreadLibraryCalls(hModule);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		hModul = hModule;
#ifdef DEBUG
		CreateConsole();
		Print("Kardofflhook by kardoffl. Credits to: n0xius");
#endif
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SetupHooks, 0, 0, 0);
	}
	return TRUE;
}