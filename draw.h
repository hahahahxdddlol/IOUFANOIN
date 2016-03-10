#ifndef __DRAW_H__
#define __DRAW_H__

#include "Includes.h"

class CDraw
{
public:
	void FontInit(vgui::HFont &hFont, const char* szFontName, int size, FontFlags_t Flags);
	void Border(int x, int y, int w, int h, int line, int r, int g, int b, int a);
	void FillRGBA(int x, int y, int w, int h, int r, int g, int b, int a);
	void Text(int x, int y, int r, int g, int b, int a, bool bCenter, vgui::HFont font, const char *fmt, ...);
};

#endif