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
#include <string.h>

#include "main.h"
#include "score.h"
#include "game.h"
#include "palette.h"

#include "nomisugi.bmh"

static int wait;
static char str[32];

void
gameover_init()
{
	int i;
	
	/* とりあえず消しとく */
	display_control(0);
	screen_fill_char(0, 0, 0, 32, 32, BLANK_CHAR);
	
	/* テキスト画面 */
	text_set_screen(0);
	text_set_palette(0);
	text_set_mode(TEXT_MODE_ANK_SJIS);
	text_window_init(10, 0, 18, 18, 8*10);
	
	/* キャラクタ設定 */
	font_set_colordata(0, 10*8, bmp_nomisugi);
	palette_set_color(0, 0x7530);
	
	for (i = 0; i < 8*10; i++) {
		screen_fill_char(0, 2+(i%8), 4+i/8, 1, 1, i);
	}
	
	/* テキスト描画 */
	
	text_put_string((18-8)/2, 2, "GAMEOVER");
	text_put_string((18-4)/2, 5, "支払金額");
	sprintf(str, "%07ld/%02d", game_get_score(), game_get_stage()+1);
	text_put_string((18-strlen(str))/2, 7, str);
	
	/* ハイスコア登録 */
	if (game_mode == GAME_MODE_NORMAL) {
		
		if (score_set(game_get_score(), game_get_stage())) {
			text_put_string((18-4)/2, 9, "↓更新！");
		}
	}
	/* ステージハイスコア？(ゲームクリアしないと登録しないことにした) */
	/*
	if (score_set_stage(game_get_stage_score(), game_get_stage())) {
	}
	*/
	
	text_put_string((18-6)/2, 11, "最高支払金額");
	switch (game_mode) {
	default:
	case GAME_MODE_NORMAL:
		sprintf(str, "%07ld/%02d", score_get_hiscore(), score_get_histage()+1);
		break;
	case GAME_MODE_SCOREATTACK:
	case GAME_MODE_REPLAY:
		sprintf(str, "%07ld/%02d",
			score_get_stage_hiscore(game_get_stage()),
			game_get_stage()+1);
		break;
	}
	
	text_put_string((18-strlen(str))/2, 13, str);
	
	
	text_put_string((18-12)/2, 17, "お酒は二十歳になってから。");
	
	/* 変数とか */
	
	wait = 1;
	
	/* フェードイン */
	
	display_control(DCM_SCR1);
	palette_fadein(32);
}

int
gameover_frame(int key)
{
	if (wait) {
		if (!key)
			wait = 0;
	} else if (key) {
		game_state = GAME_STATE_TITLE;
		palette_fadeout(20);
	}
}

