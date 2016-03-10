#include "Util.h"

void NormalizeAngles(QAngle& angles)
{
	while (angles.x > 180.0)
		angles.x -= 360.0;

	while (angles.x < -180.0)
		angles.x += 360.0;

	while (angles.y > 180.0)
		angles.y -= 360.0;

	while (angles.y < -180.0)
		angles.y += 360.0;

	while (angles.z > 180.0)
		angles.z -= 360.0;

	while (angles.z < -180.0)
		angles.z += 360.0;
}

void ClampAngles(QAngle& angles)
{
	if (angles.x > 89.0f)
		angles.x = 89.0f;
	else if (angles.x < -89.0f)
		angles.x = -89.0f;

	while (angles.y > 180.0)
		angles.y -= 360.0;

	while (angles.y < -180.0)
		angles.y += 360.0;

	angles.z = 0;
}

namespace Spread
{
	QAngle SpreadAng;
	int sOffset = 0x778;
	float Spread = 0.0f, Cone = 0.0f;
	typedef float(__thiscall *getfloat)(void *thisptr);
	typedef void(__thiscall *funcvoid)(void *thisptr);
	QAngle GetSpreadAngle(const QAngle &view, int seed)
	{
		const float constInaccuracy = Cone;
		const float constSpread = Spread;
		Vector direction, right, up;
		AngleVectors(view, &direction, &right, &up);
		RandomSeed(seed + 1);
		float pi_1 = RandomFloat(0.f, 2.0f * M_PI_F);
		float inaccuracy = RandomFloat(0.f, constInaccuracy);
		Vector2D leftSpread((cos(pi_1) * inaccuracy), (sin(pi_1) * inaccuracy));
		Vector2D rightSpread[0xFF];
		for (int bullet = 0; bullet < 1; ++bullet)
		{
			float pi_2 = RandomFloat(0.f, 2.0f * M_PI_F);
			float spread = RandomFloat(0.f, constSpread);
			rightSpread[bullet] = Vector2D((cos(pi_2) * spread), (sin(pi_2) * spread));
		}
		for (int bullet = 0; bullet < 1; ++bullet)
		{
			Vector2D totalSpread = (leftSpread + rightSpread[bullet]);
			totalSpread = -totalSpread;
			Vector shot = (direction + (right * totalSpread.x) + (up * totalSpread.y));
			VectorNormalize(shot);
			float flIdentity[3][3];
			flIdentity[2][0] = 1.0f;
			flIdentity[2][1] = -totalSpread[0];
			flIdentity[2][2] = totalSpread[1];
			VectorNormalize(flIdentity[2]);
			flIdentity[0][0] = 0.0f;
			flIdentity[0][1] = -totalSpread[0];
			flIdentity[0][2] = (1.0f / totalSpread[1]) + (1.0f / flIdentity[2][2]) + totalSpread[1];
			if (totalSpread[0] > 0.0f && totalSpread[1] < 0.0f)
			{
				if (flIdentity[0][1] < 0.0f)
					flIdentity[0][1] = -flIdentity[0][1];
			}
			else if (totalSpread[0] < 0.0f && totalSpread[1] < 0.0f)
			{
				if (flIdentity[0][1] > 0.0f)
					flIdentity[0][1] = -flIdentity[0][1];
			}
			if (flIdentity[0][2] < 0.0f)
				flIdentity[0][2] = -flIdentity[0][2];
			VectorNormalize(flIdentity[0]);
			CrossProduct(flIdentity[0], flIdentity[2], flIdentity[1]);
			VectorNormalize(flIdentity[1]);
			float flCross = (flIdentity[1][1] * flIdentity[2][0]) - (flIdentity[1][0] * flIdentity[2][1]);
			float flRoll;
			if (view[0] > 84.0f || view[0] < -84.0f)
				flRoll = RAD2DEG(atan2(flIdentity[1][2], sqrt(flCross)));
			else
				flRoll = RAD2DEG(atan2(flIdentity[1][2], flCross));
			if (flRoll < 0.0f)
				flRoll += 360.0f;
			QAngle angles;
			VectorAngles(shot, up, angles);
			angles[2] += flRoll;
			NormalizeAngles(angles);
			angles -= view;
			NormalizeAngles(angles);
			return angles;
		}
	}
	void UpdateSpread(void* gun, CUserCmd* cmd)
	{
		((funcvoid)((*(DWORD*)(*(DWORD*)(gun) + sOffset + 0x8))))(gun);
		Cone = ((getfloat)((*(DWORD*)(*(DWORD*)(gun) + sOffset))))(gun);
		Spread = ((getfloat)((*(DWORD*)(*(DWORD*)(gun) + sOffset + 0x4))))(gun);
		SpreadAng = GetSpreadAngle(cmd->viewangles, cmd->random_seed & 255);
		NormalizeAngles(SpreadAng);
		cmd->viewangles += SpreadAng;
	}
}

void CUtilities::ForceMaterial(float cRed, float cGreen, float cBlue, float cAlpha, IMaterial* material)
{
	if (material != NULL)
	{
		float temp[3] =
		{
			cRed,
			cGreen,
			cBlue
		};

		Kardoffl.m_pRender->SetBlend(cAlpha);
		Kardoffl.m_pRender->SetColorModulation(temp);

		Kardoffl.m_pModelRender->ForcedMaterialOverride(material);
	}
}

#define MAT(type) "\"" + type + "\"\n{\n\t\"$basetexture\" \"VGUI/white_additive\"\n\t\"$model\" \"1\"\t\n\"$ignorez\" \"0\"\n}\n"

IMaterial* CUtilities::CreateMaterial(bool bLit)
{
	static int created = 0;

	std::string type = (!bLit) ? "UnlitGeneric" : "VertexLitGeneric";

	char materialBuffer[2048];

	std::string tmp(MAT(type));
	strcpy(materialBuffer, tmp.c_str());

	char materialName[512];
	sprintf(materialName, "custom_material_%i.vmt", created);
	++created;

	KeyValues* keyValues = new KeyValues(type.c_str());

	keyValues->LoadFromBuffer(materialName, materialBuffer);

	if (keyValues == NULL)
		return 0;

	IMaterial* createdMaterial = Kardoffl.m_pMaterialSystem->CreateMaterial(materialName, keyValues);
	createdMaterial->IncrementReferenceCount();

	return createdMaterial;
}

bool CUtilities::ScreenTransform( const Vector &point, Vector &screen )
{
	float w;
	const VMatrix &worldToScreen = Kardoffl.m_pEngine->WorldToScreenMatrix();
	screen.x = worldToScreen[0][0] * point[0] + worldToScreen[0][1] * point[1] + worldToScreen[0][2] * point[2] + worldToScreen[0][3];
	screen.y = worldToScreen[1][0] * point[0] + worldToScreen[1][1] * point[1] + worldToScreen[1][2] * point[2] + worldToScreen[1][3];
	w		 = worldToScreen[3][0] * point[0] + worldToScreen[3][1] * point[1] + worldToScreen[3][2] * point[2] + worldToScreen[3][3];
	screen.z = 0.0f;

	bool behind = false;

	if( w < 0.001f )
	{
		behind = true;
		screen.x *= 100000;
		screen.y *= 100000;
	}
	else
	{
		behind = false;
		float invw = 1.0f / w;
		screen.x *= invw;
		screen.y *= invw;
	}
	return behind;
}

bool CUtilities::WorldToScreen( const Vector &vOrigin, Vector &vScreen )
{
	if(!ScreenTransform( vOrigin, vScreen ))
	{
		int iScreenWidth, iScreenHeight;

		Kardoffl.m_pEngine->GetScreenSize( iScreenWidth, iScreenHeight );
		float x = iScreenWidth / 2;
		float y = iScreenHeight / 2;
		x += 0.5 * vScreen.x * iScreenWidth + 0.5;
		y -= 0.5 * vScreen.y * iScreenHeight + 0.5;
		vScreen.x = x;
		vScreen.y = y;
		return true;
	}
	return false;
}

void CUtilities::GetWorldSpaceCenter( IClientEntity* pBaseEnt, Vector& vWorldSpaceCenter )
{
	Vector vMin, vMax;
	pBaseEnt->GetRenderBounds( vMin, vMax );
	vWorldSpaceCenter = Kardoffl.m_pEntity->GetOrigin(pBaseEnt);
	vWorldSpaceCenter.z += (vMin.z + vMax.z) / 2.0f;
}

bool CUtilities::IsKeyPressed(int bKey)
{
	BYTE byKey = 0;

	if (bKey == 0)
		byKey = 0x04;
	else if (bKey == 1)
		byKey = 0x01;
	else if (bKey == 2)
		byKey = 0x02;
	else if (bKey == 3)
		byKey = 0x05;
	else if (bKey == 4)
		byKey = 0x06;
	else if (bKey == 5)
		byKey = 0x10;
	else if (bKey == 6)
		byKey = 0x11;
	else if (bKey == 7)
		byKey = 0x12;
	else if (bKey == 8)
		byKey = 0x51;
	else if (bKey == 9)
		byKey = 0x46;
	else if (bKey == 10)
		byKey = 0x45;

	if (GetAsyncKeyState(byKey))
		return true;
	else
		return false;
}

bool CUtilities::IsKnife(C_BaseCombatWeapon* pWeapon)
{
	if (!pWeapon)
		return false;

	int iWeaponID = Kardoffl.m_pEntity->GetWeaponID(pWeapon);

	if (iWeaponID == weapon_knife_t
		|| iWeaponID == weapon_knife_ct
		|| iWeaponID == weapon_bayonet
		|| iWeaponID == weapon_m9bayonet
		|| iWeaponID == weapon_huntsman
		|| iWeaponID == weapon_karambit
		|| iWeaponID == weapon_gut
		|| iWeaponID == weapon_flip)
		return true;
	else
		return false;
}

bool CUtilities::IsPistol(C_BaseCombatWeapon* pWeapon)
{
	if (!pWeapon)
		return false;

	int iWeaponID = Kardoffl.m_pEntity->GetWeaponID(pWeapon);

	if (iWeaponID == weapon_deagle
		|| iWeaponID == weapon_elite
		|| iWeaponID == weapon_fiveseven
		|| iWeaponID == weapon_tec9
		|| iWeaponID == weapon_glock
		|| iWeaponID == weapon_p250
		|| iWeaponID == weapon_p2000
		|| iWeaponID == weapon_usp)
		return true;
	else
		return false;
}

bool CUtilities::IsSmg(C_BaseCombatWeapon* pWeapon)
{
	if (!pWeapon)
		return false;

	int iWeaponID = Kardoffl.m_pEntity->GetWeaponID(pWeapon);

	if (iWeaponID == weapon_bizon
		|| iWeaponID == weapon_mac10
		|| iWeaponID == weapon_p90
		|| iWeaponID == weapon_ump45
		|| iWeaponID == weapon_mp7
		|| iWeaponID == weapon_mp9)
		return true;
	else
		return false;
}

bool CUtilities::IsMg(C_BaseCombatWeapon* pWeapon)
{
	if (!pWeapon)
		return false;

	int iWeaponID = Kardoffl.m_pEntity->GetWeaponID(pWeapon);

	if (iWeaponID == weapon_m249
		|| iWeaponID == weapon_negev)
		return true;
	else
		return false;
}

bool CUtilities::IsRifle(C_BaseCombatWeapon* pWeapon)
{
	if (!pWeapon)
		return false;

	int iWeaponID = Kardoffl.m_pEntity->GetWeaponID(pWeapon);

	if (iWeaponID == weapon_ak47
		|| iWeaponID == weapon_aug
		|| iWeaponID == weapon_m4a4
		|| iWeaponID == weapon_m4a1s
		|| iWeaponID == weapon_famas
		|| iWeaponID == weapon_galilar
		|| iWeaponID == weapon_sg556)
		return true;
	else
		return false;
}

bool CUtilities::IsShotgun(C_BaseCombatWeapon* pWeapon)
{
	if (!pWeapon)
		return false;

	int iWeaponID = Kardoffl.m_pEntity->GetWeaponID(pWeapon);

	if (iWeaponID == weapon_xm1014
		|| iWeaponID == weapon_nova
		|| iWeaponID == weapon_sawedoff
		|| iWeaponID == weapon_mag7)
		return true;
	else
		return false;
}


bool CUtilities::IsSniper(C_BaseCombatWeapon* pWeapon)
{
	if (!pWeapon)
		return false;

	int iWeaponID = Kardoffl.m_pEntity->GetWeaponID(pWeapon);

	if (iWeaponID == weapon_g3sg1
		|| iWeaponID == weapon_awp
		|| iWeaponID == weapon_scar20
		|| iWeaponID == weapon_ssg08)
		return true;
	else
		return false;
}

bool CUtilities::IsMisc(C_BaseCombatWeapon* pWeapon)
{
	if (!pWeapon)
		return false;

	int iWeaponID = Kardoffl.m_pEntity->GetWeaponID(pWeapon);

	if (iWeaponID == weapon_flashbang
		|| iWeaponID == weapon_hegrenade
		|| iWeaponID == weapon_smokegrenade
		|| iWeaponID == weapon_incgrenade
		|| iWeaponID == weapon_molotov
		|| iWeaponID == weapon_decoy
		|| iWeaponID == weapon_taser
		|| iWeaponID == weapon_c4)
		return true;
	else
		return false;
}

void CUtilities::RemoveSpread(CUserCmd* cmd, C_BaseCombatWeapon* pWeapon)
{
	Spread::UpdateSpread(pWeapon, cmd);
}

bool bDataCompare(const BYTE* pData, const BYTE* bMask, const char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bMask)
		if (*szMask == 'x' && *pData != *bMask)
			return false;
	return (*szMask) == NULL;
}

DWORD CUtilities::dwFindPattern(DWORD dwAddress, DWORD dwLen, BYTE *bMask, char * szMask)
{
	for (DWORD i = 0; i < dwLen; i++)
		if (bDataCompare((BYTE*)(dwAddress + i), bMask, szMask))
			return (DWORD)(dwAddress + i);

	return 0;
}