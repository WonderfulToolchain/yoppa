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
#include "gate.h"

#include "gateu.bmh"
#include "gated.bmh"

#define GATE_MAX 64

static unsigned short work[8];

/* ゲートの出現間隔, キャラクタ単位 */
static int gate_interval = 7;

/* パレットは12番を使用 */
#define PALETTE 12
static unsigned short palette = ((PALETTE-8)<<9);

/* アニメーション用に1ドットづつずらしたパターン，〜16個 */
#define GATE_CHARSET (128+16)

/* 端数表示用スプライトのリングバッファ管理変数 */
/* 最大数のキャラクタと最大数*2のスプライトを消費 */
/* キャラクタ: 128〜，スプライト: 96〜 */
#define GATE_SP_MAX 16
#define GATE_SP_CHAR 128
#define GATE_SP_SPRITE 0
static int gate_sp_next;

#define GATE_STATE_UNUSED 0
#define GATE_STATE_WAIT   1
#define GATE_STATE_ANIM   2
#define GATE_STATE_READY  3
#define GATE_STATE_CLEAR  4

static struct gate {
	int state;    /* ↑ 0: 未使用 1: 待機中 2: 伸びてる途中 3: 伸びた */
	int sleep;    /* WAIT状態からANIM状態に移行するまでの時間 */
	int x;        
	int u, d;     /* 上境界位置，下境界位置(上下からのドット数) */
	int au, ad;   /* アニメーション用，どこまで伸びたか(上下からのドット数) */
	int au8, ad8;   /* 上のメンバの8倍精度(汚い...) */
	int spu, spd; /* 端数ドット用スプライト */
} gate[GATE_MAX];

void
gate_init()
{
	int i;
	
	for (i = 0; i < GATE_MAX; i++) {
		gate[i].state = GATE_STATE_UNUSED;
	}
	
	gate_sp_next = 0;
	
	/* ずらしパターン登録 */
	for (i = 0; i < 8; i++) {
		font_set_colordata(GATE_CHARSET + i    , 1, bmp_gateu + i);
		font_set_colordata(GATE_CHARSET + i + 8, 1, bmp_gateu + i);
	}
}

/*
 * 端数ドット表示のためのスプライトを確保する
 */
int
gate_get_sp()
{
	int ret = gate_sp_next;
	gate_sp_next = (gate_sp_next + 1) % GATE_SP_MAX;
	return ret;
}

/*
 * ゲート伸びるの開始
 */
void
gate_start(struct gate *g)
{
	g->state = GATE_STATE_ANIM;
	/* アニメーション用メンバ初期化 */
	g->au = 0;
	g->ad = 0;
	g->au8 = 0;
	g->ad8 = 0;
	/* スプライト確保 */
	g->spu = gate_get_sp();
	g->spd = gate_get_sp();
}

/*
 * 1ステージ分のゲートを作成する
 * startドットから，endドットまで
 * ゲートの隙間はlevelドット
 * 乱数は，seedで初期化する
 */
void
gate_create(int start, int end, int level, int seed)
{
	int i, j, k;
	
	srand(seed);
	
	for (i = start&‾7, j = 0; i < end && j < GATE_MAX;
			i += gate_interval*8, j++) {
		gate[j].state = GATE_STATE_WAIT;
		gate[j].sleep = 0;
		gate[j].x = i;
		k = rand()%(GAME_HEIGHT*8-level);
		gate[j].u = k;
		gate[j].d = GAME_HEIGHT*8 - k - level;
	}
	
	/* 最初に一斉に出てくるのを抑制するため */
	gate[0].sleep = 0;
	gate[1].sleep = 50;
	gate[2].sleep = 100;
	gate[3].sleep = 150;
}

void
gate_release(struct gate *g)
{
	g->state = GATE_STATE_UNUSED;
}

/*
 * 各ゲート1フレーム計算＆表示
 */
void
gate_a_frame(struct gate *g)
{
	int sp, ch, i, j;

	switch (g->state) {
	case GATE_STATE_UNUSED:
		break;
	case GATE_STATE_WAIT:
		if (g->sleep) {
			g->sleep--;
		}
		if (!g->sleep &&
			vscreen_get_scrollx() + LCD_PIXEL_WIDTH - 8 >= g->x) {
			gate_start(g);
		}
		break;
	case GATE_STATE_ANIM:
		/** 上側更新 **/
		g->au8 += (g->u*8 - g->au8)/48 + 1;
		if (g->au8 > g->u*8) {
			g->au8 = g->u*8;
		}
		g->au = g->au8>>3;
		
		/** 下側更新 **/
		g->ad8 += (g->d*8 - g->ad8)/48 + 1;
		if (g->ad8 > g->d*8) {
			g->ad8 = g->d*8;
		}
		g->ad = g->ad8>>3;
		
		/*** 上側表示 ***/
		/** パターン作成 **/
		ch = GATE_SP_CHAR + g->spu;
		sp = GATE_SP_SPRITE + g->spu*2;
		
		
		/* スプライト */
		i = g->au%8;
		j = g->u%8;
		memcpy(work, &bmp_gateu[(j-i)&7], i*2);
		memset(work+i, 0, 16-i*2);
		font_set_colordata(ch, 1, work);
		
		/** キャラクタ表示 **/
		i = (j-i)&7;
		vscreen_fill_char_c((g->x&255)>>3, GAME_Y, 2, g->au>>3, 
							GATE_CHARSET + i);
		/*
		for (j = 7; j < g->au; j+=8) {
			vscreen_set_char(g->x  , GAME_Y*8 + j, GATE_CHARSET + i);
			vscreen_set_char(g->x+8, GAME_Y*8 + j, GATE_CHARSET + i);
		}
		*/
		
		/*** 下側表示 ***/
		/** パターン作成 **/
		ch = GATE_SP_CHAR + g->spd;
		sp = GATE_SP_SPRITE + g->spd*2;
		
		/* スプライト */
		i = g->ad%8;
		j = g->d%8;
		memset(work, 0, 16);
		memcpy(work+8-i, &bmp_gateu[8-j], i*2);
		font_set_colordata(ch, 1, work);
		
		/** キャラクタ表示 **/
		i = (i-j)&7;
		vscreen_fill_char_c((g->x&255)>>3, GAME_Y+GAME_HEIGHT-(g->ad>>3),
							2, g->ad>>3, GATE_CHARSET+8+i);
		/*
		for (j = 8; j <= g->ad; j+=8) {
			vscreen_set_char(g->x  , GAME_Y*8+GAME_HEIGHT*8 - j,
				GATE_CHARSET+8+i);
			vscreen_set_char(g->x+8, GAME_Y*8+GAME_HEIGHT*8 - j,
				GATE_CHARSET+8+i);
		}
		*/
		/*** 状態 ***/
		if (g->ad == g->d && g->au == g->u) {
			g->state = GATE_STATE_READY;
		}
		
		/*** そのままスプライトを表示するために↓へ ***/
		/* continuing... */
	case GATE_STATE_READY:
	case GATE_STATE_CLEAR:
		/* スプライトを表示させる */
		/* 上側 */
		ch = GATE_SP_CHAR + g->spu;
		sp = GATE_SP_SPRITE + g->spu*2;
		vsprite_set_char_location(sp, 
				g->x,
				(GAME_Y*8 + g->au)&‾7,
				ch|palette);
		vsprite_set_char_location(sp + 1, 
				g->x + 8,
				(GAME_Y*8 + g->au)&‾7,
				ch|palette);
		
		/* 下側 */
		ch = GATE_SP_CHAR + g->spd;
		sp = GATE_SP_SPRITE + g->spd*2;
		vsprite_set_char_location(sp,
				g->x,
				(GAME_Y*8 + GAME_HEIGHT*8 - g->ad)&‾7,
				ch|palette);
		vsprite_set_char_location(sp + 1,
				g->x + 8,
				(GAME_Y*8 + GAME_HEIGHT*8 - g->ad)&‾7,
				ch|palette);
				
		/* 画面から外れた？ */
		if (g->x + 16 < vscreen_get_scrollx()) {
			gate_release(g);
		}
		break;
	default:
		break;
	}
}

/*
 * 1フレーム計算&表示
 */
void
gate_frame()
{
	int i;
	struct gate *g = gate;
	
	for (i = 0; i < GATE_MAX; i++, g++) {
		gate_a_frame(g);
	}
}

/*
 * あたり判定
 * -1: 当たった
 * 0 : 当たってない
 * other: 近くにあるゲートID
 */
int
gate_collision(int x, int y, int w)
{
	int i;
	struct gate *g = gate;
	
	if (y < GAME_Y*8 || y >= GAME_Y*8 + GAME_HEIGHT*8) {
		return -1;
	}
	
	for (i = 0; i < GATE_MAX; i++, g++) {
		if (g->state == GATE_STATE_UNUSED) {
			continue;
		}
		
		if (x >= g->x - w && x < g->x + w + 16) {
			/* ほんとに当たってるか */
			if (x >= g->x && x < g->x + 16) {
				/* 上 */
				if (y >= GAME_Y*8 && y < GAME_Y*8 + g->au) {
					return -1;
				}
				/* 下 */
				if (y < GAME_Y*8 + GAME_HEIGHT*8 &&
					y >= GAME_Y*8 + GAME_HEIGHT*8 - g->ad) {
					return -1;
				}
			}
			
			/* 当たってないけど近い */
			return i+1;
		}
	}
	
	return 0;
}

/*
 * あるゲートに当たっているか
 */
int
gate_a_collision(int id, int x, int y)
{
	struct gate *g = &gate[id-1];

	if (x >= g->x && x < g->x + 16) {
		/* 上 */
		if (y >= GAME_Y*8 && y < GAME_Y*8 + g->au) {
			return GAME_Y*8 + g->au;
		}
		/* 下 */
		if (y <  GAME_Y*8 + GAME_HEIGHT*8 &&
			y >= GAME_Y*8 + GAME_HEIGHT*8 - g->ad) {
			return GAME_Y*8 + GAME_HEIGHT*8 - g->ad;
		}
	}
	return 0;
}
