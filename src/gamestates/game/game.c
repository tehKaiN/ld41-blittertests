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

#define GAME_BPP 4

void gameGsCreate(void) {
	s_pView = viewCreate(0,
		TAG_VIEW_COPLIST_MODE, VIEW_COPLIST_MODE_RAW,
		TAG_VIEW_COPLIST_RAW_COUNT, 50,
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
	TAG_DONE);

	tBitMap *pTile = bitmapCreateFromFile("data/tile.bm");
	for(UBYTE x = 0; x < 32; ++x) {
		blitCopyAligned(pTile, 0, 0, s_pBuffer->pBuffer, x*16, 0, 16 ,16);
	}
	for(UBYTE y = 0; y < 32; ++y) {
		blitCopyAligned(s_pBuffer->pBuffer, 0, 0, s_pBuffer->pBuffer, 0, 16*y, 512, 16);
	}
	bitmapDestroy(pTile);

	paletteLoad("data/amidb16.plt", s_pVPort->pPalette, 16);

	entityListCreate();
	s_ubEntityPlayer = entityAdd(32, 32, ENTITY_DIR_DOWN);
	entityAdd(48, 64, ENTITY_DIR_DOWN);
	entityAdd(80, 64, ENTITY_DIR_DOWN);
	entityAdd(112, 64, ENTITY_DIR_DOWN);

	viewLoad(s_pView);
	systemUnuse();
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
	entityMove(s_ubEntityPlayer, bDx, bDy);

	entityProcessDraw(s_pBuffer->pBuffer);
	vPortWaitForEnd(s_pVPort);
}

void gameGsDestroy(void) {
	viewLoad(0);
	systemUse();

	entityListDestroy();

	viewDestroy(s_pView);

}
