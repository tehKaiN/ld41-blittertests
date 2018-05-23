/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _LD41_GAMESTATES_GAME_ENTITY_H_
#define _LD41_GAMESTATES_GAME_ENTITY_H_

#include <ace/types.h>
#include <ace/utils/bitmap.h>
#include <ace/utils/extview.h>
#include "bob_new.h"

#define ENTITY_DIR_UP 0
#define ENTITY_DIR_DOWN 1
#define ENTITY_DIR_LEFT 2
#define ENTITY_DIR_RIGHT 3

#define ENTITY_TYPE_OFF 0
#define ENTITY_TYPE_SKELET 1

#define ENTITY_IDX_INVALID 255

typedef struct _tEntity {
	tBobNew sBob;
	UBYTE ubDir;
	UBYTE ubType;
	UBYTE ubFrame;
} tEntity;

void entityListCreate(tView *pView);

void entityListDestroy(void);

UBYTE entityAdd(UWORD uwX, UWORD uwY, UBYTE ubDir);

void entityDestroy(UBYTE ubEntityIdx);

UWORD entityProcessDraw(tBitMap *pBuffer, UBYTE ubBufferIdx);

void entityMove(UBYTE ubEntityIdx, BYTE bDx, BYTE bDy);

#endif // _LD41_GAMESTATES_GAME_ENTITY_H_
