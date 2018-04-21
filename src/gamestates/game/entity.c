#include "gamestates/game/entity.h"
#include <ace/managers/blit.h>

#define ENTITY_MAX_COUNT 10

#define ENTITY_FRAME_STAND 0
#define ENTITY_FRAME_WALK1 1
#define ENTITY_FRAME_WALK2 2
#define ENTITY_FRAME_WALK3 3
#define ENTITY_FRAME_WALK4 4
#define ENTITY_ACTION1 5
#define ENTITY_ACTION2 6

tEntity s_pEntities[ENTITY_MAX_COUNT] = {{0}};

tBitMap *s_pDirFrames[4];

void entityListCreate(void) {
	s_pDirFrames[ENTITY_DIR_UP] = bitmapCreateFromFile("data/skelet/up.bm");
	s_pDirFrames[ENTITY_DIR_DOWN] = bitmapCreateFromFile("data/skelet/down.bm");
	s_pDirFrames[ENTITY_DIR_LEFT] = bitmapCreateFromFile("data/skelet/left.bm");
	s_pDirFrames[ENTITY_DIR_RIGHT] = bitmapCreateFromFile("data/skelet/right.bm");

	for (UBYTE ubIdx = 0; ubIdx < ENTITY_MAX_COUNT; ++ubIdx) {
		s_pEntities[ubIdx].ubType = ENTITY_TYPE_OFF;
	}
}

void entityListDestroy(void) {
	for(UBYTE i = ENTITY_DIR_UP; i <= ENTITY_DIR_RIGHT; ++i) {
		bitmapDestroy(s_pDirFrames[i]);
	}
}

UBYTE entityAdd(UWORD uwX, UWORD uwY, UBYTE ubDir) {
	for (UBYTE ubIdx = 0; ubIdx < ENTITY_MAX_COUNT; ++ubIdx) {
		if(s_pEntities[ubIdx].ubType == ENTITY_TYPE_OFF) {
			s_pEntities[ubIdx].uwX = uwX;
			s_pEntities[ubIdx].uwY = uwY;
			s_pEntities[ubIdx].ubDir = ubDir;
			s_pEntities[ubIdx].ubType = ENTITY_TYPE_SKELET;
			s_pEntities[ubIdx].ubFrame = ENTITY_FRAME_STAND;
			return ubIdx;
		}
	}
	return ENTITY_IDX_INVALID;
}

void entityDestroy(UBYTE ubEntityIdx) {
	s_pEntities[ubEntityIdx].ubType = ENTITY_TYPE_OFF;
}

void entityMove(UBYTE ubEntityIdx, BYTE bDx, BYTE bDy) {
	tEntity *pEntity = &s_pEntities[ubEntityIdx];

	pEntity->uwX += bDx;
	pEntity->uwY += bDy;
	if(bDy > 0) {
		pEntity->ubDir = ENTITY_DIR_DOWN;
	}
	else if(bDy < 0) {
		pEntity->ubDir = ENTITY_DIR_UP;
	}
	else {
		if(bDx > 0) {
			pEntity->ubDir = ENTITY_DIR_RIGHT;
		}
		else if(bDx < 0) {
			pEntity->ubDir = ENTITY_DIR_LEFT;
		}
		else {
			pEntity->ubFrame = ENTITY_FRAME_STAND;
			return;
		}
	}
	if(
		pEntity->ubFrame >= ENTITY_FRAME_WALK1 &&
		pEntity->ubFrame < ENTITY_FRAME_WALK4
	) {
		++pEntity->ubFrame;
	}
	else {
		pEntity->ubFrame = ENTITY_FRAME_WALK1;
	}
}

void entityProcessDraw(tBitMap *pBuffer) {
	for (UBYTE ubIdx = 0; ubIdx < ENTITY_MAX_COUNT; ++ubIdx) {
		tEntity *pEntity = &s_pEntities[ubIdx];
		// Restore BG on old pos
		// TODO

		// Save BG on new pos

		// Draw entity on new pos
		blitCopy(
			s_pDirFrames[pEntity->ubDir], 0, s_pEntities[ubIdx].ubFrame * 20,
			pBuffer, pEntity->uwX, pEntity->uwY,
			16, 20, MINTERM_COOKIE, 0xFF
		);
	}
}
