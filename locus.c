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

#define LOCUS_MAX 128

/* 軌跡を書くモジュール
   キャラクタ0〜127番を使います */

/* 軌跡 */
static struct locus {
	int x, y;
	unsigned short pattern[8];
} locus[LOCUS_MAX];

/* リングバッファ先頭，最後(の次)，数 */
static int head, next, width;

void
locus_init()
{
	int i, j;
	
	for (i = 0; i < LOCUS_MAX; i++) {
		for (j = 0; j < 8; j++) {
			locus[i].pattern[j] = 0;
		}
		font_set_colordata(i, 1, bmp_blank);
	}
	
	head = 0;
	next = 0;
	width = 0;
}

void
locus_release()
{
	if (width) {
		head = (head + 1) % LOCUS_MAX;
		width--;
	}
}

/*
 * (x, y)が含まれる軌跡バッファを確保
 * バッファが足りない場合は古いものから消す．
 */
int
locus_new(int x, int y)
{
	int res, i;
	struct locus *lc;
	
	if (width && head == next) {
		locus_release();
	}
	
	res = next;
	next = (next + 1) % LOCUS_MAX;
	width++;
	
	/* 初期化 */
	lc = locus + res;
	for (i = 0; i < 8; i++) {
		lc->pattern[i] = 0;
	}
	lc->x = x & ‾7;
	lc->y = y & ‾7;
	
	return res;
}

/*
 * 軌跡バッファに点を打つ
 */
void
locus_set_point(int n, int x, int y, int c)
{
	struct locus *lc = locus + n;
	
	x = x&7;
	y = y&7;
	if (c & 1) {
		lc->pattern[y] |= 0x80>>(x);
	} else {
		lc->pattern[y] &= ‾(0x80>>(x));
	}
	if (c & 2) {
		lc->pattern[y] |= 0x8000>>(x);
	} else {
		lc->pattern[y] &= ‾(0x8000>>(x));
	}
	
	font_set_colordata(n, 1, lc->pattern);
}

/* 
 * (x, y)を含むlocusバッファを探す 
 * 無かったら新しいバッファを確保
 */
int
locus_search(int x, int y)
{
	int i, n;
	
	/* 後ろからサーチ */
	i = (next + LOCUS_MAX - 1) % LOCUS_MAX;
	n = width;
	while ( n > 0) {
		struct locus *lc = locus + i;
		
		if (x < lc->x) {
			/* locus[].xは単調増加なのでこれ以下には見つからない */
			break;
		}
		/* バッファキャラクタ内？ */
		if (x >= lc->x && x < lc->x + 8 &&
		    y >= lc->y && y < lc->y + 8) {
			return i;
		}
		
		/* 前 */
		i = (i + LOCUS_MAX - 1)%LOCUS_MAX; /* i = (i - i) % LOCUS_MAX */
		n--;
	}
	
	/* 新しい軌跡バッファを確保する */
	return locus_new(x, y);
}

/*
 * 仮想画面上のポイントに点を打つ
 */
void
locus_point(int x, int y, int c)
{
	int n;
	
	/* マイナスはだめ */
	if (x < 0 ||
		y <  GAME_Y*8 ||
		y >= GAME_Y*8 + GAME_HEIGHT*8)
		return;
	
	n = locus_search(x, y);
	locus_set_point(n, x, y, c);
	
	/* (x, y)にキャラクタnを置く．
	   x, y はドット単位 */
	vscreen_set_char(x, y, n|(4<<9));
}
