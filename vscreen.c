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

#include <sys/bios.h>
#include "main.h"
#include "vscreen.h"

#include "saku.bmh"
#include "yuka.bmh"
#include "num.bmh"
unsigned short bmp_blank[] = {
	0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
};

/*
 * 仮想スクリーン定義，横幅(と縦幅？)無限大のスクリーンを定義する．
 * BIOSによる表示の代わりにこっちを使う．
 * パレット4番をしようする．
 */
#define PALETTE 12
#define PALBIT (PALETTE<<9)
#define PALBITSP ((PALETTE-8)<<9)

/* 現在のスクロール位置, 最大スクロール範囲 */
static int scx = 0, scy = 0, max_scx = 0, max_scy = 0;
static int dest, palette;
static struct vsploc {
	unsigned short attr;
	unsigned char y, x;
} vsploc[128];

void
vscreen_init(int d)
{
	int i;

	/* 仮想スプライトエリア初期化 */
	for (i = 0; i < 128; i++) {
		/* とりあえず画面外へ */
		vsploc[i].x = 0;
		vsploc[i].y = 200;
		vsploc[i].attr = 0;
	}
	
	/* その他メンバ変数初期化 */
	dest = d;
	palette = PALETTE<<9;
	
	palette_set_color(PALETTE, 0x7410);
	
	/* システムフォント */
	font_set_colordata(NUM_CHAR, 13, bmp_num);
	font_set_colordata(BLANK_CHAR, 1, bmp_blank);
	
	/* 上下の柵 */
	font_set_colordata(SAKU_CHAR, 1, bmp_saku);
	font_set_colordata(YUKA_CHAR, 4, bmp_yuka);
	
	/* スクリーンを初期化 */
	sprite_set_range(0, USE_SPRITE_NUM);
	vscreen_clear();
	screen_set_scroll(0, 0, 0);
	screen_set_scroll(1, 0, 0);
	
	vscreen_display();
}

void
vscreen_clear()
{
	int i;
	static unsigned short y1[] = {
		YUKA_CHAR  |PALBIT,
		YUKA_CHAR+1|PALBIT,
		YUKA_CHAR+2|PALBIT,
		YUKA_CHAR+3|PALBIT
	};
	static unsigned short y2[] = {
		YUKA_CHAR+2|PALBIT|CFM_FLIP_V,
		YUKA_CHAR+3|PALBIT|CFM_FLIP_V,
		YUKA_CHAR  |PALBIT|CFM_FLIP_V,
		YUKA_CHAR+1|PALBIT|CFM_FLIP_V
	};
	
	/* スクリーンを初期化 */
	screen_fill_char(dest, 0, 0, 32, 32, BLANK_CHAR|palette);
	
	/* 床 */
	for (i = 0; i < 16; i++) {
		screen_set_char(dest, i*2,                  0, 2, 2, y2);
		screen_set_char(dest, i*2, GAME_Y+GAME_HEIGHT, 2, 2, y1);
	}
	
	screen_fill_char(dest, 0, GAME_Y, 32, 1, SAKU_CHAR | PALBIT | CFM_FLIP_V);
	screen_fill_char(dest, 0, GAME_Y+GAME_HEIGHT-1, 32, 1, SAKU_CHAR | PALBIT);
	
	/* 仮想スプライトエリア初期化 */
	for (i = 0; i < 128; i++) {
		/* とりあえず画面外へ */
		vsploc[i].x = 0;
		vsploc[i].y = 200;
		vsploc[i].attr = 0;
	}
}

void
vscreen_set_maxscroll(int x, int y)
{
	max_scx = x;
	max_scy = y;
}

void
vscreen_set_scroll(int x, int y)
{
	scx = (x>0)? x: 0;
	scy = (y>0)? y: 0;
	if (max_scx > 0 && scx + LCD_PIXEL_WIDTH > max_scx) {
		scx = max_scx - LCD_PIXEL_WIDTH;
	}
	if (max_scy > 0 && scy + LCD_PIXEL_HEIGHT > max_scy) {
		scy = max_scy - LCD_PIXEL_HEIGHT;
	}
}

int
vscreen_get_scrollx()
{
	return scx;
}

int
vscreen_get_scrolly()
{
	return scy;
}

void
vscreen_set_char(int x, int y, unsigned short n)
{
	x = (x & 255) >> 3;
	y = (y & 255) >> 3;
	n |= palette;
	
	screen_fill_char(dest, x, y, 1, 1, n);
}

unsigned short
vscreen_get_char(int x, int y)
{
	static unsigned short r;
	
	x = (x & 255) >> 3;
	y = (y & 255) >> 3;
	screen_get_char(dest, x, y, 1, 1, &r);
	return r;
}

void
vscreen_fill_char_c(int x, int y, int w, int h, unsigned short n)
{
	if (w > 0 && h > 0) {
		screen_fill_char(dest, x, y, w, h, n|palette);
	}
}

void
vsprite_set_char_location(int n, int x, int y, unsigned short attr)
{
	/* 仮想座標に変換 */
	x -= scx;
	y -= scy;
	
	/* 画面外？ */
	if (x < -8 || x > 224 || y < -8 || y > 144) {
		/* とりあえず画面外へ */
		x = 0;
		y = 200;
	}
	
	/* 仮想領域に待避 */
	if (n >= 0 && n < 128) {
		struct vsploc *vsp = &vsploc[n];
		vsp->x = x;
		vsp->y = y;
		vsp->attr = attr;
	}
}

void
vsprite_disable(int n)
{
	struct vsploc *vsp = &vsploc[n];

	/* とりあえず画面外へ */
	vsp->x = 0;
	vsp->y = 200;
}

void
vscreen_display()
{
	int x = ((scx - 32)&255)>>3;
	screen_fill_char(dest, x, GAME_Y+1, 2, GAME_HEIGHT-2, BLANK_CHAR|PALBIT);
	screen_fill_char(dest, x, GAME_Y, 1, 1, SAKU_CHAR|PALBIT|CFM_FLIP_V);
	screen_fill_char(dest, x, GAME_Y+GAME_HEIGHT-1, 1, 1, SAKU_CHAR|PALBIT);
	screen_fill_char(dest, (x+1)&31, GAME_Y, 1, 1, SAKU_CHAR|PALBIT|CFM_FLIP_V);
	screen_fill_char(dest, (x+1)&31, GAME_Y+GAME_HEIGHT-1, 1, 1, SAKU_CHAR|PALBIT);

	sprite_set_data(0, USE_SPRITE_NUM, vsploc);
	screen_set_scroll(dest, scx&255, scy&255);
}
