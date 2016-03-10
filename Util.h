#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include "Includes.h"

class CUtilities
{
public:
	void ForceMaterial(float cRed, float cGreen, float cBlue, float cAlpha, IMaterial* material);
	IMaterial* CreateMaterial(bool bLit);
	bool ScreenTransform( const Vector &point, Vector &screen );
	bool WorldToScreen( const Vector &vOrigin, Vector &vScreen );
	void GetWorldSpaceCenter(IClientEntity* pBaseEnt, Vector& vWorldSpaceCenter);
	bool IsKeyPressed(int bKey);
	bool IsKnife(C_BaseCombatWeapon* pWeapon);
	bool IsPistol(C_BaseCombatWeapon* pWeapon);
	bool IsSmg(C_BaseCombatWeapon* pWeapon);
	bool IsMg(C_BaseCombatWeapon* pWeapon);
	bool IsRifle(C_BaseCombatWeapon* pWeapon);
	bool IsShotgun(C_BaseCombatWeapon* pWeapon);
	bool IsSniper(C_BaseCombatWeapon* pWeapon);
	bool IsMisc(C_BaseCombatWeapon* pWeapon);
	void RemoveSpread(CUserCmd* cmd, C_BaseCombatWeapon* pWeapon);
	DWORD dwFindPattern(DWORD dwAddress, DWORD dwLen, BYTE *bMask, char * szMask);
};

extern void NormalizeAngles(QAngle& angles);
extern void ClampAngles(QAngle& angles);

#endif
