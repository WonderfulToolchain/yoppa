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
#include <string.h>
#include <sys/bios.h>

#include "main.h"
#include "game.h"
#include "score.h"
#include "vscreen.h"
#include "title.h"
#include "palette.h"
#include "replay.h"

#include "title.bmh"

/* 得点表示用バッファ */
static char str[32];

static int wait;
static int attack_stage = 0;

void
title_init()
{
	int i;
	
	/* とりあえず消しとく */
	display_control(0);
	lcd_set_color(0, 0);
	
	/* 表示設定 */
	vscreen_init(1);
	text_set_screen(0);
	text_set_palette(0);
	text_set_mode(TEXT_MODE_ANK_SJIS);
	text_window_init(0, 0, 28, 4, 28*14);
	
	/* キャラクタ初期化 */
	font_set_colordata(0, 28*14, bmp_title);
	palette_set_color( 0, 0x7530); /* 背景用 */
	palette_set_color( 1, 0x7770); /* スコア表示用 */
	
	/* 文字フォント */
	
	
	/* 画面作成 */
	/*
	screen_fill_char(0, 0,  0, 32, 18, BLANK_CHAR);
	screen_fill_char(1, 0,  0, 32, 18, BLANK_CHAR|(4<<9));
	*/
	for (i = 0; i < 28*14; i++) {
		screen_fill_char(0, i%28, i/28+4, 1, 1, i);
	}
	
	/* 変数とか */
	wait = 1;
	
	/* メニュー表示 */
	title_put_mode();
	
	text_put_string(1, 1, "'A'-PLAY.");
	text_put_string(1, 3, "'START'-EXIT.");
	
	/* リプレイ可能？ */
	if (replay_init() >= 0) {
		text_put_string(1, 2, "'B'-REPLAY.");
	}
	
	/* フェードイン */
	display_control(DCM_SCR1);
	palette_fadein(20);

}

void
title_put_mode()
{
	int i;
	
	/* ハイスコア表示 */
	sprintf(str, "HI %07ld/%02d", score_get_hiscore(), score_get_histage()+1);
	text_put_string(0, 0, str);
	
	/* メニュー */
	text_put_string(15, 0, "[お品書き]");
	text_put_string(16, 1, "いつものやつ");
	sprintf(str, "限界に挑戦: %02d", attack_stage + 1);
	text_put_string(16, 2, str);
	if (score_get_stage_hiscore(attack_stage)) {
		sprintf(str, "HI %07ld", score_get_stage_hiscore(attack_stage));
		text_put_string(16, 3, str);
	} else {
		text_put_string(16, 3, "          ");
	}
	
	for (i = 0; i < GAME_MODE_MAX; i++) {
		if (i == game_mode) {
			text_put_string(15, 1 + i, "◆");
		} else {
			text_put_string(15, 1 + i, " ");
		}
	}
}


int
title_frame(int key)
{
	static int repeat;
	static int prevkey;
	int k;
	
	if (wait) {
		if (!key)
			wait = 0;
		return 0;
	}
	
	/* キーの状態を保存しとく */
	k = prevkey;
	prevkey = key;
	
	/* キーリピート判定 */
	if (key&(KEY_LEFT1|KEY_LEFT2|KEY_RIGHT1|KEY_RIGHT2)) {
		repeat++;
		if (repeat >= 32 && !(repeat%4)) {
			prevkey &= ‾(KEY_LEFT1|KEY_LEFT2|KEY_RIGHT1|KEY_RIGHT2);
		}
	} else {
		repeat = 0;
	}
	
	/* hit判定 */
	key = key & ‾k;
	
	/* ゲームモード選択(上下) */
	if (key&(KEY_UP1|KEY_UP2)) {
		game_mode = (game_mode + GAME_MODE_MAX - 1) % GAME_MODE_MAX;
	}
	if (key&(KEY_DOWN1|KEY_DOWN2)) {
		game_mode = (game_mode + 1) % GAME_MODE_MAX;
	}
	
	/* ステージ選択 */
	if (game_mode == GAME_MODE_SCOREATTACK) {
		if (key&(KEY_LEFT1|KEY_LEFT2)) {
			attack_stage = (attack_stage + 98) % 99;
		}
		if (key&(KEY_RIGHT1|KEY_RIGHT2)) {
			attack_stage = (attack_stage + 1) % 99;
		}
	}
	
	if (key&KEY_A) {
		game_create(attack_stage);
		palette_fadeout(30);
	}
	
	if (key&KEY_B) {
		game_state = GAME_STATE_REPLAY;
	}
	
	title_put_mode();
	
	return 0;
}
