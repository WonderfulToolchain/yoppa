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
#include "game.h"
#include "title.h"
#include "gameover.h"
#include "demo.h"
#include "score.h"
#include "palette.h"
#include "replay.h"

#ifdef BMPSERVER
#include "bmpsaver.h"
#endif

int game_state; /* ゲームの状態 */
int game_mode;  /* ゲームモード */

int
vkey_press_check()
{
	int key = key_press_check();
	
	if (key & KEY_B) {
#ifdef BMPSERVER
		bs_sprite_set_range(0, USE_SPRITE_NUM);
		bs_save_bmp("ss", 0, 0, 224, 144);
#endif
	}
	if (key & KEY_START) {
		if (game_state != GAME_STATE_TITLE) {
			game_state = GAME_STATE_TITLE;
		} else {
			game_state = GAME_STATE_QUIT;
		}
	}
	
	key &= ‾KEY_START;
	
	return key;
}

void main(int argc, char *argv[])
{
	int key = 0;
	
	/* LCDパレットの保存 */
	palette_init();
	
	/* とりあえず消しとく */
	display_control(0);
	lcd_set_color(0, 0);
	
	/* 仮想画面初期化 */
	vscreen_init(1);
	vscreen_set_maxscroll(GAME_LENGTH, 0);
	
	/* ハイスコアの読み込み */
	score_load();
	
	/* グローバル変数初期化 */
	game_state = GAME_STATE_TITLE;
	game_mode = GAME_MODE_NORMAL;
	
	while (game_state != GAME_STATE_QUIT) {
		
		/* 状態によるふりわけ */
		switch (game_state) {
		case GAME_STATE_TITLE:
			title_init();
			while (game_state == GAME_STATE_TITLE) {
				key = vkey_press_check();
				
				/* 処理 */
				title_frame(key);
				
				/* 更新 */
				sys_wait(1);
				vscreen_display();
			}
			break;
		case GAME_STATE_GAME:
			vscreen_init(1);
			game_init(rand());
			
			/* リプレイデータの初期化，記録開始 */
			replay_new(game_get_stage(), game_get_seed());
			
			while (game_state == GAME_STATE_GAME) {
				key = vkey_press_check();
				
				/* リプレイデータ記録 */
				replay_set_key(key);
				
				/* 処理 */
				game_frame(key);
				
				/* 更新 */
				sys_wait(1);
				vscreen_display();
			}
			break;
		case GAME_STATE_REPLAY:
			/* リプレイデータの読み込み */
			if (replay_init() < 0) {
				/* リプレイデータが無い */
				game_state = GAME_STATE_TITLE;
				break;
			}
			
			/* リプレーモードへ */
			game_mode = GAME_MODE_REPLAY;
			
			/* ゲーム作成 */
			game_create(replay_get_stage());
			
			/* game_create()で上書きされるのでもう一度セット */
			game_state = GAME_STATE_REPLAY;
			
			vscreen_init(1);
			game_init(replay_get_seed());
			while (game_state == GAME_STATE_REPLAY) {
				/* ゲーム中断判定のため */
				vkey_press_check();
				
				/* リプレイ用キーを得る */
				key = replay_get_key();
				
				/* 処理 */
				game_frame(key);
				
				/* 更新 */
				sys_wait(1);
				vscreen_display();
			}
			break;
		case GAME_STATE_DEMO:
			demo_init();
			key = 0;
			while (game_state == GAME_STATE_DEMO) {
				/* 処理 */
				demo_frame(key);
				
				/* キーチェック */
				key = vkey_press_check();
				
				/* 更新 */
				sys_wait(1);
				/*vscreen_display();*/
			}
			demo_release();
			game_mode = GAME_MODE_NORMAL;
			break;
		case GAME_STATE_GAMEOVER:
			gameover_init();
			while (game_state == GAME_STATE_GAMEOVER) {
				key = vkey_press_check();
				
				/* 処理 */
				gameover_frame(key);
				
				/* 更新 */
				sys_wait(1);
				vscreen_display();
			}
			break;
		default:
			/* こないと思うけど */
			game_state = GAME_STATE_QUIT;
			break;
		}
	}
}
