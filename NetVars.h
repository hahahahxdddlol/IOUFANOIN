#ifndef __NETVAR_H__
#define __NETVAR_H__

#include "Includes.h"

class CEntity
{
public:
	IClientEntity* Ent();
	bool GetLifeState(IClientEntity* pBaseEnt);
	int GetFlags(IClientEntity* pBaseEnt);
	QAngle GetPunchAngle(IClientEntity* pBaseEnt);
	int GetTeamID(IClientEntity* pBaseEnt);
	int GetTickBase(IClientEntity* pBaseEnt);
	Vector GetVelocity(IClientEntity* pBaseEnt);
	int GetHealth(IClientEntity* pBaseEnt);
	MoveType_t GetMoveType(IClientEntity* pBaseEnt);
	bool HasAmmo(C_BaseCombatWeapon* m_pWeapon);
	float GetNextPrimaryAttack(C_BaseCombatWeapon* m_pWeapon);
	C_BaseCombatWeapon* GetBaseCombatWeapon(IClientEntity* pBaseEntity);
	int GetWeaponID(C_BaseCombatWeapon* m_pWeapon);
	Vector GetEyePos(IClientEntity* pBaseEnt);
	Vector GetOrigin(IClientEntity* pBaseEnt);
	int GetShotsFired(IClientEntity* pBaseEnt);
	QAngle GetAngles(IClientEntity* pBaseEnt);
	int GetHitboxSet(IClientEntity* pBaseEnt);
	color32 GetRenderColor(IClientEntity* pBaseEnt);
};

class C_NetVars
{
public:
	void GetOffsets();

	// calls GetProp wrapper to get the absolute offset of the prop
	int GetOffset(const char *tableName, const char *propName);

	// calls GetProp wrapper to get prop and sets the proxy of the prop, returns old proxy
	RecvVarProxyFn HookProp(const char *tableName, const char *propName, RecvVarProxyFn function);

	// wrapper so we can use recursion without too much performance loss
	int GetProp(const char *tableName, const char *propName, RecvProp **prop = 0);

	// uses recursion to return a the relative offset to the given prop and sets the prop param
	int GetProp(RecvTable *recvTable, const char *propName, RecvProp **prop = 0);

	RecvTable *GetTable(const char *tableName);

	std::vector<RecvTable*>	m_tables;
};

struct C_Offsets
{
	DWORD Armor,
		FOV,
		LifeState,
		Flags,
		Health,
		PunchAngle,
		ViewPunchAngle,
		TeamNum,
		TickBase,
		Clip1,
		NextPrimaryAttack,
		Velocity,
		ViewOffset,
		Origin,
		ActiveWeapon,
		Angles,
		HitboxSet,
		clrRender,
		ShotsFired,
		OwnerXuidLow,
		OwnerXuidHigh,
		PaintKit,
		Wear,
		StatTrak,
		Seed,
		ItemIDHigh,
		CustomName,
		ItemID,
		ModelIndex,
		ViewModelIndex,
		WorldModelIndex,
		OwnerEntity;
};
#endif