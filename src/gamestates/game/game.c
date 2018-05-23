/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gamestates/game/game.h"
#include <ace/managers/game.h>
#include <ace/managers/key.h>
#include <ace/managers/blit.h>
#include <ace/managers/system.h>
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/utils/extview.h>
#include <ace/utils/palette.h>
#include "gamestates/game/entity.h"

tView *s_pView;
tVPort *s_pVPort;
tSimpleBufferManager *s_pBuffer;

UBYTE s_ubEntityPlayer;
UBYTE s_ubBufferIdx = 0;

#define GAME_BPP 4

UBYTE s_ubEntityCount = 0;

void gameGsCreate(void) {
	s_pView = viewCreate(0,
		TAG_VIEW_COPLIST_MODE, VIEW_COPLIST_MODE_RAW,
		TAG_VIEW_COPLIST_RAW_COUNT, 1200,
		TAG_VIEW_GLOBAL_CLUT, 1,
	TAG_DONE);

	s_pVPort = vPortCreate(0,
		TAG_VPORT_BPP, GAME_BPP,
		TAG_VPORT_VIEW, s_pView,
	TAG_DONE);

	s_pBuffer = simpleBufferCreate(0,
		TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_SIMPLEBUFFER_COPLIST_OFFSET, 0,
		TAG_SIMPLEBUFFER_VPORT, s_pVPort,
		TAG_SIMPLEBUFFER_BOUND_WIDTH, 512,
		TAG_SIMPLEBUFFER_BOUND_HEIGHT, 512,
		TAG_SIMPLEBUFFER_IS_DBLBUF, 1,
	TAG_DONE);

	tBitMap *pTile = bitmapCreateFromFile("data/tile.bm");
	for(UBYTE x = 0; x < 32; ++x) {
		blitCopyAligned(pTile, 0, 0, s_pBuffer->pBack, x*16, 0, 16 ,16);
	}
	for(UBYTE y = 0; y < 32; ++y) {
		blitCopyAligned(s_pBuffer->pBack, 0, 0, s_pBuffer->pBack, 0, 16*y, 512, 16);
	}
	blitCopyAligned(
		s_pBuffer->pBack, 0, 0, s_pBuffer->pFront, 0, 0,
		bitmapGetByteWidth(s_pBuffer->pBack)*8, s_pBuffer->pBack->Rows
	);
	bitmapDestroy(pTile);

	paletteLoad("data/amidb16.plt", s_pVPort->pPalette, 16);

	entityListCreate(s_pView);
	s_ubEntityPlayer = entityAdd(32, 32, ENTITY_DIR_DOWN);

	viewLoad(s_pView);
	systemUnuse();
	g_pCustom->copcon = BV(1);
	systemSetDma(DMAB_BLITHOG, 1);
	s_ubBufferIdx = 0;
	s_ubEntityCount = 0;
}

void gameGsLoop(void) {
	if(keyUse(KEY_ESCAPE)) {
		gameClose();
		return;
	}
	BYTE bDx = 0, bDy = 0;
	if(keyCheck(KEY_LEFT)) {
		bDx -= 2;
	}
	if(keyCheck(KEY_RIGHT)) {
		bDx += 2;
	}
	if(keyCheck(KEY_UP)) {
		bDy -= 2;
	}
	if(keyCheck(KEY_DOWN)) {
		bDy += 2;
	}

	if(keyUse(KEY_SPACE)) {
		entityAdd(
			48 + 32*(s_ubEntityCount&7), 32 + 32*(s_ubEntityCount>>3),
			ENTITY_DIR_DOWN
		);
		++s_ubEntityCount;
	}

	entityMove(s_ubEntityPlayer, bDx, bDy);

	// Prepare copperlist for next back buffer
	UWORD uwStop = entityProcessDraw(s_pBuffer->pBack, s_ubBufferIdx);
	s_ubBufferIdx = !s_ubBufferIdx;
	copSetWait(&s_pView->pCopList->pBackBfr->pList[uwStop++].sWait, 0xFF, 0xFF);
	copSetWait(&s_pView->pCopList->pBackBfr->pList[uwStop++].sWait, 0xFF, 0xFF);

	viewProcessManagers(s_pView);
	vPortWaitForEnd(s_pVPort);
	copSwapBuffers();

	// entity manager draws on undisplayed bfr during displayed coplist
	// simple buffer manager sets bplpt on copper back bfr and swaps back/front
	// so after copper finishes drawing on back and coplist is finished for next back,
	// simple buffer sets bplpt and swaps, then copper is swapped

	// camera move
	// entity -> prepare coplist on back
	// buffer -> prepare new coplist bplpt on back
	// as atomic as possible:
	// buffer -> swap back/front
	// copper -> swap back/front <- this MUST be before vblank
}

void gameGsDestroy(void) {
	viewLoad(0);
	systemSetDma(DMAB_BLITHOG, 0);
	g_pCustom->copcon = 0;
	blitWait();
	systemUse();

	entityListDestroy();

	viewDestroy(s_pView);
}
