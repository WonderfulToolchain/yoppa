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
#include "game.h"
#include "locus.h"
#include "vscreen.h"
#include "gate.h"
#include "spark.h"
#include "score.h"
#include "saku.h"
#include "palette.h"

#include "niku.bmh"
#include "bg.bmh"
#include "num2.bmh"
#include "stage.bmh"

#define PALETTE 12
#define PALBITSP ((PALETTE-8)<<9)

/* ステータス */
static int x, y, v, xv, a, scmid;
static int stage = 0, level;
static long score = 0, start_score;
static int life; /* 後何回失敗できるか */

/* いろいろな集計 */
struct statistics game_statistics;
struct statistics stage_statistics;

/* 得点表示用バッファ */
static unsigned short str[32];

/* 軌跡遅延バッファ */
#define LOCUSBUF_MAX 8
struct {
	int x, y;
} locusbuf[LOCUSBUF_MAX];
int locusloc;

/* カウントダウンカウンタ */
static int countdown;

/*
 * ゲーム開始
 * ゲーム画面用の環境整備
 * ステージ作成
 * seed : 乱数初期化のためのシード
 */
void
game_init(int seed)
{
	int i, j;
	long tmpl;
	
	/* 表示設定 */
	display_control(0);
	vscreen_init(1);
	
	/* キャラクタ初期化 */
	font_set_colordata(NIKU_CHAR, 1, bmp_niku);
	font_set_colordata(BG_CHAR, 7*28, bmp_bg);
	font_set_colordata(NUM2_CHAR, 12, bmp_num2);
	font_set_colordata(STAGE_CHAR, 4, bmp_stage);
	
	/* パレット設定 */
	palette_set_color( 0, 0x4320); /* 背景用 */
	palette_set_color( 1, 0x7770); /* スコア表示用 */
	
	/* initialize each subsystem */
	locus_init();
	gate_init();
	spark_init();
	saku_init();
	
	/* 画面作成 */
	screen_fill_char(0, 0,  0, 32, 18, BLANK_CHAR);
	for (i = 0; i < 28*7; i++) {
		screen_fill_char(0, i%28, i/28+GAME_Y+GAME_HEIGHT-7, 1, 1,
							BG_CHAR+i);
	}
	
	/* ハイスコア表示 */
	switch (game_mode) {
	case GAME_MODE_NORMAL:
		tmpl = score_get_hiscore();
		j = score_get_histage() + 1;
		break;
	case GAME_MODE_SCOREATTACK:
	case GAME_MODE_REPLAY:
		tmpl = score_get_stage_hiscore(stage);
		j = stage + 1;
		break;
	default:
		tmpl = j = 0;
		break;
	}
	str[0] = (NUM_CHAR+11)|(1<<9);
	for (i = 7; i >= 1; i--) {
		str[i] = ((tmpl%10)+NUM_CHAR)|(1<<9);
		tmpl /= 10;
	}
	str[8] = (NUM_CHAR+10)|(1<<9);
	for (i = 10; i >= 9; i--) {
		str[i] = ((j%10)+NUM_CHAR)|(1<<9);
		j /= 10;
	}
	screen_set_char(0, 1, 17, 11, 1, str);
	
	/* ステージ作成 */
	gate_create(100, 900, level, seed);
	
	/* 変数とか: 肉君ステータス */
	x = 8<<4;
	y = (GAME_Y*8+GAME_HEIGHT*8/2-4)<<6;
	v = 0;
	
	/* 変数とか: カウントダウン，軌跡遅延バッファ */
	countdown = 50*3;
	start_score = score;
	locusloc = 0;
	for (i = 0; i < LOCUSBUF_MAX; i++) {
		locusbuf[i].x = -1;
	}
	
	/* 変数とか: ステージ情報 */
	stage_statistics.graze = 0;
	stage_statistics.walk  = 0;
	stage_statistics.fence = 0;
	stage_statistics.mukae = 0;
	stage_statistics.seed  = seed;
	
	/* 初期画面表示 */
	vscreen_set_scroll(0, 0);
	game_frame(0);
	vscreen_display();
	
	/* フェードイン */
	lcd_set_color(0, 0);
	display_control(DCM_SCR1|DCM_SCR2|DCM_SPR);
	palette_fadein(30);
}

/*
 * 新しいゲームを始める
 */
void
game_create(int attack_stage)
{
	/* 乱数初期化 */
	srand(sys_get_tick_count());
	
	/* 座標，速度，加速度 */
	x = 8<<4;
	xv = 8;
	y = (GAME_Y*8+GAME_HEIGHT*8/2-4)<<6;
	v = 0;
	a = 2;
	
	/* 2回までぶつかっても良いよ('いつもの'モードのみ) */
	if (game_mode == GAME_MODE_NORMAL) {
		life = 2;
	} else {
		life = 0;
	}
	
	/* ゲート隙間の大きさ */
	level = 30;

	/* ステージとスコア */
	stage = 0;
	score = 0;
	scmid = 100;
	
	game_statistics.graze = 0;
	game_statistics.walk  = 0;
	game_statistics.fence = 0;
	game_statistics.mukae = 0;
	game_statistics.seed  = 0;
	
	/* ゲームモードへ */
	game_state = GAME_STATE_GAME;
	
	/* スコアアタックモードかリプレイモードならステージの難易度を求める */
	if (game_mode == GAME_MODE_SCOREATTACK ||
		game_mode == GAME_MODE_REPLAY) {
		
		for (;attack_stage > 0; attack_stage--)
			game_next_stage();
	}
}

void
game_next_stage()
{
	level -= 2;
	if (level < 20) {
		level = 34;
		xv += 2;
		a++;
		scmid -= 16;
		if (scmid < 8)
			scmid = 8;
		stage_statistics.mukae = 1;
		game_statistics.mukae ++;
	}
	stage++;
	
	game_statistics.graze += stage_statistics.graze;
	game_statistics.walk  += stage_statistics.walk;
	game_statistics.fence += stage_statistics.fence;
}

/*
 * ステージクリア
 */
void
game_stage_clear()
{
	/* パラメータ計算 */
	game_next_stage();

	/* ステージ間デモへ */
	game_state = GAME_STATE_DEMO;
}

long
game_get_score()
{
	return score;
}

void
game_set_stage(int s)
{
	stage = s;
}

void
game_add_score(long s)
{
	score += s;
	if (score > 9999999)
		score = 9999999;
}

long
game_get_stage_score()
{
	return score - start_score;
}

int
game_get_stage()
{
	return stage;
}

int
game_get_seed()
{
	return stage_statistics.seed;
}

int
game_frame(int key)
{
	int i, j;
	int xx, yy;
	long tmpl;
	
	/* カウントダウン？ */
	if (countdown > 0) {
		countdown--;
		if (countdown) {
			xx = 48;
			yy = GAME_Y*8+GAME_HEIGHT*8/2-8;
			j = (NUM2_CHAR + (countdown/50)*2)|PALBITSP|CFM_SPR_UPPER;
			vsprite_set_char_location(COUNTDOWN_SP  , xx  , yy  , j  );
			vsprite_set_char_location(COUNTDOWN_SP+1, xx+8, yy  , j+1);
			vsprite_set_char_location(COUNTDOWN_SP+2, xx  , yy+8, j+6);
			vsprite_set_char_location(COUNTDOWN_SP+3, xx+8, yy+8, j+7 );
			
			/* 'STAGE' */
			j = STAGE_CHAR|PALBITSP|CFM_SPR_UPPER;
			for (i = 0; i < 4; i++)
				vsprite_set_char_location(STAGE_SP+i, xx-16+i*8, yy-16, j+i);
			
			/* ステージ数 */
			vsprite_set_char_location(STAGE_SP+i, xx-16+i*8+8, yy-16, 
				(NUM_CHAR+((stage+1)/10))|PALBITSP|CFM_SPR_UPPER); /* 十の位 */
			i++;
			vsprite_set_char_location(STAGE_SP+i, xx-16+i*8+8, yy-16, 
				(NUM_CHAR+((stage+1)%10))|PALBITSP|CFM_SPR_UPPER); /* 一の位 */
		} else {
			for (i = 0; i < 4; i++)
				vsprite_disable(COUNTDOWN_SP+i);
			for (i = 0; i < 6; i++)
				vsprite_disable(STAGE_SP+i);
		}
	} else {
		/* 移動 */
		y += v;
		if (key & KEY_A) {
			v -= a;
		} else {
			v += a;
		}
		x += xv;

		/* 定常スコア */
		game_add_score(stage+1);
		stage_statistics.walk++;
	}
	
	/* スコア表示 */
	for (i = 6, tmpl = score; i >= 0; i--) {
		str[i] = ((tmpl%10)+NUM_CHAR)|(1<<9);
		tmpl /= 10;
	}
	str[7] = (NUM_CHAR+10)|(1<<9);
	for (i = 9, j = stage+1; i >= 8; i--) {
		str[i] = ((j%10)+NUM_CHAR)|(1<<9);
		j /= 10;
	}
	screen_set_char(0, 15, 17, 10, 1, str);
	
	/* 残りライフ表示 */
	for (i = 0; i < life; i++)
		screen_fill_char(0, 26+i, 17, 1, 1, (NUM_CHAR+12)|(1<<9));
	
	/* スクリーン座標を得る */
	xx = x >> 4;
	yy = y >> 6;
	
	/* クリア判定 */
	if (xx >= GAME_LENGTH) {
		game_stage_clear();
		palette_fadeout(75);
		return 0;
	}
	
	/* サブシステム処理 */
	vscreen_set_scroll(xx - scmid, 0);
	gate_frame();
	spark_frame();
	saku_frame();
	
	/* あたり判定 */
	i = gate_collision(xx, yy, 3);
	if (i < 0) {
		/* ちょっとウェイトを入れる */
		sys_wait(35);
		palette_fadeout(40);
		
		/* ライフを減らす.0以下ならゲームオーバー */
		if (--life < 0) {
			game_state = GAME_STATE_GAMEOVER;
		} else {
			/* 同じシードでもう１度同じステージをやり直し */
			game_init(stage_statistics.seed);
		}
		return 0;
	}
	
	/* かすり判定 */
	if (i >= 1) {
		if ((j = gate_a_collision(i, xx-2, yy-4)) > 0 ||
			(j = gate_a_collision(i, xx+2, yy-4)) > 0) {
			/* 上側がかすってる */
			spark_hit(xx, j-2, SPARK_UPPER);
			game_add_score(16*(stage+1));
			stage_statistics.graze++;
		}
		if ((j = gate_a_collision(i, xx-2, yy+5)) > 0 ||
			(j = gate_a_collision(i, xx+2, yy+5)) > 0) {
			/* 下側がかすってる */
			spark_hit(xx, j+2, SPARK_LOWER);
			game_add_score(GRAZE_POINT*(stage+1));
			stage_statistics.graze++;
		}
	}
	
	/* 柵に当たってる？ */
	if ((vscreen_get_char(xx, yy)&511) == SAKU_CHAR) {
		saku_add(xx, yy);
		game_add_score(FENCE_POINT*(stage+1));
		vscreen_set_char(xx, yy, BLANK_CHAR);
		stage_statistics.fence++;
	}
	
	/* 軌跡の描画 */
	if (locusbuf[locusloc].x >= 0) {
		locus_point(locusbuf[locusloc].x, locusbuf[locusloc].y, 3);
	}
	locusbuf[locusloc].x = xx;
	locusbuf[locusloc].y = yy;
	locusloc = (locusloc + 1) % LOCUSBUF_MAX;
	
	/* 表示 */
	vsprite_set_char_location(NIKU_SP, xx-4, yy-3, NIKU_CHAR|PALBITSP);
	
	return 0;
}
