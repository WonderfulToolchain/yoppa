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

struct statistics {
	int graze; /* かすった回数 */
	int fence; /* 柵を壊した数 */
	int walk;  /* 進んだ距離 */
	int mukae; /* 迎え酒飲む? */
	int seed;  /* 乱数の初期値 */
};
extern struct statistics game_statistics;
extern struct statistics stage_statistics;

/* 得点 */
#define FENCE_POINT  256
#define GRAZE_POINT   16
#define MUKAE_POINT 8000

void game_init(int seed);
int  game_frame(int key);
void game_create(int attack_stage);
long game_get_score();
int  game_get_stage();
void game_set_stage(int stage);
void game_stage_clear();
void game_next_stage();
long game_get_stage_score();
void game_add_score(long s);
int  game_get_seed();
