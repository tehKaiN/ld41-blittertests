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
tVPort *s_pVPortMain, *s_pVPortHud;
tSimpleBufferManager *s_pBufferMain, *s_pBufferHud;

UBYTE s_pEntityPlayers[4];
UBYTE s_ubBufferIdx = 0;

#define GAME_BPP 4

UBYTE s_ubEntityCount = 0;

#define TILE_SIZE 4
#define TILE_WIDTH 16
#define MAP_WIDTH 20
#define MAP_HEIGHT 15
#define HUD_HEIGHT 16

static UBYTE s_pTiles[MAP_WIDTH][MAP_HEIGHT];


void gameGsCreate(void) {
	s_pView = viewCreate(0,
		TAG_VIEW_COPLIST_MODE, VIEW_COPLIST_MODE_RAW,
		TAG_VIEW_COPLIST_RAW_COUNT, 40,
		TAG_VIEW_GLOBAL_CLUT, 1,
	TAG_DONE);

	s_pVPortHud = vPortCreate(0,
		TAG_VPORT_BPP, GAME_BPP,
		TAG_VPORT_VIEW, s_pView,
		TAG_VPORT_HEIGHT, HUD_HEIGHT,
	TAG_DONE);

	s_pBufferHud = simpleBufferCreate(0,
		TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_SIMPLEBUFFER_COPLIST_OFFSET, 0,
		TAG_SIMPLEBUFFER_VPORT, s_pVPortHud,
		TAG_SIMPLEBUFFER_BOUND_HEIGHT, HUD_HEIGHT,
	TAG_DONE);

	s_pVPortMain = vPortCreate(0,
		TAG_VPORT_BPP, GAME_BPP,
		TAG_VPORT_VIEW, s_pView,
	TAG_DONE);

	s_pBufferMain = simpleBufferCreate(0,
		TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_SIMPLEBUFFER_COPLIST_OFFSET, 14,
		TAG_SIMPLEBUFFER_VPORT, s_pVPortMain,
		TAG_SIMPLEBUFFER_BOUND_WIDTH, MAP_WIDTH * TILE_WIDTH,
		TAG_SIMPLEBUFFER_BOUND_HEIGHT, MAP_HEIGHT * TILE_WIDTH,
		TAG_SIMPLEBUFFER_IS_DBLBUF, 1,
	TAG_DONE);

	copDumpBfr(s_pView->pCopList->pBackBfr);

	tBitMap *pTile = bitmapCreateFromFile("data/tile.bm");
	for(UBYTE y = 0; y < MAP_HEIGHT; ++y) {
		for(UBYTE x = 0; x < MAP_WIDTH; ++x) {
			if(
				x == 0 || x == MAP_WIDTH-1 || y == 0 || y == MAP_HEIGHT-1 || // edge
				(y == MAP_HEIGHT/2 && (x == MAP_WIDTH/2 || x == MAP_WIDTH/2 -1))
			) {
				s_pTiles[x][y] = 0;
				blitRect(
					s_pBufferMain->pBack, x * TILE_WIDTH, y * TILE_WIDTH,
					TILE_WIDTH, TILE_WIDTH, 0
				);
			}
			else {
				s_pTiles[x][y] = 1;
				blitCopyAligned(
					pTile, 0, 0, s_pBufferMain->pBack,
					x * TILE_WIDTH, y * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH
				);
			}
		}
	}
	blitCopyAligned(
		s_pBufferMain->pBack, 0, 0, s_pBufferMain->pFront, 0, 0,
		bitmapGetByteWidth(s_pBufferMain->pBack)*8, s_pBufferMain->pBack->Rows
	);
	bitmapDestroy(pTile);

	paletteLoad("data/amidb16.plt", s_pVPortHud->pPalette, 16);

	entityListCreate(s_pView);
	tUwCoordYX pSpawns[4] = {
		{.sUwCoord = {16, 16}},
		{.sUwCoord = {64, 16}},
		{.sUwCoord = {16, 48}},
		{.sUwCoord = {64, 48}}
	};

	for(UBYTE i = 0; i < 4; ++i) {
		s_pEntityPlayers[i] = entityAdd(
			pSpawns[i].sUwCoord.uwX, pSpawns[i].sUwCoord.uwY, ENTITY_DIR_DOWN
		);
	}

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

	entityMove(s_pEntityPlayers[0], bDx, bDy);

	// Prepare copperlist for next back buffer
	entityProcessDraw(s_pBufferMain->pBack, s_ubBufferIdx);
	s_ubBufferIdx = !s_ubBufferIdx;
	UWORD uwStop = 28;
	copSetWait(&s_pView->pCopList->pBackBfr->pList[uwStop++].sWait, 0xFF, 0xFF);
	copSetWait(&s_pView->pCopList->pBackBfr->pList[uwStop++].sWait, 0xFF, 0xFF);

	viewProcessManagers(s_pView);
	vPortWaitForEnd(s_pVPortMain);
	copSwapBuffers();
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
