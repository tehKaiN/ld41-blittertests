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

	paletteLoad("data/amidb16.plt", s_pVPort->pPalette, 16);

	entityListCreate();
	s_ubEntityPlayer = entityAdd(32, 32, ENTITY_DIR_DOWN);

	viewLoad(s_pView);
	systemUnuse();
}

void gameGsLoop(void) {
	if(keyUse(KEY_ESCAPE)) {
		gameClose();
		return;
	}

	entityProcessDraw(s_pBuffer->pBuffer);
	vPortWaitForEnd(s_pVPort);
}

void gameGsDestroy(void) {
	viewLoad(0);
	systemUse();

	entityListDestroy();

	viewDestroy(s_pView);

}
