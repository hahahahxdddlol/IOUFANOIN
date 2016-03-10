#pragma once

#include "Includes.h"

enum eCvarType
{
	CVar_NONE,
	CVar_INT,
	CVar_FLOAT,
	CVar_BOOL
};

class CVars
{
public:
	CVars();
	CVars(const char* szName, const char* szDescription);

	int GetInt(void);
	bool GetBool(void);
	float GetFloat(void);

	std::string GetVarName(void);
	std::string GetDescription(void);
	std::string GetInfoString(int stringType = CVar_FLOAT);

	void SetValue(int iValue);
	void SetValue(bool bValue);
	void SetValue(float flValue);

	eCvarType m_Type;
private:
	union
	{
		bool m_bBool;
		int m_iInt;
		float m_flFloat;
	};
	std::string m_szVarName;
	std::string m_szVarDescription;
};

class CConVars
{
public:
	CConVars();
	UINT GetConVarIndexByName(std::string szVarName);
	CVars* GetConVar(std::string szVarName);
public:
	std::vector<CVars*> m_vecConVars;
};