#include "draw.h"

void CDraw::FontInit(vgui::HFont &hFont, const char* szFontName, int size, FontFlags_t Flags)
{
	hFont = Kardoffl.m_pMatSurface->CreateFont();
	Kardoffl.m_pMatSurface->SetFontGlyphSet(hFont, szFontName, size, 300, 0, 0, Flags);
}

void CDraw::Border(int x, int y, int w, int h, int line, int r, int g, int b, int a)
{
	FillRGBA(x, y, w, line, r, g, b, a);
	FillRGBA(x, y, line, h, r, g, b, a);
	FillRGBA((x + w), y, line, h, r, g, b, a);
	FillRGBA(x, (y + h), (w + line), line, r, g, b, a);
}

void CDraw::FillRGBA(int x, int y, int w, int h, int r, int g, int b, int a)
{
	Kardoffl.m_pMatSurface->DrawSetColor(Color(r, g, b, a));
	Kardoffl.m_pMatSurface->DrawFilledRect( x, y, x + w, y + h );
}

void TextW(bool cent, vgui::HFont font, int x, int y, int r, int g, int b, int a, wchar_t *pszString)
{
	if (cent)
	{
		int wide, tall;
		Kardoffl.m_pMatSurface->GetTextSize(font, pszString, wide, tall);
		x -= wide / 2;
		y -= tall / 2;
	}
	Kardoffl.m_pMatSurface->DrawSetTextColor(Color(r, g, b, a));
	Kardoffl.m_pMatSurface->DrawSetTextFont(font);
	Kardoffl.m_pMatSurface->DrawSetTextPos(x, y);
	Kardoffl.m_pMatSurface->DrawPrintText(pszString, (int)wcslen(pszString));
}

void CDraw::Text(int x, int y, int r, int g, int b, int a, bool bCenter, vgui::HFont font, const char *fmt, ...)
{ 
	va_list va_alist;
	char szBuffer[1024] = { '\0' };

	va_start(va_alist, fmt);
	vsprintf(szBuffer, fmt, va_alist);
	va_end(va_alist);

	wchar_t *pszStringWide = reinterpret_cast< wchar_t* >(malloc((strlen(szBuffer) + 1) * sizeof(wchar_t)));

	mbstowcs(pszStringWide, szBuffer, (strlen(szBuffer) + 1) * sizeof(wchar_t));

	TextW(bCenter, font, x, y, r, g, b, a, pszStringWide);

	free(pszStringWide);
}