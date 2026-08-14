#ifndef QSOUND_H
#define QSOUND_H

#include <stdint.h>

typedef struct {
	uint8_t type;
	uint16_t frequency, duration;
	uint8_t volume;
} QSoundNote;

int qsInit();
int qsLoad(const QSoundNote* notes, int noteCount);
void qsPlay(int soundId, uint8_t volume, float speed);
void qsClose();

void qsConvert(const char* qsr_path, const char* qs_path);
int qsOpen(const char* path);

#endif
