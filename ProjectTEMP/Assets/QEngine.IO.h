#ifndef QENGINE_LIB_FILE_H
#define QENGINE_LIB_FILE_H
#include "QEngine.Memory.h"
#include <stdio.h>

static inline char* qReadFileText(const char* filename) {
	FILE* file = fopen(filename, "rb");
	if (!file) return NULL;
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	char* buffer = (char*)qMalloc(size + 1);
	if (!buffer) { fclose(file); return NULL; }
	fread(buffer, 1, size, file);
	buffer[size] = '\0';
	fclose(file);
	return buffer;
}
static inline int qWriteFileText(const char* filename, const char* text) {
	FILE* file = fopen(filename, "w");
	if (!file) return 0;
	int result = fputs(text, file);
	fclose(file);
	return result != EOF;
}
static inline int qFileExists(const char* filename) {
	FILE* file = fopen(filename, "r");
	if (file) { fclose(file); return 1; }
	return 0;
}

#endif
