#ifndef _LD41_GAMESTATES_GAME_ENTITY_H
#define _LD41_GAMESTATES_GAME_ENTITY_H

#include <ace/types.h>
#include <ace/utils/bitmap.h>
#include <ace/managers/copper.h>

#define ENTITY_DIR_UP 0
#define ENTITY_DIR_DOWN 1
#define ENTITY_DIR_LEFT 2
#define ENTITY_DIR_RIGHT 3

#define ENTITY_TYPE_OFF 0
#define ENTITY_TYPE_SKELET 1

#define ENTITY_IDX_INVALID 255

typedef struct _tEntity {
	UWORD uwX;
	UWORD uwY;
	UWORD pUndrawX[2];
	UWORD pUndrawY[2];
	UBYTE ubDir;
	UBYTE ubType;
	UBYTE ubFrame;
} tEntity;

void entityListCreate(tCopList *pCopList);

void entityListDestroy(void);

UBYTE entityAdd(UWORD uwX, UWORD uwY, UBYTE ubDir);

void entityDestroy(UBYTE ubEntityIdx);

UWORD entityProcessDraw(tBitMap *pBuffer, UBYTE ubBufferIdx);

void entityMove(UBYTE ubEntityIdx, BYTE bDx, BYTE bDy);

#endif // _LD41_GAMESTATES_GAME_ENTITY_H
