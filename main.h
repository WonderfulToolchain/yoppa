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

extern unsigned short bmp_blank[];

/* ステージの長さ */
#define GAME_LENGTH 1008

/* ゲーム画面情報，キャラクタ単位 */
#define GAME_HEIGHT 13
#define GAME_WIDTH  28
#define GAME_X 0
#define GAME_Y 2

/* ゲームの状態 */
#define GAME_STATE_GAMEOVER 0
#define GAME_STATE_TITLE    1
#define GAME_STATE_GAME     2
#define GAME_STATE_DEMO     3
#define GAME_STATE_REPLAY   4
#define GAME_STATE_QUIT     255

extern int game_state;

/*ゲームモード */
#define GAME_MODE_NORMAL       0
#define GAME_MODE_SCOREATTACK  1
#define GAME_MODE_REPLAY       2

#define GAME_MODE_MAX          2

extern int game_mode;

/* キャラクタテーブル(ゲーム中用) */
#define NIKU_CHAR        256
#define STAGE_CHAR       257 /* 4個 */
#define SPARK_CHAR_SPARK 264
#define SPARK_CHAR_FLASH 265
#define SAKU_CHAR        266
#define YUKA_CHAR        267
#define NUM2_CHAR        272 /* 12個 */
#define BG_CHAR          288 /* 7*28個 */
#define NUM_CHAR         496 /* 13個 */
#define BLANK_CHAR       511

/* スプライトマップ(ゲーム中用) */
#define LUCUS_SP       0
#define SPARK_SP_SPARK 32
#define SAKU_SP        48
#define NIKU_SP        64
#define COUNTDOWN_SP   66 /* 4個 */
#define STAGE_SP       70 /* 'STAGExx'，6個 */

/* 使用するスプライトの数 */
#define USE_SPRITE_NUM 76
