#include "gamestates/game/game.h"
#include <ace/managers/game.h>
#include <ace/managers/key.h>
#include <ace/managers/blit.h>
#include <ace/managers/system.h>
#include <ace/managers/viewport/simplebuffer.h>
#include <ace/utils/extview.h>

tView *s_pView;
tVPort *s_pVPort;
tSimpleBufferManager *s_pBuffer;

#define GAME_BPP 1

void gameGsCreate(void) {
	const UWORD pPalette[] = {0, 0xFFF};
	s_pView = viewCreate(0,
		TAG_VIEW_COPLIST_MODE, VIEW_COPLIST_MODE_RAW,
		TAG_VIEW_COPLIST_RAW_COUNT, 50,
		TAG_VIEW_GLOBAL_CLUT, 1,
	TAG_DONE);

	s_pVPort = vPortCreate(0,
		TAG_VPORT_BPP, GAME_BPP,
		TAG_VPORT_PALETTE_PTR, pPalette,
		TAG_VPORT_PALETTE_SIZE, 2,
		TAG_VPORT_VIEW, s_pView,
	TAG_DONE);

	s_pBuffer = simpleBufferCreate(0,
		TAG_SIMPLEBUFFER_BITMAP_FLAGS, BMF_CLEAR | BMF_INTERLEAVED,
		TAG_SIMPLEBUFFER_COPLIST_OFFSET, 0,
		TAG_SIMPLEBUFFER_VPORT, s_pVPort,
		TAG_SIMPLEBUFFER_BOUND_WIDTH, 512,
		TAG_SIMPLEBUFFER_BOUND_HEIGHT, 512,
	TAG_DONE);


	viewLoad(s_pView);
	systemUnuse();
	blitRect(s_pBuffer->pBuffer, 16, 16, 32, 32, 1);
}

void gameGsLoop(void) {
	if(keyUse(KEY_ESCAPE)) {
		gameClose();
		return;
	}
}

void gameGsDestroy(void) {
	viewLoad(0);
	systemUse();
	viewDestroy(s_pView);
}
