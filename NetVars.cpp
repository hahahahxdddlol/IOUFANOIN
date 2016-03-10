#include "NetVars.h"

IClientEntity* CEntity::Ent()
{
	return Kardoffl.m_pEntlist->GetClientEntity(Kardoffl.m_pEngine->GetLocalPlayer());
}

bool CEntity::GetLifeState(IClientEntity* pBaseEnt)
{
	char szLifeState = *(char*)((DWORD)pBaseEnt + Kardoffl.m_pOffsets->LifeState);
	return (szLifeState == 0);
}

int CEntity::GetFlags(IClientEntity* pBaseEnt)
{
	return *(int*)((DWORD)pBaseEnt + Kardoffl.m_pOffsets->Flags);
}

QAngle CEntity::GetPunchAngle(IClientEntity* pBaseEnt)
{
	return *(QAngle*)((DWORD)pBaseEnt + Kardoffl.m_pOffsets->PunchAngle);
}

int CEntity::GetTeamID(IClientEntity* pBaseEnt)
{
	return *(int*)((DWORD)pBaseEnt + Kardoffl.m_pOffsets->TeamNum);
}

int CEntity::GetTickBase(IClientEntity* pBaseEnt)
{
	return *(int*)((DWORD)pBaseEnt + Kardoffl.m_pOffsets->TickBase);
}

Vector CEntity::GetVelocity(IClientEntity* pBaseEnt)
{
	return *(Vector*)((DWORD)pBaseEnt + Kardoffl.m_pOffsets->Velocity);
}

int CEntity::GetHealth(IClientEntity* pBaseEnt)
{
	return *(int*)((DWORD)pBaseEnt + Kardoffl.m_pOffsets->Health);
}

MoveType_t CEntity::GetMoveType(IClientEntity* pBaseEnt)
{
	return *(MoveType_t*)((DWORD)pBaseEnt + 0x258);//hardcoded weil kein NetVar
}

bool CEntity::HasAmmo(C_BaseCombatWeapon* m_pWeapon)
{
	int iAmmo = *(int*)((DWORD)m_pWeapon + Kardoffl.m_pOffsets->Clip1);
	
	if (iAmmo == 0)
		return false;
	else
		return true;
}

float CEntity::GetNextPrimaryAttack(C_BaseCombatWeapon* m_pWeapon)
{
	return *(float*)((DWORD)m_pWeapon + Kardoffl.m_pOffsets->NextPrimaryAttack);
}

C_BaseCombatWeapon* CEntity::GetBaseCombatWeapon(IClientEntity* pBaseEntity)
{
	EHANDLE hActiveWeapon = *reinterpret_cast< EHANDLE* >((DWORD)pBaseEntity + (DWORD)Kardoffl.m_pOffsets->ActiveWeapon);
	return dynamic_cast< C_BaseCombatWeapon* >(Kardoffl.m_pEntlist->GetClientEntityFromHandle(hActiveWeapon));
}

int CEntity::GetWeaponID(C_BaseCombatWeapon* m_pWeapon)
{
	return *(int*)((DWORD)m_pWeapon + Kardoffl.m_pOffsets->ItemID);
}

Vector CEntity::GetEyePos(IClientEntity* pBaseEnt)
{
	Vector m_VecOrigin = *(Vector*)((DWORD)pBaseEnt + Kardoffl.m_pOffsets->Origin);
	Vector m_vecViewOffset = *(Vector*)((DWORD)pBaseEnt + Kardoffl.m_pOffsets->ViewOffset);

	return (m_VecOrigin + m_vecViewOffset);
}

Vector CEntity::GetOrigin(IClientEntity* pBaseEnt)
{
	return *(Vector*)((DWORD)pBaseEnt + Kardoffl.m_pOffsets->Origin);
}

int CEntity::GetShotsFired(IClientEntity* pBaseEnt)
{
	return *(int*)((DWORD)pBaseEnt + Kardoffl.m_pOffsets->ShotsFired);
}

QAngle CEntity::GetAngles(IClientEntity* pBaseEnt)
{
	return *(QAngle*)((DWORD)pBaseEnt + Kardoffl.m_pOffsets->Angles);
}

int CEntity::GetHitboxSet(IClientEntity* pBaseEnt)
{
	return *(int*)((DWORD)pBaseEnt + Kardoffl.m_pOffsets->HitboxSet);
}

color32 CEntity::GetRenderColor(IClientEntity* pBaseEnt)
{
	return *(color32*)((DWORD)pBaseEnt + Kardoffl.m_pOffsets->clrRender);
}

int C_NetVars::GetOffset(const char *tableName, const char *propName)
{
	int offset = GetProp(tableName, propName);

	if (!offset)
		return 0;

	return offset;
}

RecvVarProxyFn C_NetVars::HookProp(const char *tableName, const char *propName, RecvVarProxyFn function)
{
	RecvProp *recvProp = 0;

	if (!GetProp(tableName, propName, &recvProp))
	{
		return 0;
	}

	RecvVarProxyFn old = recvProp->m_ProxyFn;
	recvProp->m_ProxyFn = function;

	return old;
}

int C_NetVars::GetProp(const char *tableName, const char *propName, RecvProp **prop)
{
	RecvTable *recvTable = GetTable(tableName);

	if (!recvTable)
		return 0;

	int offset = GetProp(recvTable, propName, prop);

	if (!offset)
		return 0;

	return offset;
}

int C_NetVars::GetProp(RecvTable *recvTable, const char *propName, RecvProp **prop)
{
	int extraOffset = 0;

	for (int i = 0; i < recvTable->m_nProps; ++i)
	{
		RecvProp *recvProp = &recvTable->m_pProps[i];

		RecvTable *child = recvProp->m_pDataTable;

		if (child
			&& (child->m_nProps > 0))
		{
			int tmp = GetProp(child, propName, prop);

			if (tmp)
			{
				extraOffset += (recvProp->m_Offset + tmp);
			}
		}

		if (stricmp(recvProp->m_pVarName, propName))
			continue;

		if (prop && !*prop)
			*prop = recvProp;

		return (recvProp->m_Offset + extraOffset);
	}

	return extraOffset;
}


RecvTable *C_NetVars::GetTable(const char *tableName)
{
	if (m_tables.empty())
		return 0;

	for each (RecvTable *table in m_tables)
	{
		if (!table)
			continue;

		if (stricmp(table->m_pNetTableName, tableName) == 0)
			return table;
	}

	return 0;
}

void C_NetVars::GetOffsets(void)
{
	m_tables.clear();

	ClientClass *clientClass = Kardoffl.m_pClient->GetAllClasses();

	if (!clientClass)
		return;

	while (clientClass)
	{
		RecvTable *recvTable = clientClass->m_pRecvTable;

		m_tables.push_back(recvTable);

		clientClass = clientClass->m_pNext;
	}

	Kardoffl.m_pOffsets->Flags =				GetOffset("DT_BasePlayer", "m_fFlags");
	Kardoffl.m_pOffsets->FOV =				GetOffset("DT_BasePlayer", "m_iFov");
	Kardoffl.m_pOffsets->Health =				GetOffset("DT_BasePlayer", "m_iHealth");
	Kardoffl.m_pOffsets->LifeState =			GetOffset("DT_BasePlayer", "m_lifeState");
	Kardoffl.m_pOffsets->PunchAngle =			GetOffset("DT_BasePlayer", "m_aimPunchAngle");
	Kardoffl.m_pOffsets->ViewPunchAngle =		GetOffset("DT_BasePlayer", "m_viewPunchAngle");
	Kardoffl.m_pOffsets->TeamNum =			GetOffset("DT_BasePlayer", "m_iTeamNum");
	Kardoffl.m_pOffsets->TickBase =			GetOffset("DT_BasePlayer", "m_nTickBase");
	Kardoffl.m_pOffsets->Velocity =			GetOffset("DT_BasePlayer", "m_vecVelocity[0]");
	Kardoffl.m_pOffsets->ViewOffset =			GetOffset("DT_BasePlayer", "m_vecViewOffset[0]");
	Kardoffl.m_pOffsets->Origin =				GetOffset("DT_BasePlayer", "m_vecOrigin");
	Kardoffl.m_pOffsets->clrRender =			GetOffset("DT_BaseEntity", "m_clrRender");
	Kardoffl.m_pOffsets->HitboxSet =			GetOffset("DT_BaseAnimating", "m_nHitboxSet");
	Kardoffl.m_pOffsets->ShotsFired =			GetOffset("DT_CSPlayer", "m_iShotsFired");
	Kardoffl.m_pOffsets->Angles =				GetOffset("DT_CSPlayer", "m_angEyeAngles");
	Kardoffl.m_pOffsets->OwnerXuidLow =		GetOffset("DT_BaseAttributableItem", "m_OriginalOwnerXuidLow");
	Kardoffl.m_pOffsets->OwnerXuidHigh =		GetOffset("DT_BaseAttributableItem", "m_OriginalOwnerXuidHigh");
	Kardoffl.m_pOffsets->PaintKit =			GetOffset("DT_BaseAttributableItem", "m_nFallbackPaintKit");
	Kardoffl.m_pOffsets->Wear =				GetOffset("DT_BaseAttributableItem", "m_flFallbackWear");
	Kardoffl.m_pOffsets->StatTrak =			GetOffset("DT_BaseAttributableItem", "m_nFallbackStatTrak");
	Kardoffl.m_pOffsets->Seed =				GetOffset("DT_BaseAttributableItem", "m_nFallbackSeed");
	Kardoffl.m_pOffsets->ItemIDHigh =			GetOffset("DT_BaseAttributableItem", "m_iItemIDHigh");
	Kardoffl.m_pOffsets->CustomName =			GetOffset("DT_BaseAttributableItem", "m_szCustomName");
	Kardoffl.m_pOffsets->ItemID =				GetOffset("DT_BaseAttributableItem", "m_iItemDefinitionIndex");
	Kardoffl.m_pOffsets->ActiveWeapon =		GetOffset("DT_BaseCombatCharacter", "m_hActiveWeapon");
	Kardoffl.m_pOffsets->Clip1 =				GetOffset("DT_BaseCombatWeapon", "m_iClip1");
	Kardoffl.m_pOffsets->NextPrimaryAttack =	GetOffset("DT_BaseCombatWeapon", "m_flNextPrimaryAttack");
	Kardoffl.m_pOffsets->ModelIndex =			GetOffset("DT_BaseCombatWeapon", "m_nModelIndex");
	Kardoffl.m_pOffsets->ViewModelIndex =		GetOffset("DT_BaseCombatWeapon", "m_iViewModelIndex");
	Kardoffl.m_pOffsets->WorldModelIndex =	GetOffset("DT_BaseCombatWeapon", "m_iWorldModelIndex");
	Kardoffl.m_pOffsets->OwnerEntity =		GetOffset("DT_BaseEntity", "m_hOwnerEntity");

#ifdef DEBUG
	Print("NetVars:");
	Print("Flags: 0x%x", Kardoffl.m_pOffsets->Flags);
	Print("FOV: 0x%x", Kardoffl.m_pOffsets->FOV);
	Print("Health: 0x%x", Kardoffl.m_pOffsets->Health);
	Print("LifeState: 0x%x", Kardoffl.m_pOffsets->LifeState);
	Print("PunchAngle: 0x%x", Kardoffl.m_pOffsets->PunchAngle);
	Print("ViewPunchAngle: 0x%x", Kardoffl.m_pOffsets->ViewPunchAngle);
	Print("TeamNum: 0x%x", Kardoffl.m_pOffsets->TeamNum);
	Print("TickBase: 0x%x", Kardoffl.m_pOffsets->TickBase);
	Print("Velocity: 0x%x", Kardoffl.m_pOffsets->Velocity);
	Print("ViewOffset: 0x%x", Kardoffl.m_pOffsets->ViewOffset);
	Print("Origin: 0x%x", Kardoffl.m_pOffsets->Origin);
	Print("Angles: 0x%x", Kardoffl.m_pOffsets->Angles);
	Print("ShotsFired: 0x%x", Kardoffl.m_pOffsets->ShotsFired);
	Print("Clip1: 0x%x", Kardoffl.m_pOffsets->Clip1);
	Print("NextPrimaryAttack: 0x%x", Kardoffl.m_pOffsets->NextPrimaryAttack);
	Print("ActiveWeapon: 0x%x", Kardoffl.m_pOffsets->ActiveWeapon);
	Print("HitboxSet: 0x%x", Kardoffl.m_pOffsets->HitboxSet);
	Print("clrRender: 0x%x", Kardoffl.m_pOffsets->clrRender);
	Print("OwnerXuidLow: 0x%x", Kardoffl.m_pOffsets->OwnerXuidLow);
	Print("OwnerXuidHigh: 0x%x", Kardoffl.m_pOffsets->OwnerXuidHigh);
	Print("PaintKit: 0x%x", Kardoffl.m_pOffsets->PaintKit);
	Print("Wear: 0x%x", Kardoffl.m_pOffsets->Wear);
	Print("StatTrak: 0x%x", Kardoffl.m_pOffsets->StatTrak);
	Print("Seed: 0x%x", Kardoffl.m_pOffsets->Seed);
	Print("iItemIDHigh: 0x%x", Kardoffl.m_pOffsets->ItemIDHigh);
	Print("szCustomName: 0x%x", Kardoffl.m_pOffsets->CustomName);
	Print("iItemDefinitionIndex: 0x%x", Kardoffl.m_pOffsets->ItemID);
	Print("ModelIndex: 0x%x", Kardoffl.m_pOffsets->ModelIndex);
	Print("ViewModelIndex: 0x%x", Kardoffl.m_pOffsets->ViewModelIndex);
	Print("WorldModelIndex: 0x%x", Kardoffl.m_pOffsets->WorldModelIndex);
	Print("OwnerEntity: 0x%x", Kardoffl.m_pOffsets->OwnerEntity);
#endif

	//Orig_RecvProxy_Viewmodel = HookProp("DT_BaseCombatWeapon", "m_nModelIndex", (RecvVarProxyFn)hkRecvProxy_Viewmodel);
}
