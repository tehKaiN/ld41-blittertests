/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gamestates/game/entity.h"
#include <ace/managers/blit.h>
#include <ace/managers/key.h>
#include <ace/utils/bitmap.h>
#include <ace/managers/viewport/simplebuffer.h>

#define ENTITY_MAX_COUNT 65

#define ENTITY_FRAME_STAND 0
#define ENTITY_FRAME_WALK1 1
#define ENTITY_FRAME_WALK2 2
#define ENTITY_FRAME_WALK3 3
#define ENTITY_FRAME_WALK4 4
#define ENTITY_ACTION1 5
#define ENTITY_ACTION2 6

tEntity s_pEntities[ENTITY_MAX_COUNT] = {{0}};

tBitMap *s_pDirFrames[4];
tBitMap *s_pDirMasks[4];

tBitMap *s_pBgBuffer[2];

void entityListCreate(tView *pView) {
	s_pDirFrames[ENTITY_DIR_UP] = bitmapCreateFromFile("data/skelet/up.bm");
	s_pDirFrames[ENTITY_DIR_DOWN] = bitmapCreateFromFile("data/skelet/down.bm");
	s_pDirFrames[ENTITY_DIR_LEFT] = bitmapCreateFromFile("data/skelet/left.bm");
	s_pDirFrames[ENTITY_DIR_RIGHT] = bitmapCreateFromFile("data/skelet/right.bm");

	s_pDirMasks[ENTITY_DIR_UP] = bitmapCreateFromFile("data/skelet/up_mask.bm");
	s_pDirMasks[ENTITY_DIR_DOWN] = bitmapCreateFromFile("data/skelet/down_mask.bm");
	s_pDirMasks[ENTITY_DIR_LEFT] = bitmapCreateFromFile("data/skelet/left_mask.bm");
	s_pDirMasks[ENTITY_DIR_RIGHT] = bitmapCreateFromFile("data/skelet/right_mask.bm");

	for (UBYTE ubIdx = 0; ubIdx < ENTITY_MAX_COUNT; ++ubIdx) {
		s_pEntities[ubIdx].ubType = ENTITY_TYPE_OFF;
	}

	tSimpleBufferManager *pMng = (tSimpleBufferManager*)vPortGetManager(
		pView->pFirstVPort->pNext, VPM_SCROLL
	);

	bobNewManagerCreate(
		ENTITY_MAX_COUNT, 2*ENTITY_MAX_COUNT*20,
		pMng->pFront, pMng->pBack
	);
}

void entityListDestroy(void) {
	bobNewManagerDestroy();

	bitmapDestroy(s_pBgBuffer[0]);
	bitmapDestroy(s_pBgBuffer[1]);

	for(UBYTE i = ENTITY_DIR_UP; i <= ENTITY_DIR_RIGHT; ++i) {
		bitmapDestroy(s_pDirFrames[i]);
		bitmapDestroy(s_pDirMasks[i]);
	}
}

UBYTE entityAdd(UWORD uwX, UWORD uwY, UBYTE ubDir) {
	for (UBYTE ubIdx = 0; ubIdx < ENTITY_MAX_COUNT; ++ubIdx) {
		if(s_pEntities[ubIdx].ubType == ENTITY_TYPE_OFF) {
			bobNewInit(
				&s_pEntities[ubIdx].sBob, 16, 20, 1,
				s_pDirFrames[ubDir], s_pDirMasks[ubDir],
				uwX, uwY
			);

			bobNewSetBitMapOffset(&s_pEntities[ubIdx].sBob, ENTITY_FRAME_STAND * 20);
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

	pEntity->sBob.sPos.sUwCoord.uwX += bDx;
	pEntity->sBob.sPos.sUwCoord.uwY += bDy;
	UBYTE isStanding = 0;
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
			isStanding = 1;
		}
	}
	if(!isStanding) {
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
	bobNewSetBitMapOffset(&pEntity->sBob, pEntity->ubFrame * 20);
	pEntity->sBob.pBitmap = s_pDirFrames[pEntity->ubDir];
	pEntity->sBob.pMask = s_pDirMasks[pEntity->ubDir];

	g_pCustom->color[0] = 0x444;
	bobNewBegin();
}

UWORD entityProcessDraw(tBitMap *pBuffer, UBYTE ubBufferIdx) {
	UWORD uwCopIdx = 14; // From VPort manager

	g_pCustom->color[0] = 0x666;
	for(UBYTE ubIdx = 0; ubIdx < ENTITY_MAX_COUNT; ++ubIdx) {
		if(s_pEntities[ubIdx].ubType != ENTITY_TYPE_OFF) {
			bobNewPush(&s_pEntities[ubIdx].sBob);
		}
	}
	bobNewPushingDone();

	bobNewEnd();
	g_pCustom->color[0] = 0x000;

	return uwCopIdx;
}
