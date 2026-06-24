#ifndef QENGINE_LIB_TEXT_H
#define QENGINE_LIB_TEXT_H

static inline int qLen(char* text) { int i=0; while(text[i]) i++; return i; }
static inline void qCpy(char* dest, char* src) { int i=0; while(src[i]) { dest[i] = src[i]; i++; } dest[i]='\0'; }
static inline int qIs(char* s1, char* s2) { int i=0; while(s1[i] && s2[i]) { if (s1[i] != s2[i]) return 0; i++; } return s1[i] == s2[i]; }
static inline void qAdd(char* dest, char* src) { int i=0; while (dest[i]) i++; int j = 0; while (src[j]) { dest[i] = src[j]; i++; j++; } dest[i] = '\0'; }

#endif
