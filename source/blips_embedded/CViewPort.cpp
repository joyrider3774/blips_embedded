#include <stdlib.h>
#include "CViewPort.h"
#include "Common.h"

//MaxScreenX / MaxScreenY are at most (NrOfCols + NrOfColsVisible) * TileWidth pixels, a view
//wider than the level widens the limits up to the view
static_assert(((NrOfCols + NrOfColsVisible) * TileWidth <= 32767) && ((NrOfRows + NrOfRowsVisible) * TileHeight <= 32767), "viewport pixel positions do not fit in int16_t");

CViewPort* CViewPort_Create(int16_t MinX, int16_t MinY, int16_t MaxX, int16_t MaxY, int16_t MinX2, int16_t MinY2, int16_t MaxX2, int16_t MaxY2)
{
	CViewPort* Result = (CViewPort*) malloc(sizeof(CViewPort));
	if (Result)
	{
		if ((MinX < NrOfCols) && (MinX >= 0) && (MaxX < NrOfCols) && (MaxX >= 0) &&
			(MinY < NrOfRows) && (MinY >= 0) && (MaxY < NrOfRows) && (MaxY >= 0))
		{
			Result->VPMinX = MinX;
			Result->VPMinY = MinY;
			Result->VPMaxX = MaxX;
			Result->VPMaxY = MaxY;
			Result->MinScreenX = Result->VPMinX * TileWidth;
			Result->MinScreenY = Result->VPMinY * TileHeight;
			Result->MaxScreenX = Result->VPMaxX * TileWidth;
			Result->MaxScreenY = Result->VPMaxY * TileHeight;
		}
		else
		{
			Result->VPMinX = 0;
			Result->VPMinY = 0;
			Result->VPMaxX = NrOfColsVisible-1;
			Result->VPMaxY = NrOfRowsVisible-1;
			Result->MinScreenX = 0;
			Result->MinScreenY = 0;
			Result->MaxScreenX = WINDOW_WIDTH;
			Result->MaxScreenY = WINDOW_HEIGHT;
		}
		if ((MinX2 < NrOfCols) && (MinX2 >= 0) && (MaxX2 + 1 < NrOfCols) && (MaxX2 >= 0) &&
			(MinY2 < NrOfRows) && (MinY2 >= 0) && (MaxY2 + 1 < NrOfRows) && (MaxY2 >= 0))
		{
			Result->VPLimitMinX = MinX2;
			Result->VPLimitMinY = MinY2;
			Result->VPLimitMaxX = MaxX2 + 1; //needs to be one more, need to get max element + 1 as we need to get the ending of the previous tile
			Result->VPLimitMaxY = MaxY2 + 1; //needs to be one more, need to get max element + 1 as we need to get the ending of the previous tile
		}
		else
		{
			Result->VPLimitMinX = 0;
			Result->VPLimitMinY = 0;
			Result->VPLimitMaxX = NrOfCols; //needs to be one more, need to get max element + 1 as we need to get the ending of the previous tile
			Result->VPLimitMaxY = NrOfRows; //needs to be one more, need to get max element + 1 as we need to get the ending of the previous tile
		}
		Result->Width = Result->VPMaxX - Result->VPMinX;
		Result->Height = Result->VPMaxY - Result->VPMinY;
	}
	return(Result);
}

void CViewPort_SetVPLimit(CViewPort* ViewPort, int16_t MinX, int16_t MinY, int16_t MaxX, int16_t MaxY)
{
	//a level may use the last column / row, so MaxX + 1 can be NrOfCols (MaxY + 1 NrOfRows)
	if ((MinX < NrOfCols) && (MinX >= 0) && (MaxX + 1 <= NrOfCols) && (MaxX + 1 >= 0) &&
		(MinY < NrOfRows) && (MinY >= 0) && (MaxY + 1 <= NrOfRows) && (MaxY + 1 >= 0))
	{
		ViewPort->VPLimitMinX = MinX;
		ViewPort->VPLimitMinY = MinY;
		ViewPort->VPLimitMaxX = MaxX + 1; //needs to be one more, need to get max element + 1 as we need to get the ending of the previous tile
		ViewPort->VPLimitMaxY = MaxY + 1; //needs to be one more, need to get max element + 1 as we need to get the ending of the previous tile
	}
	else
	{
		ViewPort->VPLimitMinX = 0;
		ViewPort->VPLimitMinY = 0;
		ViewPort->VPLimitMaxX = NrOfCols; //needs to be one more, need to get max element + 1 as we need to get the ending of the previous tile
		ViewPort->VPLimitMaxY = NrOfRows; //needs to be one more, need to get max element + 1 as we need to get the ending of the previous tile
	}
	if (ViewPort->VPLimitMaxX - ViewPort->VPLimitMinX < ViewPort->Width)
	{
		if (ViewPort->VPLimitMaxX - ViewPort->Width >= 0)
			ViewPort->VPLimitMinX = ViewPort->VPLimitMaxX - (ViewPort->Width);
		else
		{
			ViewPort->VPLimitMinX = 0;
			ViewPort->VPLimitMaxX = ViewPort->VPLimitMinX + (ViewPort->Width);
		}
	}
	if (ViewPort->VPLimitMaxY - ViewPort->VPLimitMinY < ViewPort->Height)
	{
		if (ViewPort->VPLimitMaxY - ViewPort->Height >= 0)
			ViewPort->VPLimitMinY = ViewPort->VPLimitMaxY - (ViewPort->Height);
		else
		{
			ViewPort->VPLimitMinY = 0;
			ViewPort->VPLimitMaxY = ViewPort->VPLimitMinY + (ViewPort->Height);
		}
	}
}

void CViewPort_Move(CViewPort* ViewPort, int8_t Xi, int8_t Yi)
{
	if ((ViewPort->MinScreenX + Xi <= TileWidth*(ViewPort->VPLimitMaxX)) && (ViewPort->MinScreenX + Xi >=ViewPort->VPLimitMinX*TileWidth) && 
		(ViewPort->MaxScreenX + Xi <= TileWidth*(ViewPort->VPLimitMaxX)) && (ViewPort->MaxScreenX + Xi >=ViewPort->VPLimitMinX *TileWidth) &&
		(ViewPort->MinScreenY + Yi <= TileHeight*(ViewPort->VPLimitMaxY)) && (ViewPort->MinScreenY + Yi >=ViewPort->VPLimitMinY*TileHeight) && 
		(ViewPort->MaxScreenY + Yi <= TileHeight*(ViewPort->VPLimitMaxY)) && (ViewPort->MaxScreenY + Yi >=ViewPort->VPLimitMinY*TileHeight))
		{

			ViewPort->MinScreenX +=Xi;
			ViewPort->MaxScreenX +=Xi;
			ViewPort->MinScreenY +=Yi;
			ViewPort->MaxScreenY +=Yi;
			ViewPort->VPMinX = ViewPort->MinScreenX / TileWidth;
			ViewPort->VPMinY = ViewPort->MinScreenY  / TileHeight;
			ViewPort->VPMaxX = ViewPort->MaxScreenX  / TileWidth;
			ViewPort->VPMaxY = ViewPort->MaxScreenY  / TileHeight;
		}
}

void CViewPort_SetViewPort(CViewPort* ViewPort, int16_t MinX, int16_t MinY, int16_t MaxX, int16_t MaxY)
{
	if ((MinX <= ViewPort->VPLimitMaxX) && (MinX >= ViewPort->VPLimitMinX) && (MaxX + 1 <= ViewPort->VPLimitMaxX) && (MaxX + 1 >= ViewPort->VPLimitMinX) &&
		(MinY <= ViewPort->VPLimitMaxY) && (MinY >= ViewPort->VPLimitMinY) && (MaxY + 1 <= ViewPort->VPLimitMaxY) && (MaxY + 1 >= ViewPort->VPLimitMinY))
	{
		ViewPort->VPMinX = MinX;
		ViewPort->VPMinY = MinY;
		ViewPort->VPMaxX = MaxX + 1; //needs to be one more, need to get max element + 1 as we need to get the ending of the previous tile
		ViewPort->VPMaxY = MaxY + 1; //needs to be one more, need to get max element + 1 as we need to get the ending of the previous tile
		ViewPort->MinScreenX = ViewPort->VPMinX * TileWidth;
		ViewPort->MinScreenY = ViewPort->VPMinY * TileHeight;
		ViewPort->MaxScreenX = ViewPort->VPMaxX * TileWidth;
		ViewPort->MaxScreenY = ViewPort->VPMaxY * TileHeight;
	}
	else
	{
		ViewPort->VPMinX = MinX;
		ViewPort->VPMinY = MinY;
		ViewPort->VPMaxX = MaxX + 1; //needs to be one more, need to get max element + 1 as we need to get the ending of the previous tile
		ViewPort->VPMaxY = MaxY + 1; //needs to be one more, need to get max element + 1 as we need to get the ending of the previous tile
		if (ViewPort->VPMinX < ViewPort->VPLimitMinX)
		{
			ViewPort->VPMinX = ViewPort->VPLimitMinX;
			ViewPort->VPMaxX = ViewPort->VPLimitMinX + NrOfColsVisible;

		}
		if (ViewPort->VPMaxX > ViewPort->VPLimitMaxX)
		{
			ViewPort->VPMinX = ViewPort->VPLimitMaxX - NrOfColsVisible;
			ViewPort->VPMaxX = ViewPort->VPLimitMaxX;
		}
		if (ViewPort->VPMinY < ViewPort->VPLimitMinY)
		{
			ViewPort->VPMaxY = ViewPort->VPLimitMinY + NrOfRowsVisible;
			ViewPort->VPMinY = ViewPort->VPLimitMinY;
		}
		if (ViewPort->VPMaxY > ViewPort->VPLimitMaxY)
		{
			ViewPort->VPMinY = ViewPort->VPLimitMaxY - NrOfRowsVisible;
			ViewPort->VPMaxY = ViewPort->VPLimitMaxY;
		}
		ViewPort->MinScreenX = ViewPort->VPMinX * TileWidth;
		ViewPort->MinScreenY = ViewPort->VPMinY * TileHeight;
		ViewPort->MaxScreenX = ViewPort->VPMaxX * TileWidth;
		ViewPort->MaxScreenY = ViewPort->VPMaxY * TileHeight;
	}
	ViewPort->Width = ViewPort->VPMaxX - ViewPort->VPMinX;
	ViewPort->Height = ViewPort->VPMaxY - ViewPort->VPMinY;
}

void CViewPort_Destroy(CViewPort* ViewPort)
{
	free(ViewPort);
	ViewPort = NULL;
}



