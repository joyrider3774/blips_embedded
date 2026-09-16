#ifndef CVIEWPORT_H
#define CVIEWPORT_H

#include <stdint.h>

typedef struct CViewPort CViewPort;

//Everything here is 16 bit so the game can run on a bigger screen: the pixel values
//grow with the resolution, and so do the tile values, as they include the number of
//tiles visible (VPLimitMinX + NrOfColsVisible, 240 at 1920 px wide with 8x8 tiles)
struct CViewPort
{
	//signed: SetViewPort stores a position left of / above the level before clamping it
	int16_t VPMinX;
	int16_t VPMinY;
	int16_t VPMaxX;
	int16_t VPMaxY;
	int16_t MinScreenX;
	int16_t MinScreenY;
	int16_t MaxScreenX;
	int16_t MaxScreenY;
	//never negative, SetVPLimit clamps the limits to 0 and up
	uint16_t Width;
	uint16_t Height;
	uint16_t VPLimitMinX;
	uint16_t VPLimitMaxX;
	uint16_t VPLimitMinY;
	uint16_t VPLimitMaxY;
};

CViewPort* CViewPort_Create(int16_t MinX, int16_t MinY, int16_t MaxX, int16_t MaxY, int16_t MinX2, int16_t MinY2, int16_t MaxX2, int16_t MaxY2);
void CViewPort_SetVPLimit(CViewPort* ViewPort, int16_t MinX, int16_t MinY, int16_t MaxX, int16_t MaxY);
void CViewPort_Move(CViewPort* ViewPort, int8_t Xi, int8_t Yi);
void CViewPort_SetViewPort(CViewPort* ViewPort, int16_t MinX, int16_t MinY, int16_t MaxX, int16_t MaxY);
void CViewPort_Destroy(CViewPort* ViewPort);

#endif