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
#include <fcntl.h>
#include <sys/bios.h>

#define KEYMASK    KEY_A
#define MAXKEYDATA 4096
#define FILENAME   "/ram0/yoppa.rpl"

static struct key_data {
	unsigned short key;
	unsigned short time;
} key_data[MAXKEYDATA];
static int length;
static int play_loc;
static int play_time;
static int stage;
static int seed;

int
replay_init()
{
	int h;
	
	if ((h = open(FILENAME, FMODE_R, 0)) >= 0) {
		read(h, (char far *)&stage, sizeof(stage));
		read(h, (char far *)&seed, sizeof(seed));
		read(h, (char far *)&length, sizeof(length));
		read(h, (char far *)&key_data, sizeof(struct key_data) * length);
		close(h);
	} else {
		stage  = -1;
		seed   = 0;
		length = 0;
	}
	
	play_loc = 0;
	play_time = key_data[play_loc].time;
	
	return stage;
}

int
replay_get_stage()
{
	return stage;
}

int
replay_get_seed()
{
	return seed;
}

void
replay_new(int astage, int aseed)
{
	stage = astage;
	seed  = aseed;
	length = 0;
}

void
replay_save()
{
	int h;
	
	if ((h = open(FILENAME, FMODE_W, 0)) >= 0) {
		write(h, (char far *)&stage, sizeof(stage));
		write(h, (char far *)&seed, sizeof(seed));
		write(h, (char far *)&length, sizeof(length));
		write(h, (char far *)&key_data, sizeof(struct key_data) * length);
		close(h);
	}
}

unsigned short
replay_get_key()
{
	if (play_loc >= MAXKEYDATA) {
		return -1;
	}
	
	if (play_time <= 0) {
		play_loc++;
 		if (play_loc < MAXKEYDATA) {
	 		play_time = key_data[play_loc].time;
	 	} else {
			return -1;
		}
	}
	play_time--;
	
	return key_data[play_loc].key;
}


unsigned short
replay_set_key(unsigned short key)
{
	unsigned short masked = key & KEYMASK;
	
	if (length && key_data[length-1].key == masked) {
		key_data[length-1].time++;
	} else if (length < MAXKEYDATA) {
		key_data[length].key = masked;
		key_data[length].time = 1;
		length++;
	}
	
	return key;
}
