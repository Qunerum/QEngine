/*
 * Copyright (c) 2026 Qunerum
 *
 * This file is part of QGPU.
 * This software is licensed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define QGPU_COLORS
#include "qgpu.h"

#define QGPU_VERSION_MAJOR 2
#define QGPU_VERSION_MINOR 6
#define QGPU_VERSION_PATCH 0
// ========================================================================================================================================================================
// ===== QFONT.h ==========================================================================================================================================================
// ========================================================================================================================================================================
#define QFONT_REGULAR       0b00000000
#define QFONT_BOLD          0b00000001
#define QFONT_ITALIC        0b00000010
#define QFONT_UNDERLINE     0b00000100
#define QFONT_STRIKETHROUGH 0b00001000
#define QFONT_UPPERCASE     0b00010000
#define QFONT_LOWERCASE     0b00100000

#define QFONT_Y_UP 0
#define QFONT_Y_DOWN 1

typedef struct { uint8_t sizeX, sizeY; } qfCharData;
// ========================================================================================================================================================================
// ===== QFONT.c ==========================================================================================================================================================
// ========================================================================================================================================================================
#define NKO "\033[1;38;5;240m[\033[1;38;5;"
#define NKC "\033[1;38;5;240m]\033[1;38;5;251m"
#define QFONT_PREFIX NKO "211mQFont" NKC
static float
	pivX = 0.0f, pivY = 0.0f,
	qfSize = 32.0f,
	qfR = 1.0f, qfG = 1.0f, qfB = 1.0f, qfA = 1.0f;
static uint8_t qfStyle = QFONT_REGULAR, qfAxis = QFONT_Y_UP;
static void (*drawFunc)(const float posX, const float posY, const float sizeX, const float sizeY, const float r, const float g, const float b, const float a);
void qfSetDrawRect(const int8_t px, const int8_t py, const uint8_t ya, void (*drawRectFunc)(const float posX, const float posY, const float sizeX, const float sizeY, const float r, const float g, const float b, const float a)) {
	pivX = px <= -1 ? 0.0f : px >= 1 ? 1.0f : 0.5f;
	pivY = py <= -1 ? 0.0f : py >= 1 ? 1.0f : 0.5f;
	qfAxis = ya;
	drawFunc = drawRectFunc;
}
void qfConfigure(const float fontSize, const uint8_t style, const float r, const float g, const float b, const float a) {
	qfSize = fontSize;
	qfStyle = style;
	qfR = r;
	qfG = g;
	qfB = b;
	qfA = a;
}
static void* mc(void* dest, const void* src, const uint32_t n) {
	unsigned char* d = (unsigned char*)dest;
	const unsigned char* s = (const unsigned char*)src;
	for (uint32_t i = 0; i < n; i++) d[i] = s[i];
	return dest;
}
qfCharData qfCreate(const uint8_t sizeX, const uint8_t sizeY) { return (qfCharData) { .sizeX = sizeX, .sizeY = sizeY }; }
static int8_t** allChars = 0, isFile = 0;
static uint32_t charCount = 0, highestID = 0;
#define u8d (uint8_t)d
void qfAddChar(const qfCharData charData, const int8_t* d) {
	if (!d) return;
	const uint32_t id =
		((uint32_t)u8d[0] << 24) |
		((uint32_t)u8d[1] << 16) |
		((uint32_t)u8d[2] << 8)  |
		 (uint32_t)u8d[3];
	if (id >= highestID) {
		const uint32_t newCapacity = id + 1;
		int8_t** temp = (int8_t**)realloc(allChars, newCapacity * sizeof(int8_t*));
		if (!temp) return;
		allChars = temp;
		for (uint32_t i = highestID; i < newCapacity; i++) allChars[i] = 0;
		highestID = newCapacity;
	}
	if (allChars[id]) free(allChars[id]);
	const uint8_t goNext = u8d[5];
	uint16_t size = 4, actualID = 6;
	for (uint8_t i = 0; i < goNext; i++) {
		const uint8_t len = 1 + u8d[actualID];
		actualID += len;
		size += len;
	}
	allChars[id] = (int8_t*)malloc(size);
	mc(allChars[id], d + 2, size);
	allChars[id][0] = charData.sizeX;
	allChars[id][1] = charData.sizeY;
	charCount++;
	const uint8_t c = isFile ? 12 : 245;
	printf(QFONT_PREFIX "%c" NKO "245m%4u" NKC
		" %s " NKO "%um%c" NKC
		": WH:" NKO "%um%ux%u" NKC
		" ID:" NKO "%um0x%08X" NKC
		" LS:" NKO "%um%4uB" NKC "\n",
		isFile ? 0 : ' ', charCount, isFile ? "Readed char" : "Added char", c, (char)(id > 31 && id < 127 ? id : '?'), c, charData.sizeX, charData.sizeY, c, id, c, size);
}
void qfWriteFile(const uint8_t add, const char* path, const qfCharData charData, const int8_t* d) {
	if (!d) return;
	FILE* f = fopen(path, add ? "ab" : "wb");
	if (!f) return;
	const uint32_t id =
	(((uint32_t)u8d[0] << 24)) |
	(((uint32_t)u8d[1] << 16)) |
	(((uint32_t)u8d[2] << 8))  |
	((uint32_t)u8d[3]);
	const uint8_t startAt = u8d[4], goNext = u8d[5];
	uint16_t l = 0, actualID = 6;
	for (uint8_t i = 0; i < goNext; i++) {
		const uint8_t len = 1 + u8d[actualID];
		actualID += len;
		l += len;
	}
	fwrite(&d[0], 1, 1, f);
	fwrite(&d[1], 1, 1, f);
	fwrite(&d[2], 1, 1, f);
	fwrite(&d[3], 1, 1, f);
	fwrite(&charData.sizeX, 1, 1, f);
	fwrite(&charData.sizeY, 1, 1, f);
	fwrite(&startAt, 1, 1, f);
	fwrite(&goNext, 1, 1, f);
	fwrite(&d[6], 1, l, f);
	fclose(f);
	const uint16_t totalRecordSize = 6 + l;
	printf(QFONT_PREFIX
	"   Saved to file   " NKO "115m%c" NKC
	": WH:" NKO "115m%ux%u" NKC
	" ID:" NKO "115m0x%08X" NKC
	" LS:" NKO "115m%4uB" NKC "\n",
	(char)(id > 31 && id < 127 ? id : '?'), charData.sizeX, charData.sizeY, id, totalRecordSize);
}
void qfReadFile(const char* path) {
	FILE* f = fopen(path, "rb");
	if (!f) return;
	isFile = 1;
	uint32_t id = 0;
	while (fread(&id, sizeof(uint32_t), 1, f) == 1) {
		uint8_t sizeX = 0, sizeY = 0, startAt = 0, goNext = 0;
		if (fread(&sizeX, 1, 1, f) != 1 ||
			fread(&sizeY, 1, 1, f) != 1 ||
			fread(&startAt, 1, 1, f) != 1 ||
			fread(&goNext, 1, 1, f) != 1) break;
		const qfCharData cd = (qfCharData){ .sizeX = sizeX, .sizeY = sizeY };
		uint16_t size = 6;
		for (uint8_t i = 0; i < goNext; i++) {
			uint8_t v = 0;
			if (fread(&v, 1, 1, f) != 1) break;
			fseek(f, (long)v, SEEK_CUR);
			size += (1 + v);
		}
		int8_t* d = (int8_t*)malloc(size);
		if (!d) {
			fclose(f);
			return;
		}
		fseek(f, -(long)(size + 2), SEEK_CUR);
		d[3] = (int8_t)(id >> 24);
		d[2] = (int8_t)(id >> 16);
		d[1] = (int8_t)(id >> 8);
		d[0] = (int8_t)id;
		fseek(f, 6, SEEK_CUR);
		fread(&d[4], 1, size - 4, f);
		qfAddChar(cd, d);
	}
	isFile = 0;
	fclose(f);
}
float qfDrawChar(const float posX, const float posY, const uint32_t c) {
	if (c >= highestID || !allChars[c]) return .0f;
	const uint32_t cr =
		((qfStyle & QFONT_UPPERCASE) && c >= 'a' && c <= 'z') ? c - 32 :
		((qfStyle & QFONT_LOWERCASE) && c >= 'A' && c <= 'Z') ? c + 32 : c;
	const int8_t* d = allChars[cr];
	const uint8_t
		w = u8d[0], h = u8d[1],
		s = u8d[2], rn = u8d[3],
		n = s + rn > h ? h - s : rn,
		fw = (qfStyle & QFONT_BOLD) ? w + 1 : w;
	if (s >= h) return .0f;
	uint8_t x = 0, y = s, i = 4;
	const float sc = qfSize / h;
	for (uint8_t r = 0; r < n; r++) {
		const uint8_t len = u8d[i] + 1;
		const float st = (qfStyle & QFONT_ITALIC) ? -(y / (float)h) * 3.0f : 0.0f;
		for (uint8_t l = 1; l < len; l++) {
			const int8_t dil = d[i + l];
			const uint8_t
				draw = dil < 0,
				rv = draw ? -dil : dil,
				v = rv > w ? w : rv,
				dv = (qfStyle & QFONT_BOLD) && draw ? v + 1 : v;
			const float
				px = qfAxis ? (x + v * pivX - st) : (x + v * pivX + st),
				pyt = (y + pivY) * sc,
				py = qfAxis ? posY + pyt : posY - pyt;
			if (draw) drawFunc(posX + px * sc, py, dv * sc, sc, qfR,qfG,qfB,qfA);
			x += v;
		}
		i += len;
		x = 0;
		y++;
	}
	if (qfStyle & QFONT_UNDERLINE) drawFunc(posX + (fw * sc * 0.5f), posY - qfSize, fw * sc, sc, qfR,qfG,qfB,qfA);
	if (qfStyle & QFONT_STRIKETHROUGH) drawFunc(posX + (fw * sc * 0.5f), posY - qfSize * 0.5f, fw * sc, sc, qfR,qfG,qfB,qfA);
	return fw * sc;
}
static uint8_t tmpi = 0;
static char tmp[11];
void qfDrawText(const float posX, const float posY, const char* t) {
	float x = posX, y = posY;
	while (*t) {
		if (*t == '\n') {
			x = posX;
			y -= qfSize;
			t++;
			continue;
		}
		if (*t == '\33') {
			t++;
			const uint8_t type = *t == 'd' ? 1 : *t == 'x' ? 2 : 0;
			if (!type || !*t) continue;
			tmpi = 0;
			tmp[0] = 0;
			for (uint8_t i = 0; i < sizeof(tmp) - 1; i++) {
				t++;
				if (!*t) return;
				if (*t != 'q') tmp[tmpi++] = *t; else break;
			}
			uint32_t res = 0;
			uint8_t i = 0;
			if (type == 1) {
				while (tmp[i] >= '0' && tmp[i] <= '9') {
					res = res * 10 + tmp[i] - '0';
					i++;
				}
			} else {
				for (i = 0; i < tmpi; i++) if (tmp[i] >= 'A' && tmp[i] <= 'F') tmp[i] += 32;
				i = 0;
				while (i < tmpi) {
					const char c = tmp[i];
					uint32_t val = 0;
					if (c >= '0' && c <= '9') val = c - '0';
					else if (c >= 'a' && c <= 'f') val = c - 'a' + 10;
					else break;
					res = (res << 4) | val;
					i++;
				}
			}
			x += qfDrawChar(x, y, res);
			t++;
			continue;
		}
		x += qfDrawChar(x, y, (unsigned char)*t++);
	}
}
#define qfl qfFont,(int8_t[]){0,0,0
void qfInit() {
	qfCharData qfFont = qfCreate(8, 12);
	qfAddChar(qfl,0x7e, 0,0}); // alloc
	charCount--;
	qfAddChar(qfl,0x1f, 0,12, 1, -8, 1, -8, 1, -8, 1, -8, 1, -8, 1, -8, 1, -8, 1, -8, 1, -8, 1, -8, 1, -8, 1, -8});
	qfAddChar(qfl,0x20, 0,0});
	qfAddChar(qfl,0x21, 2,8, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2, 0, 2, 3,-2, 2, 3,-2});
	qfAddChar(qfl,0x22, 2,2, 4, 2,-1,1,-1, 4, 2,-1,1,-1});
	qfAddChar(qfl,0x23, 2,8, 4, 2,-1,2,-1, 4, 2,-1,2,-1, 2, 1,-6, 4, 2,-1,2,-1, 4, 2,-1,2,-1, 2, 1,-6, 4, 2,-1,2,-1, 4, 2,-1,2,-1});
	qfAddChar(qfl,0x24, 1,10, 2, 3,-2, 2, 2,-4, 4, 1,-1,4,-1, 2, 1,-1, 2, 2,-2, 2, 4,-2, 2, 6,-1, 4, 1,-1,4,-1, 2, 2,-4, 2, 3,-2});
	qfAddChar(qfl,0x25, 2,8, 4, 1,-2,2,-1, 4, 1,-2,2,-1, 2, 4,-1, 2, 4,-1, 2, 3,-1, 2, 3,-1, 4, 2,-1,2,-2, 4, 2,-1,2,-2});
	qfAddChar(qfl,0x26, 2,8, 2, 2,-3, 4, 1,-1,3,-1, 2, 1,-1, 2, 2,-1, 4, 1,-1,1,-1, 6, 1,-1,2,-1,1,-1, 4, 1,-1,3,-1, 4, 2,-3,1,-1});
	qfAddChar(qfl,0x27, 2,2, 2, 4,-1, 2, 4,-1});
	qfAddChar(qfl,0x28, 2,8, 2, 5,-1, 2, 4,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 4,-1, 2, 5,-1});
	qfAddChar(qfl,0x29, 2,8, 2, 2,-1, 2, 3,-1, 2, 4,-1, 2, 4,-1, 2, 4,-1, 2, 4,-1, 2, 3,-1, 2, 2,-1});
	qfAddChar(qfl,0x2a, 2,3, 4, 2,-1,2,-1, 2, 3,-2, 4, 2,-1,2,-1});
	qfAddChar(qfl,0x2b, 3,5, 2, 3,-1, 2, 3,-1, 2, 1,-5, 2, 3,-1, 2, 3,-1,});
	qfAddChar(qfl,0x2c, 8,4, 2, 3,-2, 2, 3,-2, 2, 4,-1, 2, 3,-1});
	qfAddChar(qfl,0x2d, 5,1,  2, 1,-6 });
	qfAddChar(qfl,0x2e, 8,2, 2, 3,-2, 2, 3,-2});
	qfAddChar(qfl,0x2f, 2,8, 2, 5,-1, 2, 5,-1, 2, 4,-1, 2, 4,-1, 2, 3,-1, 2, 3,-1, 2, 2,-1, 2, 2,-1});
	qfAddChar(qfl,0x30, 2,8, 2, 2,-4, 4, 1,-1,4,-1, 4, 1,-2,3,-1, 6, 1,-1,1,-1,2,-1, 6, 1,-1,2,-1,1,-1, 4, 1,-1,3,-2, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x31, 2,8, 2, 4,-1, 2, 3,-2, 4, 2,-1,1,-1, 2, 4,-1, 2, 4,-1, 2, 4,-1, 2, 4,-1, 2, 2,-5});
	qfAddChar(qfl,0x32, 2,8, 2, 2,-4, 4, 1,-1,4,-1, 2, 6,-1, 2, 5,-1, 2, 4,-1, 2, 3,-1, 2, 2,-1, 2, 1,-6});
	qfAddChar(qfl,0x33, 2,8, 2, 2,-4, 4, 1,-1,4,-1, 2, 6,-1, 2, 3,-3, 2, 6,-1, 2, 6,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x34, 2,8, 2, 4,-2, 4, 3,-1,1,-1, 4, 2,-1,2,-1, 4, 1,-1,3,-1, 2, 1,-6, 2, 5,-1, 2, 5,-1, 2, 5,-1});
	qfAddChar(qfl,0x35, 2,8, 2, 1,-6, 2, 1,-1, 2, 1,-1, 2, 1,-5, 2, 6,-1, 2, 6,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x36, 2,8, 2, 2,-4, 4, 1,-1,4,-1, 2, 1,-1, 2, 1,-5, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x37, 2,8, 2, 1,-6, 2, 6,-1, 2, 5,-1, 2, 4,-1, 2, 4,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1});
	qfAddChar(qfl,0x38, 2,8, 2, 2,-4, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-4, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x39, 2,8, 2, 2,-4, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-5, 2, 6,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x3a, 3,6, 2, 3,-2, 2, 3,-2, 0, 0, 2, 3,-2, 2, 3,-2});
	qfAddChar(qfl,0x3b, 3,8, 2, 3,-2, 2, 3,-2, 0, 0, 2, 3,-2, 2, 3,-2, 2, 4,-1, 2, 3,-1});
	qfAddChar(qfl,0x3c, 3,6, 2, 5,-2, 2, 3,-2, 2, 1,-2, 2, 1,-2, 2, 3,-2, 2, 5,-2});
	qfAddChar(qfl,0x3d, 4,4, 2, 1,-6, 0, 0, 2, 1,-6});
	qfAddChar(qfl,0x3e, 3,6, 2, 1,-2, 2, 3,-2, 2, 5,-2, 2, 5,-2, 2, 3,-2, 2, 1,-2});
	qfAddChar(qfl,0x3f, 2,8, 2, 3,-3, 4, 2,-1,3,-1, 2, 6,-1, 2, 5,-1, 2, 4,-1, 2, 4,-1, 0, 2, 4,-1});
	qfAddChar(qfl,0x40, 2,8, 2, 2,-4, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 6, 1,-1,1,-2,1,-1, 4, 1,-1,1,-3, 2, 1,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x41, 2,8, 2, 3,-2, 4, 2,-1,2,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 1,-6, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1});
	qfAddChar(qfl,0x42, 2,8, 2, 1,-5, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 1,-5, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 1,-5});
	qfAddChar(qfl,0x43, 2,8, 2, 2,-4, 4, 1,-1,4,-1, 2, 1,-1, 2, 1,-1, 2, 1,-1, 2, 1,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x44, 2,8, 2, 1,-5, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 1,-5});
	qfAddChar(qfl,0x45, 2,8, 2, 1,-6, 2, 1,-1, 2, 1,-1, 2, 1,-4, 2, 1,-1, 2, 1,-1, 2, 1,-1, 2, 1,-6});
	qfAddChar(qfl,0x46, 2,8, 2, 1,-6, 2, 1,-1, 2, 1,-1, 2, 1,-4, 2, 1,-1, 2, 1,-1, 2, 1,-1, 2, 1,-1});
	qfAddChar(qfl,0x47, 2,8, 2, 2,-4, 4, 1,-1,4,-1, 2, 1,-1, 2, 1,-1, 4, 1,-1,2,-3, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x48, 2,8, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 1,-6, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1});
	qfAddChar(qfl,0x49, 2,8, 2, 1,-5, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 1,-5});
	qfAddChar(qfl,0x4a, 2,8, 2, 1,-5, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 1,-2});
	qfAddChar(qfl,0x4b, 2,8, 4, 1,-1,4,-1, 4, 1,-1,3,-1, 4, 1,-1,2,-1, 2, 1,-3, 4, 1,-1,2,-1, 4, 1,-1,3,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1,});
	qfAddChar(qfl,0x4c, 2,8, 2, 1,-1, 2, 1,-1, 2, 1,-1, 2, 1,-1, 2, 1,-1, 2, 1,-1, 2, 1,-1, 2, 1,-6});
	qfAddChar(qfl,0x4d, 2,8, 4, 1,-1,4,-1, 4, 1,-2,2,-2, 6, 1,-1,1,-2,1,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1});
	qfAddChar(qfl,0x4e, 2,8, 4, 1,-1,4,-1, 4, 1,-2,3,-1, 6, 1,-1,1,-1,2,-1, 6, 1,-1,1,-1,2,-1, 6, 1,-1,2,-1,1,-1, 6, 1,-1,2,-1,1,-1, 4, 1,-1,3,-2, 4, 1,-1,4,-1});
	qfAddChar(qfl,0x4f, 2,8, 2, 2,-4, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x50, 2,8, 2, 1,-5, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 1,-5, 2, 1,-1, 2, 1,-1, 2, 1,-1});
	qfAddChar(qfl,0x51, 2,9, 2, 2,-4, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-5, 2, 6,-2});
	qfAddChar(qfl,0x52, 2,8, 2, 1,-5, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 1,-5, 4, 1,-1,3,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1,});
	qfAddChar(qfl,0x53, 2,8, 2, 2,-4, 4, 1,-1,4,-1, 2, 1,-1, 2, 2,-2, 2, 4,-2, 2, 6,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x54, 2,8, 2, 1,-5, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1,});
	qfAddChar(qfl,0x55, 2,8, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x56, 2,8, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 2,-1,2,-1, 4, 2,-1,2,-1, 4, 2,-1,2,-1, 2, 3,-2, 2, 3,-2});
	qfAddChar(qfl,0x57, 2,8, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 6, 1,-1,1,-2,1,-1, 4, 1,-2,2,-2, 4, 1,-1,4,-1});
	qfAddChar(qfl,0x58, 2,8, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 2,-1,2,-1, 2, 3,-2, 2, 3,-2, 4, 2,-1,2,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1});
	qfAddChar(qfl,0x59, 2,8, 4, 1,-1,3,-1, 4, 1,-1,3,-1, 4, 2,-1,1,-1, 4, 2,-1,1,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1});
	qfAddChar(qfl,0x5a, 2,8, 2, 1,-6, 2, 6,-1, 2, 5,-1, 2, 4,-1, 2, 3,-1, 2, 2,-1, 2, 1,-1, 2, 1,-6});
	qfAddChar(qfl,0x5b, 2,8, 2, 3,-3, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-3});
	qfAddChar(qfl,0x5c, 2,8, 2, 2,-1, 2, 2,-1, 2, 3,-1, 2, 3,-1, 2, 4,-1, 2, 4,-1, 2, 5,-1, 2, 5,-1});
	qfAddChar(qfl,0x5d, 2,8, 2, 2,-3, 2, 4,-1, 2, 4,-1, 2, 4,-1, 2, 4,-1, 2, 4,-1, 2, 4,-1, 2, 2,-3});
	qfAddChar(qfl,0x5e, 2,2, 2, 3,-2, 4, 2,-1,2,-1});
	qfAddChar(qfl,0x5f, 9,1,   2, 1,-6 });
	qfAddChar(qfl,0x60, 2,2, 2, 3,-1, 2, 4,-1});
	qfAddChar(qfl,0x61, 4,6, 2, 2,-4, 2, 6,-1, 2, 2,-5, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-5});
	qfAddChar(qfl,0x62, 2,8, 2, 1,-1, 2, 1,-1, 4, 1,-1,1,-3, 4, 1,-2,3,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 1,-5,});
	qfAddChar(qfl,0x63, 4,6, 2, 2,-4, 4, 1,-1,4,-1, 2, 1,-1, 2, 1,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x64, 2,8, 2, 6,-1, 2, 6,-1, 4, 2,-3,1,-1, 4, 1,-1,3,-2, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-5});
	qfAddChar(qfl,0x65, 4,6, 2, 2,-4, 4, 1,-1,4,-1, 2, 1,-6, 2, 1,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x66, 2,8, 2, 4,-2, 2, 3,-1, 2, 3,-1, 2, 1,-5, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1});
	qfAddChar(qfl,0x67, 4,8, 4, 2,-3,1,-1, 4, 1,-1,3,-2, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-5, 2, 6,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x68, 2,8, 2, 1,-1, 2, 1,-1, 4, 1,-1,1,-3, 4, 1,-2,3,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1});
	qfAddChar(qfl,0x69, 2,8, 2, 3,-1, 2, 3,-1, 0, 2, 2,-2, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 1,-5});
	qfAddChar(qfl,0x6a, 2,10, 2, 4,-1, 2, 4,-1, 0, 2, 3,-2, 2, 4,-1, 2, 4,-1, 2, 4,-1, 2, 4,-1, 4, 2,-1,1,-1, 2, 3,-1});
	qfAddChar(qfl,0x6b, 2,8, 2, 1,-1, 2, 1,-1, 4, 1,-1,4,-1, 4, 1,-1,3,-1, 2, 1,-4, 4, 1,-1,3,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1});
	qfAddChar(qfl,0x6c, 2,8, 2, 2,-2, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 1,-5});
	qfAddChar(qfl,0x6d, 4,6, 3, -3,1,-2, 5, -1,2,-1,2,-1, 5, -1,2,-1,2,-1, 5, -1,2,-1,2,-1, 5, -1,2,-1,2,-1, 5, -1,2,-1,2,-1});
	qfAddChar(qfl,0x6e, 4,6, 4, 1,-1,1,-3, 4, 1,-2,3,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1});
	qfAddChar(qfl,0x6f, 4,6, 2, 2,-4, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x70, 4,8, 4, 1,-1,1,-3, 4, 1,-2,3,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 1,-5, 2, 1,-1, 2, 1,-1});
	qfAddChar(qfl,0x71, 4,8, 4, 2,-3,1,-1, 4, 1,-1,3,-2, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 2, 2,-5, 2, 6,-1, 2, 6,-1});
	qfAddChar(qfl,0x72, 4,6, 4, 1,-1,1,-3, 4, 1,-2,3,-1, 2, 1,-1, 2, 1,-1, 2, 1,-1, 2, 1,-1});
	qfAddChar(qfl,0x73, 4,6, 2, 2,-5, 2, 1,-1, 2, 2,-4, 2, 6,-1, 2, 6,-1, 2, 1,-5});
	qfAddChar(qfl,0x74, 2,8, 2, 3,-1, 2, 3,-1, 2, 1,-5, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 3,-1, 2, 4,-2});
	qfAddChar(qfl,0x75, 4,6, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,3,-2, 4, 2,-3,1,-1});
	qfAddChar(qfl,0x76, 4,6, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 2,-1,2,-1, 4, 2,-1,2,-1, 2, 3,-2, 2, 3,-2});
	qfAddChar(qfl,0x77, 4,6, 3, -1,5,-1, 3, -1,5,-1, 5, -1,2,-1,2,-1, 5, -1,2,-1,2,-1, 5, -1,2,-1,2,-1, 4, 1,-2,1,-2});
	qfAddChar(qfl,0x78, 4,6, 4, 1,-1,4,-1, 4, 2,-1,2,-1, 2, 3,-2, 2, 3,-2, 4, 2,-1,2,-1, 4, 1,-1,4,-1});
	qfAddChar(qfl,0x79, 4,8, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,4,-1, 4, 1,-1,3,-2, 4, 2,-3,1,-1, 2, 6,-1, 4, 1,-1,4,-1, 2, 2,-4});
	qfAddChar(qfl,0x7a, 4,6, 2, 1,-6, 2, 5,-1, 2, 4,-1, 2, 3,-1, 2, 2,-1, 2, 1,-6});
	qfAddChar(qfl,0x7b, 2,8, 2, 5,-1, 2, 4,-1, 2, 4,-1, 2, 3,-2, 2, 3,-2, 2, 4,-1, 2, 4,-1, 2, 5,-1});
	qfAddChar(qfl,0x7c, 0,12, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2, 2, 3,-2});
	qfAddChar(qfl,0x7d, 2,8, 2, 2,-1, 2, 3,-1, 2, 3,-1, 2, 3,-2, 2, 3,-2, 2, 3,-1, 2, 3,-1, 2, 2,-1});
	qfAddChar(qfl,0x7e, 5,2, 4, 2,-2,2,-1, 4, 1,-1,2,-2});
}
// ========================================================================================================================================================================
// ===== QGPU =============================================================================================================================================================
// ========================================================================================================================================================================
typedef struct {
	GLFWwindow* window;
	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue graphicsQueue;
	uint32_t graphicsQueueFamilyIndex;

	VkSwapchainKHR swapchain;
	uint32_t imageCount;
	VkImage* swapchainImages;
	VkImageView* swapchainImageViews;
	VkFramebuffer* swapchainFramebuffers;

	VkImage colorImageMSAA;
	VkDeviceMemory colorImageMSAAMemory;
	VkImageView colorImageViewMSAA;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkRenderPass renderPass;
	VkPipeline graphicsPipeline;

	VkImage shadowImage;
	VkDeviceMemory shadowImageMemory;
	VkImageView shadowImageView;
	VkSampler shadowSampler;
	VkFramebuffer shadowFramebuffer;
	VkRenderPass shadowRenderPass;
	VkPipeline shadowPipeline;

	VkPipelineLayout pipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet;
	VkBuffer uboBuffer;
	VkDeviceMemory uboBufferMemory;
	void* mappedUbo;
	VkBuffer lightsBuffer;
	VkDeviceMemory lightsBufferMemory;
	void* mappedLights;

	VkPipeline uiPipeline;
	VkPipelineLayout uiPipelineLayout;
	VkBuffer uiVertexBuffer;
	VkDeviceMemory uiVertexBufferMemory;
	void* mappedUIVertexBuffer;
	VkBuffer uiIndexBuffer;
	VkDeviceMemory uiIndexBufferMemory;
	void* mappedUIIndexBuffer;
	uint32_t currentUIVOffset, currentUIIOffset;

	VkCommandPool commandPool;
	VkCommandBuffer currentCmd;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	void* mappedVertexBuffer;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	void* mappedIndexBuffer;

	VkSemaphore imageAvailableSemaphore, renderFinishedSemaphore;

	uint32_t currentVOffset, currentIOffset;
	uint8_t lastKeyState[GLFW_KEY_LAST], lastMouseState[GLFW_MOUSE_BUTTON_LAST];

	float pivotX, pivotY, pivotZ, rotX, rotY, rotZ;
	uint8_t hasRotation;
} InternalContext;
typedef struct { uint8_t ambientOcclusion, msaaLevel, shadows; } GraphicsSettings;
typedef struct { float viewProj[16], lightViewProj[16], lightPosRange[4], lightPowerShadow[4], sceneInfo[4]; } CameraUBO;
typedef struct { float pos[3], normal[3], color[4]; } QGPU_Vertex;
#define QGPU_LIGHT_POINT       0
#define QGPU_LIGHT_SPOT        1
#define QGPU_LIGHT_DIRECTIONAL 2
typedef struct { float posRange[4], dirPower[4], typeAngle[4]; } GPULight;
typedef struct { float pos[2], color[4]; } QGPU_Vertex2D;

static InternalContext g_ctx;
static GraphicsSettings g_settings = { .ambientOcclusion = 1, .msaaLevel = 4, .shadows = 1 };
static double lastTime = 0;
static ColorRGB backgroundClr;
static uint8_t inInit = 0, framebufferResized = 0;
static void onFramebufferResize(GLFWwindow* window, int w, int h) { (void)window; (void)w; (void)h; framebufferResized = 1; }
static Vector3 camPos = {0.0f, 0.0f, 3.0f}, camTarget = {0.0f, 0.0f, 0.0f}, camUp = {0.0f, 1.0f, 0.0f};
static float camFovDeg = 60.0f, camNear = 0.05f, camFar = 1000.0f, currentFPS = 0;
static GPULight lights[MAX_LIGHTS];
static uint frameCount, lightCount;

#define MAX_UI_TRIANGLES (MAX_UI_VERTICES / 3)
typedef struct { float p[3][2]; float color[4]; uint32_t layer, order; } UITriangle2D;
static UITriangle2D uiTriangles[MAX_UI_TRIANGLES];
static uint32_t uiTriangleCount, currentUILayer;
static int compareUITriangle(const void* a, const void* b) {
	const UITriangle2D *ta = (const UITriangle2D*)a, *tb = (const UITriangle2D*)b;
	if (ta->layer != tb->layer) return (ta->layer < tb->layer) ? -1 : 1;
	if (ta->order != tb->order) return (ta->order < tb->order) ? -1 : 1;
	return 0;
}
// ========================================================================================================================================================================
// ===== TOOLS ============================================================================================================================================================
// ========================================================================================================================================================================
static float PI = 3.14159265358979323846f;
static int qclamp(const int v, const int min, const int max) { return v < min ? min : v > max ? max : v; }
static float qclampf(const float v, const float min, const float max) { return v < min ? min : v > max ? max : v; }
static float qpow(const float v, const float exp) {
	if (exp == 0) return 1.0f;
	float r = 1.0f;
	for (int i = 0; i < exp; i++) r *= v;
	return r;
}
static float qsqrt(const float number) {
	if (number <= 0.0f) return 0.0f;
	float x = number * 0.5f;
	for (uint8_t i = 0; i < 6; i++) x = 0.5f * (x + number / x);
	return x;
}
static unsigned long long factorial(const int n) {
	unsigned long long result = 1.0f;
	for (int i = 1; i <= n; i++) result *= i;
	return result;
}
static float qSin(const float rad) {
	float sum = 0.0f;
	for (uint8_t i = 0; i < 10; i++) {
		int8_t sign = (i % 2 == 0) ? 1 : -1, power_exp = 2 * i + 1;
		sum += sign * (qpow(rad, power_exp) / (float)factorial(power_exp));
	}
	return sum;
}
static float qCos(const float rad) {
	float sum = 0.0f;
	for (uint8_t i = 0; i < 10; i++) {
		int8_t sign = (i % 2 == 0) ? 1 : -1, power_exp = 2 * i;
		sum += sign * (qpow(rad, power_exp) / (float)factorial(power_exp));
	}
	return sum;
}
static float qTan(const float rad) { float c = qCos(rad); if (c == 0.0f) c = 0.0001f; return qSin(rad) / c; }

static Vector3 v3Add(const Vector3 a, const Vector3 b) { return (Vector3){a.x + b.x, a.y + b.y, a.z + b.z}; }
static Vector3 v3Sub(const Vector3 a, const Vector3 b) { return (Vector3){a.x - b.x, a.y - b.y, a.z - b.z}; }
static Vector3 v3Scale(const Vector3 a, const float s) { return (Vector3){a.x * s, a.y * s, a.z * s}; }
static Vector3 v3Cross(const Vector3 a, const Vector3 b) { return (Vector3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }
static float v3Dot(const Vector3 a, const Vector3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static float v3Length(const Vector3 a) { return qsqrt(v3Dot(a, a)); }
static Vector3 v3Normalize(const Vector3 a) { float l = v3Length(a); if (l < 0.00001f) return (Vector3){0, 0, 0}; return v3Scale(a, 1.0f / l); }

static void mat4Identity(float* m) { memset(m, 0, sizeof(float) * 16); m[0] = m[5] = m[10] = m[15] = 1.0f; }
static void mat4Multiply(float* out, const float* a, const float* b) {
	float r[16];
	for (uint8_t col = 0; col < 4; col++) for (uint8_t row = 0; row < 4; row++) {
			float sum = 0.0f;
			for (uint8_t k = 0; k < 4; k++) sum += a[k * 4 + row] * b[col * 4 + k];
			r[col * 4 + row] = sum;
		}
	memcpy(out, r, sizeof(r));
}
static void mat4LookAt(float* m, const Vector3 eye, const Vector3 center, const Vector3 up) {
	Vector3 f = v3Normalize(v3Sub(center, eye)), s = v3Normalize(v3Cross(f, up)), u = v3Cross(s, f);
	mat4Identity(m);
	m[0] = s.x;  m[4] = s.y;  m[8]  = s.z;  m[12] = -v3Dot(s, eye);
	m[1] = u.x;  m[5] = u.y;  m[9]  = u.z;  m[13] = -v3Dot(u, eye);
	m[2] = -f.x; m[6] = -f.y; m[10] = -f.z; m[14] =  v3Dot(f, eye);
	m[3] = 0;    m[7] = 0;    m[11] = 0;    m[15] = 1;
}
static void mat4Perspective(float* m, const float fovyRad, const float aspect, const float znear, const float zfar) {
	memset(m, 0, sizeof(float) * 16);
	float f = 1.0f / qTan(fovyRad * 0.5f);
	m[0] = f / aspect;
	m[5] = -f;
	m[10] = zfar / (znear - zfar);
	m[11] = -1.0f;
	m[14] = (znear * zfar) / (znear - zfar);
}
static void mat4Ortho(float* m, const float left, const float right, const float bottom, const float top, const float znear, const float zfar) {
	memset(m, 0, sizeof(float) * 16);
	m[0] = 2.0f / (right - left);
	m[5] = -2.0f / (top - bottom);
	m[10] = 1.0f / (znear - zfar);
	m[12] = -(right + left) / (right - left);
	m[13] = (top + bottom) / (top - bottom);
	m[14] = znear / (znear - zfar);
	m[15] = 1.0f;
}
static void transformPoint(float* x, float* y, float* z) {
	if (!g_ctx.hasRotation) return;
	float px = *x - g_ctx.pivotX, py = *y - g_ctx.pivotY, pz = *z - g_ctx.pivotZ, radX = g_ctx.rotX * (PI / 180.0f), radY = g_ctx.rotY * (PI / 180.0f), radZ = g_ctx.rotZ * (PI / 180.0f),
	cx = qCos(radX), sx = qSin(radX), cy = qCos(radY), sy = qSin(radY), cz = qCos(radZ), sz = qSin(radZ),
	y1 = py * cx - pz * sx, z1 = py * sx + pz * cx, x1 = px, x2 = x1 * cy + z1 * sy, z2 = -x1 * sy + z1 * cy, y2 = y1, x3 = x2 * cz - y2 * sz, y3 = x2 * sz + y2 * cz, z3 = z2;
	*x = x3 + g_ctx.pivotX;
	*y = y3 + g_ctx.pivotY;
	*z = z3 + g_ctx.pivotZ;
}
// ========================================================================================================================================================================
// ===== CONSOLE ==========================================================================================================================================================
// ========================================================================================================================================================================
static uint8_t _showBanner = 1, _madeWith = 1, _showInfo = 1, _showColors = 1, _showLogs = 1, qgpuClr = MAGENTA, creator = LIGHT_RED, title = YELLOW, frame = GRAY, frmTxt = LIGHT_GRAY,
oldClr = 255, actClr = 255, actStyle = 0;
void qgVprintc(const int color, const char* format, va_list args) {
	printf("\033[%i;38;5;%im", actStyle, color);
	vprintf(format, args);
	printf("\033[0m");
}
void qgSetColor(const uint8_t color) {
	oldClr = actClr;
	actClr = qclamp(color, 0, 255);
}
void qgRestoreColor() {
	const uint8_t x = actClr;
	actClr = oldClr;
	oldClr = x;
}
void qgSetStyle(const uint8_t style) { actStyle = qclamp(style, 0, 1); }
void qgPrintc(const int color, const char* format, ...) {
	va_list args;
	va_start(args, format);
	qgVprintc(color, format, args);
	va_end(args);
}
void qgPrint(const char* format, ...) {
	va_list args;
	va_start(args, format);
	qgVprintc(actClr, format, args);
	va_end(args);
}
void qgLog(const char* format, ...) {
	if (!_showLogs) return;
	va_list args;
	va_start(args, format);
	qgVprintc(DARK_GRAY, format, args);
	va_end(args);
}
void qgLogVertices() {
	if (!_showLogs) return;
	const float p = ((float)g_ctx.currentVOffset / MAX_VERTICES) * 100.0f,
				p2 = ((float)g_ctx.currentUIVOffset / MAX_UI_VERTICES) * 100.0f;
	uint8_t clr = 0;
	if (p > 95) clr = LIGHT_RED;
	else if (p > 75) clr = RED;
	else if (p > 50) clr = ORANGE;
	else if (p > 25) clr = YELLOW;
	else if (p > 10) clr = DARK_YELLOW;
	else clr = GREEN;
	qgPrint("3D: ");
	qgPrintc(clr, "%u/%u (%.0f%%)", g_ctx.currentVOffset, MAX_VERTICES, p);
	if (p2 > 95) clr = LIGHT_RED;
	else if (p2 > 75) clr = RED;
	else if (p2 > 50) clr = ORANGE;
	else if (p2 > 25) clr = YELLOW;
	else if (p2 > 10) clr = DARK_YELLOW;
	else clr = GREEN;
	qgPrint(" 2D: ");
	qgPrintc(clr, "%u/%u (%.0f%%)\n", g_ctx.currentUIVOffset, MAX_UI_VERTICES, p2);
}
void qgWarn(const char* format, ...) {
	va_list args;
	va_start(args, format);
	qgVprintc(DARK_YELLOW, format, args);
	va_end(args);
}
void qgError(const char* format, ...) {
	va_list args;
	va_start(args, format);
	qgVprintc(DARK_RED, format, args);
	va_end(args);
	exit(1);
}
void qgSetShow(const uint8_t shower, const uint8_t state) {
	switch (shower) {
		case QGPU_SHOW_BANNER: _showBanner = state; break;
		case QGPU_SHOW_MADE_WITH_QGPU: _madeWith = state; break;
		case QGPU_SHOW_INFO: _showInfo = state; break;
		case QGPU_SHOW_COLORS: _showColors = state; break;
		case QGPU_SHOW_LOGS: _showLogs = state; break;
	}
}
static void printBanner() {
	qgPrintc(165, "╔═════╗ ╔═════╗ ╔═════╗ ╔═╗ ╔═╗\n");
	qgPrintc(164, "║ ╔═╗ ║ ║ ╔═══╝ ║ ╔═╗ ║ ║ ║ ║ ║\n");
	qgPrintc(163, "║ ║ ║ ║ ║ ║ ╔═╗ ║ ╚═╝ ║ ║ ║ ║ ║\n");
	qgPrintc(162, "║ ╚═╝ ║ ║ ╚═╝ ║ ║ ╔═══╝ ║ ╚═╝ ║\n");
	qgPrintc(161, "╚═══╗ ║ ╚═════╝ ╚═╝     ╚═════╝\n");
	qgPrintc(160, "    ╚═╝\n");
}
static void printMadeWith() { qgPrintc(ORANGE, "The application was made with the "); qgPrintc(qgpuClr, "QGPU"); qgPrintc(ORANGE, " library.\n"); }
static void printInfo() {
	qgPrintc(frame, "╔═╣   "); qgPrintc(title, "Info"); qgPrintc(frame, "   ╠═══════════╦╗\n");
	qgPrintc(frame, "║ "); qgPrintc(frmTxt, "Name: "); qgPrintc(qgpuClr, "QGPU"); qgPrintc(frame, "             ╚╝\n");
	qgPrintc(frame, "║ "); qgPrintc(frmTxt, "Version: "); qgPrintc(qgpuClr, "%i.%i.%i\n", QGPU_VERSION_MAJOR, QGPU_VERSION_MINOR, QGPU_VERSION_PATCH);
	qgPrintc(frame, "║ "); qgPrintc(frmTxt, "Creator: "); qgPrintc(creator, "Qunerum"); qgPrintc(frame, "       ╔╗\n");
	qgPrintc(frame, "╚════════════════════════╩╝\n");
}
static void c(const uint8_t v) { printf("\033[0;38;5;%im██ ", v); }
static void printColors() {
	qgPrintc(frame,"╔═╣  "); qgPrintc(title, "Colors"); qgPrintc(frame, "  ╠═╗  ╔═══════╗\n");
	qgPrintc(frame,"╚═╦══════════╦═╝  ║ "); c(WHITE); c(BLACK); qgPrintc(frame,"║\n");
	qgPrintc(frame,"╔═╩══════════╩════╩═══════╣\n");
	qgPrintc(frame,"║ "); c(LIGHT_GRAY); c(LIGHT_RED); c(LIGHT_GREEN); c(LIGHT_YELLOW); c(LIGHT_ORANGE); c(LIGHT_BLUE); c(LIGHT_MAGENTA); c(LIGHT_CYAN); qgPrintc(frame,"║\n");
	qgPrintc(frame,"║ "); c(GRAY);       c(RED);       c(GREEN);       c(YELLOW);       c(ORANGE);       c(BLUE);       c(MAGENTA);       c(CYAN);       qgPrintc(frame,"║\n");
	qgPrintc(frame,"║ "); c(DARK_GRAY);  c(DARK_RED);  c(DARK_GREEN);  c(DARK_YELLOW);  c(DARK_ORANGE);  c(DARK_BLUE);  c(DARK_MAGENTA);  c(DARK_CYAN);  qgPrintc(frame,"║\n");
	qgPrintc(frame,"╚═════════════════════════╝\n");
}
static const uint32_t mainVertCode[] = {
	0x07230203,0x00010000,0x000d000b,0x00000037,0x00000000,0x00020011,0x00000001,0x0006000b,0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x000d000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x00000009,0x0000000b,0x0000000d,0x0000000e,0x00000012,0x00000014,0x00000016,0x0000002c,0x00030003,0x00000002,0x000001c2,
	0x000a0004,0x475f4c47,0x4c474f4f,0x70635f45,0x74735f70,0x5f656c79,0x656e696c,0x7269645f,0x69746365,0x00006576,0x00080004,0x475f4c47,0x4c474f4f,0x6e695f45,0x64756c63,0x69645f65,
	0x74636572,0x00657669,0x00040005,0x00000004,0x6e69616d,0x00000000,0x00060005,0x00000009,0x67617266,0x6c726f57,0x736f5064,0x00000000,0x00040005,0x0000000b,0x6f506e69,0x00000073,
	0x00050005,0x0000000d,0x67617266,0x6d726f4e,0x00006c61,0x00050005,0x0000000e,0x6f4e6e69,0x6c616d72,0x00000000,0x00050005,0x00000012,0x67617266,0x6f6c6f43,0x00000072,0x00040005,
	0x00000014,0x6f436e69,0x00726f6c,0x00070005,0x00000016,0x67617266,0x6867694c,0x61705374,0x6f506563,0x00000073,0x00050005,0x00000018,0x656d6143,0x42556172,0x0000004f,0x00060006,
	0x00000018,0x00000000,0x77656976,0x6a6f7250,0x00000000,0x00070006,0x00000018,0x00000001,0x6867696c,0x65695674,0x6f725077,0x0000006a,0x00070006,0x00000018,0x00000002,0x6867696c,
	0x736f5074,0x676e6152,0x00000065,0x00080006,0x00000018,0x00000003,0x6867696c,0x776f5074,0x68537265,0x776f6461,0x00000000,0x00060006,0x00000018,0x00000004,0x6e656373,0x666e4965,
	0x0000006f,0x00030005,0x0000001a,0x006d6163,0x00060005,0x0000002a,0x505f6c67,0x65567265,0x78657472,0x00000000,0x00060006,0x0000002a,0x00000000,0x505f6c67,0x7469736f,0x006e6f69,
	0x00070006,0x0000002a,0x00000001,0x505f6c67,0x746e696f,0x657a6953,0x00000000,0x00070006,0x0000002a,0x00000002,0x435f6c67,0x4470696c,0x61747369,0x0065636e,0x00070006,0x0000002a,
	0x00000003,0x435f6c67,0x446c6c75,0x61747369,0x0065636e,0x00030005,0x0000002c,0x00000000,0x00040047,0x00000009,0x0000001e,0x00000000,0x00040047,0x0000000b,0x0000001e,0x00000000,
	0x00040047,0x0000000d,0x0000001e,0x00000001,0x00040047,0x0000000e,0x0000001e,0x00000001,0x00040047,0x00000012,0x0000001e,0x00000002,0x00040047,0x00000014,0x0000001e,0x00000002,
	0x00040047,0x00000016,0x0000001e,0x00000003,0x00030047,0x00000018,0x00000002,0x00040048,0x00000018,0x00000000,0x00000005,0x00050048,0x00000018,0x00000000,0x00000007,0x00000010,
	0x00050048,0x00000018,0x00000000,0x00000023,0x00000000,0x00040048,0x00000018,0x00000001,0x00000005,0x00050048,0x00000018,0x00000001,0x00000007,0x00000010,0x00050048,0x00000018,
	0x00000001,0x00000023,0x00000040,0x00050048,0x00000018,0x00000002,0x00000023,0x00000080,0x00050048,0x00000018,0x00000003,0x00000023,0x00000090,0x00050048,0x00000018,0x00000004,
	0x00000023,0x000000a0,0x00040047,0x0000001a,0x00000021,0x00000000,0x00040047,0x0000001a,0x00000022,0x00000000,0x00030047,0x0000002a,0x00000002,0x00050048,0x0000002a,0x00000000,
	0x0000000b,0x00000000,0x00050048,0x0000002a,0x00000001,0x0000000b,0x00000001,0x00050048,0x0000002a,0x00000002,0x0000000b,0x00000003,0x00050048,0x0000002a,0x00000003,0x0000000b,
	0x00000004,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,0x00000003,0x00040020,0x00000008,0x00000003,
	0x00000007,0x0004003b,0x00000008,0x00000009,0x00000003,0x00040020,0x0000000a,0x00000001,0x00000007,0x0004003b,0x0000000a,0x0000000b,0x00000001,0x0004003b,0x00000008,0x0000000d,
	0x00000003,0x0004003b,0x0000000a,0x0000000e,0x00000001,0x00040017,0x00000010,0x00000006,0x00000004,0x00040020,0x00000011,0x00000003,0x00000010,0x0004003b,0x00000011,0x00000012,
	0x00000003,0x00040020,0x00000013,0x00000001,0x00000010,0x0004003b,0x00000013,0x00000014,0x00000001,0x0004003b,0x00000011,0x00000016,0x00000003,0x00040018,0x00000017,0x00000010,
	0x00000004,0x0007001e,0x00000018,0x00000017,0x00000017,0x00000010,0x00000010,0x00000010,0x00040020,0x00000019,0x00000002,0x00000018,0x0004003b,0x00000019,0x0000001a,0x00000002,
	0x00040015,0x0000001b,0x00000020,0x00000001,0x0004002b,0x0000001b,0x0000001c,0x00000001,0x00040020,0x0000001d,0x00000002,0x00000017,0x0004002b,0x00000006,0x00000021,0x3f800000,
	0x00040015,0x00000027,0x00000020,0x00000000,0x0004002b,0x00000027,0x00000028,0x00000001,0x0004001c,0x00000029,0x00000006,0x00000028,0x0006001e,0x0000002a,0x00000010,0x00000006,
	0x00000029,0x00000029,0x00040020,0x0000002b,0x00000003,0x0000002a,0x0004003b,0x0000002b,0x0000002c,0x00000003,0x0004002b,0x0000001b,0x0000002d,0x00000000,0x00050036,0x00000002,
	0x00000004,0x00000000,0x00000003,0x000200f8,0x00000005,0x0004003d,0x00000007,0x0000000c,0x0000000b,0x0003003e,0x00000009,0x0000000c,0x0004003d,0x00000007,0x0000000f,0x0000000e,
	0x0003003e,0x0000000d,0x0000000f,0x0004003d,0x00000010,0x00000015,0x00000014,0x0003003e,0x00000012,0x00000015,0x00050041,0x0000001d,0x0000001e,0x0000001a,0x0000001c,0x0004003d,
	0x00000017,0x0000001f,0x0000001e,0x0004003d,0x00000007,0x00000020,0x0000000b,0x00050051,0x00000006,0x00000022,0x00000020,0x00000000,0x00050051,0x00000006,0x00000023,0x00000020,
	0x00000001,0x00050051,0x00000006,0x00000024,0x00000020,0x00000002,0x00070050,0x00000010,0x00000025,0x00000022,0x00000023,0x00000024,0x00000021,0x00050091,0x00000010,0x00000026,
	0x0000001f,0x00000025,0x0003003e,0x00000016,0x00000026,0x00050041,0x0000001d,0x0000002e,0x0000001a,0x0000002d,0x0004003d,0x00000017,0x0000002f,0x0000002e,0x0004003d,0x00000007,
	0x00000030,0x0000000b,0x00050051,0x00000006,0x00000031,0x00000030,0x00000000,0x00050051,0x00000006,0x00000032,0x00000030,0x00000001,0x00050051,0x00000006,0x00000033,0x00000030,
	0x00000002,0x00070050,0x00000010,0x00000034,0x00000031,0x00000032,0x00000033,0x00000021,0x00050091,0x00000010,0x00000035,0x0000002f,0x00000034,0x00050041,0x00000011,0x00000036,
	0x0000002c,0x0000002d,0x0003003e,0x00000036,0x00000035,0x000100fd,0x00010038
}, mainFragCode[] = {
	0x07230203,0x00010000,0x000d000b,0x0000011d,0x00000000,0x00020011,0x00000001,0x0006000b,0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x000a000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x0000001e,0x00000075,0x000000bf,0x00000111,0x00000112,0x00030010,0x00000004,0x00000007,0x00030003,0x00000002,0x000001c2,
	0x000a0004,0x475f4c47,0x4c474f4f,0x70635f45,0x74735f70,0x5f656c79,0x656e696c,0x7269645f,0x69746365,0x00006576,0x00080004,0x475f4c47,0x4c474f4f,0x6e695f45,0x64756c63,0x69645f65,
	0x74636572,0x00657669,0x00040005,0x00000004,0x6e69616d,0x00000000,0x00060005,0x00000008,0x64616873,0x6146776f,0x726f7463,0x00000028,0x00050005,0x0000000c,0x656d6143,0x42556172,
	0x0000004f,0x00060006,0x0000000c,0x00000000,0x77656976,0x6a6f7250,0x00000000,0x00070006,0x0000000c,0x00000001,0x6867696c,0x65695674,0x6f725077,0x0000006a,0x00070006,0x0000000c,
	0x00000002,0x6867696c,0x736f5074,0x676e6152,0x00000065,0x00080006,0x0000000c,0x00000003,0x6867696c,0x776f5074,0x68537265,0x776f6461,0x00000000,0x00060006,0x0000000c,0x00000004,
	0x6e656373,0x666e4965,0x0000006f,0x00030005,0x0000000e,0x006d6163,0x00070005,0x0000001e,0x67617266,0x6867694c,0x61705374,0x6f506563,0x00000073,0x00040005,0x0000002a,0x6a6f7270,
	0x00000000,0x00030005,0x00000033,0x00007675,0x00040005,0x0000005e,0x73616962,0x00000000,0x00060005,0x00000060,0x736f6c63,0x44747365,0x68747065,0x00000000,0x00050005,0x00000064,
	0x64616873,0x614d776f,0x00000070,0x00030005,0x00000073,0x0000006e,0x00050005,0x00000075,0x67617266,0x6d726f4e,0x00006c61,0x00040005,0x00000078,0x61746f74,0x0000006c,0x00040005,
	0x0000007d,0x6e756f63,0x00000074,0x00040005,0x00000081,0x64616873,0x0000776f,0x00030005,0x00000083,0x00000069,0x00050005,0x0000008d,0x4c555047,0x74686769,0x00000000,0x00060006,
	0x0000008d,0x00000000,0x52736f70,0x65676e61,0x00000000,0x00060006,0x0000008d,0x00000001,0x50726964,0x7265776f,0x00000000,0x00060006,0x0000008d,0x00000002,0x65707974,0x6c676e41,
	0x00000065,0x00030005,0x0000008f,0x0000004c,0x00050005,0x00000090,0x4c555047,0x74686769,0x00000000,0x00060006,0x00000090,0x00000000,0x52736f70,0x65676e61,0x00000000,0x00060006,
	0x00000090,0x00000001,0x50726964,0x7265776f,0x00000000,0x00060006,0x00000090,0x00000002,0x65707974,0x6c676e41,0x00000065,0x00060005,0x00000092,0x6867694c,0x75427374,0x72656666,
	0x00000000,0x00050006,0x00000092,0x00000000,0x6867696c,0x00007374,0x00050005,0x00000094,0x6867696c,0x75427374,0x00000066,0x00040005,0x000000a2,0x65776f70,0x00000072,0x00040005,
	0x000000aa,0x746e6f63,0x00626972,0x00040005,0x000000bb,0x694c6f74,0x00746867,0x00060005,0x000000bf,0x67617266,0x6c726f57,0x736f5064,0x00000000,0x00040005,0x000000c2,0x676e6172,
	0x00000065,0x00040005,0x000000c5,0x74736964,0x00000000,0x00050005,0x000000d1,0x6867696c,0x72694474,0x00000000,0x00040005,0x000000d8,0x65747461,0x0000006e,0x00040005,0x000000e2,
	0x746f7073,0x00736f43,0x00050005,0x00000111,0x4374756f,0x726f6c6f,0x00000000,0x00050005,0x00000112,0x67617266,0x6f6c6f43,0x00000072,0x00030047,0x0000000c,0x00000002,0x00040048,
	0x0000000c,0x00000000,0x00000005,0x00050048,0x0000000c,0x00000000,0x00000007,0x00000010,0x00050048,0x0000000c,0x00000000,0x00000023,0x00000000,0x00040048,0x0000000c,0x00000001,
	0x00000005,0x00050048,0x0000000c,0x00000001,0x00000007,0x00000010,0x00050048,0x0000000c,0x00000001,0x00000023,0x00000040,0x00050048,0x0000000c,0x00000002,0x00000023,0x00000080,
	0x00050048,0x0000000c,0x00000003,0x00000023,0x00000090,0x00050048,0x0000000c,0x00000004,0x00000023,0x000000a0,0x00040047,0x0000000e,0x00000021,0x00000000,0x00040047,0x0000000e,
	0x00000022,0x00000000,0x00040047,0x0000001e,0x0000001e,0x00000003,0x00040047,0x00000064,0x00000021,0x00000001,0x00040047,0x00000064,0x00000022,0x00000000,0x00040047,0x00000075,
	0x0000001e,0x00000001,0x00050048,0x00000090,0x00000000,0x00000023,0x00000000,0x00050048,0x00000090,0x00000001,0x00000023,0x00000010,0x00050048,0x00000090,0x00000002,0x00000023,
	0x00000020,0x00040047,0x00000091,0x00000006,0x00000030,0x00030047,0x00000092,0x00000003,0x00040048,0x00000092,0x00000000,0x00000018,0x00050048,0x00000092,0x00000000,0x00000023,
	0x00000000,0x00030047,0x00000094,0x00000018,0x00040047,0x00000094,0x00000021,0x00000002,0x00040047,0x00000094,0x00000022,0x00000000,0x00040047,0x000000bf,0x0000001e,0x00000000,
	0x00040047,0x00000111,0x0000001e,0x00000000,0x00040047,0x00000112,0x0000001e,0x00000002,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,0x00000020,
	0x00030021,0x00000007,0x00000006,0x00040017,0x0000000a,0x00000006,0x00000004,0x00040018,0x0000000b,0x0000000a,0x00000004,0x0007001e,0x0000000c,0x0000000b,0x0000000b,0x0000000a,
	0x0000000a,0x0000000a,0x00040020,0x0000000d,0x00000002,0x0000000c,0x0004003b,0x0000000d,0x0000000e,0x00000002,0x00040015,0x0000000f,0x00000020,0x00000001,0x0004002b,0x0000000f,
	0x00000010,0x00000003,0x00040015,0x00000011,0x00000020,0x00000000,0x0004002b,0x00000011,0x00000012,0x00000001,0x00040020,0x00000013,0x00000002,0x00000006,0x0004002b,0x00000006,
	0x00000016,0x3f000000,0x00020014,0x00000017,0x0004002b,0x00000006,0x0000001b,0x3f800000,0x00040020,0x0000001d,0x00000001,0x0000000a,0x0004003b,0x0000001d,0x0000001e,0x00000001,
	0x0004002b,0x00000011,0x0000001f,0x00000003,0x00040020,0x00000020,0x00000001,0x00000006,0x0004002b,0x00000006,0x00000023,0x00000000,0x00040017,0x00000028,0x00000006,0x00000003,
	0x00040020,0x00000029,0x00000007,0x00000028,0x00040017,0x00000031,0x00000006,0x00000002,0x00040020,0x00000032,0x00000007,0x00000031,0x0004002b,0x00000011,0x00000039,0x00000000,
	0x00040020,0x0000003a,0x00000007,0x00000006,0x0004002b,0x00000011,0x00000056,0x00000002,0x0004002b,0x00000006,0x0000005f,0x3951b717,0x00090019,0x00000061,0x00000006,0x00000001,
	0x00000000,0x00000000,0x00000000,0x00000001,0x00000000,0x0003001b,0x00000062,0x00000061,0x00040020,0x00000063,0x00000000,0x00000062,0x0004003b,0x00000063,0x00000064,0x00000000,
	0x0004002b,0x00000006,0x0000006f,0x3eb33333,0x00040020,0x00000074,0x00000001,0x00000028,0x0004003b,0x00000074,0x00000075,0x00000001,0x0004002b,0x0000000f,0x00000079,0x00000004,
	0x00040020,0x0000007c,0x00000007,0x0000000f,0x0004002b,0x0000000f,0x00000084,0x00000000,0x0005001e,0x0000008d,0x0000000a,0x0000000a,0x0000000a,0x00040020,0x0000008e,0x00000007,
	0x0000008d,0x0005001e,0x00000090,0x0000000a,0x0000000a,0x0000000a,0x0003001d,0x00000091,0x00000090,0x0003001e,0x00000092,0x00000091,0x00040020,0x00000093,0x00000002,0x00000092,
	0x0004003b,0x00000093,0x00000094,0x00000002,0x00040020,0x00000096,0x00000002,0x00000090,0x00040020,0x0000009a,0x00000007,0x0000000a,0x0004002b,0x0000000f,0x0000009d,0x00000001,
	0x0004002b,0x0000000f,0x000000a0,0x00000002,0x0004002b,0x00000006,0x000000ad,0x3fc00000,0x0004003b,0x00000074,0x000000bf,0x00000001,0x0004002b,0x00000006,0x000000d4,0x38d1b717,
	0x0004002b,0x00000006,0x000000f4,0x3d4ccccd,0x0004002b,0x00000006,0x0000010e,0x40000000,0x00040020,0x00000110,0x00000003,0x0000000a,0x0004003b,0x00000110,0x00000111,0x00000003,
	0x0004003b,0x0000001d,0x00000112,0x00000001,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,0x00000005,0x0004003b,0x00000029,0x00000073,0x00000007,0x0004003b,
	0x0000003a,0x00000078,0x00000007,0x0004003b,0x0000007c,0x0000007d,0x00000007,0x0004003b,0x0000003a,0x00000081,0x00000007,0x0004003b,0x0000007c,0x00000083,0x00000007,0x0004003b,
	0x0000008e,0x0000008f,0x00000007,0x0004003b,0x0000003a,0x000000a2,0x00000007,0x0004003b,0x0000003a,0x000000aa,0x00000007,0x0004003b,0x00000029,0x000000bb,0x00000007,0x0004003b,
	0x0000003a,0x000000c2,0x00000007,0x0004003b,0x0000003a,0x000000c5,0x00000007,0x0004003b,0x00000029,0x000000d1,0x00000007,0x0004003b,0x0000003a,0x000000d8,0x00000007,0x0004003b,
	0x0000003a,0x000000e2,0x00000007,0x0004003d,0x00000028,0x00000076,0x00000075,0x0006000c,0x00000028,0x00000077,0x00000001,0x00000045,0x00000076,0x0003003e,0x00000073,0x00000077,
	0x00060041,0x00000013,0x0000007a,0x0000000e,0x00000079,0x00000039,0x0004003d,0x00000006,0x0000007b,0x0000007a,0x0003003e,0x00000078,0x0000007b,0x00060041,0x00000013,0x0000007e,
	0x0000000e,0x00000079,0x00000012,0x0004003d,0x00000006,0x0000007f,0x0000007e,0x0004006e,0x0000000f,0x00000080,0x0000007f,0x0003003e,0x0000007d,0x00000080,0x00040039,0x00000006,
	0x00000082,0x00000008,0x0003003e,0x00000081,0x00000082,0x0003003e,0x00000083,0x00000084,0x000200f9,0x00000085,0x000200f8,0x00000085,0x000400f6,0x00000087,0x00000088,0x00000000,
	0x000200f9,0x00000089,0x000200f8,0x00000089,0x0004003d,0x0000000f,0x0000008a,0x00000083,0x0004003d,0x0000000f,0x0000008b,0x0000007d,0x000500b1,0x00000017,0x0000008c,0x0000008a,
	0x0000008b,0x000400fa,0x0000008c,0x00000086,0x00000087,0x000200f8,0x00000086,0x0004003d,0x0000000f,0x00000095,0x00000083,0x00060041,0x00000096,0x00000097,0x00000094,0x00000084,
	0x00000095,0x0004003d,0x00000090,0x00000098,0x00000097,0x00050051,0x0000000a,0x00000099,0x00000098,0x00000000,0x00050041,0x0000009a,0x0000009b,0x0000008f,0x00000084,0x0003003e,
	0x0000009b,0x00000099,0x00050051,0x0000000a,0x0000009c,0x00000098,0x00000001,0x00050041,0x0000009a,0x0000009e,0x0000008f,0x0000009d,0x0003003e,0x0000009e,0x0000009c,0x00050051,
	0x0000000a,0x0000009f,0x00000098,0x00000002,0x00050041,0x0000009a,0x000000a1,0x0000008f,0x000000a0,0x0003003e,0x000000a1,0x0000009f,0x00060041,0x0000003a,0x000000a3,0x0000008f,
	0x0000009d,0x0000001f,0x0004003d,0x00000006,0x000000a4,0x000000a3,0x0003003e,0x000000a2,0x000000a4,0x0004003d,0x00000006,0x000000a5,0x000000a2,0x000500bc,0x00000017,0x000000a6,
	0x000000a5,0x00000023,0x000300f7,0x000000a8,0x00000000,0x000400fa,0x000000a6,0x000000a7,0x000000a8,0x000200f8,0x000000a7,0x000200f9,0x00000088,0x000200f8,0x000000a8,0x0003003e,
	0x000000aa,0x00000023,0x00060041,0x0000003a,0x000000ab,0x0000008f,0x000000a0,0x00000039,0x0004003d,0x00000006,0x000000ac,0x000000ab,0x000500ba,0x00000017,0x000000ae,0x000000ac,
	0x000000ad,0x000300f7,0x000000b0,0x00000000,0x000400fa,0x000000ae,0x000000af,0x000000ba,0x000200f8,0x000000af,0x0004003d,0x00000006,0x000000b1,0x000000a2,0x0004003d,0x00000028,
	0x000000b2,0x00000073,0x00050041,0x0000009a,0x000000b3,0x0000008f,0x0000009d,0x0004003d,0x0000000a,0x000000b4,0x000000b3,0x0008004f,0x00000028,0x000000b5,0x000000b4,0x000000b4,
	0x00000000,0x00000001,0x00000002,0x0004007f,0x00000028,0x000000b6,0x000000b5,0x00050094,0x00000006,0x000000b7,0x000000b2,0x000000b6,0x0007000c,0x00000006,0x000000b8,0x00000001,
	0x00000028,0x000000b7,0x00000023,0x00050085,0x00000006,0x000000b9,0x000000b1,0x000000b8,0x0003003e,0x000000aa,0x000000b9,0x000200f9,0x000000b0,0x000200f8,0x000000ba,0x00050041,
	0x0000009a,0x000000bc,0x0000008f,0x00000084,0x0004003d,0x0000000a,0x000000bd,0x000000bc,0x0008004f,0x00000028,0x000000be,0x000000bd,0x000000bd,0x00000000,0x00000001,0x00000002,
	0x0004003d,0x00000028,0x000000c0,0x000000bf,0x00050083,0x00000028,0x000000c1,0x000000be,0x000000c0,0x0003003e,0x000000bb,0x000000c1,0x00060041,0x0000003a,0x000000c3,0x0000008f,
	0x00000084,0x0000001f,0x0004003d,0x00000006,0x000000c4,0x000000c3,0x0003003e,0x000000c2,0x000000c4,0x0004003d,0x00000028,0x000000c6,0x000000bb,0x0006000c,0x00000006,0x000000c7,
	0x00000001,0x00000042,0x000000c6,0x0003003e,0x000000c5,0x000000c7,0x0004003d,0x00000006,0x000000c8,0x000000c2,0x000500bc,0x00000017,0x000000c9,0x000000c8,0x00000023,0x0004003d,
	0x00000006,0x000000ca,0x000000c5,0x0004003d,0x00000006,0x000000cb,0x000000c2,0x000500ba,0x00000017,0x000000cc,0x000000ca,0x000000cb,0x000500a6,0x00000017,0x000000cd,0x000000c9,
	0x000000cc,0x000300f7,0x000000cf,0x00000000,0x000400fa,0x000000cd,0x000000ce,0x000000cf,0x000200f8,0x000000ce,0x000200f9,0x00000088,0x000200f8,0x000000cf,0x0004003d,0x00000028,
	0x000000d2,0x000000bb,0x0004003d,0x00000006,0x000000d3,0x000000c5,0x0007000c,0x00000006,0x000000d5,0x00000001,0x00000028,0x000000d3,0x000000d4,0x00060050,0x00000028,0x000000d6,
	0x000000d5,0x000000d5,0x000000d5,0x00050088,0x00000028,0x000000d7,0x000000d2,0x000000d6,0x0003003e,0x000000d1,0x000000d7,0x0004003d,0x00000006,0x000000d9,0x000000c5,0x0004003d,
	0x00000006,0x000000da,0x000000c2,0x00050088,0x00000006,0x000000db,0x000000d9,0x000000da,0x00050083,0x00000006,0x000000dc,0x0000001b,0x000000db,0x0003003e,0x000000d8,0x000000dc,
	0x00060041,0x0000003a,0x000000dd,0x0000008f,0x000000a0,0x00000039,0x0004003d,0x00000006,0x000000de,0x000000dd,0x000500ba,0x00000017,0x000000df,0x000000de,0x00000016,0x000300f7,
	0x000000e1,0x00000000,0x000400fa,0x000000df,0x000000e0,0x000000e1,0x000200f8,0x000000e0,0x0004003d,0x00000028,0x000000e3,0x000000d1,0x0004007f,0x00000028,0x000000e4,0x000000e3,
	0x00050041,0x0000009a,0x000000e5,0x0000008f,0x0000009d,0x0004003d,0x0000000a,0x000000e6,0x000000e5,0x0008004f,0x00000028,0x000000e7,0x000000e6,0x000000e6,0x00000000,0x00000001,
	0x00000002,0x00050094,0x00000006,0x000000e8,0x000000e4,0x000000e7,0x0003003e,0x000000e2,0x000000e8,0x0004003d,0x00000006,0x000000e9,0x000000e2,0x00060041,0x0000003a,0x000000ea,
	0x0000008f,0x000000a0,0x00000012,0x0004003d,0x00000006,0x000000eb,0x000000ea,0x000500b8,0x00000017,0x000000ec,0x000000e9,0x000000eb,0x000300f7,0x000000ee,0x00000000,0x000400fa,
	0x000000ec,0x000000ed,0x000000ee,0x000200f8,0x000000ed,0x000200f9,0x00000088,0x000200f8,0x000000ee,0x0004003d,0x00000006,0x000000f0,0x000000e2,0x00060041,0x0000003a,0x000000f1,
	0x0000008f,0x000000a0,0x00000012,0x0004003d,0x00000006,0x000000f2,0x000000f1,0x00050083,0x00000006,0x000000f3,0x000000f0,0x000000f2,0x00050088,0x00000006,0x000000f5,0x000000f3,
	0x000000f4,0x0008000c,0x00000006,0x000000f6,0x00000001,0x0000002b,0x000000f5,0x00000023,0x0000001b,0x0004003d,0x00000006,0x000000f7,0x000000d8,0x00050085,0x00000006,0x000000f8,
	0x000000f7,0x000000f6,0x0003003e,0x000000d8,0x000000f8,0x000200f9,0x000000e1,0x000200f8,0x000000e1,0x0004003d,0x00000006,0x000000f9,0x000000a2,0x0004003d,0x00000006,0x000000fa,
	0x000000d8,0x00050085,0x00000006,0x000000fb,0x000000f9,0x000000fa,0x0004003d,0x00000028,0x000000fc,0x00000073,0x0004003d,0x00000028,0x000000fd,0x000000d1,0x00050094,0x00000006,
	0x000000fe,0x000000fc,0x000000fd,0x0007000c,0x00000006,0x000000ff,0x00000001,0x00000028,0x000000fe,0x00000023,0x00050085,0x00000006,0x00000100,0x000000fb,0x000000ff,0x0003003e,
	0x000000aa,0x00000100,0x000200f9,0x000000b0,0x000200f8,0x000000b0,0x0004003d,0x0000000f,0x00000101,0x00000083,0x000500aa,0x00000017,0x00000102,0x00000101,0x00000084,0x000300f7,
	0x00000104,0x00000000,0x000400fa,0x00000102,0x00000103,0x00000104,0x000200f8,0x00000103,0x0004003d,0x00000006,0x00000105,0x00000081,0x0004003d,0x00000006,0x00000106,0x000000aa,
	0x00050085,0x00000006,0x00000107,0x00000106,0x00000105,0x0003003e,0x000000aa,0x00000107,0x000200f9,0x00000104,0x000200f8,0x00000104,0x0004003d,0x00000006,0x00000108,0x000000aa,
	0x0004003d,0x00000006,0x00000109,0x00000078,0x00050081,0x00000006,0x0000010a,0x00000109,0x00000108,0x0003003e,0x00000078,0x0000010a,0x000200f9,0x00000088,0x000200f8,0x00000088,
	0x0004003d,0x0000000f,0x0000010b,0x00000083,0x00050080,0x0000000f,0x0000010c,0x0000010b,0x0000009d,0x0003003e,0x00000083,0x0000010c,0x000200f9,0x00000085,0x000200f8,0x00000087,
	0x0004003d,0x00000006,0x0000010d,0x00000078,0x0008000c,0x00000006,0x0000010f,0x00000001,0x0000002b,0x0000010d,0x00000023,0x0000010e,0x0003003e,0x00000078,0x0000010f,0x0004003d,
	0x0000000a,0x00000113,0x00000112,0x0008004f,0x00000028,0x00000114,0x00000113,0x00000113,0x00000000,0x00000001,0x00000002,0x0004003d,0x00000006,0x00000115,0x00000078,0x0005008e,
	0x00000028,0x00000116,0x00000114,0x00000115,0x00050041,0x00000020,0x00000117,0x00000112,0x0000001f,0x0004003d,0x00000006,0x00000118,0x00000117,0x00050051,0x00000006,0x00000119,
	0x00000116,0x00000000,0x00050051,0x00000006,0x0000011a,0x00000116,0x00000001,0x00050051,0x00000006,0x0000011b,0x00000116,0x00000002,0x00070050,0x0000000a,0x0000011c,0x00000119,
	0x0000011a,0x0000011b,0x00000118,0x0003003e,0x00000111,0x0000011c,0x000100fd,0x00010038,0x00050036,0x00000006,0x00000008,0x00000000,0x00000007,0x000200f8,0x00000009,0x0004003b,
	0x00000029,0x0000002a,0x00000007,0x0004003b,0x00000032,0x00000033,0x00000007,0x0004003b,0x0000003a,0x0000005e,0x00000007,0x0004003b,0x0000003a,0x00000060,0x00000007,0x00060041,
	0x00000013,0x00000014,0x0000000e,0x00000010,0x00000012,0x0004003d,0x00000006,0x00000015,0x00000014,0x000500b8,0x00000017,0x00000018,0x00000015,0x00000016,0x000300f7,0x0000001a,
	0x00000000,0x000400fa,0x00000018,0x00000019,0x0000001a,0x000200f8,0x00000019,0x000200fe,0x0000001b,0x000200f8,0x0000001a,0x00050041,0x00000020,0x00000021,0x0000001e,0x0000001f,
	0x0004003d,0x00000006,0x00000022,0x00000021,0x000500bc,0x00000017,0x00000024,0x00000022,0x00000023,0x000300f7,0x00000026,0x00000000,0x000400fa,0x00000024,0x00000025,0x00000026,
	0x000200f8,0x00000025,0x000200fe,0x0000001b,0x000200f8,0x00000026,0x0004003d,0x0000000a,0x0000002b,0x0000001e,0x0008004f,0x00000028,0x0000002c,0x0000002b,0x0000002b,0x00000000,
	0x00000001,0x00000002,0x00050041,0x00000020,0x0000002d,0x0000001e,0x0000001f,0x0004003d,0x00000006,0x0000002e,0x0000002d,0x00060050,0x00000028,0x0000002f,0x0000002e,0x0000002e,
	0x0000002e,0x00050088,0x00000028,0x00000030,0x0000002c,0x0000002f,0x0003003e,0x0000002a,0x00000030,0x0004003d,0x00000028,0x00000034,0x0000002a,0x0007004f,0x00000031,0x00000035,
	0x00000034,0x00000034,0x00000000,0x00000001,0x0005008e,0x00000031,0x00000036,0x00000035,0x00000016,0x00050050,0x00000031,0x00000037,0x00000016,0x00000016,0x00050081,0x00000031,
	0x00000038,0x00000036,0x00000037,0x0003003e,0x00000033,0x00000038,0x00050041,0x0000003a,0x0000003b,0x00000033,0x00000039,0x0004003d,0x00000006,0x0000003c,0x0000003b,0x000500b8,
	0x00000017,0x0000003d,0x0000003c,0x00000023,0x000400a8,0x00000017,0x0000003e,0x0000003d,0x000300f7,0x00000040,0x00000000,0x000400fa,0x0000003e,0x0000003f,0x00000040,0x000200f8,
	0x0000003f,0x00050041,0x0000003a,0x00000041,0x00000033,0x00000039,0x0004003d,0x00000006,0x00000042,0x00000041,0x000500ba,0x00000017,0x00000043,0x00000042,0x0000001b,0x000200f9,
	0x00000040,0x000200f8,0x00000040,0x000700f5,0x00000017,0x00000044,0x0000003d,0x00000026,0x00000043,0x0000003f,0x000400a8,0x00000017,0x00000045,0x00000044,0x000300f7,0x00000047,
	0x00000000,0x000400fa,0x00000045,0x00000046,0x00000047,0x000200f8,0x00000046,0x00050041,0x0000003a,0x00000048,0x00000033,0x00000012,0x0004003d,0x00000006,0x00000049,0x00000048,
	0x000500b8,0x00000017,0x0000004a,0x00000049,0x00000023,0x000200f9,0x00000047,0x000200f8,0x00000047,0x000700f5,0x00000017,0x0000004b,0x00000044,0x00000040,0x0000004a,0x00000046,
	0x000400a8,0x00000017,0x0000004c,0x0000004b,0x000300f7,0x0000004e,0x00000000,0x000400fa,0x0000004c,0x0000004d,0x0000004e,0x000200f8,0x0000004d,0x00050041,0x0000003a,0x0000004f,
	0x00000033,0x00000012,0x0004003d,0x00000006,0x00000050,0x0000004f,0x000500ba,0x00000017,0x00000051,0x00000050,0x0000001b,0x000200f9,0x0000004e,0x000200f8,0x0000004e,0x000700f5,
	0x00000017,0x00000052,0x0000004b,0x00000047,0x00000051,0x0000004d,0x000400a8,0x00000017,0x00000053,0x00000052,0x000300f7,0x00000055,0x00000000,0x000400fa,0x00000053,0x00000054,
	0x00000055,0x000200f8,0x00000054,0x00050041,0x0000003a,0x00000057,0x0000002a,0x00000056,0x0004003d,0x00000006,0x00000058,0x00000057,0x000500ba,0x00000017,0x00000059,0x00000058,
	0x0000001b,0x000200f9,0x00000055,0x000200f8,0x00000055,0x000700f5,0x00000017,0x0000005a,0x00000052,0x0000004e,0x00000059,0x00000054,0x000300f7,0x0000005c,0x00000000,0x000400fa,
	0x0000005a,0x0000005b,0x0000005c,0x000200f8,0x0000005b,0x000200fe,0x0000001b,0x000200f8,0x0000005c,0x0003003e,0x0000005e,0x0000005f,0x0004003d,0x00000062,0x00000065,0x00000064,
	0x0004003d,0x00000031,0x00000066,0x00000033,0x00050057,0x0000000a,0x00000067,0x00000065,0x00000066,0x00050051,0x00000006,0x00000068,0x00000067,0x00000000,0x0003003e,0x00000060,
	0x00000068,0x00050041,0x0000003a,0x00000069,0x0000002a,0x00000056,0x0004003d,0x00000006,0x0000006a,0x00000069,0x0004003d,0x00000006,0x0000006b,0x0000005e,0x00050083,0x00000006,
	0x0000006c,0x0000006a,0x0000006b,0x0004003d,0x00000006,0x0000006d,0x00000060,0x000500ba,0x00000017,0x0000006e,0x0000006c,0x0000006d,0x000600a9,0x00000006,0x00000070,0x0000006e,
	0x0000006f,0x0000001b,0x000200fe,0x00000070,0x00010038
}, shadowVertCode[] = {
	0x07230203,0x00010000,0x000d000b,0x00000024,0x00000000,0x00020011,0x00000001,0x0006000b,0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x0007000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000000d,0x0000001a,0x00030003,0x00000002,0x000001c2,0x000a0004,0x475f4c47,0x4c474f4f,0x70635f45,0x74735f70,0x5f656c79,
	0x656e696c,0x7269645f,0x69746365,0x00006576,0x00080004,0x475f4c47,0x4c474f4f,0x6e695f45,0x64756c63,0x69645f65,0x74636572,0x00657669,0x00040005,0x00000004,0x6e69616d,0x00000000,
	0x00060005,0x0000000b,0x505f6c67,0x65567265,0x78657472,0x00000000,0x00060006,0x0000000b,0x00000000,0x505f6c67,0x7469736f,0x006e6f69,0x00070006,0x0000000b,0x00000001,0x505f6c67,
	0x746e696f,0x657a6953,0x00000000,0x00070006,0x0000000b,0x00000002,0x435f6c67,0x4470696c,0x61747369,0x0065636e,0x00070006,0x0000000b,0x00000003,0x435f6c67,0x446c6c75,0x61747369,
	0x0065636e,0x00030005,0x0000000d,0x00000000,0x00050005,0x00000011,0x656d6143,0x42556172,0x0000004f,0x00060006,0x00000011,0x00000000,0x77656976,0x6a6f7250,0x00000000,0x00070006,
	0x00000011,0x00000001,0x6867696c,0x65695674,0x6f725077,0x0000006a,0x00070006,0x00000011,0x00000002,0x6867696c,0x736f5074,0x676e6152,0x00000065,0x00080006,0x00000011,0x00000003,
	0x6867696c,0x776f5074,0x68537265,0x776f6461,0x00000000,0x00060006,0x00000011,0x00000004,0x6e656373,0x666e4965,0x0000006f,0x00030005,0x00000013,0x006d6163,0x00040005,0x0000001a,
	0x6f506e69,0x00000073,0x00030047,0x0000000b,0x00000002,0x00050048,0x0000000b,0x00000000,0x0000000b,0x00000000,0x00050048,0x0000000b,0x00000001,0x0000000b,0x00000001,0x00050048,
	0x0000000b,0x00000002,0x0000000b,0x00000003,0x00050048,0x0000000b,0x00000003,0x0000000b,0x00000004,0x00030047,0x00000011,0x00000002,0x00040048,0x00000011,0x00000000,0x00000005,
	0x00050048,0x00000011,0x00000000,0x00000007,0x00000010,0x00050048,0x00000011,0x00000000,0x00000023,0x00000000,0x00040048,0x00000011,0x00000001,0x00000005,0x00050048,0x00000011,
	0x00000001,0x00000007,0x00000010,0x00050048,0x00000011,0x00000001,0x00000023,0x00000040,0x00050048,0x00000011,0x00000002,0x00000023,0x00000080,0x00050048,0x00000011,0x00000003,
	0x00000023,0x00000090,0x00050048,0x00000011,0x00000004,0x00000023,0x000000a0,0x00040047,0x00000013,0x00000021,0x00000000,0x00040047,0x00000013,0x00000022,0x00000000,0x00040047,
	0x0000001a,0x0000001e,0x00000000,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040015,
	0x00000008,0x00000020,0x00000000,0x0004002b,0x00000008,0x00000009,0x00000001,0x0004001c,0x0000000a,0x00000006,0x00000009,0x0006001e,0x0000000b,0x00000007,0x00000006,0x0000000a,
	0x0000000a,0x00040020,0x0000000c,0x00000003,0x0000000b,0x0004003b,0x0000000c,0x0000000d,0x00000003,0x00040015,0x0000000e,0x00000020,0x00000001,0x0004002b,0x0000000e,0x0000000f,
	0x00000000,0x00040018,0x00000010,0x00000007,0x00000004,0x0007001e,0x00000011,0x00000010,0x00000010,0x00000007,0x00000007,0x00000007,0x00040020,0x00000012,0x00000002,0x00000011,
	0x0004003b,0x00000012,0x00000013,0x00000002,0x0004002b,0x0000000e,0x00000014,0x00000001,0x00040020,0x00000015,0x00000002,0x00000010,0x00040017,0x00000018,0x00000006,0x00000003,
	0x00040020,0x00000019,0x00000001,0x00000018,0x0004003b,0x00000019,0x0000001a,0x00000001,0x0004002b,0x00000006,0x0000001c,0x3f800000,0x00040020,0x00000022,0x00000003,0x00000007,
	0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,0x00000005,0x00050041,0x00000015,0x00000016,0x00000013,0x00000014,0x0004003d,0x00000010,0x00000017,0x00000016,
	0x0004003d,0x00000018,0x0000001b,0x0000001a,0x00050051,0x00000006,0x0000001d,0x0000001b,0x00000000,0x00050051,0x00000006,0x0000001e,0x0000001b,0x00000001,0x00050051,0x00000006,
	0x0000001f,0x0000001b,0x00000002,0x00070050,0x00000007,0x00000020,0x0000001d,0x0000001e,0x0000001f,0x0000001c,0x00050091,0x00000007,0x00000021,0x00000017,0x00000020,0x00050041,
	0x00000022,0x00000023,0x0000000d,0x0000000f,0x0003003e,0x00000023,0x00000021,0x000100fd,0x00010038
}, uiVertCode[] = {
	0x07230203,0x00010000,0x000d000b,0x00000030,0x00000000,0x00020011,0x00000001,0x0006000b,0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x0009000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000000b,0x00000023,0x0000002c,0x0000002e,0x00030003,0x00000002,0x000001c2,0x000a0004,0x475f4c47,0x4c474f4f,0x70635f45,
	0x74735f70,0x5f656c79,0x656e696c,0x7269645f,0x69746365,0x00006576,0x00080004,0x475f4c47,0x4c474f4f,0x6e695f45,0x64756c63,0x69645f65,0x74636572,0x00657669,0x00040005,0x00000004,
	0x6e69616d,0x00000000,0x00030005,0x00000009,0x0063646e,0x00040005,0x0000000b,0x6f506e69,0x00000073,0x00030005,0x0000000d,0x00004350,0x00060006,0x0000000d,0x00000000,0x65726373,
	0x69536e65,0x0000657a,0x00030005,0x0000000f,0x00006370,0x00060005,0x00000021,0x505f6c67,0x65567265,0x78657472,0x00000000,0x00060006,0x00000021,0x00000000,0x505f6c67,0x7469736f,
	0x006e6f69,0x00070006,0x00000021,0x00000001,0x505f6c67,0x746e696f,0x657a6953,0x00000000,0x00070006,0x00000021,0x00000002,0x435f6c67,0x4470696c,0x61747369,0x0065636e,0x00070006,
	0x00000021,0x00000003,0x435f6c67,0x446c6c75,0x61747369,0x0065636e,0x00030005,0x00000023,0x00000000,0x00050005,0x0000002c,0x67617266,0x6f6c6f43,0x00000072,0x00040005,0x0000002e,
	0x6f436e69,0x00726f6c,0x00040047,0x0000000b,0x0000001e,0x00000000,0x00030047,0x0000000d,0x00000002,0x00050048,0x0000000d,0x00000000,0x00000023,0x00000000,0x00030047,0x00000021,
	0x00000002,0x00050048,0x00000021,0x00000000,0x0000000b,0x00000000,0x00050048,0x00000021,0x00000001,0x0000000b,0x00000001,0x00050048,0x00000021,0x00000002,0x0000000b,0x00000003,
	0x00050048,0x00000021,0x00000003,0x0000000b,0x00000004,0x00040047,0x0000002c,0x0000001e,0x00000000,0x00040047,0x0000002e,0x0000001e,0x00000001,0x00020013,0x00000002,0x00030021,
	0x00000003,0x00000002,0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,0x00000002,0x00040020,0x00000008,0x00000007,0x00000007,0x00040020,0x0000000a,0x00000001,
	0x00000007,0x0004003b,0x0000000a,0x0000000b,0x00000001,0x0003001e,0x0000000d,0x00000007,0x00040020,0x0000000e,0x00000009,0x0000000d,0x0004003b,0x0000000e,0x0000000f,0x00000009,
	0x00040015,0x00000010,0x00000020,0x00000001,0x0004002b,0x00000010,0x00000011,0x00000000,0x00040020,0x00000012,0x00000009,0x00000007,0x0004002b,0x00000006,0x00000015,0x3f000000,
	0x00040015,0x00000018,0x00000020,0x00000000,0x0004002b,0x00000018,0x00000019,0x00000001,0x00040020,0x0000001a,0x00000007,0x00000006,0x00040017,0x0000001f,0x00000006,0x00000004,
	0x0004001c,0x00000020,0x00000006,0x00000019,0x0006001e,0x00000021,0x0000001f,0x00000006,0x00000020,0x00000020,0x00040020,0x00000022,0x00000003,0x00000021,0x0004003b,0x00000022,
	0x00000023,0x00000003,0x0004002b,0x00000006,0x00000025,0x00000000,0x0004002b,0x00000006,0x00000026,0x3f800000,0x00040020,0x0000002a,0x00000003,0x0000001f,0x0004003b,0x0000002a,
	0x0000002c,0x00000003,0x00040020,0x0000002d,0x00000001,0x0000001f,0x0004003b,0x0000002d,0x0000002e,0x00000001,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,
	0x00000005,0x0004003b,0x00000008,0x00000009,0x00000007,0x0004003d,0x00000007,0x0000000c,0x0000000b,0x00050041,0x00000012,0x00000013,0x0000000f,0x00000011,0x0004003d,0x00000007,
	0x00000014,0x00000013,0x0005008e,0x00000007,0x00000016,0x00000014,0x00000015,0x00050088,0x00000007,0x00000017,0x0000000c,0x00000016,0x0003003e,0x00000009,0x00000017,0x00050041,
	0x0000001a,0x0000001b,0x00000009,0x00000019,0x0004003d,0x00000006,0x0000001c,0x0000001b,0x0004007f,0x00000006,0x0000001d,0x0000001c,0x00050041,0x0000001a,0x0000001e,0x00000009,
	0x00000019,0x0003003e,0x0000001e,0x0000001d,0x0004003d,0x00000007,0x00000024,0x00000009,0x00050051,0x00000006,0x00000027,0x00000024,0x00000000,0x00050051,0x00000006,0x00000028,
	0x00000024,0x00000001,0x00070050,0x0000001f,0x00000029,0x00000027,0x00000028,0x00000025,0x00000026,0x00050041,0x0000002a,0x0000002b,0x00000023,0x00000011,0x0003003e,0x0000002b,
	0x00000029,0x0004003d,0x0000001f,0x0000002f,0x0000002e,0x0003003e,0x0000002c,0x0000002f,0x000100fd,0x00010038
}, uiFragCode[] = {
	0x07230203,0x00010000,0x000d000b,0x0000000d,0x00000000,0x00020011,0x00000001,0x0006000b,0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x0007000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000009,0x0000000b,0x00030010,0x00000004,0x00000007,0x00030003,0x00000002,0x000001c2,0x000a0004,0x475f4c47,0x4c474f4f,
	0x70635f45,0x74735f70,0x5f656c79,0x656e696c,0x7269645f,0x69746365,0x00006576,0x00080004,0x475f4c47,0x4c474f4f,0x6e695f45,0x64756c63,0x69645f65,0x74636572,0x00657669,0x00040005,
	0x00000004,0x6e69616d,0x00000000,0x00050005,0x00000009,0x4374756f,0x726f6c6f,0x00000000,0x00050005,0x0000000b,0x67617266,0x6f6c6f43,0x00000072,0x00040047,0x00000009,0x0000001e,
	0x00000000,0x00040047,0x0000000b,0x0000001e,0x00000000,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,
	0x00000004,0x00040020,0x00000008,0x00000003,0x00000007,0x0004003b,0x00000008,0x00000009,0x00000003,0x00040020,0x0000000a,0x00000001,0x00000007,0x0004003b,0x0000000a,0x0000000b,
	0x00000001,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,0x00000005,0x0004003d,0x00000007,0x0000000c,0x0000000b,0x0003003e,0x00000009,0x0000000c,0x000100fd,
	0x00010038
};
// ========================================================================================================================================================================
// ===== UTILITY ==========================================================================================================================================================
// ========================================================================================================================================================================
static uint32_t findMemoryType(const uint32_t typeFilter, const VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(g_ctx.physicalDevice, &memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) return i;
	return 0;
}
static void createBuffer(const VkDeviceSize size, const VkBufferUsageFlags usage, const VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory) {
	const VkBufferCreateInfo bufferInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	vkCreateBuffer(g_ctx.device, &bufferInfo, NULL, buffer);
	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(g_ctx.device, *buffer, &memReqs);
	const VkMemoryAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memReqs.size,
		.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, properties)
	};
	vkAllocateMemory(g_ctx.device, &allocInfo, NULL, bufferMemory);
	vkBindBufferMemory(g_ctx.device, *buffer, *bufferMemory, 0);
}
static void createImage2D(const uint32_t w, const uint32_t h, const VkFormat format, const VkSampleCountFlagBits samples, const VkImageUsageFlags usage, VkImage* image, VkDeviceMemory* memory) {
	const VkImageCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.extent = {w, h, 1},
		.mipLevels = 1,
		.arrayLayers = 1,
		.format = format,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.usage = usage,
		.samples = samples,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};
	vkCreateImage(g_ctx.device, &info, NULL, image);
	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(g_ctx.device, *image, &memReqs);
	const VkMemoryAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memReqs.size,
		.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};
	vkAllocateMemory(g_ctx.device, &allocInfo, NULL, memory);
	vkBindImageMemory(g_ctx.device, *image, *memory, 0);
}
static void createImageView2D(const VkImage image, const VkFormat format, const VkImageAspectFlags aspect, VkImageView* view) {
	const VkImageViewCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.subresourceRange = { aspect, 0, 1, 0, 1 }
	};
	vkCreateImageView(g_ctx.device, &info, NULL, view);
}
static VkShaderModule createShaderModule(const uint32_t* code, const size_t codeSize) {
	const VkShaderModuleCreateInfo info = { .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, .codeSize = codeSize, .pCode = code };
	VkShaderModule module = VK_NULL_HANDLE;
	vkCreateShaderModule(g_ctx.device, &info, NULL, &module);
	return module;
}
// ========================================================================================================================================================================
// ===== SWAPCHAIN / FRAMEBUFFERS / SHADOW MAP ============================================================================================================================
// ========================================================================================================================================================================
static void createSwapchainAndImageViews() {
	int fbW, fbH;
	glfwGetFramebufferSize(g_ctx.window, &fbW, &fbH);
	while (fbW == 0 || fbH == 0) {
		glfwGetFramebufferSize(g_ctx.window, &fbW, &fbH);
		glfwWaitEvents();
	}
	const VkSwapchainCreateInfoKHR swapchainInfo = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = g_ctx.surface,
		.minImageCount = 2,
		.imageFormat = VK_FORMAT_B8G8R8A8_UNORM,
		.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
		.imageExtent = {(uint32_t)fbW, (uint32_t)fbH},
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = VK_PRESENT_MODE_FIFO_KHR
	};
	vkCreateSwapchainKHR(g_ctx.device, &swapchainInfo, NULL, &g_ctx.swapchain);
	vkGetSwapchainImagesKHR(g_ctx.device, g_ctx.swapchain, &g_ctx.imageCount, NULL);
	g_ctx.swapchainImages = malloc(sizeof(VkImage) * g_ctx.imageCount);
	vkGetSwapchainImagesKHR(g_ctx.device, g_ctx.swapchain, &g_ctx.imageCount, g_ctx.swapchainImages);
	g_ctx.swapchainImageViews = malloc(sizeof(VkImageView) * g_ctx.imageCount);
	for (uint32_t i = 0; i < g_ctx.imageCount; i++) createImageView2D(g_ctx.swapchainImages[i], VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &g_ctx.swapchainImageViews[i]);
}
static void createColorAndDepthResources(const uint32_t w, const uint32_t h) {
	VkSampleCountFlagBits samples = (VkSampleCountFlagBits)g_settings.msaaLevel;
	if (samples == 0) samples = VK_SAMPLE_COUNT_1_BIT;
	createImage2D(w, h, VK_FORMAT_B8G8R8A8_UNORM, samples, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &g_ctx.colorImageMSAA, &g_ctx.colorImageMSAAMemory);
	createImageView2D(g_ctx.colorImageMSAA, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, &g_ctx.colorImageViewMSAA);
	createImage2D(w, h, VK_FORMAT_D32_SFLOAT, samples, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &g_ctx.depthImage, &g_ctx.depthImageMemory);
	createImageView2D(g_ctx.depthImage, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT, &g_ctx.depthImageView);
}
static void createFramebuffers(const uint32_t w, const uint32_t h) {
	g_ctx.swapchainFramebuffers = malloc(sizeof(VkFramebuffer) * g_ctx.imageCount);
	for (uint32_t i = 0; i < g_ctx.imageCount; i++) {
		const VkImageView attachments[3] = { g_ctx.colorImageViewMSAA, g_ctx.depthImageView, g_ctx.swapchainImageViews[i] };
		const VkFramebufferCreateInfo fbInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = g_ctx.renderPass,
			.attachmentCount = 3,
			.pAttachments = attachments,
			.width = w, .height = h, .layers = 1
		};
		vkCreateFramebuffer(g_ctx.device, &fbInfo, NULL, &g_ctx.swapchainFramebuffers[i]);
	}
}
static void cleanupColorDepthAndFramebuffers() {
	vkDestroyImageView(g_ctx.device, g_ctx.colorImageViewMSAA, NULL);
	vkDestroyImage(g_ctx.device, g_ctx.colorImageMSAA, NULL);
	vkFreeMemory(g_ctx.device, g_ctx.colorImageMSAAMemory, NULL);
	vkDestroyImageView(g_ctx.device, g_ctx.depthImageView, NULL);
	vkDestroyImage(g_ctx.device, g_ctx.depthImage, NULL);
	vkFreeMemory(g_ctx.device, g_ctx.depthImageMemory, NULL);
	for (uint32_t i = 0; i < g_ctx.imageCount; i++) {
		vkDestroyFramebuffer(g_ctx.device, g_ctx.swapchainFramebuffers[i], NULL);
		vkDestroyImageView(g_ctx.device, g_ctx.swapchainImageViews[i], NULL);
	}
	free(g_ctx.swapchainFramebuffers);
	free(g_ctx.swapchainImageViews);
	free(g_ctx.swapchainImages);
	vkDestroySwapchainKHR(g_ctx.device, g_ctx.swapchain, NULL);
}
static void recreateSwapchain() {
	vkDeviceWaitIdle(g_ctx.device);
	cleanupColorDepthAndFramebuffers();
	createSwapchainAndImageViews();
	int fbW, fbH;
	glfwGetFramebufferSize(g_ctx.window, &fbW, &fbH);
	createColorAndDepthResources((uint32_t)fbW, (uint32_t)fbH);
	createFramebuffers((uint32_t)fbW, (uint32_t)fbH);
}
static void createShadowResources() {
	createImage2D(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, VK_FORMAT_D32_SFLOAT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, &g_ctx.shadowImage, &g_ctx.shadowImageMemory);
	createImageView2D(g_ctx.shadowImage, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT, &g_ctx.shadowImageView);
	const VkSamplerCreateInfo samplerInfo = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
		.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
		.maxLod = 1.0f
	};
	vkCreateSampler(g_ctx.device, &samplerInfo, NULL, &g_ctx.shadowSampler);
	const VkFramebufferCreateInfo fbInfo = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.renderPass = g_ctx.shadowRenderPass,
		.attachmentCount = 1,
		.pAttachments = &g_ctx.shadowImageView,
		.width = SHADOW_MAP_SIZE, .height = SHADOW_MAP_SIZE, .layers = 1
	};
	vkCreateFramebuffer(g_ctx.device, &fbInfo, NULL, &g_ctx.shadowFramebuffer);
}
// ========================================================================================================================================================================
// ===== RENDER PASSES ====================================================================================================================================================
// ========================================================================================================================================================================
static void createMainRenderPass() {
	const VkAttachmentDescription colorAttachment = {
		.format = VK_FORMAT_B8G8R8A8_UNORM, .samples = (VkSampleCountFlagBits)g_settings.msaaLevel,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	};
	const VkAttachmentDescription depthAttachment = {
		.format = VK_FORMAT_D32_SFLOAT, .samples = (VkSampleCountFlagBits)g_settings.msaaLevel,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};
	const VkAttachmentDescription resolveAttachment = {
		.format = VK_FORMAT_B8G8R8A8_UNORM, .samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};
	const VkAttachmentReference colorRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
	depthRef = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL },
	resolveRef = { 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	const VkSubpassDescription subpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1, .pColorAttachments = &colorRef,
		.pResolveAttachments = &resolveRef, .pDepthStencilAttachment = &depthRef
	};
	const VkSubpassDependency dependency = {
		.srcSubpass = VK_SUBPASS_EXTERNAL, .dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.srcAccessMask = 0,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
	};
	const VkAttachmentDescription attachments[3] = { colorAttachment, depthAttachment, resolveAttachment };
	const VkRenderPassCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 3, .pAttachments = attachments,
		.subpassCount = 1, .pSubpasses = &subpass,
		.dependencyCount = 1, .pDependencies = &dependency
	};
	vkCreateRenderPass(g_ctx.device, &info, NULL, &g_ctx.renderPass);
}
static void createShadowRenderPass() {
	const VkAttachmentDescription depthAttachment = {
		.format = VK_FORMAT_D32_SFLOAT, .samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
	};
	const VkAttachmentReference depthRef = { 0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
	const VkSubpassDescription subpass = { .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS, .pDepthStencilAttachment = &depthRef };
	const VkSubpassDependency dependencies[2] = {
		{
			.srcSubpass = VK_SUBPASS_EXTERNAL, .dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, .srcAccessMask = VK_ACCESS_SHADER_READ_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
		},
		{
			.srcSubpass = 0, .dstSubpass = VK_SUBPASS_EXTERNAL,
			.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, .dstAccessMask = VK_ACCESS_SHADER_READ_BIT
		}
	};
	const VkRenderPassCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1, .pAttachments = &depthAttachment,
		.subpassCount = 1, .pSubpasses = &subpass,
		.dependencyCount = 2, .pDependencies = dependencies
	};
	vkCreateRenderPass(g_ctx.device, &info, NULL, &g_ctx.shadowRenderPass);
}
// ========================================================================================================================================================================
// ===== DESCRIPTORS / UBO ================================================================================================================================================
// ========================================================================================================================================================================
static void createDescriptorsAndUbo() {
	createBuffer(sizeof(CameraUBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g_ctx.uboBuffer, &g_ctx.uboBufferMemory);
	vkMapMemory(g_ctx.device, g_ctx.uboBufferMemory, 0, sizeof(CameraUBO), 0, &g_ctx.mappedUbo);
	createBuffer(sizeof(GPULight) * MAX_LIGHTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g_ctx.lightsBuffer, &g_ctx.lightsBufferMemory);
	vkMapMemory(g_ctx.device, g_ctx.lightsBufferMemory, 0, sizeof(GPULight) * MAX_LIGHTS, 0, &g_ctx.mappedLights);
	const VkDescriptorSetLayoutBinding bindings[3] = {
		{ .binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT },
		{ .binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT },
		{ .binding = 2, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT }
	};
	const VkDescriptorSetLayoutCreateInfo layoutInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, .bindingCount = 3, .pBindings = bindings };
	vkCreateDescriptorSetLayout(g_ctx.device, &layoutInfo, NULL, &g_ctx.descriptorSetLayout);
	const VkDescriptorPoolSize poolSizes[3] = {
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 }
	};
	const VkDescriptorPoolCreateInfo poolInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, .maxSets = 1, .poolSizeCount = 3, .pPoolSizes = poolSizes };
	vkCreateDescriptorPool(g_ctx.device, &poolInfo, NULL, &g_ctx.descriptorPool);
	const VkDescriptorSetAllocateInfo allocInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, .descriptorPool = g_ctx.descriptorPool, .descriptorSetCount = 1, .pSetLayouts = &g_ctx.descriptorSetLayout };
	vkAllocateDescriptorSets(g_ctx.device, &allocInfo, &g_ctx.descriptorSet);
	const VkDescriptorBufferInfo bufferInfo = { .buffer = g_ctx.uboBuffer, .offset = 0, .range = sizeof(CameraUBO) };
	const VkDescriptorBufferInfo lightsInfo = { .buffer = g_ctx.lightsBuffer, .offset = 0, .range = sizeof(GPULight) * MAX_LIGHTS };
	const VkDescriptorImageInfo imageInfo = { .sampler = g_ctx.shadowSampler, .imageView = g_ctx.shadowImageView, .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL };
	const VkWriteDescriptorSet writes[3] = {
		{ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .dstSet = g_ctx.descriptorSet, .dstBinding = 0, .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .pBufferInfo = &bufferInfo },
		{ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .dstSet = g_ctx.descriptorSet, .dstBinding = 1, .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .pImageInfo = &imageInfo },
		{ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .dstSet = g_ctx.descriptorSet, .dstBinding = 2, .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .pBufferInfo = &lightsInfo }
	};
	vkUpdateDescriptorSets(g_ctx.device, 3, writes, 0, NULL);
}
// ========================================================================================================================================================================
// ===== PIPELINES ========================================================================================================================================================
// ========================================================================================================================================================================
static void createPipelines() {
	const VkPipelineLayoutCreateInfo layoutInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, .setLayoutCount = 1, .pSetLayouts = &g_ctx.descriptorSetLayout };
	vkCreatePipelineLayout(g_ctx.device, &layoutInfo, NULL, &g_ctx.pipelineLayout);

	const VkVertexInputBindingDescription bindingDesc = { .binding = 0, .stride = sizeof(QGPU_Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX };
	const VkVertexInputAttributeDescription attribDescs[3] = {
		{ .binding = 0, .location = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(QGPU_Vertex, pos) },
		{ .binding = 0, .location = 1, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(QGPU_Vertex, normal) },
		{ .binding = 0, .location = 2, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(QGPU_Vertex, color) }
	};
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1, .pVertexBindingDescriptions = &bindingDesc,
		.vertexAttributeDescriptionCount = 3, .pVertexAttributeDescriptions = attribDescs
	};
	const VkPipelineInputAssemblyStateCreateInfo inputAssembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };
	const VkPipelineViewportStateCreateInfo viewportState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .viewportCount = 1, .scissorCount = 1 };
	const VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	const VkPipelineDynamicStateCreateInfo dynamicState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, .dynamicStateCount = 2, .pDynamicStates = dynamicStates };

	VkShaderModule vertModule = createShaderModule(mainVertCode, sizeof(mainVertCode)), fragModule = createShaderModule(mainFragCode, sizeof(mainFragCode));
	const VkPipelineShaderStageCreateInfo mainStages[2] = {
		{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = vertModule, .pName = "main" },
		{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = fragModule, .pName = "main" }
	};
	const VkPipelineRasterizationStateCreateInfo mainRasterizer = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.lineWidth = 1.0f,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE
	};
	const VkPipelineMultisampleStateCreateInfo multisampling = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = (VkSampleCountFlagBits)g_settings.msaaLevel,
		.sampleShadingEnable = g_settings.msaaLevel > 1 ? VK_TRUE : VK_FALSE, .minSampleShading = 0.2f
	};
	const VkPipelineDepthStencilStateCreateInfo mainDepthStencil = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS
	};
	const VkPipelineColorBlendAttachmentState colorBlendAttachment = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, .colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, .alphaBlendOp = VK_BLEND_OP_ADD
	};
	const VkPipelineColorBlendStateCreateInfo colorBlending = { .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, .attachmentCount = 1, .pAttachments = &colorBlendAttachment };
	const VkGraphicsPipelineCreateInfo mainPipelineInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2, .pStages = mainStages,
		.pVertexInputState = &vertexInputInfo, .pInputAssemblyState = &inputAssembly, .pViewportState = &viewportState,
		.pRasterizationState = &mainRasterizer, .pMultisampleState = &multisampling, .pDepthStencilState = &mainDepthStencil,
		.pColorBlendState = &colorBlending, .pDynamicState = &dynamicState,
		.layout = g_ctx.pipelineLayout, .renderPass = g_ctx.renderPass, .subpass = 0
	};
	vkCreateGraphicsPipelines(g_ctx.device, VK_NULL_HANDLE, 1, &mainPipelineInfo, NULL, &g_ctx.graphicsPipeline);
	vkDestroyShaderModule(g_ctx.device, fragModule, NULL);
	vkDestroyShaderModule(g_ctx.device, vertModule, NULL);

	VkShaderModule shadowVertModule = createShaderModule(shadowVertCode, sizeof(shadowVertCode));
	const VkPipelineShaderStageCreateInfo shadowStages[1] = {{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = shadowVertModule,
		.pName = "main"
	}};
	const VkPipelineRasterizationStateCreateInfo shadowRasterizer = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, .lineWidth = 1.0f,
		.cullMode = VK_CULL_MODE_FRONT_BIT, .frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_TRUE, .depthBiasConstantFactor = 1.0f, .depthBiasSlopeFactor = 1.5f, .depthBiasClamp = 0.005f
	};
	const VkPipelineMultisampleStateCreateInfo shadowMultisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT };
	const VkPipelineDepthStencilStateCreateInfo shadowDepthStencil = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS
	};
	const VkPipelineColorBlendStateCreateInfo shadowColorBlending = { .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, .attachmentCount = 0 };
	const VkGraphicsPipelineCreateInfo shadowPipelineInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 1, .pStages = shadowStages,
		.pVertexInputState = &vertexInputInfo, .pInputAssemblyState = &inputAssembly, .pViewportState = &viewportState,
		.pRasterizationState = &shadowRasterizer, .pMultisampleState = &shadowMultisampling, .pDepthStencilState = &shadowDepthStencil,
		.pColorBlendState = &shadowColorBlending, .pDynamicState = &dynamicState,
		.layout = g_ctx.pipelineLayout, .renderPass = g_ctx.shadowRenderPass, .subpass = 0
	};
	vkCreateGraphicsPipelines(g_ctx.device, VK_NULL_HANDLE, 1, &shadowPipelineInfo, NULL, &g_ctx.shadowPipeline);
	vkDestroyShaderModule(g_ctx.device, shadowVertModule, NULL);
}
static void createUIPipeline() {
	const VkPushConstantRange pcRange = { .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, .offset = 0, .size = sizeof(float) * 2 };
	const VkPipelineLayoutCreateInfo uiLayoutInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, .pushConstantRangeCount = 1, .pPushConstantRanges = &pcRange };
	vkCreatePipelineLayout(g_ctx.device, &uiLayoutInfo, NULL, &g_ctx.uiPipelineLayout);

	const VkVertexInputBindingDescription bindingDesc = { .binding = 0, .stride = sizeof(QGPU_Vertex2D), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX };
	const VkVertexInputAttributeDescription attribDescs[2] = {
		{ .binding = 0, .location = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(QGPU_Vertex2D, pos) },
		{ .binding = 0, .location = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(QGPU_Vertex2D, color) }
	};
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1, .pVertexBindingDescriptions = &bindingDesc,
		.vertexAttributeDescriptionCount = 2, .pVertexAttributeDescriptions = attribDescs
	};
	const VkPipelineInputAssemblyStateCreateInfo inputAssembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };
	const VkPipelineViewportStateCreateInfo viewportState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .viewportCount = 1, .scissorCount = 1 };
	const VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	const VkPipelineDynamicStateCreateInfo dynamicState = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, .dynamicStateCount = 2, .pDynamicStates = dynamicStates };
	const VkPipelineRasterizationStateCreateInfo rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, .lineWidth = 1.0f, .cullMode = VK_CULL_MODE_NONE };
	const VkPipelineMultisampleStateCreateInfo multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, .rasterizationSamples = (VkSampleCountFlagBits)g_settings.msaaLevel };
	const VkPipelineDepthStencilStateCreateInfo depthStencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, .depthTestEnable = VK_FALSE, .depthWriteEnable = VK_FALSE };
	const VkPipelineColorBlendAttachmentState colorBlendAttachment = {
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, .colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, .alphaBlendOp = VK_BLEND_OP_ADD
	};
	const VkPipelineColorBlendStateCreateInfo colorBlending = { .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, .attachmentCount = 1, .pAttachments = &colorBlendAttachment };

	VkShaderModule vertModule = createShaderModule(uiVertCode, sizeof(uiVertCode)), fragModule = createShaderModule(uiFragCode, sizeof(uiFragCode));
	const VkPipelineShaderStageCreateInfo stages[2] = {
		{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = vertModule, .pName = "main" },
		{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = fragModule, .pName = "main" }
	};
	const VkGraphicsPipelineCreateInfo pipelineInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2, .pStages = stages,
		.pVertexInputState = &vertexInputInfo, .pInputAssemblyState = &inputAssembly, .pViewportState = &viewportState,
		.pRasterizationState = &rasterizer, .pMultisampleState = &multisampling, .pDepthStencilState = &depthStencil,
		.pColorBlendState = &colorBlending, .pDynamicState = &dynamicState,
		.layout = g_ctx.uiPipelineLayout, .renderPass = g_ctx.renderPass, .subpass = 0
	};
	vkCreateGraphicsPipelines(g_ctx.device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &g_ctx.uiPipeline);
	vkDestroyShaderModule(g_ctx.device, fragModule, NULL);
	vkDestroyShaderModule(g_ctx.device, vertModule, NULL);
}
void qgSetGraphicsSetting(const uint8_t setting, const uint8_t value) {
	switch (setting) {
		case QGPU_SETTINGS_AMBIENT_OCCLUSION: g_settings.ambientOcclusion = value; break;
		case QGPU_SETTINGS_MSAA_LEVEL:
			if (inInit && g_settings.msaaLevel != value && (value == 1 || value == 2 || value == 4 || value == 8)) g_settings.msaaLevel = value;
			break;
		case QGPU_SETTINGS_SHADOWS: g_settings.shadows = value; break;
	}
}
// ========================================================================================================================================================================
// ===== CAMERA ===========================================================================================================================================================
// ========================================================================================================================================================================
void qgSetCamera(const Vector3 position, const Vector3 target, const float fovDegrees) {
	camPos = position;
	camTarget = target;
	camFovDeg = fovDegrees;
}
void qgSetCameraUp(const Vector3 up) { camUp = up; }
void qgSetCameraClip(const float nearZ, const float farZ) { camNear = nearZ; camFar = farZ; }
Vector3 qgGetCameraPosition() { return camPos; }
// ========================================================================================================================================================================
// ===== INIT =============================================================================================================================================================
// ========================================================================================================================================================================
extern void qgAddRect(const Vector2 position, const Vector2 size, const ColorRGBA color);
void qgAqfDraw(const float x, const float y, const float sx, const float sy, const float r, const float g, const float b, const float a) { qgAddRect((Vector2){x, y}, (Vector2){sx, sy}, (ColorRGBA){r,g,b,a}); }
int qgpuCreate(const uint width, const uint height, const char* title, void (*initFunc)(), void (*updateFunc)()) {
	if (!glfwInit()) return 1;
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	g_ctx.window = glfwCreateWindow(width, height, title, NULL, NULL);
	glfwSetFramebufferSizeCallback(g_ctx.window, onFramebufferResize);
	glfwSwapInterval(0);

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	const VkInstanceCreateInfo instanceInfo = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, .enabledExtensionCount = glfwExtensionCount, .ppEnabledExtensionNames = glfwExtensions };
	vkCreateInstance(&instanceInfo, NULL, &g_ctx.instance);
	glfwCreateWindowSurface(g_ctx.instance, g_ctx.window, NULL, &g_ctx.surface);

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(g_ctx.instance, &deviceCount, NULL);
	VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
	vkEnumeratePhysicalDevices(g_ctx.instance, &deviceCount, devices);
	g_ctx.physicalDevice = devices[0];
	free(devices);

	const float queuePriority = 1.0f;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(g_ctx.physicalDevice, &queueFamilyCount, NULL);
	VkQueueFamilyProperties* queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(g_ctx.physicalDevice, &queueFamilyCount, queueFamilies);
	uint32_t graphicsQueueFamilyIndex = UINT32_MAX;
	for (uint32_t i = 0; i < queueFamilyCount; i++) if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) { graphicsQueueFamilyIndex = i; break; }
	free(queueFamilies);
	g_ctx.graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;

	const VkDeviceQueueCreateInfo queueCreateInfo = { .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueFamilyIndex = graphicsQueueFamilyIndex, .queueCount = 1, .pQueuePriorities = &queuePriority };
	const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	const VkDeviceCreateInfo deviceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queueCreateInfo,
		.enabledExtensionCount = 1,
		.ppEnabledExtensionNames = deviceExtensions
	};
	vkCreateDevice(g_ctx.physicalDevice, &deviceCreateInfo, NULL, &g_ctx.device);
	vkGetDeviceQueue(g_ctx.device, 0, 0, &g_ctx.graphicsQueue);

	qgSetStyle(BOLD);
	if (_showBanner) printBanner();
	if (_madeWith) printMadeWith();
	if (_showInfo) printInfo();
	if (_showColors) printColors();
	qgSetStyle(REGULAR);
	putchar('\n');

	inInit = 1;
	qfSetDrawRect(0,0, QFONT_Y_UP, qgAqfDraw);
	qfInit();
	if (initFunc) initFunc();
	inInit = 0;

	createSwapchainAndImageViews();
	int fbW, fbH;
	glfwGetFramebufferSize(g_ctx.window, &fbW, &fbH);
	createColorAndDepthResources((uint32_t)fbW, (uint32_t)fbH);
	createMainRenderPass();
	createShadowRenderPass();
	createShadowResources();
	createFramebuffers((uint32_t)fbW, (uint32_t)fbH);
	createDescriptorsAndUbo();
	createPipelines();
	createUIPipeline();

	const VkCommandPoolCreateInfo poolInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, .queueFamilyIndex = graphicsQueueFamilyIndex, .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT };
	vkCreateCommandPool(g_ctx.device, &poolInfo, NULL, &g_ctx.commandPool);
	const VkCommandBufferAllocateInfo cmdAllocInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .commandPool = g_ctx.commandPool, .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, .commandBufferCount = 1 };
	vkAllocateCommandBuffers(g_ctx.device, &cmdAllocInfo, &g_ctx.currentCmd);

	createBuffer(sizeof(QGPU_Vertex) * MAX_VERTICES, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g_ctx.vertexBuffer, &g_ctx.vertexBufferMemory);
	createBuffer(sizeof(uint32_t) * MAX_VERTICES * 3, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g_ctx.indexBuffer, &g_ctx.indexBufferMemory);
	vkMapMemory(g_ctx.device, g_ctx.vertexBufferMemory, 0, sizeof(QGPU_Vertex) * MAX_VERTICES, 0, &g_ctx.mappedVertexBuffer);
	vkMapMemory(g_ctx.device, g_ctx.indexBufferMemory, 0, sizeof(uint32_t) * MAX_VERTICES * 3, 0, &g_ctx.mappedIndexBuffer);

	createBuffer(sizeof(QGPU_Vertex2D) * MAX_UI_VERTICES, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g_ctx.uiVertexBuffer, &g_ctx.uiVertexBufferMemory);
	createBuffer(sizeof(uint32_t) * MAX_UI_VERTICES * 3, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g_ctx.uiIndexBuffer, &g_ctx.uiIndexBufferMemory);
	vkMapMemory(g_ctx.device, g_ctx.uiVertexBufferMemory, 0, sizeof(QGPU_Vertex2D) * MAX_UI_VERTICES, 0, &g_ctx.mappedUIVertexBuffer);
	vkMapMemory(g_ctx.device, g_ctx.uiIndexBufferMemory, 0, sizeof(uint32_t) * MAX_UI_VERTICES * 3, 0, &g_ctx.mappedUIIndexBuffer);

	const VkSemaphoreCreateInfo semaphoreInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	vkCreateSemaphore(g_ctx.device, &semaphoreInfo, NULL, &g_ctx.imageAvailableSemaphore);
	vkCreateSemaphore(g_ctx.device, &semaphoreInfo, NULL, &g_ctx.renderFinishedSemaphore);
	memset(g_ctx.lastKeyState, 0, sizeof(g_ctx.lastKeyState));
	memset(g_ctx.lastMouseState, 0, sizeof(g_ctx.lastMouseState));

	while (!glfwWindowShouldClose(g_ctx.window)) {
		for (uint16_t i = 0; i < GLFW_KEY_LAST; i++) g_ctx.lastKeyState[i] = glfwGetKey(g_ctx.window, i);
		for (uint8_t i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++) g_ctx.lastMouseState[i] = glfwGetMouseButton(g_ctx.window, i);
		glfwPollEvents();

		uint32_t imageIndex;
		const VkResult acquireResult = vkAcquireNextImageKHR(g_ctx.device, g_ctx.swapchain, UINT64_MAX, g_ctx.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
		if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR || acquireResult == VK_SUBOPTIMAL_KHR || framebufferResized) { framebufferResized = 0; recreateSwapchain(); continue; }
		else if (acquireResult != VK_SUCCESS) qgError("Failed to acquire swap chain image!\n");

		g_ctx.currentIOffset = 0;
		g_ctx.currentUIIOffset = 0;
		uiTriangleCount = 0;
		currentUILayer = 0;
		lightCount = 0;
		qgResetRotation();

		vkResetCommandBuffer(g_ctx.currentCmd, 0);
		const VkCommandBufferBeginInfo beginInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT };
		vkBeginCommandBuffer(g_ctx.currentCmd, &beginInfo);

		if (updateFunc) updateFunc();
		g_ctx.currentVOffset = 0;
		g_ctx.currentUIVOffset = 0;

		if (uiTriangleCount > 0) {
			qsort(uiTriangles, uiTriangleCount, sizeof(UITriangle2D), compareUITriangle);
			QGPU_Vertex2D* vb = (QGPU_Vertex2D*)g_ctx.mappedUIVertexBuffer;
			uint32_t* ib = (uint32_t*)g_ctx.mappedUIIndexBuffer;
			for (uint32_t i = 0; i < uiTriangleCount; i++) {
				const UITriangle2D* t = &uiTriangles[i];
				const uint32_t base = g_ctx.currentUIVOffset;
				for (int v = 0; v < 3; v++) {
					vb[base + v].pos[0] = t->p[v][0];
					vb[base + v].pos[1] = t->p[v][1];
					vb[base + v].color[0] = t->color[0];
					vb[base + v].color[1] = t->color[1];
					vb[base + v].color[2] = t->color[2];
					vb[base + v].color[3] = t->color[3];
				}
				ib[g_ctx.currentUIIOffset + 0] = base + 0;
				ib[g_ctx.currentUIIOffset + 1] = base + 1;
				ib[g_ctx.currentUIIOffset + 2] = base + 2;
				g_ctx.currentUIIOffset += 3;
				g_ctx.currentUIVOffset += 3;
			}
		}

		int curW, curH;
		glfwGetFramebufferSize(g_ctx.window, &curW, &curH);
		float aspect = curH > 0 ? (float)curW / (float)curH : 1.0f;
		CameraUBO ubo;
		float view[16], proj[16];
		mat4LookAt(view, camPos, camTarget, camUp);
		mat4Perspective(proj, camFovDeg * (PI / 180.0f), aspect, camNear, camFar);
		mat4Multiply(ubo.viewProj, proj, view);

		uint8_t haveShadowLight = (lightCount > 0 && g_settings.shadows);
		if (haveShadowLight) {
			const GPULight* L0 = &lights[0];
			const Vector3 lpos = { L0->posRange[0], L0->posRange[1], L0->posRange[2] }, ldir = { L0->dirPower[0], L0->dirPower[1], L0->dirPower[2] };
			const float lrange = L0->posRange[3], farZ = lrange > 1.0f ? lrange : 100.0f;
			float lightView[16], lightProj[16];
			if (L0->typeAngle[0] == QGPU_LIGHT_DIRECTIONAL) {
				const float extent = 20.0f;
				mat4LookAt(lightView, v3Sub(camTarget, v3Scale(ldir, extent * 2.0f)), camTarget, (Vector3){0.0f, 1.0f, 0.0f});
				mat4Ortho(lightProj, -extent, extent, -extent, extent, 0.1f, extent * 4.0f);
			} else if (L0->typeAngle[0] == QGPU_LIGHT_SPOT) {
				mat4LookAt(lightView, lpos, v3Add(lpos, ldir), (Vector3){0.0f, 1.0f, 0.0f});
				mat4Perspective(lightProj, qclampf(L0->typeAngle[2] * 2.0f, 2.0f, 170.0f) * (PI / 180.0f), 1.0f, 0.1f, farZ);
			} else {
				mat4LookAt(lightView, lpos, camTarget, (Vector3){0.0f, 1.0f, 0.0f});
				mat4Perspective(lightProj, 100.0f * (PI / 180.0f), 1.0f, 0.1f, farZ);
			}
			mat4Multiply(ubo.lightViewProj, lightProj, lightView);
			ubo.lightPosRange[0] = lpos.x; ubo.lightPosRange[1] = lpos.y; ubo.lightPosRange[2] = lpos.z; ubo.lightPosRange[3] = lrange;
			ubo.lightPowerShadow[0] = L0->dirPower[3]; ubo.lightPowerShadow[1] = 1.0f; ubo.lightPowerShadow[2] = 0.1f; ubo.lightPowerShadow[3] = lrange;
		} else {
			mat4Identity(ubo.lightViewProj);
			ubo.lightPosRange[0] = ubo.lightPosRange[1] = ubo.lightPosRange[2] = ubo.lightPosRange[3] = 0.0f;
			ubo.lightPowerShadow[0] = 0.0f; ubo.lightPowerShadow[1] = 0.0f; ubo.lightPowerShadow[2] = 0.0f; ubo.lightPowerShadow[3] = 0.0f;
		}
		ubo.sceneInfo[0] = g_settings.ambientOcclusion ? 0.05f : 0.2f;
		ubo.sceneInfo[1] = (float)lightCount;
		ubo.sceneInfo[2] = ubo.sceneInfo[3] = 0.0f;
		if (lightCount > 0) memcpy(g_ctx.mappedLights, lights, sizeof(GPULight) * lightCount);
		memcpy(g_ctx.mappedUbo, &ubo, sizeof(CameraUBO));

		if (haveShadowLight && g_ctx.currentIOffset > 0) {
			const VkClearValue shadowClear = { .depthStencil = {1.0f, 0} };
			const VkRenderPassBeginInfo shadowPassInfo = {
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, .renderPass = g_ctx.shadowRenderPass, .framebuffer = g_ctx.shadowFramebuffer,
				.renderArea = {{0, 0}, {SHADOW_MAP_SIZE, SHADOW_MAP_SIZE}}, .clearValueCount = 1, .pClearValues = &shadowClear
			};
			vkCmdBeginRenderPass(g_ctx.currentCmd, &shadowPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(g_ctx.currentCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g_ctx.shadowPipeline);
			vkCmdBindDescriptorSets(g_ctx.currentCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g_ctx.pipelineLayout, 0, 1, &g_ctx.descriptorSet, 0, NULL);
			const VkViewport shadowViewport = {0.0f, 0.0f, (float)SHADOW_MAP_SIZE, (float)SHADOW_MAP_SIZE, 0.0f, 1.0f};
			const VkRect2D shadowScissor = {{0, 0}, {SHADOW_MAP_SIZE, SHADOW_MAP_SIZE}};
			vkCmdSetViewport(g_ctx.currentCmd, 0, 1, &shadowViewport);
			vkCmdSetScissor(g_ctx.currentCmd, 0, 1, &shadowScissor);
			const VkDeviceSize offsets[] = {0};
			vkCmdBindVertexBuffers(g_ctx.currentCmd, 0, 1, &g_ctx.vertexBuffer, offsets);
			vkCmdBindIndexBuffer(g_ctx.currentCmd, g_ctx.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(g_ctx.currentCmd, g_ctx.currentIOffset, 1, 0, 0, 0);
			vkCmdEndRenderPass(g_ctx.currentCmd);
		}

		const VkClearValue clearValues[2] = { {{{backgroundClr.r, backgroundClr.g, backgroundClr.b, 1.0f}}}, {.depthStencil = {1.0f, 0}} };
		const VkRenderPassBeginInfo renderPassInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, .renderPass = g_ctx.renderPass, .framebuffer = g_ctx.swapchainFramebuffers[imageIndex],
			.renderArea = {{0, 0}, {(uint32_t)curW, (uint32_t)curH}}, .clearValueCount = 2, .pClearValues = clearValues
		};
		vkCmdBeginRenderPass(g_ctx.currentCmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(g_ctx.currentCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g_ctx.graphicsPipeline);
		vkCmdBindDescriptorSets(g_ctx.currentCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g_ctx.pipelineLayout, 0, 1, &g_ctx.descriptorSet, 0, NULL);
		const VkViewport viewport = {0.0f, 0.0f, (float)curW, (float)curH, 0.0f, 1.0f};
		const VkRect2D scissor = {{0, 0}, {(uint32_t)curW, (uint32_t)curH}};
		vkCmdSetViewport(g_ctx.currentCmd, 0, 1, &viewport);
		vkCmdSetScissor(g_ctx.currentCmd, 0, 1, &scissor);
		const VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(g_ctx.currentCmd, 0, 1, &g_ctx.vertexBuffer, offsets);
		vkCmdBindIndexBuffer(g_ctx.currentCmd, g_ctx.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		if (g_ctx.currentIOffset > 0) vkCmdDrawIndexed(g_ctx.currentCmd, g_ctx.currentIOffset, 1, 0, 0, 0);
		if (g_ctx.currentUIIOffset > 0) {
			vkCmdBindPipeline(g_ctx.currentCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g_ctx.uiPipeline);
			const float screenSize[2] = { (float)curW, (float)curH };
			vkCmdPushConstants(g_ctx.currentCmd, g_ctx.uiPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(screenSize), screenSize);
			const VkDeviceSize uiOffsets[] = {0};
			vkCmdBindVertexBuffers(g_ctx.currentCmd, 0, 1, &g_ctx.uiVertexBuffer, uiOffsets);
			vkCmdBindIndexBuffer(g_ctx.currentCmd, g_ctx.uiIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(g_ctx.currentCmd, g_ctx.currentUIIOffset, 1, 0, 0, 0);
		}
		vkCmdEndRenderPass(g_ctx.currentCmd);
		vkEndCommandBuffer(g_ctx.currentCmd);

		const VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		const VkSubmitInfo submitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1, .pWaitSemaphores = &g_ctx.imageAvailableSemaphore, .pWaitDstStageMask = waitStages,
			.commandBufferCount = 1, .pCommandBuffers = &g_ctx.currentCmd,
			.signalSemaphoreCount = 1, .pSignalSemaphores = &g_ctx.renderFinishedSemaphore
		};
		if (vkQueueSubmit(g_ctx.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) printf("Queue submit error!\n");
		const VkPresentInfoKHR presentInfo = {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1, .pWaitSemaphores = &g_ctx.renderFinishedSemaphore,
			.swapchainCount = 1, .pSwapchains = &g_ctx.swapchain, .pImageIndices = &imageIndex
		};
		const VkResult presentResult = vkQueuePresentKHR(g_ctx.graphicsQueue, &presentInfo);
		vkDeviceWaitIdle(g_ctx.device);
		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || framebufferResized) { framebufferResized = 0; recreateSwapchain(); }

		const double currentTime = glfwGetTime();
		frameCount++;
		if (currentTime - lastTime >= 0.5) {
			currentFPS = (float)frameCount / (float)(currentTime - lastTime);
			frameCount = 0;
			lastTime = currentTime;
		}
	}

	vkDeviceWaitIdle(g_ctx.device);
	vkUnmapMemory(g_ctx.device, g_ctx.vertexBufferMemory);
	vkUnmapMemory(g_ctx.device, g_ctx.indexBufferMemory);
	vkUnmapMemory(g_ctx.device, g_ctx.uboBufferMemory);
	vkUnmapMemory(g_ctx.device, g_ctx.lightsBufferMemory);
	vkDestroyBuffer(g_ctx.device, g_ctx.lightsBuffer, NULL);
	vkFreeMemory(g_ctx.device, g_ctx.lightsBufferMemory, NULL);
	vkUnmapMemory(g_ctx.device, g_ctx.uiVertexBufferMemory);
	vkUnmapMemory(g_ctx.device, g_ctx.uiIndexBufferMemory);
	vkDestroySemaphore(g_ctx.device, g_ctx.renderFinishedSemaphore, NULL);
	vkDestroySemaphore(g_ctx.device, g_ctx.imageAvailableSemaphore, NULL);
	vkDestroyBuffer(g_ctx.device, g_ctx.indexBuffer, NULL);
	vkFreeMemory(g_ctx.device, g_ctx.indexBufferMemory, NULL);
	vkDestroyBuffer(g_ctx.device, g_ctx.vertexBuffer, NULL);
	vkFreeMemory(g_ctx.device, g_ctx.vertexBufferMemory, NULL);
	vkDestroyBuffer(g_ctx.device, g_ctx.uiIndexBuffer, NULL);
	vkFreeMemory(g_ctx.device, g_ctx.uiIndexBufferMemory, NULL);
	vkDestroyBuffer(g_ctx.device, g_ctx.uiVertexBuffer, NULL);
	vkFreeMemory(g_ctx.device, g_ctx.uiVertexBufferMemory, NULL);
	vkDestroyBuffer(g_ctx.device, g_ctx.uboBuffer, NULL);
	vkFreeMemory(g_ctx.device, g_ctx.uboBufferMemory, NULL);
	vkDestroyCommandPool(g_ctx.device, g_ctx.commandPool, NULL);
	vkDestroyDescriptorPool(g_ctx.device, g_ctx.descriptorPool, NULL);
	vkDestroyDescriptorSetLayout(g_ctx.device, g_ctx.descriptorSetLayout, NULL);
	vkDestroyPipeline(g_ctx.device, g_ctx.graphicsPipeline, NULL);
	vkDestroyPipeline(g_ctx.device, g_ctx.shadowPipeline, NULL);
	vkDestroyPipeline(g_ctx.device, g_ctx.uiPipeline, NULL);
	vkDestroyPipelineLayout(g_ctx.device, g_ctx.pipelineLayout, NULL);
	vkDestroyPipelineLayout(g_ctx.device, g_ctx.uiPipelineLayout, NULL);
	vkDestroyRenderPass(g_ctx.device, g_ctx.renderPass, NULL);
	vkDestroyRenderPass(g_ctx.device, g_ctx.shadowRenderPass, NULL);
	vkDestroyFramebuffer(g_ctx.device, g_ctx.shadowFramebuffer, NULL);
	vkDestroySampler(g_ctx.device, g_ctx.shadowSampler, NULL);
	vkDestroyImageView(g_ctx.device, g_ctx.shadowImageView, NULL);
	vkDestroyImage(g_ctx.device, g_ctx.shadowImage, NULL);
	vkFreeMemory(g_ctx.device, g_ctx.shadowImageMemory, NULL);
	cleanupColorDepthAndFramebuffers();
	vkDestroyDevice(g_ctx.device, NULL);
	vkDestroySurfaceKHR(g_ctx.instance, g_ctx.surface, NULL);
	vkDestroyInstance(g_ctx.instance, NULL);
	glfwDestroyWindow(g_ctx.window);
	glfwTerminate();
	return 0;
}
float qgGetFPS() { return currentFPS; }
// ========================================================================================================================================================================
// ===== DRAWING ==========================================================================================================================================================
// ========================================================================================================================================================================
void qgSetBackground(const ColorRGB clr) { backgroundClr = clr; }
void qgSetRotationPivot(const Vector3 p) {
	g_ctx.pivotX = p.x;
	g_ctx.pivotY = p.y;
	g_ctx.pivotZ = p.z;
}
static float rndToNrm(const float v) { return v - ((int)(v / 360.0f) * 360.0f); }
void qgSetRotation(const Vector3 r) {
	g_ctx.rotX = rndToNrm(r.x);
	g_ctx.rotY = rndToNrm(r.y);
	g_ctx.rotZ = rndToNrm(r.z);
	g_ctx.hasRotation = 1;
}
void qgResetRotation() {
	g_ctx.pivotX = g_ctx.pivotY = g_ctx.pivotZ = 0.0f;
	g_ctx.rotX = g_ctx.rotY = g_ctx.rotZ = 0.0f;
	g_ctx.hasRotation = 0;
}
void qgAddTriangle(const Vector3 p1, const Vector3 p2, const Vector3 p3, const ColorRGBA clr) {
	if (g_ctx.currentVOffset + 3 > MAX_VERTICES) { qgWarn("qgAddTriangle: MAX_VERTICES reached, triangle skipped\n"); return; }
	Vector3 wp[3] = { p1, p2, p3 };
	for (uint8_t i = 0; i < 3; i++) transformPoint(&wp[i].x, &wp[i].y, &wp[i].z);
	const Vector3 normal = v3Normalize(v3Cross(v3Sub(wp[2], wp[0]), v3Sub(wp[1], wp[0])));
	QGPU_Vertex* vb = (QGPU_Vertex*)g_ctx.mappedVertexBuffer;
	uint32_t* ib = (uint32_t*)g_ctx.mappedIndexBuffer;
	const uint32_t base = g_ctx.currentVOffset;
	for (uint8_t i = 0; i < 3; i++) {
		vb[base + i].pos[0] = wp[i].x;
		vb[base + i].pos[1] = wp[i].y;
		vb[base + i].pos[2] = wp[i].z;
		vb[base + i].normal[0] = normal.x;
		vb[base + i].normal[1] = normal.y;
		vb[base + i].normal[2] = normal.z;
		vb[base + i].color[0] = qclampf(clr.r, 0.0f, 1.0f);
		vb[base + i].color[1] = qclampf(clr.g, 0.0f, 1.0f);
		vb[base + i].color[2] = qclampf(clr.b, 0.0f, 1.0f);
		vb[base + i].color[3] = qclampf(clr.a, 0.0f, 1.0f);
	}
	ib[g_ctx.currentIOffset + 0] = base + 0;
	ib[g_ctx.currentIOffset + 1] = base + 1;
	ib[g_ctx.currentIOffset + 2] = base + 2;
	g_ctx.currentIOffset += 3;
	g_ctx.currentVOffset += 3;
}
void qgAddTriangle2D(const Vector2 p1, const Vector2 p2, const Vector2 p3, const ColorRGBA clr) {
	if (uiTriangleCount >= MAX_UI_TRIANGLES) { qgWarn("qgAddTriangle2D: MAX_UI_VERTICES reached, triangle skipped\n"); return; }
	UITriangle2D* t = &uiTriangles[uiTriangleCount];
	t->p[0][0] = p1.x; t->p[0][1] = p1.y;
	t->p[1][0] = p2.x; t->p[1][1] = p2.y;
	t->p[2][0] = p3.x; t->p[2][1] = p3.y;
	t->color[0] = qclampf(clr.r, 0.0f, 1.0f);
	t->color[1] = qclampf(clr.g, 0.0f, 1.0f);
	t->color[2] = qclampf(clr.b, 0.0f, 1.0f);
	t->color[3] = qclampf(clr.a, 0.0f, 1.0f);
	t->layer = currentUILayer;
	t->order = uiTriangleCount;
	uiTriangleCount++;
}
void qgSetLayerUI(const uint layer) { currentUILayer = layer; }
void qgAddRect(const Vector2 p, const Vector2 size, const ColorRGBA clr) {
	const float x = size.x / 2.0f, y = size.y / 2.0f;
	const Vector2
	mp = {p.x - x, p.y + y},
	pp = {p.x + x, p.y + y},
	pm = {p.x + x, p.y - y},
	mm = {p.x - x, p.y - y};
	qgAddTriangle2D(mp, pp, pm, clr);
	qgAddTriangle2D(mp, pm, mm, clr);
}
void qgAddCircle(const Vector2 p, const float radius, const uint segments, const ColorRGBA clr) {
	if (radius <= 0 || segments < 3) return;
	Vector2 last = {p.x, p.y + radius};
	const float seg = 360.0f / segments;
	for (uint i = 1; i <= segments; i++) {
		const float rad = (seg * i) * (PI / 180.0f);
		const Vector2 n = (Vector2){p.x + qSin(rad) * radius, p.y + qCos(rad) * radius};
		qgAddTriangle2D(p, n, last, clr);
		last = n;
	}
}

void qgAddPlane(const Vector3 p, const Vector2 size, const ColorRGBA clr) {
	const float x = size.x / 2.0f, z = size.y / 2.0f;
	const Vector3
	mm = {p.x - x, p.y, p.z - z},
	mp = {p.x - x, p.y, p.z + z},
	pm = {p.x + x, p.y, p.z - z},
	pp = {p.x + x, p.y, p.z + z};
	qgAddTriangle(mm, pm, pp, clr);
	qgAddTriangle(mm, pp, mp, clr);
}
void qgAddBox(const Vector3 p, const Vector3 size, const ColorRGBA clr) {
	const float x = size.x / 2.0f, y = size.y / 2.0f, z = size.z / 2.0f;
	const Vector3
	mmm = {p.x - x, p.y - y, p.z - z},
	mmp = {p.x - x, p.y - y, p.z + z},
	mpm = {p.x - x, p.y + y, p.z - z},
	mpp = {p.x - x, p.y + y, p.z + z},
	pmm = {p.x + x, p.y - y, p.z - z},
	pmp = {p.x + x, p.y - y, p.z + z},
	ppm = {p.x + x, p.y + y, p.z - z},
	ppp = {p.x + x, p.y + y, p.z + z};
	qgAddTriangle(mmm, pmm, ppm, clr);
	qgAddTriangle(mmm, ppm, mpm, clr);
	qgAddTriangle(pmm, pmp, ppp, clr);
	qgAddTriangle(pmm, ppp, ppm, clr);
	qgAddTriangle(pmp, mmp, mpp, clr);
	qgAddTriangle(pmp, mpp, ppp, clr);
	qgAddTriangle(mmp, mmm, mpm, clr);
	qgAddTriangle(mmp, mpm, mpp, clr);
	qgAddTriangle(mpm, ppm, ppp, clr);
	qgAddTriangle(mpm, ppp, mpp, clr);
	qgAddTriangle(mmp, pmp, pmm, clr);
	qgAddTriangle(mmp, pmm, mmm, clr);
}
// ===== Lights
static GPULight* newLight(const char* fn) {
	if (lightCount >= MAX_LIGHTS) { qgWarn("%s: MAX_LIGHTS reached\n", fn); return NULL; }
	GPULight* L = &lights[lightCount++];
	memset(L, 0, sizeof(GPULight));
	return L;
}
void qgAddPointLight(const Vector3 position, const float range, const float power) {
	GPULight* L = newLight("qgAddPointLight");
	if (!L) return;
	L->posRange[0] = position.x; L->posRange[1] = position.y; L->posRange[2] = position.z; L->posRange[3] = range;
	L->dirPower[3] = power;
	L->typeAngle[0] = QGPU_LIGHT_POINT;
}
void qgAddSpotLight(const Vector3 position, const Vector3 direction, const float angleDegrees, const float range, const float power) {
	GPULight* L = newLight("qgAddSpotLight");
	if (!L) return;
	const Vector3 d = v3Normalize(direction);
	L->posRange[0] = position.x; L->posRange[1] = position.y; L->posRange[2] = position.z; L->posRange[3] = range;
	L->dirPower[0] = d.x; L->dirPower[1] = d.y; L->dirPower[2] = d.z; L->dirPower[3] = power;
	L->typeAngle[0] = QGPU_LIGHT_SPOT;
	L->typeAngle[1] = qCos(qclampf(angleDegrees, 1.0f, 89.0f) * (PI / 180.0f));
	L->typeAngle[2] = qclampf(angleDegrees, 1.0f, 89.0f);
}
void qgAddDirectionalLight(const Vector3 direction, const float power) {
	GPULight* L = newLight("qgAddDirectionalLight");
	if (!L) return;
	const Vector3 d = v3Normalize(direction);
	L->dirPower[0] = d.x; L->dirPower[1] = d.y; L->dirPower[2] = d.z; L->dirPower[3] = power;
	L->typeAngle[0] = QGPU_LIGHT_DIRECTIONAL;
}

void qgAddLight(const Vector3 position, const float range, const float power) { qgAddPointLight(position, range, power); } // OBSOLETE
// ========================================================================================================================================================================
// ===== TEXT =============================================================================================================================================================
// ========================================================================================================================================================================
void qgLoadFont(const char* path) {
	if (!inInit) { qgWarn("Couldn't load font in Update function! Please load it in Init function.\n"); return; }
	qfReadFile(path);
}
void qgSetFontData(const float fontSize, const uint8_t style, const ColorRGBA clr) { qfConfigure(fontSize, style, clr.r,clr.g,clr.b,clr.a); }
void qgAddChar(const Vector2 pos, const uint32_t c) { qfDrawChar(pos.x, pos.y, c); }
void qgAddText(const Vector2 pos, const char* text) { qfDrawText(pos.x, pos.y, text); }
// ========================================================================================================================================================================
// ===== INPUT ============================================================================================================================================================
// ========================================================================================================================================================================
uint8_t qgGetKey(const uint key) {
	if (!g_ctx.window || key >= GLFW_KEY_LAST) return 0;
	return glfwGetKey(g_ctx.window, key) == GLFW_PRESS;
}
uint8_t qgOnKey(const uint key) {
	if (!g_ctx.window || key >= GLFW_KEY_LAST) return 0;
	uint8_t current = glfwGetKey(g_ctx.window, key), last = g_ctx.lastKeyState[key];
	return (current == GLFW_PRESS && last == GLFW_RELEASE);
}
uint8_t qgGetMouse(const uint button) {
	if (!g_ctx.window || button >= GLFW_MOUSE_BUTTON_LAST) return 0;
	return glfwGetMouseButton(g_ctx.window, button) == GLFW_PRESS;
}
uint8_t qgOnMouse(const uint button) {
	if (!g_ctx.window || button >= GLFW_MOUSE_BUTTON_LAST) return 0;
	uint8_t current = glfwGetMouseButton(g_ctx.window, button), last = g_ctx.lastMouseState[button];
	return (current == GLFW_PRESS && last == GLFW_RELEASE);
}
void qgGetMousePos(float* x, float* y) {
	if (!g_ctx.window || !x || !y) return;
	double lx = 0, ly = 0;
	glfwGetCursorPos(g_ctx.window, &lx, &ly);
	*x = lx - (double)qgGetWidth() / 2;
	*y = -(ly - (double)qgGetHeight() / 2);
}
uint qgGetWidth() {
	if (!g_ctx.window) return 0;
	int w, h;
	glfwGetWindowSize(g_ctx.window, &w, &h);
	return w;
}
uint qgGetHeight() {
	if (!g_ctx.window) return 0;
	int w, h;
	glfwGetWindowSize(g_ctx.window, &w, &h);
	return h;
}
