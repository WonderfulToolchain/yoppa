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

static int lcd_palette[8];

void
palette_init()
{
	unsigned long c;
	int i;
	
	c = lcd_get_color();
	for (i = 0; i < 8; i++) {
		lcd_palette[i] = c&15;
		c >>= 4;
	}
}

void
palette_fadein(int time)
{
	int i, j;
	unsigned long c = 0;
	
	for (i = time; i >= 0; i--) {
		for (j = 7; j >= 0; j--) {
			unsigned int e;
			e = (lcd_palette[j]*(time-i)) / time;
			c <<= 4;
			c |= e&15;
		}
		lcd_set_color(c&0xffff, (c>>16)&0xffff);
		sys_wait(0);
	}
}

void
palette_fadeout(int time)
{
	int i, j;
	unsigned long c = 0;
	
	for (i = 0; i <= time; i++) {
		for (j = 7; j >= 0; j--) {
			unsigned int e;
			e = (lcd_palette[j]*(time-i)) / time;
			c <<= 4;
			c |= e&15;
		}
		lcd_set_color(c&0xffff, (c>>16)&0xffff);
		sys_wait(0);
	}
}
