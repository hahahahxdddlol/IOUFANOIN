#include "esp.h"

void DrawHealthbar(int Health, Vector vScreen, int x, int y)
{
	int r = 255 - (Health * 2.55);
	int g = Health * 2.55;
	Kardoffl.m_pDraw->FillRGBA(vScreen.x - x - 1, vScreen.y - y - 1, 5, 43, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(vScreen.x - x, vScreen.y - y, 3, 41, r, g, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(vScreen.x - x, vScreen.y - y, 3, 40 - (Health / 2.5), 0, 0, 0, 255);
}

void DrawCornerBox(int x, int y, int w, int h, int borderPx, int r, int g, int b, int a)
{
	//Blackborder
	Kardoffl.m_pDraw->FillRGBA((x - (w / 2)) - 1, (y - h + borderPx) - 1, (w / 3) + 2, borderPx + 2, 0, 0, 0, 255); //top
	Kardoffl.m_pDraw->FillRGBA((x - (w / 2) + w - w / 3) - 1, (y - h + borderPx) - 1, w / 3, borderPx + 2, 0, 0, 0, 255); //top
	Kardoffl.m_pDraw->FillRGBA(x - (w / 2) - 1, (y - h + borderPx), borderPx + 2, (w / 3) + 1, 0, 0, 0, 255); //left 
	Kardoffl.m_pDraw->FillRGBA(x - (w / 2) - 1, ((y - h + borderPx) + h - w / 3) - 1, borderPx + 2, (w / 3) + 2, 0, 0, 0, 255); //left 
	Kardoffl.m_pDraw->FillRGBA(x - (w / 2), y - 1, (w / 3) + 1, borderPx + 2, 0, 0, 0, 255); //bottom 
	Kardoffl.m_pDraw->FillRGBA(x - (w / 2) + w - (w / 3 + 1), y - 1, (w / 3) + 2, borderPx + 2, 0, 0, 0, 255); //bottom 
	Kardoffl.m_pDraw->FillRGBA((x + w - borderPx) - (w / 2) - 1, (y - h + borderPx) - 1, borderPx + 2, w / 3 + 2, 0, 0, 0, 255); //right 
	Kardoffl.m_pDraw->FillRGBA((x + w - borderPx) - (w / 2) - 1, ((y - h + borderPx) + h - w / 3) - 1, borderPx + 2, (w / 3) + 2, 0, 0, 0, 255); //right 
	//Color
	Kardoffl.m_pDraw->FillRGBA(x - (w / 2), (y - h + borderPx), w / 3, borderPx, r, g, b, a); //top
	Kardoffl.m_pDraw->FillRGBA(x - (w / 2) + w - w / 3, (y - h + borderPx), w / 3, borderPx, r, g, b, a); //top
	Kardoffl.m_pDraw->FillRGBA(x - (w / 2), (y - h + borderPx), borderPx, w / 3, r, g, b, a); //left 
	Kardoffl.m_pDraw->FillRGBA(x - (w / 2), (y - h + borderPx) + h - w / 3, borderPx, w / 3, r, g, b, a); //left 
	Kardoffl.m_pDraw->FillRGBA(x - (w / 2), y, w / 3, borderPx, r, g, b, a); //bottom 
	Kardoffl.m_pDraw->FillRGBA(x - (w / 2) + w - w / 3, y, w / 3, borderPx, r, g, b, a); //bottom 
	Kardoffl.m_pDraw->FillRGBA((x + w - borderPx) - (w / 2), (y - h + borderPx), borderPx, w / 3, r, g, b, a); //right 
	Kardoffl.m_pDraw->FillRGBA((x + w - borderPx) - (w / 2), (y - h + borderPx) + h - w / 3, borderPx, w / 3, r, g, b, a); //right 
}

void CESP::DrawHeader(int x, int y, int w, int r, int g, int b, int HealthBarWidth)
{
	int i = 0;
	int xCoord = x;
	int yCoord = y;

	// Male Male :)
	for (i = 0; i < 5; i++)
	{
		Kardoffl.m_pDraw->FillRGBA(x - i, y + i, w, 1, 0, 0, 0, 255);
		Kardoffl.m_pDraw->FillRGBA(x - i, y + i, HealthBarWidth + 2, 1, r, g, b, 255);
		Kardoffl.m_pDraw->FillRGBA(x - 3, y + 3, HealthBarWidth + 2, 1, (int)(r / 1.5f), (int)(g / 1.5f), (int)(b / 1.5f), 255);
		Kardoffl.m_pDraw->FillRGBA(x - 4, y + 4, HealthBarWidth + 2, 1, (int)(r / 1.5f), (int)(g / 1.5f), (int)(b / 1.5f), 255);
	}

	// Oben
	Kardoffl.m_pDraw->FillRGBA(x, y, w, 1, 255, 255, 255, 255);

	// Unten
	Kardoffl.m_pDraw->FillRGBA((x + 1) - 5, y + 5, w, 1, 255, 255, 255, 255);

	for (i = 0; i < 5; i++)
	{
		// Schräg links
		Kardoffl.m_pDraw->FillRGBA(x, y, 1, 1, 255, 255, 255, 255);
		x--;
		y++;
	}

	x = xCoord;
	y = yCoord;

	for (i = 0; i < 5; i++)
	{
		// Schräg rechts
		if (i != 0)
			Kardoffl.m_pDraw->FillRGBA(x + w, y, 1, 1, 255, 255, 255, 255);

		x--;
		y++;
	}
}

void CESP::DrawESP(void)
{
	if (!Kardoffl.m_pEntity->Ent())
		return;

	player_info_t pInfo;
	Vector vScreen, vSpot, vWorldPosition, vHead, out[9];
	C_BaseCombatWeapon* pWeapon = Kardoffl.m_pEntity->GetBaseCombatWeapon(Kardoffl.m_pEntity->Ent());
	//----------------------------------//
	for (int index = Kardoffl.m_pEntlist->GetHighestEntityIndex(); index >= 1; index--)
	{
		if (index == Kardoffl.m_pEngine->GetLocalPlayer())
			continue;

		IClientEntity* ClientEntity = Kardoffl.m_pEntlist->GetClientEntity(index);

		if (ClientEntity == NULL
			|| ClientEntity->IsDormant())
			continue;

		if (!Kardoffl.m_pEngine->GetPlayerInfo(index, &pInfo))
			continue;

		int iTeamIndex = Kardoffl.m_pEntity->GetTeamID(ClientEntity);

		int iTeamColorR = 255,
			iTeamColorG = 255,
			iTeamColorB = 255;

		if (GetVisible(Kardoffl.m_pEntity->GetEyePos(Kardoffl.m_pEntity->Ent()), Kardoffl.m_pEntity->GetEyePos(ClientEntity), ClientEntity))
		{
			if (iTeamIndex == 2)
			{
				iTeamColorR = 255;
				iTeamColorG = 0;
				iTeamColorB = 0;
			}
			else if (iTeamIndex == 3)
			{
				iTeamColorR = 0;
				iTeamColorG = 0;
				iTeamColorB = 255;
			}
		}
		else
		{
			if (iTeamIndex == 2)
			{
				iTeamColorR = 255;
				iTeamColorG = 255;
				iTeamColorB = 0;
			}
			else if (iTeamIndex == 3)
			{
				iTeamColorR = 0;
				iTeamColorG = 255;
				iTeamColorB = 0;
			}
		}

		Kardoffl.m_pUtilities->GetWorldSpaceCenter(ClientEntity, vWorldPosition);

		if (!Kardoffl.m_pEntity->GetLifeState(ClientEntity))
			continue;

		if (Kardoffl.m_pUtilities->WorldToScreen(vWorldPosition, vScreen))
		{
			int iHealth = clamp(Kardoffl.m_pEntity->GetHealth(ClientEntity), 0, 100);
			int iFlags = Kardoffl.m_pEntity->GetFlags(ClientEntity);

			Vector vPlayerFoot = Kardoffl.m_pEntity->GetOrigin(ClientEntity);
			Vector vPlayerHead;
			if (iFlags & FL_DUCKING)
				vPlayerHead = vPlayerFoot + Vector(0, 0, 52);
			else
				vPlayerHead = vPlayerFoot + Vector(0, 0, 72);

			Vector vPlayerFootScreen, vPlayerHeadScreen;
			Kardoffl.m_pUtilities->WorldToScreen(vPlayerFoot, vPlayerFootScreen);
			Kardoffl.m_pUtilities->WorldToScreen(vPlayerHead, vPlayerHeadScreen);

			float width = (vPlayerFootScreen.y - vPlayerHeadScreen.y) / 4;
			float height = (vPlayerFootScreen.x - vPlayerHeadScreen.x) / 4;
			int h = vPlayerFootScreen.y - vPlayerHeadScreen.y;

			if (Kardoffl.m_pUtilities->WorldToScreen(Kardoffl.m_pEntity->GetEyePos(ClientEntity), vPlayerHead))
			{
				DrawCornerBox(vPlayerHeadScreen.x, vPlayerFootScreen.y, width * 2, h, 1, iTeamColorR, iTeamColorG, iTeamColorB, 255);

				Kardoffl.m_pDraw->Text(vPlayerHeadScreen.x + 2, vPlayerHeadScreen.y - 8, 255, 255, 255, 255, 1, Kardoffl.m_pFont, pInfo.name);

				DrawHeader(vPlayerHeadScreen.x - width + 2, vPlayerFootScreen.y + 5, width * 2, iTeamColorR, iTeamColorG, iTeamColorB, (iHealth / (100 / (width * 2))) - 2);
			}
		}
	}
}