#ifndef __ESP_H__
#define __ESP_H__

#include "Includes.h"

class CESP
{
public:
	void DrawHeader(int x, int y, int w, int r, int g, int b, int HealthBarWidth);
	void DrawRadar(void);
	void DrawChickenESP(void);
	void DrawESP(void);
};

static const char *Ranks[] =
{
	"Not Ranked",
	"Silver I",
	"Silver II",
	"Silver III",
	"Silver IV",
	"Silver Elite",
	"Silver Elite Master",

	"Gold Nova I",
	"Gold Nova II",
	"Gold Nova III",
	"Gold Nova Master",
	"Master Guardian I",
	"Master Guardian II",

	"Master Guardian Elite",
	"Distinguished Master Guardian",
	"Legendary Eagle",
	"Legendary Eagle Master",
	"Supreme Master First Class",
	"The Global Elite"
};

#endif