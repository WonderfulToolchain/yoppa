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
#include "spark.h"

#include "spark.bmh"

#define SPARK_MAX 16

#define SPARK_STATE_UNUSED 0
#define SPARK_STATE_RUN    1

static struct spark {
	int state;
	int x, y, xv, yv, a;
} spark[SPARK_MAX];
static int next_spark;

void
spark_init()
{
	int i;
	
	next_spark = 0;
	
	for (i = 0; i < SPARK_MAX; i ++) {
		spark[i].state = SPARK_STATE_UNUSED;
	}
	
	font_set_colordata(SPARK_CHAR_SPARK, 1, bmp_spark);
}

void
spark_hit(int x, int y, int side)
{
	struct spark *s;

	s= &spark[next_spark];
	next_spark = (next_spark + 1) % SPARK_MAX;
	
	s->state = SPARK_STATE_RUN;
	s->x = (x-3)<<3;
	s->y = (y-3)<<3;
	s->xv = ((rand()%32)+16);
	s->yv = -((rand()%8)+4)*(side*2-1);
	s->a = 1;
}

void
spark_frame()
{
	int i, x, y;
	struct spark *s = spark;
	
	for (i = 0; i < SPARK_MAX; i++, s++) {
		switch (s->state) {
		case SPARK_STATE_UNUSED:
		default:
			break;
		case SPARK_STATE_RUN:
			s->x += s->xv;
			s->y += s->yv;
			s->yv += s->a;
			/* ドットに修正 */
			x = s->x>>3;
			y = s->y>>3;
			/* 画面外? */
			if (y < 0 || y > LCD_PIXEL_HEIGHT) {
				s->state = SPARK_STATE_UNUSED;
				vsprite_disable(SPARK_SP_SPARK + i);
			}
			/* 表示 */
			vsprite_set_char_location(SPARK_SP_SPARK + i, x, y, 
					SPARK_CHAR_SPARK|CFM_SPR_UPPER|(4<<9));
			break;
		}
	}
}
