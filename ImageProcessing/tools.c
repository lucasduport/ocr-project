#include <math.h>
#include "tools.h"

float COS[360];
float SIN[360];

void initTrig()
{
	static int trig_init = 0;
	if (trig_init)
		return;
	trig_init = 1;
	float c;
	for (int theta = 1; theta < 90; theta++)
	{
		c = cos(theta * PI / 180);
		COS[theta] = c;
		COS[360 - theta] = c;
		COS[180 + theta] = -c;
		COS[180 - theta] = -c;
		SIN[90 + theta] = c;
		SIN[90 - theta] = c;
		SIN[270 + theta] = -c;
		SIN[270 - theta] = -c;
	}
	SIN[180] = SIN[0] = COS[270] = COS[90] = 0;
	SIN[90] = COS[0] = 1;
	SIN[270] = COS[180] = -1;
}
