#include "Cvars.h"

CVars::CVars()
{
	m_Type = CVar_NONE;
	m_bBool = false;
	m_iInt = 0;
	m_flFloat = 0.0f;
	m_szVarName = "\0";
	m_szVarDescription = "\0";
}

CVars::CVars(const char* szName, const char* szDescription)
{
	m_Type = CVar_NONE;
	m_bBool = false;
	m_iInt = 0;
	m_flFloat = 0.0f;
	m_szVarName = szName;
	m_szVarDescription = szDescription;
}

int CVars::GetInt(void)
{
	if (m_Type == CVar_INT)
		return m_iInt;
	else if (m_Type == CVar_FLOAT)
		return int(m_flFloat);
	else if (m_Type == CVar_BOOL)
		return m_bBool ? 1 : 0;
	else
		return 0;
}

bool CVars::GetBool(void)
{
	if (m_Type == CVar_INT)
		if (m_iInt > 0)
			return true;
		else
			return false;
	else if (m_Type == CVar_FLOAT)
		if (m_flFloat > 0)
			return true;
		else
			return false;
	else if (m_Type == CVar_BOOL)
		return m_bBool;
	else
		return false;
}

float CVars::GetFloat(void)
{
	if (m_Type == CVar_INT)
		return float(m_iInt);
	else if (m_Type == CVar_FLOAT)
		return m_flFloat;
	else if (m_Type == CVar_BOOL)
		return float(m_bBool);
	else
		return 0.0f;
}

std::string CVars::GetVarName(void)
{
	return m_szVarName;
}

std::string CVars::GetDescription(void)
{
	return m_szVarDescription;
}

std::string CVars::GetInfoString(int stringType)
{
	char szInfoString[256];

	if (stringType == CVar_NONE || stringType == CVar_FLOAT)
		sprintf_s(szInfoString, "%s = %.2f", m_szVarName.c_str(), GetFloat());
	else if (stringType == CVar_INT)
		sprintf_s(szInfoString, "%s = %d", m_szVarName.c_str(), GetInt());
	else if (stringType == CVar_BOOL)
		sprintf_s(szInfoString, "%s is %s", m_szVarName.c_str(), GetBool() ? "enabled" : "disabled");

	return szInfoString;
}

void CVars::SetValue(int iValue)
{
	m_Type = CVar_INT;
	m_iInt = iValue;
}

void CVars::SetValue(bool bValue)
{
	m_Type = CVar_BOOL;
	m_bBool = bValue;
}

void CVars::SetValue(float flValue)
{
	m_Type = CVar_FLOAT;
	m_flFloat = flValue;
}

CConVars::CConVars()
{
	m_vecConVars.push_back(new CVars("menu_x", "Menu X Position."));
	m_vecConVars.push_back(new CVars("menu_y", "Menu Y Position."));

	m_vecConVars.push_back(new CVars("aimbot_active", "Set aimbot type"));
	m_vecConVars.push_back(new CVars("aimbot_autoshoot", "Autoshoots if enemy is visible"));
	m_vecConVars.push_back(new CVars("aimbot_spot", "Set spot for the aimbot"));
	m_vecConVars.push_back(new CVars("aimbot_fov", "Set field of view for aimbot"));
	m_vecConVars.push_back(new CVars("aimbot_silent", "Silent & pSilent = Makes aimbot invisible for you (Spectator and GOTV can still see it)"));
	m_vecConVars.push_back(new CVars("aimbot_hitscan", "Scans for visible hitboxes"));
	m_vecConVars.push_back(new CVars("aimbot_autowall", "Aimbot calculates if wall is penetratable, if aimbot is set to autoshoot it will shoot through the wall if penetratable"));
	m_vecConVars.push_back(new CVars("aimbot_rcs", "Controls your recoil when aimbot kicks in"));
	m_vecConVars.push_back(new CVars("aimbot_smooth", "Smooths your aimbot movement"));
	m_vecConVars.push_back(new CVars("aimbot_stopcrouch", "Stops and crouches if player is visible"));

	m_vecConVars.push_back(new CVars("triggerbot_active", ""));
	m_vecConVars.push_back(new CVars("triggerbot_team", ""));
	m_vecConVars.push_back(new CVars("triggerbot_head", ""));
	m_vecConVars.push_back(new CVars("triggerbot_chest", ""));
	m_vecConVars.push_back(new CVars("triggerbot_arms", ""));
	m_vecConVars.push_back(new CVars("triggerbot_legs", ""));

	m_vecConVars.push_back(new CVars("esp_active", "Enables/disables ESP"));

	m_vecConVars.push_back(new CVars("misc_airstuck", "Makes you get stuck in air if you press X while in air. (Can lead to an untrust ban)"));
	m_vecConVars.push_back(new CVars("misc_antiaim", ""));
	m_vecConVars.push_back(new CVars("misc_antiuntrust", "Disables some features wich can lead to an untrust ban"));
	m_vecConVars.push_back(new CVars("misc_autopistol", "Makes your pistol fire like an automatic rifle"));
	m_vecConVars.push_back(new CVars("misc_autostrafer", "1. Legit strafer 2. pAutostrafer (Number 2 will make screen shake sometimes)"));
	m_vecConVars.push_back(new CVars("misc_bunnyhop", "Allows you to jump countinously if you hold spacebar"));
	m_vecConVars.push_back(new CVars("misc_crosshair", "Draws a crosshair"));
	m_vecConVars.push_back(new CVars("misc_fakelag", "Simulates lag"));
	m_vecConVars.push_back(new CVars("misc_fovwarning", "Checks if aimbot is set to on shoot & if fov is over 3 it will display a warning in the corner, this is quite useful due to the pSilent fix"));
	m_vecConVars.push_back(new CVars("misc_knife", "Change knifemodel to any knife (Only visible for you)"));
	m_vecConVars.push_back(new CVars("misc_watermark", "Enable/disable watermark in the upper corner."));
	m_vecConVars.push_back(new CVars("misc_skinchanger", "Enable/disable Skinchanger and knifechanger."));

	m_vecConVars.push_back(new CVars("removals_recoil", "Removes Recoil"));
	m_vecConVars.push_back(new CVars("removals_spread", "Removes Spread (Do not use on valve official servers, will lead to untrust)"));

	m_vecConVars.push_back(new CVars("skin_deagle", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_elites", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_fiveseven", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_glock", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_cz", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_ak", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_aug", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_awp", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_famas", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_g3sg1", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_galil", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_m249", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_m4a4", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_m4a1s", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_mac10", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_p90", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_ump45", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_xm1014", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_bizon", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_mag7", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_negev", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_sawed", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_tec9", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_p2000", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_usp", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_mp7", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_mp9", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_nova", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_p250", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_scar20", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_sg556", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_ssg08", "Skin ID"));
	m_vecConVars.push_back(new CVars("skin_knifemodel", "Knife Model."));
	m_vecConVars.push_back(new CVars("skin_bayonet", "Knife Skin."));
	m_vecConVars.push_back(new CVars("skin_flip", "Knife Skin."));
	m_vecConVars.push_back(new CVars("skin_gut", "Knife Skin."));
	m_vecConVars.push_back(new CVars("skin_karambit", "Knife Skin."));
	m_vecConVars.push_back(new CVars("skin_m9", "Knife Skin."));
	m_vecConVars.push_back(new CVars("skin_huntsman", "Knife Skin."));
	m_vecConVars.push_back(new CVars("skin_butterfly", "Knife Skin."));
	m_vecConVars.push_back(new CVars("skin_falchion", "Knife Skin."));
	m_vecConVars.push_back(new CVars("skin_dagger", "Knife Skin."));
}

UINT CConVars::GetConVarIndexByName(std::string szVarName)
{
	if (szVarName.empty())
		return -1;

	for (UINT i = 0; i < m_vecConVars.size(); ++i)
		if (!m_vecConVars.at(i)->GetVarName().compare(szVarName)
			|| !strncmp(m_vecConVars.at(i)->GetVarName().c_str(), szVarName.c_str(), szVarName.size()))
			return i;

	return -1;
}

CVars* CConVars::GetConVar(std::string szVarName)
{
	UINT iIndex = Kardoffl.m_pConvars->GetConVarIndexByName(szVarName);
	if (iIndex == -1)
		return nullptr;

	return Kardoffl.m_pConvars->m_vecConVars.at(iIndex);
}