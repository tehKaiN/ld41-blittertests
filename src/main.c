/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "main.h"
#include <ace/generic/main.h>
#include <ace/managers/game.h>
#include <ace/managers/joy.h>
#include <ace/managers/key.h>
#include "gamestates/game/game.h"
// #include "gamestates/menu/menu.h"

tFont *g_pFont;
tTextBitMap *g_pLineBuffer;

void inputProcess(void) {
	joyProcess();
	keyProcess();
}

void genericCreate(void) {
	keyCreate();
	joyOpen();

	g_pFont = fontCreate("data/silkscreen5.fnt");
	g_pLineBuffer = fontCreateTextBitMap(320, g_pFont->uwHeight);

	gamePushState(gameGsCreate, gameGsLoop, gameGsDestroy); // gamePushState vs gameChangeState
}

void genericProcess(void) {
	inputProcess();
	gameProcess();
}

void genericDestroy(void) {
	fontDestroyTextBitMap(g_pLineBuffer);
	fontDestroy(g_pFont);
	keyDestroy();
	joyClose();
}
