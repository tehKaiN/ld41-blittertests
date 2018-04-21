#include "gamestates/game/entity.h"
#include <ace/managers/blit.h>

#define ENTITY_MAX_COUNT 10

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
			return ubIdx;
		}
	}
	return ENTITY_IDX_INVALID;
}

void entityDestroy(UBYTE ubEntityIdx) {
	s_pEntities[ubEntityIdx].ubType = ENTITY_TYPE_OFF;
}

void entityProcessDraw(tBitMap *pBuffer) {
	for (UBYTE ubIdx = 0; ubIdx < ENTITY_MAX_COUNT; ++ubIdx) {
		tEntity *pEntity = &s_pEntities[ubIdx];
		// Restore BG on old pos
		// TODO

		// Save BG on new pos

		// Draw entity on new pos
		blitCopy(
			s_pDirFrames[pEntity->ubDir], 0, 0,
			pBuffer, pEntity->uwX, pEntity->uwY,
			16, 20, MINTERM_COOKIE, 0xFF
		);
	}
}
