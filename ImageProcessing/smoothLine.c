#include "smoothLine.h"

#define SMOOTH_ERROR 1.5

void smoothLine(uc *line, st threshold, st len, st *i_start, st *i_end)
{
	// Gets the best_value as the threshold
	for (st i = 0; i < len; i++)
		line[i] = line[i] >= threshold ? 1 : 0;
	// Fills the gaps smaller than min_gap
	int change = 1;
	st min_gap = SMOOTH_ERROR / 100.0 * len;
	while (change)
	{
		change = 0;
		for (size_t i = 0; i < len; i++)
		{
			if (line[i] == 0)
			{
				st j = i;
				while (j < len && line[j] == 0)
					j++;
				if (j - i < min_gap)
				{
					for (st k = i; k < j; k++)
						line[k] = 1;
					change = 1;
				}
				i = j;
			}
		}
	}
	// Keeps only the longest line
	st max_len = 0;
	st max_start = 0;
	for (size_t i = 0; i < len; i++)
	{
		if (line[i] == 1)
		{
			st j = i;
			while (j < len && line[j] == 1)
				j++;
			if (j - i > max_len)
			{
				max_len = j - i;
				max_start = i;
			}
			i = j;
		}
	}
	for (size_t i = 0; i < len; i++)
		line[i] = i >= max_start && i < max_start + max_len ? 1 : 0;
	*i_start = max_start;
	*i_end = max_start + max_len;
}
