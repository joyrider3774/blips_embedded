#ifndef SPOINT_H
#define SPOINT_H

#include <stdint.h>

typedef struct SPoint SPoint;
struct SPoint
{
	//playfield positions, like CWorldPart::PlayFieldX / PlayFieldY
	int8_t X,Y;
};

#endif
