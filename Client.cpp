#include "Includes.h"

ScreenSize_t sScreenSize2;
bool bResChange = false;
int iTarget = -1;
float flDistance = 9999.0f;
QAngle qAngle;

IClientEntity* GetOwnerEntity(void* Ent)
{
	EHANDLE hOwner = *reinterpret_cast<EHANDLE*>((DWORD)Ent + Kardoffl.m_pOffsets->OwnerEntity);
	return Kardoffl.m_pEntlist->GetClientEntityFromHandle(hOwner);
}

float GetHitgroupModifiedDamage(int hitgroup)
{
	static float hitgroupModifiers[] = { 1.f, 4.f, 1.f, 1.25f, 1.f, 1.f, .75f, .75f };
	return hitgroupModifiers[hitgroup];
}

struct FireBulletData
{
	FireBulletData(const Vector &eye_pos)
		: src(eye_pos)
	{
	}

	Vector           src;
	trace_t          enter_trace;
	Vector           direction;
	CTraceFilterNoPlayers    filter;
	float           trace_length;
	float           trace_length_remaining;
	float           current_damage;
	int             penetrate_count;
};

void ScaleDamage(int hitgroup, IClientEntity *enemy, float weapon_armor_ratio, float &current_damage)
{
	current_damage *= GetHitgroupModifiedDamage(hitgroup);

	/*if (enemy->m_ArmorValue() > 0)
	{
	if (hitgroup == HITGROUP_HEAD)
	{
	if (enemy->m_bHasHelmet())
	current_damage *= (weapon_armor_ratio * .5f);
	}
	else
	{
	current_damage *= (weapon_armor_ratio * .5f);
	}
	}*/
}

CCSWeaponInfo GetCSWpnData(C_BaseCombatWeapon* pWeapon)
{
	typedef CCSWeaponInfo& (__thiscall* GetCSWpnDataFn)(void*);
	return ((GetCSWpnDataFn)((*(DWORD**)pWeapon)[454]))(pWeapon);
}

bool TraceToExit(Vector &end, trace_t *enter_trace, Vector start, Vector dir, trace_t *exit_trace)
{
	float distance = 0.0f;

	while (distance <= 90.0f)
	{
		distance += 4.0f;
		end = start + dir * distance;

		auto point_contents = Kardoffl.m_pEnginetrace->GetPointContents(end, MASK_SHOT_HULL | CONTENTS_HITBOX, NULL);

		if (point_contents & MASK_SHOT_HULL && (!(point_contents & CONTENTS_HITBOX)))
			continue;

		auto new_end = end - (dir * 4.0f);

		Ray_t ray;
		ray.Init(end, new_end);
		Kardoffl.m_pEnginetrace->TraceRay(ray, 0x4600400B, 0, exit_trace);

		if (exit_trace->startsolid && exit_trace->surface.flags & SURF_HITBOX)
		{
			CTraceFilterNoPlayers filter;
			filter.pSkip = exit_trace->m_pEnt;
			Ray_t ray;
			ray.Init(end, start);
			Kardoffl.m_pEnginetrace->TraceRay(ray, 0x600400B, &filter, exit_trace);

			if ((exit_trace->fraction < 1.0f || exit_trace->allsolid) && !exit_trace->startsolid)
			{
				end = exit_trace->endpos;
				return true;
			}
			continue;
		}

		if (!(exit_trace->fraction < 1.0 || exit_trace->allsolid || exit_trace->startsolid) || exit_trace->startsolid)
		{
			if (exit_trace->m_pEnt)
			{
				if (enter_trace->DidHitNonWorldEntity() /*&& (!strcmp(enter_trace->m_pEnt->GetClassname(), "func_breakable") || !strcmp(enter_trace->m_pEnt->GetClassname(), "func_breakable_surf"))*/)
					return true;
			}
			continue;
		}

		if (((exit_trace->surface.flags >> 7) & 1) && !((enter_trace->surface.flags >> 7) & 1))
			continue;

		if (exit_trace->plane.normal.Dot(dir) <= 1.0f)
		{
			auto fraction = exit_trace->fraction * 4.0f;
			end = end - (dir * fraction);
			return true;
		}
	}
	return false;
}

bool HandleBulletPenetration(CCSWeaponInfo *wpn_data, FireBulletData &data)
{
	surfacedata_t *enter_surface_data = Kardoffl.m_pPhysicAPI->GetSurfaceData(data.enter_trace.surface.surfaceProps);
	int enter_material = enter_surface_data->game.material;
	float enter_surf_penetration_mod = *(float*)((DWORD)enter_surface_data + 76);

	data.trace_length += data.enter_trace.fraction * data.trace_length_remaining;
	data.current_damage *= pow(wpn_data->m_flRangeModifier, (data.trace_length * 0.002));

	if ((data.trace_length > 3000.f) || (enter_surf_penetration_mod < 0.1f))
		data.penetrate_count = 0;

	if (data.penetrate_count <= 0)
		return false;

	Vector dummy;
	trace_t trace_exit;
	if (!TraceToExit(dummy, &data.enter_trace, data.enter_trace.endpos, data.direction, &trace_exit))
		return false;

	surfacedata_t *exit_surface_data = Kardoffl.m_pPhysicAPI->GetSurfaceData(trace_exit.surface.surfaceProps);
	int exit_material = exit_surface_data->game.material;

	float exit_surf_penetration_mod = *(float*)((DWORD)exit_surface_data + 76);
	float final_damage_modifier = 0.16f;
	float combined_penetration_modifier = 0.0f;

	if (((data.enter_trace.contents & CONTENTS_GRATE) != 0) || (enter_material == 89) || (enter_material == 71))
	{
		combined_penetration_modifier = 3.0f;
		final_damage_modifier = 0.05f;
	}
	else
	{
		combined_penetration_modifier = (enter_surf_penetration_mod + exit_surf_penetration_mod) * 0.5f;
	}

	if (enter_material == exit_material)
	{
		if (exit_material == 87 || exit_material == 85)
			combined_penetration_modifier = 3.0f;
		else if (exit_material == 76)
			combined_penetration_modifier = 2.0f;
	}

	float v34 = fmaxf(0.f, 1.0f / combined_penetration_modifier);
	float v35 = (data.current_damage * final_damage_modifier) + v34 * 3.0f * fmaxf(0.0f, (3.0f / wpn_data->m_flPenetration) * 1.25f);
	float thickness = VectorLength(trace_exit.endpos - data.enter_trace.endpos);

	thickness *= thickness;
	thickness *= v34;
	thickness /= 24.0f;

	float lost_damage = fmaxf(0.0f, v35 + thickness);

	if (lost_damage > data.current_damage)
		return false;

	if (lost_damage >= 0.0f)
		data.current_damage -= lost_damage;

	if (data.current_damage < 1.0f)
		return false;

	data.src = trace_exit.endpos;
	data.penetrate_count--;

	return true;
}

bool SimulateFireBullet(IClientEntity *local, C_BaseCombatWeapon* weapon, FireBulletData &data)
{
	data.penetrate_count = 4;
	data.trace_length = 0.0f;
	CCSWeaponInfo wpn_data = GetCSWpnData(weapon);

	data.current_damage = (float)wpn_data.m_iDamage;

	while ((data.penetrate_count > 0) && (data.current_damage >= 1.0f))
	{
		data.trace_length_remaining = wpn_data.m_flRange - data.trace_length;

		Vector end = data.src + data.direction * data.trace_length_remaining;

		CTraceFilterNoPlayers filter;
		filter.pSkip = local;
		Ray_t ray;
		ray.Init(data.src, end);
		Kardoffl.m_pEnginetrace->TraceRay(ray, 0x4600400B, &filter, &data.enter_trace);
		UTIL_ClipTraceToPlayers(data.src, end + data.direction * 40.f, 0x4600400B, &data.filter, &data.enter_trace);

		if (data.enter_trace.fraction == 1.0f)
			break;

		if ((data.enter_trace.hitgroup <= 7)
			&& (data.enter_trace.hitgroup > 0)
			&& (Kardoffl.m_pEntity->GetTeamID(local) != Kardoffl.m_pEntity->GetTeamID(data.enter_trace.m_pEnt)))
		{
			data.trace_length += data.enter_trace.fraction * data.trace_length_remaining;
			data.current_damage *= pow(wpn_data.m_flRangeModifier, data.trace_length * 0.002);
			ScaleDamage(data.enter_trace.hitgroup, data.enter_trace.m_pEnt, 0, data.current_damage);

			return true;
		}

		if (!HandleBulletPenetration(&wpn_data, data))
			break;
	}

	return false;
}

float CanHit(Vector &vecEyePos, Vector &point)
{
	FireBulletData data = FireBulletData(vecEyePos);
	data.filter.pSkip = Kardoffl.m_pEntity->Ent();

	QAngle angles;
	Vector tmp = point - data.src;
	VectorAngles(tmp, angles);
	AngleVectors(angles, &data.direction);
	VectorNormalize(data.direction);

	if (SimulateFireBullet(Kardoffl.m_pEntity->Ent(), Kardoffl.m_pEntity->GetBaseCombatWeapon(Kardoffl.m_pEntity->Ent()), data))
	{
		return data.current_damage;
	}

	return -1;
}

float GetFov(QAngle angle, Vector src, Vector dst)
{
	QAngle ang, aim;
	float fov;

	Vector tmp = dst - src;
	VectorAngles(tmp, ang);
	QAngle vTempAngles = ang - angle;
	NormalizeAngles(vTempAngles);

	return vTempAngles.Length();
}

bool GetVisible(Vector& vAbsStart, Vector& vAbsEnd, IClientEntity* pEntity)
{
	trace_t tr;
	Ray_t ray;
	CTraceFilterNoPlayers filter;
	filter.pSkip = Kardoffl.m_pEntity->Ent();

	ray.Init(vAbsStart, vAbsEnd);
	Kardoffl.m_pEnginetrace->TraceRay(ray, 0x4600400B, &filter, &tr);

	return (tr.m_pEnt == pEntity || tr.fraction > 0.99f);
}

bool GetHitboxPosition(IClientEntity* ClientEntity, int iHitBox, Vector* vOut)
{
	matrix3x4a_t pmatrix[128];

	Vector vMin, vMax;

	const model_t* model = ClientEntity->GetModel();

	if (!model)
		return false;

	studiohdr_t *pStudioHdr = Kardoffl.m_pModelinfo->GetStudiomodel(model);

	if (pStudioHdr == NULL)
		return false;

	if (!ClientEntity->SetupBones(pmatrix, 128, 0x00000100, 0))
		return false;

	mstudiohitboxset_t *set = pStudioHdr->pHitboxSet(Kardoffl.m_pEntity->GetHitboxSet(ClientEntity));

	if (!set)
		return false;

	mstudiobbox_t* pbox = set->pHitbox(iHitBox);

	// center and all the points of the hitbox
	Vector points[9] = { ((pbox->bbmin + pbox->bbmax) * .5f), // center
		Vector(pbox->bbmin.x, pbox->bbmin.y, pbox->bbmin.z), // left bottom back corner
		Vector(pbox->bbmin.x, pbox->bbmax.y, pbox->bbmin.z), // left bottom front corner
		Vector(pbox->bbmax.x, pbox->bbmax.y, pbox->bbmin.z), // left top front corner
		Vector(pbox->bbmax.x, pbox->bbmin.y, pbox->bbmin.z), // left top back corner
		Vector(pbox->bbmax.x, pbox->bbmax.y, pbox->bbmax.z), // right top front corner
		Vector(pbox->bbmin.x, pbox->bbmax.y, pbox->bbmax.z), // right bottom front corner
		Vector(pbox->bbmin.x, pbox->bbmin.y, pbox->bbmax.z), // right bottom back corner
		Vector(pbox->bbmax.x, pbox->bbmin.y, pbox->bbmax.z)  // right top back corner
	};

	for (int index = 0; index <= 8; ++index)
	{
		if (index != 0)
		{
			// scale down the hitbox size
			points[index] = ((((points[index] + points[0]) * .5f) + points[index]) * .5f);
		}

		// transform the vector
		VectorTransform(points[index], pmatrix[pbox->bone], vOut[index]);
	}

	return true;
}

void Triggerbot(CUserCmd* cmd)
{
	trace_t tr;
	Ray_t ray;
	QAngle qFix;
	Vector vEyePos = Kardoffl.m_pEntity->GetEyePos(Kardoffl.m_pEntity->Ent());
	Vector vEnd;
	player_info_t pInfo;
	CTraceFilterNoPlayers filter;
	filter.pSkip = Kardoffl.m_pEntity->Ent();

	AngleVectors(cmd->viewangles, &vEnd);

	vEnd = vEnd * 8192 + vEyePos;
	ray.Init(vEyePos, vEnd);

	Kardoffl.m_pEnginetrace->TraceRay(ray, 0x46004003, &filter, &tr);

	if (tr.m_pEnt->GetIClientEntity() == NULL)
		return;

	if (tr.m_pEnt->GetIClientEntity()->entindex() <= 0)
		return;

	if (tr.m_pEnt->GetIClientEntity()->entindex() > 64)
		return;

	Kardoffl.m_pEngine->GetPlayerInfo(tr.m_pEnt->GetIClientEntity()->entindex(), &pInfo);

	if (!Kardoffl.m_pEntity->GetLifeState(tr.m_pEnt->GetIClientEntity()))
		return;

	if (Kardoffl.m_pEntity->GetTeamID(tr.m_pEnt->GetIClientEntity()) <= 1)
		return;

	if (!Kardoffl.m_pConvars->GetConVar("triggerbot_team")->GetBool())
	{
		if (Kardoffl.m_pEntity->GetTeamID(tr.m_pEnt->GetIClientEntity()) == Kardoffl.m_pEntity->GetTeamID(Kardoffl.m_pEntity->Ent()))
			return;
	}

	if (tr.hitbox == 0 || tr.hitbox == 2)
	{
		if (!Kardoffl.m_pConvars->GetConVar("triggerbot_head")->GetBool())
			return;

		cmd->buttons |= IN_ATTACK;
	}
	else if (tr.hitbox >= 3 && tr.hitbox <= 7)
	{
		if (!Kardoffl.m_pConvars->GetConVar("triggerbot_chest")->GetBool())
			return;

		cmd->buttons |= IN_ATTACK;
	}
	else if (tr.hitbox >= 16 && tr.hitbox <= 19)
	{
		if (!Kardoffl.m_pConvars->GetConVar("triggerbot_arms")->GetBool())
			return;

		cmd->buttons |= IN_ATTACK;
	}
	else if (tr.hitbox >= 8 && tr.hitbox <= 13)
	{
		if (!Kardoffl.m_pConvars->GetConVar("triggerbot_legs")->GetBool())
			return;

		cmd->buttons |= IN_ATTACK;
	}
}

void SmoothAngles(QAngle &src, QAngle &back, QAngle &flLocalAngles, float smooth)
{
	float smoothdiff[2];
	src[0] -= flLocalAngles.x;
	src[1] -= flLocalAngles.y;
	NormalizeAngles(src);
	smoothdiff[0] = src[0] / smooth;
	smoothdiff[1] = src[1] / smooth;
	back[0] = flLocalAngles.x + smoothdiff[0];
	back[1] = flLocalAngles.y + smoothdiff[1];
	back[2] = 0;
	NormalizeAngles(back);
}

BYTE bMoveData[0x200];

void Prediction(CUserCmd* pCmd, C_BasePlayer* LocalPlayer)
{
	float curtime = Kardoffl.m_pGlobals->curtime;
	float frametime = Kardoffl.m_pGlobals->frametime;
	int iFlags = Kardoffl.m_pEntity->GetFlags(LocalPlayer);

	Kardoffl.m_pGlobals->curtime = (float)Kardoffl.m_pEntity->GetTickBase(LocalPlayer) * Kardoffl.m_pGlobals->interval_per_tick;
	Kardoffl.m_pGlobals->frametime = Kardoffl.m_pGlobals->interval_per_tick;

	Kardoffl.m_pMoveHelper->SetHost(LocalPlayer);

	Kardoffl.m_pPrediction->SetupMove(LocalPlayer, pCmd, NULL, (CMoveData*)bMoveData);
	Kardoffl.m_pGameMovement->ProcessMovement(LocalPlayer, (CMoveData*)bMoveData);
	Kardoffl.m_pPrediction->FinishMove(LocalPlayer, pCmd, (CMoveData*)bMoveData);

	Kardoffl.m_pMoveHelper->SetHost(0);

	Kardoffl.m_pGlobals->curtime = curtime;
	Kardoffl.m_pGlobals->frametime = frametime;
	*(int*)((DWORD)LocalPlayer + Kardoffl.m_pOffsets->Flags) = iFlags;
}

void Aimbot(CUserCmd* cmd, C_BaseCombatWeapon* Weapon, int iHitbox)
{
	iTarget = -1;
	flDistance = 9999.0f;
	qAngle.Init();
	player_info_t playerInfo;
	Vector vecTarget, vecEntity[9], vecEyePos = Kardoffl.m_pEntity->GetEyePos(Kardoffl.m_pEntity->Ent());
	QAngle qPunch = Kardoffl.m_pEntity->GetPunchAngle(Kardoffl.m_pEntity->Ent()) * 2;

	for (int i = Kardoffl.m_pEntlist->GetHighestEntityIndex(); i > 0; --i)
	{
		if (i == Kardoffl.m_pEngine->GetLocalPlayer())
			continue;

		IClientEntity* Entity = Kardoffl.m_pEntlist->GetClientEntity(i);

		if (Entity == NULL
			|| Entity->IsDormant()
			|| !Kardoffl.m_pEngine->GetPlayerInfo(i, &playerInfo)
			|| !Kardoffl.m_pEntity->GetLifeState(Entity)
			|| Kardoffl.m_pEntity->GetRenderColor(Entity).a < 255
			|| Kardoffl.m_pEntity->GetTeamID(Kardoffl.m_pEntity->Ent()) == Kardoffl.m_pEntity->GetTeamID(Entity))
			continue;

		for (int i = iHitbox; i <= 18; ++i)
		{
			int iHitBox = Kardoffl.m_pConvars->GetConVar("aimbot_hitscan")->GetBool() ? i : iHitbox;

			if (!GetHitboxPosition(Entity, iHitBox, vecEntity))
				continue;

			if (!GetVisible(vecEyePos, vecEntity[0], Entity))
			{
				if (Kardoffl.m_pConvars->GetConVar("aimbot_autowall")->GetBool())
				{
					if (CanHit(vecEyePos, vecEntity[0]) <= 0)
						continue;
				}
				else
					continue;
			}

			float flFieldofView = GetFov(cmd->viewangles + qPunch, vecEyePos, vecEntity[0]);

			if (flFieldofView <= Kardoffl.m_pConvars->GetConVar("aimbot_fov")->GetFloat() && flFieldofView < flDistance)
			{
				flDistance = flFieldofView;
				iTarget = i;
				vecTarget = vecEntity[0];
				break;
			}
		}
	}

	if (iTarget > -1)
	{
		QAngle qPreAim, qAim;
		Vector tmp = vecTarget - vecEyePos;
		VectorAngles(tmp, qPreAim);

		if (Kardoffl.m_pConvars->GetConVar("aimbot_rcs")->GetBool() && !Kardoffl.m_pUtilities->IsPistol(Weapon) && !Kardoffl.m_pUtilities->IsSniper(Weapon))
			qPreAim -= qPunch;

		NormalizeAngles(qPreAim);

		SmoothAngles(qPreAim, qAim, cmd->viewangles, Kardoffl.m_pConvars->GetConVar("aimbot_smooth")->GetFloat());

		if (Kardoffl.m_pConvars->GetConVar("aimbot_stopcrouch")->GetBool())
		{
			cmd->forwardmove = 0.f;
			cmd->sidemove = 0.f;
			cmd->buttons |= IN_DUCK;
		}

		if (Kardoffl.m_pConvars->GetConVar("aimbot_silent")->GetInt() == 0)
			Kardoffl.m_pEngine->SetViewAngles(qAim);
		else if (Kardoffl.m_pConvars->GetConVar("aimbot_silent")->GetInt() == 1)
			cmd->viewangles = qAim;
		else if (Kardoffl.m_pConvars->GetConVar("aimbot_silent")->GetInt() == 2)
			qAngle = qAim;

		if (Kardoffl.m_pConvars->GetConVar("aimbot_autoshoot")->GetBool())
			cmd->buttons |= IN_ATTACK;
	}
}

void CL_FixMove(CUserCmd* cmd, QAngle viewangles)
{
	Vector move, movenorm, dir, set;
	QAngle movenormang, adjusted;

	float len;

	move.Init(cmd->forwardmove, cmd->sidemove, cmd->upmove);

	movenorm = move;

	VectorNormalize(movenorm);

	len = move.Length();

	VectorAngles(movenorm, movenormang);

	if ((cmd->viewangles.x >= 89.0f) || (cmd->viewangles.x <= -89.0f))
	{
		if (viewangles.x >= 0.0f && viewangles.x <= 89.0f)
		{
			viewangles.x = cmd->viewangles.x + 180.0f;
		}
		if (viewangles.x <= 0.0f && viewangles.x >= -89.0f)
		{
			viewangles.x = cmd->viewangles.x - 180.0f;
		}
	}

	adjusted = movenormang + (cmd->viewangles - viewangles);

	AngleVectors(adjusted, &dir);

	set = dir * len;

	if ((cmd->viewangles.x > 89.0f) || (cmd->viewangles.x < -89.0f))
		cmd->forwardmove = set.x;
	else if ((cmd->viewangles.x == 89.0f || cmd->viewangles.x == -89.0f))
		cmd->forwardmove = -set.x;
	else
		cmd->forwardmove = set.x;

	if ((cmd->viewangles.x >= 89.0f) || (cmd->viewangles.x <= -89.0f))
		cmd->sidemove = -set.y;
	else
		cmd->sidemove = set.y;

	if (cmd->sidemove < -450)
		cmd->sidemove = -450;
	if (cmd->sidemove > 450)
		cmd->sidemove = 450;
	if (cmd->forwardmove < -450)
		cmd->forwardmove = -450;
	if (cmd->forwardmove > 450)
		cmd->forwardmove = 450;
}

void AntiAntiAimXProxy(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
	float flPitch = pData->m_Value.m_Float;

	if (flPitch > 180.0)
		flPitch -= 360.0;
	else if (flPitch < -180.0)
		flPitch += 360.0;

	//Fakedown autofix
	if (flPitch < -179.648438f || flPitch > 179.648438f)
	{
		flPitch = -10.0f;
	}

	if (flPitch <= -88.945313f && flPitch >= -179.648438f)
	{
		flPitch = -89.0f;
	}

	if (flPitch >= 88.945313f && flPitch <= 179.648438f)
	{
		flPitch = 89.0f;
	}

	*(float*)pOut = flPitch;
}

void RecvProxy_Viewmodel(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	int iModel = pData->m_Value.m_Int;

	int iCTDefault = Kardoffl.m_pModelinfo->GetModelIndex("models/weapons/v_knife_default_ct.mdl");
	int iTDefault = Kardoffl.m_pModelinfo->GetModelIndex("models/weapons/v_knife_default_t.mdl");
	int iBayonet = Kardoffl.m_pModelinfo->GetModelIndex("models/weapons/v_knife_bayonet.mdl");
	int iButterfly = Kardoffl.m_pModelinfo->GetModelIndex("models/weapons/v_knife_butterfly.mdl");
	int iFlip = Kardoffl.m_pModelinfo->GetModelIndex("models/weapons/v_knife_flip.mdl");
	int iGunGame = Kardoffl.m_pModelinfo->GetModelIndex("models/weapons/v_knife_gg.mdl");
	int iGut = Kardoffl.m_pModelinfo->GetModelIndex("models/weapons/v_knife_gut.mdl");
	int iKarambit = Kardoffl.m_pModelinfo->GetModelIndex("models/weapons/v_knife_karam.mdl");
	int iM9Bayonet = Kardoffl.m_pModelinfo->GetModelIndex("models/weapons/v_knife_m9_bay.mdl");
	int iHuntsman = Kardoffl.m_pModelinfo->GetModelIndex("models/weapons/v_knife_tactical.mdl");
	int iFalchion = Kardoffl.m_pModelinfo->GetModelIndex("models/weapons/v_knife_falchion_advanced.mdl");
	int iDagger = Kardoffl.m_pModelinfo->GetModelIndex("models/weapons/v_knife_push.mdl");

	if (Kardoffl.m_pEntity->Ent())
	{
		if (Kardoffl.m_pEntity->GetBaseCombatWeapon(Kardoffl.m_pEntity->Ent()))
		{
			if (iModel == iBayonet
				|| iModel == iButterfly
				|| iModel == iCTDefault
				|| iModel == iTDefault
				|| iModel == iFlip
				|| iModel == iGunGame
				|| iModel == iGut
				|| iModel == iKarambit
				|| iModel == iM9Bayonet
				|| iModel == iHuntsman
				|| iModel == iFalchion)
			{
				int iKnife = Kardoffl.m_pConvars->GetConVar("misc_knife")->GetInt();

				if (iKnife == 1)
				{
					iModel = iBayonet;
				}
				else if (iKnife == 2)
				{
					iModel = iFlip;
				}
				else if (iKnife == 3)
				{
					iModel = iGut;
				}
				else if (iKnife == 4)
				{
					iModel = iKarambit;
				}
				else if (iKnife == 5)
				{
					iModel = iM9Bayonet;
				}
				else if (iKnife == 6)
				{
					iModel = iHuntsman;
				}
				else if (iKnife == 7)
				{
					iModel = iButterfly;
				}
				else if (iKnife == 8)
				{
					iModel = iFalchion;
				}
				else if (iKnife == 9)
				{
					iModel = iDagger;
				}
			}
		}
	}

	*(int*)pOut = iModel;
}

void RecvProxy_Sequence(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	int iSequence = pData->m_Value.m_Int;

	IClientEntity* Weapon = (IClientEntity*)pStruct;

	if (Weapon && Weapon->GetModel())
	{
		std::string strModelName = Kardoffl.m_pModelinfo->GetModelName(Weapon->GetModel());

		static bool bWasOpen = false;

		if (strModelName.find("v_knife_push.mdl") != std::string::npos)
		{
			if (iSequence == 2)//random stab fix
				iSequence = 1;

			if (iSequence == 12)//fix inspect animation
				iSequence = 15;
		}
		else if (strModelName.find("v_knife_falchion_advanced.mdl") != std::string::npos)
		{
			srand(GetTickCount());

			int iBalanceChance = rand() % 100;

			if (iSequence == 12 && iBalanceChance > 70)//balance animation
				iSequence = 13;

			if (iSequence == 2)//random stab fix
				iSequence = 1;
		}
		else if (strModelName.find("v_knife_butterfly.mdl") != std::string::npos)
		{
			srand(GetTickCount());

			int iOpenChance = rand() % 100;

			if (iSequence == 0 && iOpenChance <= 70)
				iSequence = 1;

			bWasOpen = ((iSequence == 1) || (iSequence == 0));

			int iBalanceChance = rand() % 100;

			if (iSequence == 12 && iBalanceChance <= 33)//inspect animation
				iSequence = 13;

			if (iSequence == 12 && iBalanceChance <= 66 && iBalanceChance > 33)//inspect animation
				iSequence = 14;

			if (iSequence == 12 && iBalanceChance > 66)//inspect animation
				iSequence = 15;

			if (iSequence == 1 && bWasOpen)//random stab fix
				iSequence = 2;

			if (iSequence == 8 || iSequence == 9)
				iSequence = 10;
		}
		else
		{
			bWasOpen = false;
		}
	}

	*(int*)pOut = iSequence;
}

void __stdcall new_PaintTraverse(vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce)
{
	Kardoffl.m_pPanelVMT->Function<PaintTraverse_t>(41)(vguiPanel, forceRepaint, allowForce);

	const char* pszPanelName = Kardoffl.m_pPanel->GetName(vguiPanel);
	bool isValidPanel = false;

	if (pszPanelName && pszPanelName[0] == 'F' && pszPanelName[5] == 'O')
		isValidPanel = true;

	if (isValidPanel)
	{
		Kardoffl.m_pEngine->GetScreenSize(sScreenSize2.iWidth, sScreenSize2.iHeight);

		if (!bResChange && (sScreenSize2.iWidth != sScreenSize.iWidth || sScreenSize2.iHeight != sScreenSize.iHeight))
		{
			sScreenSize.iWidth = sScreenSize2.iWidth;
			sScreenSize.iHeight = sScreenSize2.iHeight;
			bResChange = true;
		}

		if (bResChange)
		{
			Kardoffl.m_pDraw->FontInit(Kardoffl.m_pFont, "Tahoma", 13, FONTFLAG_OUTLINE);
			bResChange = false;
		}

		if (Kardoffl.m_pConvars->GetConVar("misc_antiuntrust")->GetBool()){
			Kardoffl.m_pConvars->GetConVar("removals_spread")->SetValue(0);
			Kardoffl.m_pConvars->GetConVar("misc_airstuck")->SetValue(0);
			if (Kardoffl.m_pConvars->GetConVar("misc_antiaim")->GetInt() > 4)
				Kardoffl.m_pConvars->GetConVar("misc_antiaim")->SetValue(4);
		}

		if (Kardoffl.m_pEngine->IsTakingScreenshot())
			return;

		if (Kardoffl.m_pEngine->IsInGame())
		{
			if (Kardoffl.m_pConvars->GetConVar("esp_active")->GetBool())
				Kardoffl.m_pESP->DrawESP();

			if (Kardoffl.m_pConvars->GetConVar("misc_crosshair")->GetBool())
			{
				Kardoffl.m_pDraw->FillRGBA((sScreenSize.iWidth / 2) - 5, (sScreenSize.iHeight / 2) - 1, 11, 3, 0, 0, 0, 255);
				Kardoffl.m_pDraw->FillRGBA((sScreenSize.iWidth / 2) - 1, (sScreenSize.iHeight / 2) - 5, 3, 11, 0, 0, 0, 255);
				Kardoffl.m_pDraw->FillRGBA((sScreenSize.iWidth / 2) - 4, (sScreenSize.iHeight / 2), 9, 1, 255, 255, 255, 255);
				Kardoffl.m_pDraw->FillRGBA((sScreenSize.iWidth / 2), (sScreenSize.iHeight / 2) - 4, 1, 9, 255, 255, 255, 255);
			}
		}

		if (Kardoffl.m_pConvars->GetConVar("misc_watermark")->GetBool())
			Kardoffl.m_pDraw->Text(5, 5, 255, 255, 255, 255, 0, Kardoffl.m_pFont, "Prediction - Undetection is the key.");

		if (Kardoffl.m_pConvars->GetConVar("misc_fovwarning")->GetBool() && Kardoffl.m_pEngine->IsInGame())
		{
			if ((Kardoffl.m_pConvars->GetConVar("aimbot_active")->GetInt() == 2 && Kardoffl.m_pConvars->GetConVar("aimbot_fov")->GetInt() > 3))
				Kardoffl.m_pDraw->Text(5, 20, 255, 255, 255, 255, 0, Kardoffl.m_pFont, "Warning! FOV is over 3!");
		}


		Kardoffl.m_pMenu->DrawMenu();
	}
}

#define WEAPON_SPREAD_OFFSET  0x788

QAngle GetSpreadAngle(UINT iSeed, CUserCmd* cmd, C_BaseCombatWeapon* Weapon)
{
	typedef float(__thiscall *getfloat)(void*);
	typedef void(__thiscall *funcvoid)(void*);
	((funcvoid)((*(DWORD*)(*(DWORD*)(Weapon)+WEAPON_SPREAD_OFFSET + 0x8))))(Weapon);
	const float constInaccuracy = ((getfloat)((*(DWORD*)(*(DWORD*)(Weapon)+WEAPON_SPREAD_OFFSET))))(Weapon);
	const float constSpread = ((getfloat)((*(DWORD*)(*(DWORD*)(Weapon)+WEAPON_SPREAD_OFFSET + 0x4))))(Weapon);
	QAngle view(cmd->viewangles);
	Vector direction, right, up;
	AngleVectors(view, &direction, &right, &up);
	iSeed &= 0xFF;
	RandomSeed(iSeed + 1);
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
		angles -= view;
		NormalizeAngles(angles);
		return angles;
	}
	return QAngle(0, 0, 0);
}

bool WasShooting = false;

void DoAntiaim(CUserCmd* cmd)
{
	if (Kardoffl.m_pConvars->GetConVar("misc_antiaim")->GetInt() == 1){
		static bool chocked = false;
		if (!chocked) {
			cmd->viewangles.y = 135.0f;
			chocked = true;
			//bSendPacket = false;
		}
		else {
			cmd->viewangles.y = -45.0f;
			chocked = false;
			//bSendPacket = true;
		}

		cmd->viewangles.x = 89.0f;
	}
	else if (Kardoffl.m_pConvars->GetConVar("misc_antiaim")->GetInt() == 2){
		cmd->viewangles.y = cmd->viewangles.y - (90.f + (float)(rand() % 55));
		cmd->viewangles.x = 89.0f;
	}
	else if (Kardoffl.m_pConvars->GetConVar("misc_antiaim")->GetInt() == 3){
		cmd->viewangles.y = cmd->viewangles.y + 90.f + (float)(rand() % 360);
		cmd->viewangles.x = 89.0f;
	}
	else if (Kardoffl.m_pConvars->GetConVar("misc_antiaim")->GetInt() == 4){
		cmd->viewangles.y = cmd->viewangles.y + 90.0f + (float)(rand() % 360);
		cmd->viewangles.y = cmd->viewangles.y + 150.0f + (float)(rand() % 360);
		cmd->viewangles.y = cmd->viewangles.y - 43.0f + (float)(rand() % 360);
		cmd->viewangles.y = cmd->viewangles.y - 44.0f + (float)(rand() % 360);
		cmd->viewangles.x = 89.0f;
	}
	else if (Kardoffl.m_pConvars->GetConVar("misc_antiaim")->GetInt() == 5 && !Kardoffl.m_pConvars->GetConVar("misc_antiuntrust")->GetBool()){
		cmd->viewangles.y -= 180;
		cmd->viewangles.x = 180;
	}

	if (Kardoffl.m_pConvars->GetConVar("misc_airstuck")->GetBool() && !Kardoffl.m_pConvars->GetConVar("misc_antiuntrust")->GetBool()){
		static bool bStuck = false;

		if (GetAsyncKeyState('X') & 1)
			bStuck = !bStuck;

		if (bStuck)
			cmd->tick_count = INT_MAX;
	}


	if (Kardoffl.m_pConvars->GetConVar("misc_fakelag")->GetBool()){
		int iChoke = 12;
		static int iFakeLag = 0;

		if (!WasShooting)
		{
			if (iFakeLag <= iChoke && iFakeLag > 0)
			{
				bSendPacket = false;
			}
			else
			{
				bSendPacket = true;
				iFakeLag = 0;
			}
		}
		else
		{
			if (bSendPacket)
				WasShooting = false;
			else
				WasShooting = true;
		}

		iFakeLag++;
	}
}

void __stdcall new_CreateMove(int sequence_number)
{
	if (Kardoffl.m_pInput)
	{
		CUserCmd* cmd = Kardoffl.m_pInput->GetUserCmd(-1, sequence_number);
		CVerifiedUserCmd* vcmd = *(CVerifiedUserCmd**)((DWORD)Kardoffl.m_pInput + 0xF0) + (sequence_number % 150);

		bSendPacket = 1;

		if (!cmd || !vcmd || !Kardoffl.m_pEntity->Ent())
			return;

		C_BaseCombatWeapon*	pWeapon = Kardoffl.m_pEntity->GetBaseCombatWeapon(Kardoffl.m_pEntity->Ent());

		if (Kardoffl.m_pConvars->GetConVar("misc_bunnyhop")->GetBool() && !(Kardoffl.m_pEntity->GetFlags(Kardoffl.m_pEntity->Ent()) & FL_ONGROUND) && !(cmd->buttons & IN_BACK) && !(cmd->buttons & IN_FORWARD))
		{
			if (Kardoffl.m_pConvars->GetConVar("misc_autostrafer")->GetInt() == 1)
			{
				static float OldY;

				float Difference = AngleNormalize(cmd->viewangles.y - OldY);

				if (Difference > 0.0f)
				{
					cmd->sidemove = -400;
				}
				if (Difference < 0.0f)
				{
					cmd->sidemove = 400;
				}

				OldY = cmd->viewangles.y;
			}
			else if (Kardoffl.m_pConvars->GetConVar("misc_autostrafer")->GetInt() == 2)
			{
				Vector vVelocity = Kardoffl.m_pEntity->GetVelocity(Kardoffl.m_pEntity->Ent());
				float Speed = sqrt(vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y);

				float Tickrate = 1.1;

				static float OldY;
				float Difference = AngleNormalize(cmd->viewangles.y - OldY);

				float Value = (8.15 - Tickrate) - (Speed / 340);

				if (Speed > 160 && Speed < 420)
				{
					Value = (4.6 - Tickrate) - (Speed / 340);
				}
				if (Speed > 420)
				{
					Value = (3 - Tickrate) - (Speed / 1000);
				}
				if (Value <= 0.275)
				{
					Value = 0.275;
				}

				if (abs(Difference) < Value)
				{
					static bool Flip = false;
					if (Flip)
					{
						cmd->viewangles.y -= Value;
						cmd->sidemove = -400;
						Flip = false;
					}
					else
					{
						cmd->viewangles.y += Value;
						cmd->sidemove = 400;
						Flip = true;
					}
				}
				else
				{
					if (Difference > 0.0f)
					{
						cmd->sidemove = -400;
					}
					if (Difference < 0.0f)
					{
						cmd->sidemove = 400;
					}
				}

				OldY = cmd->viewangles.y;
			}
		}

		QAngle qOldView(cmd->viewangles);

		if (pWeapon)
		{
			float flCurTime = float(*(int*)((DWORD)Kardoffl.m_pEntity->Ent() + Kardoffl.m_pOffsets->TickBase) * Kardoffl.m_pGlobals->interval_per_tick);
			float flNextAttack = *(float*)((DWORD)pWeapon + Kardoffl.m_pOffsets->NextPrimaryAttack);
			static int iChokedPackets = 0;

			Prediction(cmd, (C_BasePlayer*)Kardoffl.m_pEntity->Ent());

			if ((Kardoffl.m_pConvars->GetConVar("aimbot_active")->GetInt() == 1 || (Kardoffl.m_pConvars->GetConVar("aimbot_active")->GetInt() == 2 && GetAsyncKeyState(VK_LBUTTON))) && !Kardoffl.m_pUtilities->IsKnife(pWeapon) && !Kardoffl.m_pUtilities->IsMisc(pWeapon))
			{
				Aimbot(cmd, pWeapon, Kardoffl.m_pConvars->GetConVar("aimbot_spot")->GetInt());

				if (cmd->buttons & IN_ATTACK)
				{
					if (Kardoffl.m_pConvars->GetConVar("aimbot_silent")->GetInt() == 2.0f)
					{
						if ((flNextAttack <= flCurTime))
						{
							if (iChokedPackets <= 10)
							{
								++iChokedPackets;
								bSendPacket = false;

								if (iTarget > -1)
									cmd->viewangles = qAngle;

								if (Kardoffl.m_pConvars->GetConVar("removals_spread")->GetBool() && !Kardoffl.m_pConvars->GetConVar("misc_antiuntrust")->GetBool())
									cmd->viewangles += GetSpreadAngle(cmd->random_seed, cmd, pWeapon);

								if (Kardoffl.m_pConvars->GetConVar("removals_recoil")->GetBool())
									cmd->viewangles -= Kardoffl.m_pEntity->GetPunchAngle(Kardoffl.m_pEntity->Ent()) * 2;
							}
							else
							{
								iChokedPackets = 0;
								cmd->buttons &= ~IN_ATTACK;
								bSendPacket = 1;
							}
						}
						else
						{
							if (Kardoffl.m_pConvars->GetConVar("misc_autopistol")->GetBool())
								cmd->buttons &= ~IN_ATTACK;
						}
					}
					else
					{
						if (Kardoffl.m_pConvars->GetConVar("removals_spread")->GetBool() && !Kardoffl.m_pConvars->GetConVar("misc_antiuntrust")->GetBool())
							cmd->viewangles += GetSpreadAngle(cmd->random_seed, cmd, pWeapon);

						if (Kardoffl.m_pConvars->GetConVar("removals_recoil")->GetBool())
							cmd->viewangles -= Kardoffl.m_pEntity->GetPunchAngle(Kardoffl.m_pEntity->Ent()) * 2;

						if (Kardoffl.m_pConvars->GetConVar("misc_autopistol")->GetBool())
						{
							if (flNextAttack > flCurTime)
							{
								cmd->buttons &= ~IN_ATTACK;
							}
						}
					}
				}
				else
				{
					if (Kardoffl.m_pConvars->GetConVar("aimbot_silent")->GetInt() == 2)
					{
						iChokedPackets = 0;
						bSendPacket = true;
						cmd->viewangles = qOldView;
					}
				}
			}

			if (Kardoffl.m_pConvars->GetConVar("aimbot_active")->GetBool() && Kardoffl.m_pConvars->GetConVar("aimbot_silent")->GetInt() == 2)
			{
				if ((cmd->buttons & IN_ATTACK && bSendPacket) || !(cmd->buttons & IN_ATTACK))
				{
					DoAntiaim(cmd);
					WasShooting = false;
				}
				else
					WasShooting = true;
			}
			else
			{
				if (!(cmd->buttons & IN_ATTACK))
				{
					DoAntiaim(cmd);
					WasShooting = false;
				}
				else
					WasShooting = true;
			}
			
			if (Kardoffl.m_pConvars->GetConVar("triggerbot_active")->GetBool())
			{
				if (Kardoffl.m_pConvars->GetConVar("triggerbot_active")->GetInt() == 1)
				{
					if (GetAsyncKeyState(0x06))
						Triggerbot(cmd);
				}
				else
				{
					Triggerbot(cmd);
				}
			}

			if (Kardoffl.m_pConvars->GetConVar("misc_bunnyhop")->GetBool())
			{
				if (cmd->buttons & IN_JUMP && !(Kardoffl.m_pEntity->GetFlags(Kardoffl.m_pEntity->Ent()) & FL_ONGROUND))
					cmd->buttons &= ~IN_JUMP;
			}
		}

		CL_FixMove(cmd, qOldView);

		if (Kardoffl.m_pConvars->GetConVar("misc_antiuntrust")->GetBool())
			ClampAngles(cmd->viewangles);
		else
			NormalizeAngles(cmd->viewangles);

		vcmd->m_cmd = *cmd;
		vcmd->m_crc = cmd->GetChecksum();
	}
}

void __stdcall new_FrameStageNotify(ClientFrameStage_t curStage)
{
	Kardoffl.m_pClientVMT->Function<FrameStageNotify_t>(36)(curStage);

	if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START && Kardoffl.m_pEngine->IsInGame() && Kardoffl.m_pEntity->Ent())
	{
		int iBayonet = Kardoffl.m_pModelinfo->GetModelIndex("models/weapons/v_knife_bayonet.mdl");
		int iButterfly = Kardoffl.m_pModelinfo->GetModelIndex("models/weapons/v_knife_butterfly.mdl");
		int iFlip = Kardoffl.m_pModelinfo->GetModelIndex("models/weapons/v_knife_flip.mdl");
		int iGunGame = Kardoffl.m_pModelinfo->GetModelIndex("models/weapons/v_knife_gg.mdl");
		int iGut = Kardoffl.m_pModelinfo->GetModelIndex("models/weapons/v_knife_gut.mdl");
		int iKarambit = Kardoffl.m_pModelinfo->GetModelIndex("models/weapons/v_knife_karam.mdl");
		int iM9Bayonet = Kardoffl.m_pModelinfo->GetModelIndex("models/weapons/v_knife_m9_bay.mdl");
		int iHuntsman = Kardoffl.m_pModelinfo->GetModelIndex("models/weapons/v_knife_tactical.mdl");
		int iFalchion = Kardoffl.m_pModelinfo->GetModelIndex("models/weapons/v_knife_falchion_advanced.mdl");
		int iDagger = Kardoffl.m_pModelinfo->GetModelIndex("models/weapons/v_knife_push.mdl");

		for (int i = Kardoffl.m_pEntlist->GetHighestEntityIndex(); i >= 0; --i)
		{
			C_BaseCombatWeapon* Weapon = (C_BaseCombatWeapon*)Kardoffl.m_pEntlist->GetClientEntity(i);

			if (Weapon == NULL)
				continue;

			if (GetOwnerEntity(Weapon) == NULL
				|| GetOwnerEntity(Weapon)->entindex() == 0
				|| Kardoffl.m_pEntity->Ent()->entindex() != GetOwnerEntity(Weapon)->entindex()
				|| Weapon->GetIClientEntity()->GetClientClass() == NULL)
				continue;

			std::string strWeaponName = Weapon->GetIClientEntity()->GetClientClass()->GetName();

			if (strWeaponName.empty())
				continue;

			if (strWeaponName.find("CWeapon") != std::string::npos
				|| strWeaponName.find("47") != std::string::npos
				|| strWeaponName.find("DEagle") != std::string::npos
				|| strWeaponName.find("Knife") != std::string::npos)
			{
				if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_cz75)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_cz")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_deagle)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_deagle")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_elite)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_elites")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_fiveseven)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_fiveseven")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_glock)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_glock")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_p2000)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_p2000")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_usp)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_usp")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_p250)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_p250")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_tec9)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_tec9")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_ak47)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_ak")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_aug)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_aug")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_famas)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_famas")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_galilar)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_galil")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_m4a4)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_m4a4")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_m4a1s)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_m4a1s")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_bizon)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_bizon")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_mac10)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_mac10")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_mp7)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_mp7")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_mp9)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_mp9")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_p90)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_p90")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_ump45)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_ump45")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_m249)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_m249")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_negev)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_negev")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_mag7)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_mag7")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_nova)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_nova")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_sawedoff)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_sawed")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_xm1014)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_xm1014")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_awp)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_awp")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_g3sg1)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_g3sg1")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_scar20)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_scar20")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_ssg08)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_ssg08")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_sg556)
				{
					*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_sg556")->GetInt();
				}
				else if (Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_knife_t
					|| Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_knife_ct
					|| Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_bayonet
					|| Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_gut
					|| Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_huntsman
					|| Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_karambit
					|| Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_m9bayonet
					|| Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_flip
					|| Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_butterfly
					|| Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_falchion
					|| Kardoffl.m_pEntity->GetWeaponID(Weapon) == weapon_pushdagger)
				{
					int iKnife = Kardoffl.m_pConvars->GetConVar("misc_knife")->GetInt();

					if (iKnife == 1)
					{
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->ViewModelIndex) = iBayonet;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->WorldModelIndex) = iBayonet + 1;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->ItemID) = 500;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_bayonet")->GetInt();
						//*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets-> Seed) = 0;
					}
					else if (iKnife == 2)
					{
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->ViewModelIndex) = iFlip;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->WorldModelIndex) = iFlip + 1;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->ItemID) = 505;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_flip")->GetInt();
						//*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets-> Seed) = 0;
					}
					else if (iKnife == 3)
					{
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->ViewModelIndex) = iGut;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->WorldModelIndex) = iGut + 1;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->ItemID) = 506;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_gut")->GetInt();
						//*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets-> Seed) = 0;
					}
					else if (iKnife == 4)
					{
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->ViewModelIndex) = iKarambit;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->WorldModelIndex) = iKarambit + 1;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->ItemID) = 507;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_karambit")->GetInt();
						//*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets-> Seed) = 0;
					}
					else if (iKnife == 5)
					{
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->ViewModelIndex) = iM9Bayonet;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->WorldModelIndex) = iM9Bayonet + 1;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->ItemID) = 508;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_m9")->GetInt();
						//*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets-> Seed) = 0;
					}
					else if (iKnife == 6)
					{
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->ViewModelIndex) = iHuntsman;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->WorldModelIndex) = iHuntsman + 1;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->ItemID) = 509;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_huntsman")->GetInt();
						//*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets-> Seed) = 0;
					}
					else if (iKnife == 7)
					{
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->ViewModelIndex) = iButterfly;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->WorldModelIndex) = iButterfly + 1;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->ItemID) = 515;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_butterfly")->GetInt();
						//*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets-> Seed) = 0;
					}
					else if (iKnife == 8)
					{
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->ViewModelIndex) = iFalchion;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->WorldModelIndex) = iFalchion + 1;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->ItemID) = 512;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_falchion")->GetInt();
						//*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets-> Seed) = 0;
					}
					else if (iKnife == 9)
					{
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->ViewModelIndex) = iDagger;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->WorldModelIndex) = iDagger + 1;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->ItemID) = 516;
						*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->PaintKit) = Kardoffl.m_pConvars->GetConVar("skin_dagger")->GetInt();
						//*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets-> Seed) = 0;
					}
				}

				*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->OwnerXuidLow) = 0;
				*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->OwnerXuidHigh) = 0;
				*(float*)((DWORD)Weapon + Kardoffl.m_pOffsets->Wear) = 0.001f;
				*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->StatTrak) = 1;
				*(int*)((DWORD)Weapon + Kardoffl.m_pOffsets->ItemIDHigh) = 1;
			}
		}
	}
}
