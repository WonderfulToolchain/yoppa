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
#include <fcntl.h>

#include "main.h"

#define FILENAME "/ram0/yoppa.scr"

static long hiscore = 0;
static int histage = 0;
static long stage_hiscore[99];

void
score_load()
{
	int h, i;
	
	hiscore = 0;
	histage = 0;
	for (i = 0; i < 99; i++) {
		stage_hiscore[i] = 0;
	}
	
	if ((h = open(FILENAME, FMODE_R, 0)) >= 0) {
		read(h, (char far *)&hiscore, sizeof(hiscore));
		read(h, (char far *)&histage, sizeof(histage));
		for (i = 0; i < 99; i++) {
			read(h, (char far *)&stage_hiscore[i], sizeof(long));
		}
		close(h);
	}
}

void
score_save()
{
	int h, i;
	
	if ((h = open(FILENAME, FMODE_W, 0)) >= 0) {
		write(h, (char far *)&hiscore, sizeof(hiscore));
		write(h, (char far *)&histage, sizeof(histage));
		for (i = 0; i < 99; i++) {
			write(h, (char far *)&stage_hiscore[i], sizeof(long));
		}
		close(h);
	}
}

int
score_set_stage(long score, int stage)
{
	if (stage >= 0 && stage < 99 &&
		score > stage_hiscore[stage]) {
		
		stage_hiscore[stage] = score;
		score_save();
		
		return 1;
	}
	
	return 0;
}

int
score_set(long score, int stage)
{
	if (score > hiscore) {
		
		hiscore = score;
		histage = stage;
		score_save();
		
		return 1;
	}
	
	return 0;
}

long
score_get_hiscore()
{
	return hiscore;
}

int
score_get_histage()
{
	return histage;
}

long
score_get_stage_hiscore(int stage)
{
	if (stage >= 0 && stage < 99) {
		return stage_hiscore[stage];
	}
	
	return 0;
}

