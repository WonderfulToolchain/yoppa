/*
 * Copyright (c) 2000, 2001
 *       Yes/No Software (Yoshiyuki Nakano). All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/bios.h>

#include "main.h"
#include "vscreen.h"
#include "saku.h"

#define SAKU_MAX 16

#define SAKU_STATE_UNUSED 0
#define SAKU_STATE_RUN    1

static struct saku {
	int state;
	int x, y, yv, sp;
} saku[SAKU_MAX];
static int next_saku;

void
saku_init()
{
	int i;
	
	next_saku = 0;
	
	for (i = 0; i < SAKU_MAX; i ++) {
		saku[i].state = SAKU_STATE_UNUSED;
	}
	
}

void
saku_add(int x, int y)
{
	struct saku *s;

	s= &saku[next_saku];
	
	s->state = SAKU_STATE_RUN;
	s->x = (x&‾7)<<5;
	s->y = (y&‾7)<<5;
	s->yv = 0;
	s->sp = SAKU_CHAR|CFM_SPR_UPPER|(4<<9);
	if (y < 144/2) {
		s->sp |= CFM_FLIP_V;
	}
	
	next_saku = (next_saku + 1) % SAKU_MAX;
}

void
saku_frame()
{
	int i, x, y;
	struct saku *s = saku;
	
	for (i = 0; i < SAKU_MAX; i++, s++) {
		switch (s->state) {
		case SAKU_STATE_UNUSED:
		default:
			break;
		case SAKU_STATE_RUN:
			s->y  += s->yv;
			s->yv += 1;
			/* ドットに修正 */
			x = s->x>>5;
			y = s->y>>5;
			/* 画面外? */
			if (y > LCD_PIXEL_HEIGHT) {
				s->state = SAKU_STATE_UNUSED;
				vsprite_disable(SAKU_SP + i);
			}
			/* 表示 */
			vsprite_set_char_location(SAKU_SP + i, x, y, s->sp);
			break;
		}
	}
}
