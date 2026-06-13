#ifndef QE_TEXT_H
#define QE_TEXT_H

static inline int qStrlen(char* text) { int i=0; while(text[i]) i++; return i; }
static inline void qStrcpy(char* dest, char* src) { int i=0; while(src[i]) { dest[i] = src[i]; i++; } dest[i]='\0'; }
static inline int qStrcmp(char* s1, char* s2) { int i=0; while(s1[i] && s2[i]) { if (s1[i] != s2[i]) return 0; i++; } return s1[i] == s2[i]; }
static inline void qStradd(char* dest, char* src) { int i=0; while (dest[i]) i++; int j = 0; while (src[j]) { dest[i] = src[j]; i++; j++; } dest[i] = '\0'; }

#endif
