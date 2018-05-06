#include "gamestates/game/entity.h"
#include <ace/managers/blit.h>
#include <ace/utils/bitmap.h>
#include <ace/managers/key.h>

#define ENTITY_MAX_COUNT 50

#define ENTITY_FRAME_STAND 0
#define ENTITY_FRAME_WALK1 1
#define ENTITY_FRAME_WALK2 2
#define ENTITY_FRAME_WALK3 3
#define ENTITY_FRAME_WALK4 4
#define ENTITY_ACTION1 5
#define ENTITY_ACTION2 6

tEntity s_pEntities[ENTITY_MAX_COUNT] = {{0}};

tCopList *s_pCopList;

tBitMap *s_pDirFrames[4];
tBitMap *s_pDirMasks[4];

tBitMap *s_pBgBuffer[2];

void entityListCreate(tCopList *pCopList) {
	s_pDirFrames[ENTITY_DIR_UP] = bitmapCreateFromFile("data/skelet/up.bm");
	s_pDirFrames[ENTITY_DIR_DOWN] = bitmapCreateFromFile("data/skelet/down.bm");
	s_pDirFrames[ENTITY_DIR_LEFT] = bitmapCreateFromFile("data/skelet/left.bm");
	s_pDirFrames[ENTITY_DIR_RIGHT] = bitmapCreateFromFile("data/skelet/right.bm");

	s_pDirMasks[ENTITY_DIR_UP] = bitmapCreateFromFile("data/skelet/up_mask.bm");
	s_pDirMasks[ENTITY_DIR_DOWN] = bitmapCreateFromFile("data/skelet/down_mask.bm");
	s_pDirMasks[ENTITY_DIR_LEFT] = bitmapCreateFromFile("data/skelet/left_mask.bm");
	s_pDirMasks[ENTITY_DIR_RIGHT] = bitmapCreateFromFile("data/skelet/right_mask.bm");

	s_pBgBuffer[0] = bitmapCreate(
		bitmapGetByteWidth(s_pDirFrames[0])*8 +16, 20*ENTITY_MAX_COUNT,
		s_pDirFrames[0]->Depth, BMF_CLEAR | BMF_INTERLEAVED
	);
	s_pBgBuffer[1] = bitmapCreate(
		bitmapGetByteWidth(s_pDirFrames[0])*8 +16, 20*ENTITY_MAX_COUNT,
		s_pDirFrames[0]->Depth, BMF_CLEAR | BMF_INTERLEAVED
	);

	for (UBYTE ubIdx = 0; ubIdx < ENTITY_MAX_COUNT; ++ubIdx) {
		s_pEntities[ubIdx].ubType = ENTITY_TYPE_OFF;
	}

	s_pCopList = pCopList;
}

void entityListDestroy(void) {
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
			s_pEntities[ubIdx].pUndrawX[0] = 0;
			s_pEntities[ubIdx].pUndrawY[0] = 0;
			s_pEntities[ubIdx].pUndrawX[1] = 0;
			s_pEntities[ubIdx].pUndrawY[1] = 0;
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

UWORD entityProcessDraw(tBitMap *pBuffer, UBYTE ubBufferIdx) {
	tCopperUlong REGPTR pBltPtA = (tCopperUlong REGPTR)&g_pCustom->bltapt;
	tCopperUlong REGPTR pBltPtB = (tCopperUlong REGPTR)&g_pCustom->bltbpt;
	tCopperUlong REGPTR pBltPtC = (tCopperUlong REGPTR)&g_pCustom->bltcpt;
	tCopperUlong REGPTR pBltPtD = (tCopperUlong REGPTR)&g_pCustom->bltdpt;
	tCopCmd *s_pCopCmds = s_pCopList->pBackBfr->pList;
	UWORD uwCopIdx = 14; // From VPort manager
	// Let's imitate new Copper pipeline
	// Restore BG on old pos
	UWORD uwWidth = 32;
	UWORD uwHeight = 20*s_pDirFrames[0]->Depth;
	UWORD uwBlitWords = uwWidth >> 4;
	UWORD uwBlitSize = (uwHeight << 6) | uwBlitWords;
	UWORD uwBltCon0 = USEA|USED | MINTERM_A;
	WORD wSrcModulo = bitmapGetByteWidth(s_pBgBuffer[ubBufferIdx]) - (uwBlitWords<<1);
	WORD wDstModulo = bitmapGetByteWidth(pBuffer) - (uwBlitWords<<1);
	ULONG ulA, ulB, ulCD;

	copSetWait(&s_pCopCmds[uwCopIdx].sWait, 0, 0);
	s_pCopCmds[uwCopIdx].sWait.bfBlitterIgnore = 0;
	s_pCopCmds[uwCopIdx].sWait.bfVE = 0;
	s_pCopCmds[uwCopIdx++].sWait.bfHE = 0;
	copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &g_pCustom->color[0], 0x111);
	copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &g_pCustom->bltcon0, uwBltCon0);
	copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &g_pCustom->bltcon1, 0);
	copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &g_pCustom->bltafwm, 0xFFFF);
	copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &g_pCustom->bltalwm, 0xFFFF);
	copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &g_pCustom->bltamod, wSrcModulo);
	copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &g_pCustom->bltdmod, wDstModulo);
	ulA = (ULONG)(s_pBgBuffer[ubBufferIdx]->Planes[0]);
	copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &pBltPtA->uwHi, ulA >> 16);
	copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &pBltPtA->uwLo, ulA & 0xFFFF);

	for (UBYTE ubIdx = 0; ubIdx < ENTITY_MAX_COUNT; ++ubIdx) {
		tEntity *pEntity = &s_pEntities[ubIdx];
		if(pEntity->ubType != ENTITY_TYPE_OFF) {
			ULONG ulDstOffs = (
				pBuffer->BytesPerRow * pEntity->pUndrawY[ubBufferIdx] +
				(pEntity->pUndrawX[ubBufferIdx]>>3)
			);
			ulCD = (ULONG)(pBuffer->Planes[0]) + ulDstOffs;
			copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &pBltPtD->uwHi, ulCD >> 16);
			copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &pBltPtD->uwLo, ulCD & 0xFFFF);
			copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &g_pCustom->bltsize, uwBlitSize);
			copSetWait(&s_pCopCmds[uwCopIdx].sWait, 0, 0);
			s_pCopCmds[uwCopIdx].sWait.bfHE = 0;
			s_pCopCmds[uwCopIdx].sWait.bfVE = 0;
			s_pCopCmds[uwCopIdx++].sWait.bfBlitterIgnore = 0;
		}
	}
	// Copper cost: 9+4n
	// Initial WAIT @ pos & blitter
	// 8 MOVES
	// For each bob: 3 MOVEs and WAIT for blitter

	// Save BG on new pos
	copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &g_pCustom->color[0], 0x222);
	wSrcModulo = bitmapGetByteWidth(pBuffer) - (uwBlitWords<<1);
	wDstModulo = bitmapGetByteWidth(s_pBgBuffer[ubBufferIdx]) - (uwBlitWords<<1);
	copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &g_pCustom->bltamod, wSrcModulo);
	copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &g_pCustom->bltdmod, wDstModulo);
	ulCD = (ULONG)(s_pBgBuffer[ubBufferIdx]->Planes[0]);
	copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &pBltPtD->uwHi, ulCD >> 16);
	copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &pBltPtD->uwLo, ulCD & 0xFFFF);
	for(UBYTE ubIdx = 0; ubIdx < ENTITY_MAX_COUNT; ++ubIdx) {
		tEntity *pEntity = &s_pEntities[ubIdx];
		if(pEntity->ubType != ENTITY_TYPE_OFF) {
			ULONG ulSrcOffs = pBuffer->BytesPerRow * pEntity->uwY + (pEntity->uwX>>3);
			ULONG ulA = (ULONG)(pBuffer->Planes[0]) + ulSrcOffs;
			copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &pBltPtA->uwHi, ulA >> 16);
			copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &pBltPtA->uwLo, ulA & 0xFFFF);
			copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &g_pCustom->bltsize, uwBlitSize);
			copSetWait(&s_pCopCmds[uwCopIdx].sWait, 0, 0);
			s_pCopCmds[uwCopIdx].sWait.bfHE = 0;
			s_pCopCmds[uwCopIdx].sWait.bfVE = 0;
			s_pCopCmds[uwCopIdx++].sWait.bfBlitterIgnore = 0;
		}
	}
	// Copper cost: 4+4n
	// 4 MOVES
	// For each bob: 3 MOVEs and WAIT for blitter

	// Draw entity on new pos
	copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &g_pCustom->color[0], 0x333);
	for (UBYTE ubIdx = 0; ubIdx < ENTITY_MAX_COUNT; ++ubIdx) {
		tEntity *pEntity = &s_pEntities[ubIdx];
		if(pEntity->ubType != ENTITY_TYPE_OFF) {
			UBYTE ubDstOffs = pEntity->uwX & 0xF;
			UWORD uwBlitWidth = (16+ubDstOffs+15) & 0xFFF0;
			UWORD uwBlitWords = uwBlitWidth >> 4;
			uwBlitSize = (uwHeight << 6) | uwBlitWords;
			wSrcModulo = bitmapGetByteWidth(s_pDirFrames[0]) - (uwBlitWords<<1);
			UWORD uwLastMask = 0xFFFF << (uwBlitWidth-16);
			UWORD uwBltCon1 = ubDstOffs << BSHIFTSHIFT;
			UWORD uwBltCon0 = uwBltCon1 | USEA|USEB|USEC|USED | MINTERM_COOKIE;
			ULONG ulSrcOffs = s_pDirFrames[pEntity->ubDir]->BytesPerRow * s_pEntities[ubIdx].ubFrame * 20;
			ULONG ulDstOffs = pBuffer->BytesPerRow * pEntity->uwY + (pEntity->uwX>>3);

			wDstModulo = bitmapGetByteWidth(pBuffer) - (uwBlitWords<<1);
			ulA = (ULONG)(s_pDirMasks[pEntity->ubDir]->Planes[0]) + ulSrcOffs;
			ulB = (ULONG)(s_pDirFrames[pEntity->ubDir]->Planes[0]) + ulSrcOffs;
			ulCD = (ULONG)(pBuffer->Planes[0]) + ulDstOffs;

			copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &g_pCustom->bltcon0, uwBltCon0);
			copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &g_pCustom->bltcon1, uwBltCon1);
			copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &g_pCustom->bltalwm, uwLastMask);

			copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &g_pCustom->bltamod, wSrcModulo);
			copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &g_pCustom->bltbmod, wSrcModulo);
			copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &g_pCustom->bltcmod, wDstModulo);
			copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &g_pCustom->bltdmod, wDstModulo);

			copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &pBltPtA->uwHi, ulA >> 16);
			copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &pBltPtA->uwLo, ulA & 0xFFFF);
			copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &pBltPtB->uwHi, ulB >> 16);
			copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &pBltPtB->uwLo, ulB & 0xFFFF);
			copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &pBltPtC->uwHi, ulCD >> 16);
			copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &pBltPtC->uwLo, ulCD & 0xFFFF);
			copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &pBltPtD->uwHi, ulCD >> 16);
			copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &pBltPtD->uwLo, ulCD & 0xFFFF);

			copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &g_pCustom->bltsize, uwBlitSize);

			copSetWait(&s_pCopCmds[uwCopIdx].sWait, 0, 0);
			s_pCopCmds[uwCopIdx].sWait.bfHE = 0;
			s_pCopCmds[uwCopIdx].sWait.bfVE = 0;
			s_pCopCmds[uwCopIdx++].sWait.bfBlitterIgnore = 0;
		}
	}
	// Copper cost: 17n
	// For each bob: 16 MOVEs and WAIT for blitter
	copSetMove(&s_pCopCmds[uwCopIdx++].sMove, &g_pCustom->color[0], 0x000);

	for (UBYTE ubIdx = 0; ubIdx < ENTITY_MAX_COUNT; ++ubIdx) {
		tEntity *pEntity = &s_pEntities[ubIdx];
		if(pEntity->ubType != ENTITY_TYPE_OFF) {
			pEntity->pUndrawX[ubBufferIdx] = pEntity->uwX;
			pEntity->pUndrawY[ubBufferIdx] = pEntity->uwY;
		}
	}
	// Total copper cost:
	// 9+4n + 4+4n + 17n = 13+25n
	// For 8 bobs it's 13+25*8 = 213 instructions = 852B = 1704 px = 5 lines

	return uwCopIdx;
}
