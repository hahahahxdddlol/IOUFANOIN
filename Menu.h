#pragma once

#include "Includes.h"

class CVars;

class CMouse
{
public:
	void MouseCursor();
	bool IsInBox(int x, int y, int w, int h);
	void PositionHandler();
	void ClickHandler();
public:
	bool bMouse1pressed,
		bMouse2pressed,
		bMouse1released,
		bMouse2released;
	int iMouseX;
	int iMouseY;
	HWND hWnd;
	tagPOINT Mouse;
};

class CMenu
{
public:
	CMenu();
	void InitConfig(HMODULE hModule);
	void ReloadSkinsButton(int x, int y);
	void LoadButton(int x, int y);
	void SaveButton(int x, int y);
	void DrawWindow(int x, int y, int w, int h, const char* Name, ...);
	void DrawGroupbox(int x, int y, int w, int h, const char* Name, ...);
	void DrawCheckbox(int x, int y, CVars* cvar, const char* szName);
	void DrawCheckbox(int x, int y, bool &toggle, const char* szName, const char* szDescription);
	void CloseAllComboBoxes();
	void DrawCombobox(int x, int y, CVars* Selection, int num, int max, std::string* structs, const char* szName);
	void DrawCombobox(int x, int y, int &Selection, int num, int max, std::string* structs, const char* szName, const char* szDescription);
	void DrawSlider(int x, int y, bool bInt, CVars* Input, float iMin, float iMax, const char* szName);
	void DrawSlider(int x, int y, bool bInt, float &Input, float iMin, float iMax, const char* szName, const char* szDescription);
	void DrawTabs(int x, int y);
	void DrawMenu();

	int iTab;
	bool bLock;
	bool bOpen;
};