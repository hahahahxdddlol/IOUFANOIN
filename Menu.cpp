#include "Menu.h"

bool bSavePos = true;
bool bMouseOnce = true;
int SavedX, SavedY;

int iMenuRed = 141;
int iMenuGreen = 141;
int iMenuBlue = 255;
int iMenuWidth = 798;
int iMenuHeight = 430;
std::string strPath;

std::string Tabs[4] = { 
	"Aimbot", 
	"ESP & Visuals",
	"Misc",
	"Config" 
};

std::string AimbotActivation[3] = {
	"Off",
	"Auto",
	"On Shoot"
};

std::string AimbotStyle[3] = {
	"Normal",
	"Silent",
	"pSilent"
};

std::string TriggebotActivation[3] = {
	"Off",
	"On Mouse 5",
	"Auto"
};

std::string Autostrafer[3] = {
	"Off",
	"Assist",
	"Full"
};

struct sComboBox
{
	bool Open, Active;
	int iSelect;
	char* Name;
} cComboBox[22];

void ForceUpdate()
{
	DWORD dwRelAddr = *(DWORD*)(dwForceUpdate + 1);
	DWORD dwFunc = ((dwForceUpdate + 5) + dwRelAddr);

	__asm
	{
		pushad
		mov edi, dwClientState
		lea ecx, dword ptr[edi + 0x8]
		call dwFunc
		mov dword ptr[edi + 0x154], 0xFFFFFFFF
		popad
	}
}

void DrawInfo(const char* szText)
{
	Kardoffl.m_pDraw->Text(Kardoffl.m_pConvars->GetConVar("menu_x")->GetInt() + 5, Kardoffl.m_pConvars->GetConVar("menu_y")->GetInt() + iMenuHeight + 12, 255, 255, 255, 255, false, Kardoffl.m_pFont, szText);
}

int ReadInteger(const char* szSection, const char* szKey, int iDefaultValue, const char* szPath)
{
	return GetPrivateProfileIntA(szSection, szKey, iDefaultValue, szPath);
}

float ReadFloat(const char* szSection, const char* szKey, float fltDefaultValue, const char* szPath)
{
	char szResult[255];
	char szDefault[255];
	sprintf(szDefault, "%.02f", fltDefaultValue);
	GetPrivateProfileStringA(szSection, szKey, szDefault, szResult, 255, szPath);
	return atof(szResult);
}

void WriteInteger(const char* szSection, const char* szKey, int iValue, const char* szPath)
{
	char szValue[255];
	sprintf(szValue, "%d", iValue);
	WritePrivateProfileStringA(szSection, szKey, szValue, szPath);
}

void WriteFloat(const char* szSection, const char* szKey, float fltValue, const char* szPath)
{
	char szValue[255];
	sprintf(szValue, "%.02f", fltValue);
	WritePrivateProfileStringA(szSection, szKey, szValue, szPath);
}

void ReadOrWriteConfig(const char* szPath, bool bRead)
{
	for (int i = 0; i < Kardoffl.m_pConvars->m_vecConVars.size(); ++i)
	{
		char szName[256];
		char szFunction[256];
		bool bFunction = false;
		int iFunction = 0;

		for (int in = 0; in < Kardoffl.m_pConvars->m_vecConVars[i]->GetVarName().size(); ++in)
		{
			if (Kardoffl.m_pConvars->m_vecConVars[i]->GetVarName().c_str()[in] == '_')
			{
				szName[in] = 0;
				++iFunction;
				break;
			}

			szName[in] = Kardoffl.m_pConvars->m_vecConVars[i]->GetVarName().c_str()[in];
			++iFunction;
		}

		for (int in = 0; in <= (Kardoffl.m_pConvars->m_vecConVars[i]->GetVarName().size() - iFunction); ++in)
		{
			szFunction[in] = Kardoffl.m_pConvars->m_vecConVars[i]->GetVarName().c_str()[in + iFunction];
			szFunction[in + 1] = 0;
		}
#ifdef DEBUG
		Print("Convar: %s, Name: %s, Function: %s, Type: %i", Kardoffl.m_pConvars->m_vecConVars[i]->GetVarName().c_str(), szName, szFunction, Kardoffl.m_pConvars->m_vecConVars[i]->m_Type);
#endif
		if (Kardoffl.m_pConvars->m_vecConVars[i]->m_Type == CVar_INT)
		{
			if (bRead)
				Kardoffl.m_pConvars->m_vecConVars[i]->SetValue(ReadInteger(szName, szFunction, 0, szPath));
			else
				WriteInteger(szName, szFunction, Kardoffl.m_pConvars->m_vecConVars[i]->GetInt(), szPath);
		}
		else if (Kardoffl.m_pConvars->m_vecConVars[i]->m_Type == CVar_FLOAT)
		{
			if (bRead)
				Kardoffl.m_pConvars->m_vecConVars[i]->SetValue(ReadFloat(szName, szFunction, 0, szPath));
			else
				WriteFloat(szName, szFunction, Kardoffl.m_pConvars->m_vecConVars[i]->GetFloat(), szPath);
		}
		else if (Kardoffl.m_pConvars->m_vecConVars[i]->m_Type == CVar_BOOL)
		{
			if (bRead)
				Kardoffl.m_pConvars->m_vecConVars[i]->SetValue((ReadInteger(szName, szFunction, 0, szPath) > 0) ? true : false);
			else
				WriteInteger(szName, szFunction, Kardoffl.m_pConvars->m_vecConVars[i]->GetInt(), szPath);
		}
		else if (Kardoffl.m_pConvars->m_vecConVars[i]->m_Type == CVar_NONE)
		{
			if (bRead)
				Kardoffl.m_pConvars->m_vecConVars[i]->SetValue(ReadInteger(szName, szFunction, 0, szPath));
			else
				WriteInteger(szName, szFunction, Kardoffl.m_pConvars->m_vecConVars[i]->GetInt(), szPath);
		}
	}
}

void CMouse::MouseCursor()
{
	int x = iMouseX;
	int y = iMouseY;
	Kardoffl.m_pDraw->FillRGBA(x, y, 1, 17, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x, y + 16, 2, 1, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 2, y + 15, 1, 1, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 3, y + 14, 1, 1, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 4, y + 13, 1, 1, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 5, y + 14, 1, 2, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 6, y + 16, 1, 2, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 7, y + 18, 2, 1, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 8, y + 14, 1, 2, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 9, y + 16, 1, 2, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 7, y + 12, 1, 2, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 7, y + 12, 5, 1, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 11, y + 11, 1, 1, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 10, y + 10, 1, 1, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 9, y + 9, 1, 1, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 8, y + 8, 1, 1, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 7, y + 7, 1, 1, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 6, y + 6, 1, 1, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 5, y + 5, 1, 1, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 4, y + 4, 1, 1, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 3, y + 3, 1, 1, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 2, y + 2, 1, 1, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 1, y + 1, 1, 1, 0, 0, 0, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 1, y + 2, 1, 14, 255, 255, 255, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 2, y + 3, 1, 12, 255, 255, 255, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 3, y + 4, 1, 10, 255, 255, 255, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 4, y + 5, 1, 8, 255, 255, 255, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 5, y + 6, 1, 8, 255, 255, 255, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 6, y + 7, 1, 9, 255, 255, 255, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 7, y + 8, 1, 4, 255, 255, 255, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 8, y + 9, 1, 3, 255, 255, 255, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 9, y + 10, 1, 2, 255, 255, 255, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 10, y + 11, 1, 1, 255, 255, 255, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 7, y + 14, 1, 4, 255, 255, 255, 255);
	Kardoffl.m_pDraw->FillRGBA(x + 8, y + 16, 1, 2, 255, 255, 255, 255);
}

bool CMouse::IsInBox(int x, int y, int w, int h)
{
	if (iMouseX >= x &&
		iMouseX <= x + w &&
		iMouseY >= y &&
		iMouseY <= y + h)
		return true;
	else
		return false;
}

void CMouse::PositionHandler()
{
	HWND hWnd = FindWindowA(0, "Counter-Strike: Global Offensive");

	if (hWnd)
	{
		GetCursorPos(&Mouse);
		ScreenToClient(hWnd, &Mouse);
		iMouseX = Mouse.x;
		iMouseY = Mouse.y;
	}
}

void CMouse::ClickHandler()
{
	if ((GetAsyncKeyState(VK_LBUTTON)))
	{
		bMouse1pressed = true;
	}
	else if (!(GetAsyncKeyState(VK_LBUTTON)))
	{
		if (bMouse1pressed)
			bMouse1released = true;
		else
		{
			bMouse1released = false;
		}
		bMouse1pressed = false;
	}

	if ((GetAsyncKeyState(VK_RBUTTON)))
	{
		bMouse2pressed = true;
	}
	else if (!(GetAsyncKeyState(VK_RBUTTON)))
	{
		if (bMouse2pressed)
			bMouse2released = true;
		else
		{
			bMouse2released = false;
		}
		bMouse2pressed = false;
	}
}

CMenu::CMenu()
{
	iTab = 1;
	bLock = false;
	bOpen = false;
}

void CMenu::InitConfig(HMODULE hModule)
{
	char* szPath = new char[255];

	GetModuleFileNameA(hModule, szPath, 255);
	for (int i = strlen(szPath); i > 0; i--)
	{
		if (szPath[i] == '\\')
		{
			szPath[i + 1] = 0;
			break;
		}
	}

	strPath = szPath;
	delete[] szPath;

	strPath.append("config.cfg");

	if (!PathFileExistsA(strPath.c_str()))
	{
		HANDLE hFile = CreateFileA(strPath.c_str(), GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
#ifdef DEBUG
		if (hFile == INVALID_HANDLE_VALUE)
		{
			Print("config file could not be created.");
		}
#endif
		CloseHandle(hFile);

		Kardoffl.m_pConvars->GetConVar("menu_x")->SetValue(70);
		Kardoffl.m_pConvars->GetConVar("menu_y")->SetValue(200);
		Kardoffl.m_pConvars->GetConVar("aimbot_smooth")->SetValue(1);
		Kardoffl.m_pConvars->GetConVar("aimbot_fov")->SetValue(10);
		Kardoffl.m_pConvars->GetConVar("aimbot_spot")->SetValue(0);
		Kardoffl.m_pConvars->GetConVar("esp_active")->SetValue(1);
		Kardoffl.m_pConvars->GetConVar("misc_antiuntrust")->SetValue(1);
		Kardoffl.m_pConvars->GetConVar("misc_bunnyhop")->SetValue(1);
		Kardoffl.m_pConvars->GetConVar("misc_crosshair")->SetValue(1);
		Kardoffl.m_pConvars->GetConVar("misc_fovwarning")->SetValue(1);
		Kardoffl.m_pConvars->GetConVar("misc_watermark")->SetValue(1);
		ReadOrWriteConfig(strPath.c_str(), false);
	}
	else
	{
		ReadOrWriteConfig(strPath.c_str(), true);
	}
}

void CMenu::ReloadSkinsButton(int x, int y)
{
	if (Kardoffl.m_pMouse->IsInBox(x, y, 100, 15))
	{
		Kardoffl.m_pDraw->FillRGBA(x, y, 100, 15, iMenuRed, iMenuGreen, iMenuBlue, 255);
		if (Kardoffl.m_pMouse->bMouse1released)
		{
			ForceUpdate();
		}
	}
	else
	{
		Kardoffl.m_pDraw->FillRGBA(x, y, 100, 15, iMenuRed, iMenuGreen, iMenuBlue, 255);
	}

	Kardoffl.m_pDraw->Border(x, y, 100, 15, 1, 0, 0, 0, 255);

	Kardoffl.m_pDraw->Text(x + 50, y + 7, 255, 255, 255, 255, true, Kardoffl.m_pFont, "Force Update");
}

void CMenu::LoadButton(int x, int y)
{
	if (Kardoffl.m_pMouse->IsInBox(x, y, 100, 15))
	{
		Kardoffl.m_pDraw->FillRGBA(x, y, 100, 15, iMenuRed, iMenuGreen, iMenuBlue, 255);
		if (Kardoffl.m_pMouse->bMouse1released)
		{
			ReadOrWriteConfig(strPath.c_str(), true);
		}
	}
	else
	{
		Kardoffl.m_pDraw->FillRGBA(x, y, 100, 15, iMenuRed, iMenuGreen, iMenuBlue, 255);
	}

	Kardoffl.m_pDraw->Border(x, y, 100, 15, 1, 0, 0, 0, 255);

	Kardoffl.m_pDraw->Text(x + 50, y + 7, 255, 255, 255, 255, true, Kardoffl.m_pFont, "Load Config");
}

void CMenu::SaveButton(int x, int y)
{
	if (Kardoffl.m_pMouse->IsInBox(x, y, 100, 15))
	{
		Kardoffl.m_pDraw->FillRGBA(x, y, 100, 15, iMenuRed, iMenuGreen, iMenuBlue, 255);
		if (Kardoffl.m_pMouse->bMouse1released)
		{
			ReadOrWriteConfig(strPath.c_str(), false);
		}
	}
	else
	{
		Kardoffl.m_pDraw->FillRGBA(x, y, 100, 15, iMenuRed, iMenuGreen, iMenuBlue, 255);
	}

	Kardoffl.m_pDraw->Border(x, y, 100, 15, 1, 0, 0, 0, 255);

	Kardoffl.m_pDraw->Text(x + 50, y + 7, 255, 255, 255, 255, true, Kardoffl.m_pFont, "Save Config");
}

void CMenu::DrawWindow(int x, int y, int w, int h, const char* Name, ...)
{
	va_list va_alist;
	char szBuffer[1024] = { '\0' };

	va_start(va_alist, Name);
	vsprintf(szBuffer, Name, va_alist);
	va_end(va_alist);

	Kardoffl.m_pDraw->FillRGBA(x, y, w, h, 65, 65, 65, 255);//Background
	Kardoffl.m_pDraw->Border(x, y, w, h, 6, iMenuRed, iMenuGreen, iMenuBlue, 255);//Border
	Kardoffl.m_pDraw->FillRGBA(x, y, w, 28, iMenuRed, iMenuGreen, iMenuBlue, 255);//Topbar
	Kardoffl.m_pDraw->Text(x + (w / 2) - 2, (y + 14), 255, 255, 255, 255, true, Kardoffl.m_pFont, szBuffer);//Name
	Kardoffl.m_pDraw->Border(x, y, w + 5, h + 5, 1, 0, 0, 0, 255);//Black 1px Outline
	Kardoffl.m_pDraw->Border(x + 5, y + 28, w - 5, h - 28, 1, 0, 0, 0, 255);//Black 1px Inline
}

void CMenu::DrawGroupbox(int x, int y, int w, int h, const char* Name, ...)
{
	va_list va_alist;
	char szBuffer[1024] = { '\0' };

	va_start(va_alist, Name);
	vsprintf(szBuffer, Name, va_alist);
	va_end(va_alist);

	Kardoffl.m_pDraw->FillRGBA(x, y, w, h, 60, 60, 60, 255);//Background
	Kardoffl.m_pDraw->FillRGBA(x, y, w, 20, iMenuRed, iMenuGreen, iMenuBlue, 255);//Topbar
	Kardoffl.m_pDraw->Text(x + 3, y + 4, 255, 255, 255, 255, false, Kardoffl.m_pFont, szBuffer);
}

void CMenu::DrawCheckbox(int x, int y, CVars* cvar, const char* szName)
{
	Kardoffl.m_pDraw->FillRGBA(x + 2, y - 3, 14, 14, 40, 40, 40, 255);
	if (Kardoffl.m_pMouse->IsInBox(x + 2, y - 3, 14, 14))
	{
		Kardoffl.m_pDraw->Border(x + 2, y - 3, 13, 13, 1, iMenuRed, iMenuGreen, iMenuBlue, 255);
		DrawInfo(cvar->GetDescription().c_str());
		if (Kardoffl.m_pMouse->bMouse1released)
			cvar->SetValue(!cvar->GetBool());
	}
	if (cvar->GetBool())
	{
		Kardoffl.m_pDraw->FillRGBA(x + 4, y - 1, 10, 10, iMenuRed, iMenuGreen, iMenuBlue, 255);
	}
	Kardoffl.m_pDraw->Text(x + 20, y - 2, 255, 255, 255, 255, false, Kardoffl.m_pFont, szName);
}

void CMenu::DrawCheckbox(int x, int y, bool &toggle, const char* szName, const char* szDescription)
{
	Kardoffl.m_pDraw->FillRGBA(x + 2, y - 3, 14, 14, 40, 40, 40, 255);
	if (Kardoffl.m_pMouse->IsInBox(x + 2, y - 3, 14, 14))
	{
		Kardoffl.m_pDraw->Border(x + 2, y - 3, 13, 13, 1, iMenuRed, iMenuGreen, iMenuBlue, 255);
		DrawInfo(szDescription);
		if (Kardoffl.m_pMouse->bMouse1released && !bLock)
			toggle = !toggle;
	}
	if (toggle)
	{
		Kardoffl.m_pDraw->FillRGBA(x + 4, y - 1, 10, 10, iMenuRed, iMenuGreen, iMenuBlue, 255);
	}
	Kardoffl.m_pDraw->Text(x + 20, y - 2, 255, 255, 255, 255, false, Kardoffl.m_pFont, szName);
}

void CMenu::CloseAllComboBoxes()
{
	for (int n = 0; n <= 21; n++)
	{
		cComboBox[n].Open = false;
		cComboBox[n].Active = true;
	}
	bLock = false;
}

void CMenu::DrawCombobox(int x, int y, CVars* Selection, int num, int max, std::string* structs, const char* Name)
{
	cComboBox[num].Name = const_cast<char*>(structs[Selection->GetInt()].c_str());
	Kardoffl.m_pDraw->FillRGBA(x + 2, y - 3, 85, 2, iMenuRed, iMenuGreen, iMenuBlue, 255);//Top
	Kardoffl.m_pDraw->FillRGBA(x + 2, y - 3, 2, 22, iMenuRed, iMenuGreen, iMenuBlue, 255);//Left
	Kardoffl.m_pDraw->FillRGBA(x + 87, y - 3, 2, 22, iMenuRed, iMenuGreen, iMenuBlue, 255);//Right
	if (cComboBox[num].Active)
	{
		cComboBox[num].iSelect = Selection->GetInt();
		if (Kardoffl.m_pMouse->IsInBox(x + 25, y - 1, 69, 23) && Kardoffl.m_pMouse->bMouse1released && !bLock)
		{
			cComboBox[num].Open = true;
			cComboBox[num].Active = false;
		}
		Kardoffl.m_pDraw->FillRGBA(x + 2, y + 17, 85, 2, iMenuRed, iMenuGreen, iMenuBlue, 255);//Bottom
		Kardoffl.m_pDraw->Text(x + 45, y + 9, 255, 255, 255, 255, true, Kardoffl.m_pFont, cComboBox[num].Name);
		if (Kardoffl.m_pMouse->IsInBox(x + 2, y - 3, 85, 22) && !bLock)
		{
			DrawInfo(Selection->GetDescription().c_str());
		}
	}
	else
	{
		if (cComboBox[num].Open)
		{
			bLock = true;
			DrawInfo(Selection->GetDescription().c_str());
			for (int i = 0; i < max; i++)
			{
				Kardoffl.m_pDraw->FillRGBA(x + 2, y + (i * 20), 87, 20, 50, 50, 50, 220);//Back 1
				Kardoffl.m_pDraw->FillRGBA(x + 2, y - 1 + (i * 20), 2, 24, iMenuRed, iMenuGreen, iMenuBlue, 255);//Left * count
				Kardoffl.m_pDraw->FillRGBA(x + 87, y - 1 + (i * 20), 2, 24, iMenuRed, iMenuGreen, iMenuBlue, 255);//Right * count
				if (Kardoffl.m_pMouse->IsInBox(x + 6, y + 1 + (i * 20), 79, 20))
				{
					Kardoffl.m_pDraw->FillRGBA(x + 6, y + 1 + (i * 20), 79, 20, iMenuRed - 20, iMenuGreen - 20, iMenuBlue - 20, 255);//highlight selection
					if (Kardoffl.m_pMouse->bMouse1released)
					{
						cComboBox[num].iSelect = i;
						cComboBox[num].Name = const_cast<char*>(structs[cComboBox[num].iSelect].c_str());
						Selection->SetValue(cComboBox[num].iSelect);
						cComboBox[num].Open = false;
						cComboBox[num].Active = true;
						bLock = false;
					}
				}
				Kardoffl.m_pDraw->Text(x + 45, y + 10 + (i * 20), 255, 255, 255, 255, true, Kardoffl.m_pFont, structs[i].c_str());//List struct
			}
			Kardoffl.m_pDraw->FillRGBA(x + 2, y + 3 + (max * 20), 87, 2, iMenuRed, iMenuGreen, iMenuBlue, 255);//Bottom
		}
	}

	Kardoffl.m_pDraw->Text(x + 92, y + 1, 255, 255, 255, 255, false, Kardoffl.m_pFont, Name);
}

void CMenu::DrawCombobox(int x, int y, int &Selection, int num, int max, std::string* structs, const char* szName, const char* szDescription)
{
	cComboBox[num].Name = const_cast<char*>(structs[Selection].c_str());
	Kardoffl.m_pDraw->FillRGBA(x + 2, y - 3, 85, 2, iMenuRed, iMenuGreen, iMenuBlue, 255);//Top
	Kardoffl.m_pDraw->FillRGBA(x + 2, y - 3, 2, 22, iMenuRed, iMenuGreen, iMenuBlue, 255);//Left
	Kardoffl.m_pDraw->FillRGBA(x + 87, y - 3, 2, 22, iMenuRed, iMenuGreen, iMenuBlue, 255);//Right
	if (cComboBox[num].Active)
	{
		cComboBox[num].iSelect = Selection;
		if (Kardoffl.m_pMouse->IsInBox(x + 25, y - 1, 69, 23) && Kardoffl.m_pMouse->bMouse1released)
		{
			cComboBox[num].Open = true;
			cComboBox[num].Active = false;
		}
		Kardoffl.m_pDraw->FillRGBA(x + 2, y + 17, 85, 2, iMenuRed, iMenuGreen, iMenuBlue, 255);//Bottom
		Kardoffl.m_pDraw->Text(x + 45, y + 9, 255, 255, 255, 255, true, Kardoffl.m_pFont, cComboBox[num].Name);
		if (Kardoffl.m_pMouse->IsInBox(x + 2, y - 3, 85, 22) && !bLock)
		{
			DrawInfo(szDescription);
		}
	}
	else
	{
		if (cComboBox[num].Open)
		{
			bLock = true;
			DrawInfo(szDescription);
			for (int i = 0; i < max; i++)
			{
				Kardoffl.m_pDraw->FillRGBA(x + 2, y + (i * 20), 87, 20, 50, 50, 50, 220);//Back 1
				Kardoffl.m_pDraw->FillRGBA(x + 2, y - 1 + (i * 20), 2, 24, iMenuRed, iMenuGreen, iMenuBlue, 255);//Left * count
				Kardoffl.m_pDraw->FillRGBA(x + 87, y - 1 + (i * 20), 2, 24, iMenuRed, iMenuGreen, iMenuBlue, 255);//Right * count
				if (Kardoffl.m_pMouse->IsInBox(x + 6, y + 1 + (i * 20), 79, 20))
				{
					Kardoffl.m_pDraw->FillRGBA(x + 6, y + 1 + (i * 20), 79, 20, iMenuRed - 20, iMenuGreen - 20, iMenuBlue - 20, 255);//highlight selection
					if (Kardoffl.m_pMouse->bMouse1released)
					{
						cComboBox[num].iSelect = i;
						cComboBox[num].Name = const_cast<char*>(structs[cComboBox[num].iSelect].c_str());
						Selection = cComboBox[num].iSelect;
						cComboBox[num].Open = false;
						cComboBox[num].Active = true;
						bLock = false;
					}
				}
				Kardoffl.m_pDraw->Text(x + 45, y + 10 + (i * 20), 255, 255, 255, 255, true, Kardoffl.m_pFont, structs[i].c_str());//List struct
			}
			Kardoffl.m_pDraw->FillRGBA(x + 2, y + 3 + (max * 20), 87, 2, iMenuRed, iMenuGreen, iMenuBlue, 255);//Bottom
		}
	}

	Kardoffl.m_pDraw->Text(x + 92, y + 1, 255, 255, 255, 255, false, Kardoffl.m_pFont, szName);
}

void CMenu::DrawSlider(int x, int y, bool bInt, CVars* Input, float iMin, float iMax, const char* szName)
{
	float sliderpos, barpos;

	if (Kardoffl.m_pMouse->IsInBox(x, y + 3, 200, 9))
	{
		DrawInfo(Input->GetDescription().c_str());
	}

	if (Kardoffl.m_pMouse->IsInBox(x, y + 3, 200, 9) && Kardoffl.m_pMouse->bMouse1pressed && !bLock)
	{
		sliderpos = float(Kardoffl.m_pMouse->iMouseX - x);
	}
	else
	{
		sliderpos = (((Input->GetInt() * 100) * 2) / iMax);
	}

	Input->SetValue(iMax * (sliderpos / 2) / 100);
	barpos = (sliderpos / 200 * 100) * 2;

	if (barpos > 200)
		barpos = 200;

	if (bInt)
	{
		int iOutput;
		double integral;
		float fraction = (float)modf(Input->GetFloat(), &integral);
		iOutput = (int)integral;
		if (fraction >= 0.5)
			iOutput = int(iOutput + 1);
		Input->SetValue(iOutput);
	}

	Input->SetValue(clamp(Input->GetFloat(), iMin, iMax));

	Kardoffl.m_pDraw->FillRGBA(x, y + 5, 202, 5, 75, 75, 75, 255);

	Kardoffl.m_pDraw->FillRGBA(x + 1, y + 6, int(barpos), 3, iMenuRed, iMenuGreen, iMenuBlue, 255);

	Kardoffl.m_pDraw->FillRGBA(x + int(barpos) - 2, y + 3, 5, 9, iMenuRed, iMenuGreen, iMenuBlue, 255);

	Kardoffl.m_pDraw->Text(x + 208, y, 255, 255, 255, 255, false, Kardoffl.m_pFont, bInt ? "%s: %0.0f" : "%s: %0.1f", szName, Input->GetFloat());
}

void CMenu::DrawSlider(int x, int y, bool bInt, float &Input, float iMin, float iMax, const char* szName, const char* szDescription)
{
	float sliderpos, barpos;

	if (Kardoffl.m_pMouse->IsInBox(x, y + 3, 200, 9))
	{
		DrawInfo(szDescription);
	}

	if (Kardoffl.m_pMouse->IsInBox(x, y + 3, 200, 9) && Kardoffl.m_pMouse->bMouse1pressed && !bLock)
	{
		sliderpos = float(Kardoffl.m_pMouse->iMouseX - x);
	}
	else
	{
		sliderpos = (((Input * 100) * 2) / iMax);
	}

	Input = (iMax * (sliderpos / 2) / 100);
	barpos = (sliderpos / 200 * 100) * 2;

	if (barpos > 200)
		barpos = 200;

	if (bInt)
	{
		int iOutput;
		double integral;
		float fraction = (float)modf(Input, &integral);
		iOutput = (int)integral;
		if (fraction >= 0.5)
			iOutput = int(iOutput + 1);
		Input = float(iOutput);
	}

	Input = clamp(Input, iMin, iMax);

	Kardoffl.m_pDraw->FillRGBA(x, y + 5, 202, 5, 75, 75, 75, 255);

	Kardoffl.m_pDraw->FillRGBA(x + 1, y + 6, int(barpos), 3, iMenuRed, iMenuGreen, iMenuBlue, 255);

	Kardoffl.m_pDraw->FillRGBA(x + int(barpos) - 2, y + 3, 5, 9, iMenuRed, iMenuGreen, iMenuBlue, 255);

	Kardoffl.m_pDraw->Text(x + 208, y, 255, 255, 255, 255, false, Kardoffl.m_pFont, bInt ? "%s: %0.0f" : "%s: %0.1f", szName, Input);
}

void CMenu::DrawTabs(int x, int y)
{
	int iTabBar = 794;
	int iTabSize = iTabBar / 4;

	for (int i = 0; i < 4; ++i)
	{
		if (iTab - 1 == i)
			Kardoffl.m_pDraw->FillRGBA(x + (i * iTabSize), y, iTabSize, 28, iMenuRed - 20, iMenuGreen - 20, iMenuBlue - 20, 255);
		else
			Kardoffl.m_pDraw->FillRGBA(x + (i * iTabSize), y, iTabSize, 28, iMenuRed, iMenuGreen, iMenuBlue, 255);
		
		Kardoffl.m_pDraw->Text(x + (i * iTabSize) + (iTabSize / 2), y + 15, 255, 255, 255, 255, true, Kardoffl.m_pFont, Tabs[i].c_str());

		if (Kardoffl.m_pMouse->IsInBox(x + (i * iTabSize), y, iTabSize, 28) && Kardoffl.m_pMouse->bMouse1released)
		{
			iTab = i + 1;
		}
	}

	Kardoffl.m_pDraw->FillRGBA(x, y + 28, iTabBar - 2, 1, 0, 0, 0, 255);

	if (iTab == 1)
	{
		DrawSlider(x + 7, y + 89, true, Kardoffl.m_pConvars->GetConVar("aimbot_spot"), 0, 18, "Hitbox");
		DrawSlider(x + 7, y + 115, true, Kardoffl.m_pConvars->GetConVar("aimbot_fov"), 1, 180, "Field of View");
		DrawCheckbox(x + 5, y + 137, Kardoffl.m_pConvars->GetConVar("aimbot_autowall"), "Autowall");
		DrawCheckbox(x + 5, y + 159, Kardoffl.m_pConvars->GetConVar("aimbot_hitscan"), "Hitscan");
		DrawCheckbox(x + 5, y + 181, Kardoffl.m_pConvars->GetConVar("aimbot_rcs"), "Recoil Control");
		DrawSlider(x + 7, y + 197, true, Kardoffl.m_pConvars->GetConVar("aimbot_smooth"), 1, 100, "Smooth");
		DrawCheckbox(x + 5, y + 219, Kardoffl.m_pConvars->GetConVar("aimbot_stopcrouch"), "Stop and Crouch");
		DrawCombobox(x + 5, y + 63, Kardoffl.m_pConvars->GetConVar("aimbot_silent"), 2, 3, AimbotStyle, "Style");
		DrawCombobox(x + 5, y + 38, Kardoffl.m_pConvars->GetConVar("aimbot_active"), 1, 3, AimbotActivation, "Aimbot");
		DrawCheckbox(x + 5, y + 241, Kardoffl.m_pConvars->GetConVar("aimbot_autoshoot"), "Autoshoot");

		DrawCheckbox(x + 355, y + 65, Kardoffl.m_pConvars->GetConVar("triggerbot_team"), "Trigger on team");
		DrawCheckbox(x + 355, y + 88, Kardoffl.m_pConvars->GetConVar("triggerbot_head"), "Head");
		DrawCheckbox(x + 355, y + 111, Kardoffl.m_pConvars->GetConVar("triggerbot_chest"), "Chest");
		DrawCheckbox(x + 355, y + 133, Kardoffl.m_pConvars->GetConVar("triggerbot_arms"), "Arms");
		DrawCheckbox(x + 355, y + 155, Kardoffl.m_pConvars->GetConVar("triggerbot_legs"), "Legs");
		DrawCombobox(x + 355, y + 38, Kardoffl.m_pConvars->GetConVar("triggerbot_active"), 4, 3, TriggebotActivation, "Triggerbot");
	}
	else if (iTab == 2)
	{
		DrawCheckbox(x + 5, y + 38, Kardoffl.m_pConvars->GetConVar("esp_active"), "ESP");
		DrawCheckbox(x + 5, y + 61, Kardoffl.m_pConvars->GetConVar("misc_crosshair"), "Crosshair");
		DrawCheckbox(x + 5, y + 84, Kardoffl.m_pConvars->GetConVar("misc_fovwarning"), "pSilent FOV Warning");
		DrawCheckbox(x + 5, y + 108, Kardoffl.m_pConvars->GetConVar("misc_watermark"), "Watermark");
		DrawCheckbox(x + 5, y + 132, Kardoffl.m_pConvars->GetConVar("misc_skinchanger"), "Skinchanger");
	}
	else if (iTab == 3)
	{
		DrawCheckbox(x + 5, y + 38, Kardoffl.m_pConvars->GetConVar("misc_airstuck"), "Airstuck");
		DrawSlider(x + 5, y + 59, true, Kardoffl.m_pConvars->GetConVar("misc_antiaim"), 0, 5, "Anti Aim");
		DrawCheckbox(x + 5, y + 84, Kardoffl.m_pConvars->GetConVar("misc_antiuntrust"), "Anti Untrust");
		DrawCheckbox(x + 5, y + 108, Kardoffl.m_pConvars->GetConVar("misc_autopistol"), "Auto Pistol");
		DrawCheckbox(x + 5, y + 131, Kardoffl.m_pConvars->GetConVar("misc_bunnyhop"), "Bunny Hop");
		DrawCheckbox(x + 5, y + 154, Kardoffl.m_pConvars->GetConVar("misc_fakelag"), "Fakelag");
		DrawSlider(x + 5, y + 175, true, Kardoffl.m_pConvars->GetConVar("misc_knife"), 0, 9, "Knife Changer");
		DrawCheckbox(x + 5, y + 211 + 15, Kardoffl.m_pConvars->GetConVar("removals_recoil"), "No Recoil");
		DrawCheckbox(x + 5, y + 234 + 15, Kardoffl.m_pConvars->GetConVar("removals_spread"), "No Spread (!)");
		DrawCombobox(x + 5, y + 194, Kardoffl.m_pConvars->GetConVar("misc_autostrafer"), 3, 3, Autostrafer, "Autostrafer");
	}
	else if (iTab == 4)
	{
		LoadButton(x + 5, y + 38);
		SaveButton(x + 5, y + 66);
		ReloadSkinsButton(x + 5, y + 94);
	}
}

void CMenu::DrawMenu()
{
	int x = Kardoffl.m_pConvars->GetConVar("menu_x")->GetInt();
	int y = Kardoffl.m_pConvars->GetConVar("menu_y")->GetInt();
	ConVar* mouseenable = Kardoffl.m_pCvar->FindVar("cl_mouseenable");

	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		bOpen = !bOpen;

		if (bOpen)
			Kardoffl.m_pMenu->CloseAllComboBoxes();
	}

	if (bOpen)
	{
		if(!bMouseOnce)
		{
			mouseenable->SetValue(0);
			bMouseOnce = true;
		}
		
		Kardoffl.m_pMouse->PositionHandler();
		Kardoffl.m_pMouse->ClickHandler();
		DrawWindow(x, y, iMenuWidth, iMenuHeight, "Prediction Framework");
		DrawTabs(x + 6, y + 29);

		Kardoffl.m_pMouse->MouseCursor();

		if (Kardoffl.m_pMouse->IsInBox(x - 1, y - 1, iMenuWidth + 2, 37) && Kardoffl.m_pMouse->bMouse1pressed)
		{
			if (!bSavePos)
			{
				SavedX = Kardoffl.m_pMouse->iMouseX - x;
				SavedY = Kardoffl.m_pMouse->iMouseY - y;
				bSavePos = true;
			}
			Kardoffl.m_pConvars->GetConVar("menu_x")->SetValue(Kardoffl.m_pMouse->iMouseX);
			Kardoffl.m_pConvars->GetConVar("menu_y")->SetValue(Kardoffl.m_pMouse->iMouseY);
			Kardoffl.m_pConvars->GetConVar("menu_x")->SetValue(Kardoffl.m_pConvars->GetConVar("menu_x")->GetInt() - SavedX);
			Kardoffl.m_pConvars->GetConVar("menu_y")->SetValue(Kardoffl.m_pConvars->GetConVar("menu_y")->GetInt() - SavedY);
			bSavePos = true;
		}
		else
		{
			bSavePos = false;
		}
	}
	else
	{
		if (bMouseOnce)
		{
			mouseenable->SetValue(1);
			bMouseOnce = false;
		}
	}
}