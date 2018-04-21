#include "gamestates/game/entity.h"
#include <ace/managers/blit.h>
#include <ace/utils/bitmap.h>
#include <ace/managers/key.h>

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
tBitMap *s_pDirMasks[4];

tBitMap *s_pBgBuffer;

void entityListCreate(void) {
	s_pDirFrames[ENTITY_DIR_UP] = bitmapCreateFromFile("data/skelet/up.bm");
	s_pDirFrames[ENTITY_DIR_DOWN] = bitmapCreateFromFile("data/skelet/down.bm");
	s_pDirFrames[ENTITY_DIR_LEFT] = bitmapCreateFromFile("data/skelet/left.bm");
	s_pDirFrames[ENTITY_DIR_RIGHT] = bitmapCreateFromFile("data/skelet/right.bm");

	s_pDirMasks[ENTITY_DIR_UP] = bitmapCreateFromFile("data/skelet/up_mask.bm");
	s_pDirMasks[ENTITY_DIR_DOWN] = bitmapCreateFromFile("data/skelet/down_mask.bm");
	s_pDirMasks[ENTITY_DIR_LEFT] = bitmapCreateFromFile("data/skelet/left_mask.bm");
	s_pDirMasks[ENTITY_DIR_RIGHT] = bitmapCreateFromFile("data/skelet/right_mask.bm");

	s_pBgBuffer = bitmapCreate(
		bitmapGetByteWidth(s_pDirFrames[0])*8 +16, 20*ENTITY_MAX_COUNT,
		s_pDirFrames[0]->Depth, BMF_CLEAR | BMF_INTERLEAVED
	);

	for (UBYTE ubIdx = 0; ubIdx < ENTITY_MAX_COUNT; ++ubIdx) {
		s_pEntities[ubIdx].ubType = ENTITY_TYPE_OFF;
	}
}

void entityListDestroy(void) {
	bitmapDestroy(s_pBgBuffer);

	for(UBYTE i = ENTITY_DIR_UP; i <= ENTITY_DIR_RIGHT; ++i) {
		bitmapDestroy(s_pDirFrames[i]);
		bitmapDestroy(s_pDirMasks[i]);
	}
}

UBYTE entityAdd(UWORD uwX, UWORD uwY, UBYTE ubDir) {
	for (UBYTE ubIdx = 0; ubIdx < ENTITY_MAX_COUNT; ++ubIdx) {
		if(s_pEntities[ubIdx].ubType == ENTITY_TYPE_OFF) {
			s_pEntities[ubIdx].uwUndrawX = 0;
			s_pEntities[ubIdx].uwUndrawY = 0;
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
	// Let's imitate new Copper pipeline
	// Restore BG on old pos
	UWORD uwWidth = 32;
	UWORD uwHeight = 20*s_pDirFrames[0]->Depth;
	UWORD uwBlitWords, uwBltCon0;
	uwBlitWords = uwWidth >> 4;
	uwBltCon0 = USEA|USED | MINTERM_A;
	WORD wDstModulo, wSrcModulo;
	wSrcModulo = bitmapGetByteWidth(s_pBgBuffer) - (uwBlitWords<<1);
	wDstModulo = bitmapGetByteWidth(pBuffer) - (uwBlitWords<<1);
	blitWait();
	g_pCustom->bltcon0 = uwBltCon0;
	g_pCustom->bltcon1 = 0;
	g_pCustom->bltafwm = 0xFFFF;
	g_pCustom->bltalwm = 0xFFFF;

	g_pCustom->bltamod = wSrcModulo;
	g_pCustom->bltdmod = wDstModulo;
	g_pCustom->bltapt = (UBYTE*)((ULONG)s_pBgBuffer->Planes[0]);

	for (UBYTE ubIdx = 0; ubIdx < ENTITY_MAX_COUNT; ++ubIdx) {
		tEntity *pEntity = &s_pEntities[ubIdx];
		if(pEntity->ubType != ENTITY_TYPE_OFF) {
			ULONG ulDstOffs = pBuffer->BytesPerRow * pEntity->uwUndrawY + (pEntity->uwUndrawX>>3);
			blitWait();
			g_pCustom->bltdpt = (UBYTE*)((ULONG)pBuffer->Planes[0] + ulDstOffs);
			g_pCustom->bltsize = (uwHeight << 6) | uwBlitWords;
		}
	}

	// Save BG on new pos
	wSrcModulo = bitmapGetByteWidth(pBuffer) - (uwBlitWords<<1);
	wDstModulo = bitmapGetByteWidth(s_pBgBuffer) - (uwBlitWords<<1);
	blitWait();
	g_pCustom->bltamod = wSrcModulo;
	g_pCustom->bltdmod = wDstModulo;
	g_pCustom->bltdpt = (UBYTE*)((ULONG)s_pBgBuffer->Planes[0]);
	for (UBYTE ubIdx = 0; ubIdx < ENTITY_MAX_COUNT; ++ubIdx) {
		tEntity *pEntity = &s_pEntities[ubIdx];
		if(pEntity->ubType != ENTITY_TYPE_OFF) {
			ULONG ulSrcOffs = pBuffer->BytesPerRow * pEntity->uwY + (pEntity->uwX>>3);
			blitWait();
			g_pCustom->bltapt = (UBYTE*)((ULONG)pBuffer->Planes[0] + ulSrcOffs);
			g_pCustom->bltsize = (uwHeight << 6) | uwBlitWords;
		}
	}

	// Draw entity on new pos
	for (UBYTE ubIdx = 0; ubIdx < ENTITY_MAX_COUNT; ++ubIdx) {
		tEntity *pEntity = &s_pEntities[ubIdx];
		if(pEntity->ubType != ENTITY_TYPE_OFF) {
			UBYTE ubDstOffs = pEntity->uwX & 0xF;
			UWORD uwBlitWidth = (16+ubDstOffs+15) & 0xFFF0;
			UWORD uwBlitWords = uwBlitWidth >> 4;
			wSrcModulo = bitmapGetByteWidth(s_pDirFrames[0]) - (uwBlitWords<<1);
			UWORD uwLastMask = 0xFFFF << (uwBlitWidth-16);
			UWORD uwBltCon1 = ubDstOffs << BSHIFTSHIFT;
			UWORD uwBltCon0 = uwBltCon1 | USEA|USEB|USEC|USED | MINTERM_COOKIE;
			ULONG ulSrcOffs = s_pDirFrames[pEntity->ubDir]->BytesPerRow * s_pEntities[ubIdx].ubFrame * 20;
			ULONG ulDstOffs = pBuffer->BytesPerRow * pEntity->uwY + (pEntity->uwX>>3);

			wDstModulo = bitmapGetByteWidth(pBuffer) - (uwBlitWords<<1);

			blitWait();
			g_pCustom->bltcon0 = uwBltCon0;
			g_pCustom->bltcon1 = uwBltCon1;
			g_pCustom->bltalwm = uwLastMask;

			g_pCustom->bltamod = wSrcModulo;
			g_pCustom->bltbmod = wSrcModulo;
			g_pCustom->bltcmod = wDstModulo;
			g_pCustom->bltdmod = wDstModulo;

			g_pCustom->bltapt = (UBYTE*)((ULONG)s_pDirMasks[pEntity->ubDir]->Planes[0] + ulSrcOffs);
			g_pCustom->bltbpt = (UBYTE*)((ULONG)s_pDirFrames[pEntity->ubDir]->Planes[0] + ulSrcOffs);
			g_pCustom->bltcpt = (UBYTE*)((ULONG)pBuffer->Planes[0] + ulDstOffs);
			g_pCustom->bltdpt = (UBYTE*)((ULONG)pBuffer->Planes[0] + ulDstOffs);

			g_pCustom->bltsize = (uwHeight << 6) | uwBlitWords;
		}
	}

	for (UBYTE ubIdx = 0; ubIdx < ENTITY_MAX_COUNT; ++ubIdx) {
		tEntity *pEntity = &s_pEntities[ubIdx];
		if(pEntity->ubType != ENTITY_TYPE_OFF) {
			pEntity->uwUndrawX = pEntity->uwX;
			pEntity->uwUndrawY = pEntity->uwY;
		}
	}
}
