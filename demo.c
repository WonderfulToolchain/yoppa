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
#include "score.h"
#include "palette.h"
#include "replay.h"

#include "mukae.bmh"

static int wait, phase;
static long tax, tax_tmp, total;
static char str[64];

/* 割り込みコールバック */
void far hblank_callback();

static int hblank_line; /* 今どこを描いているのか */
static intvector_t hb_old; /* 古いベクタ */
static int scrollx; /* スクロールさせる部分のx位置 */

void
demo_init()
{
	/* 割り込み設定用ワーク */
	static intvector_t v;
	static void (far *callback)();
	
	/* その他 */
	long point, magni;
	int y, i;
	
	/* リプレイデータを保存 */
	if (game_mode != GAME_MODE_REPLAY) {
		replay_save();
	}
	
	/* 表示設定 */
	display_control(0);
	vscreen_init(1);
	
	/* 迎え酒飲むステージなら，キャラクタ読み込み，表示 */
	if (stage_statistics.mukae) {
		font_set_colordata(496, 9, bmp_mukae);
		for (i = 0; i < 9; i++) {
			vscreen_fill_char_c(25 + (i%3), GAME_Y+GAME_HEIGHT-3 + i/3, 
								1, 1, 496+i);
		}
	}
	
	/* お酒は二十歳になってから */
	screen_fill_char(0, 0, 17, 28, 1, BLANK_CHAR);
	text_set_screen(0);
	text_set_palette(1);
	text_set_mode(TEXT_MODE_ANK_SJIS);
	text_window_init(8, 17, 12, 1, 484);
	text_put_string(0, 0, "お酒は二十歳になってから");
	
	/* initialize each subsystem */
	locus_init();
	
	/* 画面作成 */
	
	/* テキスト画面 */
	text_set_screen(1);
	text_set_palette(12);
	text_set_mode(TEXT_MODE_ANK_SJIS);
	text_window_init(0, 3, 28, 9, 0);
	
	/* テキスト(請求書)表示 */
	y = 0;
	total = 0;
	tax = 0;
	
	/* 請求書の表示 */
	text_put_string(1, 0, "請求書："); 
	
	/* 移動して疲れてストレスが溜まって飲んだ分 */
	magni = game_get_stage();
	point = stage_statistics.walk * magni;
	total += point;
	sprintf(str, " 移動分：%4dx%7ld=%7ld円",
				stage_statistics.walk, magni, point);
	text_put_string(1, 1, str); 
	
	/* 火花が散ってストレスが溜まって飲んだ分 */
	magni = game_get_stage() * (long)GRAZE_POINT;
	point = stage_statistics.graze * magni;
	total += point;
	sprintf(str, "　火花分：%4dx%7ld=%7ld円",
				stage_statistics.graze, magni, point);
	text_put_string(1, 2, str); 
	
	/* 柵修繕費 */
	magni = game_get_stage() * (long)FENCE_POINT;
	point = stage_statistics.fence * magni;
	total += point;
	sprintf(str, "柵修繕費：%4dx%7ld=%7ld円",
				stage_statistics.fence, magni, point);
	text_put_string(1, 3, str); 
	
	/* 酔いが回ってきて，迎え酒を飲んだ代金 */
	if (stage_statistics.mukae) {
		magni = (long)MUKAE_POINT;
		point = game_statistics.mukae * magni;
		total += point;
		game_add_score(point);
		sprintf(str, "　迎酒代：%4dx%7ld=%7ld円",
					game_statistics.mukae, magni, point);
		text_put_string(1, 4, str); 
	}
	
	/* 代金合計 */
	sprintf(str, "　　　　　　　　　　　　代金合計：　%7ld円", total);
	text_put_string(1, 5, str); 
	
	/* 消費税(笑) */
	tax = total*5/100;
	sprintf(str, "　　　　　　　　　　　　　消費税：　%7d円", 0);
	text_put_string(1, 6, str); 
	
	/* 区切り線 */
	text_put_string(1, 7, 
		"　　　　　　　　　　　　　―――――――――――――");
	
	/* 合計 */
	sprintf(str, "　　　　　　　　　　　　　　合計：　%7ld円", total);
	text_put_string(1, 8, str);
	
	if (total != game_get_stage_score()) {
		text_put_string(10, 0, "スコア合計が変");
	}
	
	/* フェードイン */
	display_control(DCM_SCR1|DCM_SCR2|DCM_SPR);
	vscreen_set_scroll(0, 0);
	vscreen_display();
	palette_fadein(25);
	sys_wait(50);
	
	/* その他変数初期化 */
	tax_tmp = 0;
	wait = 50;
	phase = 0;
	
	/* ラスタ割り込み設定 */
	
	hblank_line = 0;
	scrollx = 0; 
	
	#define _CS _asm_inline("¥tmov¥tax,cs");
	#define _DS _asm_inline("¥tmov¥tax,ds");
	
	callback = hblank_callback;
	v.cs = _CS;
	v.ds = _DS;
	v.callback = (void (near *)())FP_OFF(callback);
	sys_interrupt_set_hook(SYS_INT_HBLANK_COUNTUP, &v, &hb_old);
	timer_enable(TIMER_HBLANK, TIMER_AUTOPRESET, 1);
}

void
demo_release()
{
	sys_interrupt_reset_hook(SYS_INT_HBLANK_COUNTUP, &hb_old);
	timer_disable(TIMER_HBLANK);
}

void far hblank_callback()
{
	if (hblank_line == 4*8+1 || hblank_line == 18*8+1) {
		screen_set_scroll(1, 0, 0);
	} else if (hblank_line == 13*8+1 || hblank_line == 0) {
		screen_set_scroll(1, scrollx>>2, 0);
	}
	hblank_line++;
}

int
demo_frame(int key)
{
	hblank_line = 0;
	/* ウエイト用 */
	wait--;
	
	/* スクロール位置調整，更新 */
	scrollx = (scrollx + 1) & 1023;
	
	/* スコア(消費税分)カウント */
	if (!wait && (tax || tax_tmp)) {
		long score;
		wait = 10; /* 次の計算までの時間 */
		
		switch (phase) {
		case 0:
			for (score = 100000; score > 0; score /= 10) {
				if (tax >= score) {
					tax_tmp += score;
					tax -= score;
					break;
				}
			}
			if (!tax) {
				phase++;
			}
			break;
		default:
			phase++;
			break;
		case 10:
			for (score = 100000; score > 0; score /= 10) {
				if (tax_tmp >= score) {
					total += score;
					tax_tmp -= score;
					game_add_score(score);
					break;
				}
			}
			break;
		}
		
		sprintf(str, "　　　　　　　　　　　　　消費税：　%7d円", tax_tmp);
		text_put_string(1, 6, str);
		sprintf(str, "　　　　　　　　　　　　　　合計：　%7ld円", total);
		text_put_string(1, 8, str);
		
		/* ハイスコアかな */
		if (total > score_get_stage_hiscore(game_get_stage() - 1)) {
			text_put_string(1, 6, "飲みすぎ赤信号!!");
		}
	}
	
	
	if (key) {
		/* 残ってたら足す */
		game_add_score(tax + tax_tmp);
		total += tax + tax_tmp;
		
		/* ステージハイスコア？(ゲームのときのみ) */
		if (game_mode != GAME_MODE_REPLAY) {
			score_set_stage(total, game_get_stage() - 1);
		}
		
		palette_fadeout(25);
		switch (game_mode) {
		case GAME_MODE_NORMAL:
		default:
			game_state = GAME_STATE_GAME;
			break;
		case GAME_MODE_SCOREATTACK:
		case GAME_MODE_REPLAY:
			/* 次のステージになっているので戻す */
			game_set_stage(game_get_stage()-1); 
			game_state = GAME_STATE_GAMEOVER;
			break;
		}
	}
}

