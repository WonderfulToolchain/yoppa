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

/*
 * まずvscreen_set_scroll()してから
 * 他の処理を行うのが正しい手順
 */

void vscreen_init(int d);
void vscreen_clear();
void vscreen_set_maxscroll(int x, int y);
void vscreen_set_scroll(int x, int y);
int vscreen_get_scrollx();
int vscreen_get_scrolly();
void vscreen_set_char(int x, int y, unsigned short n);
unsigned short vscreen_get_char(int x, int y);
void vscreen_fill_char_c(int x, int y, int w, int h, unsigned short n);
void vsprite_set_char_location(int n, int x, int y, unsigned short attr);
void vsprite_disable(int n);
void vscreen_display();

/*
 * *_c() : 座標の単位はキャラクタ
 */
