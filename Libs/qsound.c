#include "qsound.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <alsa/asoundlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define AUDIO_SAMPLE_RATE 44100
#define AUDIO_CHANNELS    2
#define MAX_SOUNDS        1024
#define MAX_VOICES        256
#define BUFFER_FRAMES     256

typedef struct {
	int16_t* samples;
	uint32_t totalFrames;
} Sound;
typedef struct {
	int soundId;
	float cursor;
	float volume, speed;
	int active;
} Voice;
static int g_SoundCount = 0, g_AudioRunning = 0;
static Sound g_Sounds[MAX_SOUNDS];
static snd_pcm_t* g_PcmHandle = NULL;
static Voice g_Voices[MAX_VOICES];
static pthread_t g_AudioThread;
static pthread_mutex_t g_AudioMutex = PTHREAD_MUTEX_INITIALIZER;
static void MixAudioBuffer(int16_t* outputBuffer, uint32_t frameCount) {
	memset(outputBuffer, 0, frameCount * AUDIO_CHANNELS * sizeof(int16_t));
	pthread_mutex_lock(&g_AudioMutex);
	for (int v = 0; v < MAX_VOICES; v++) {
		if (!g_Voices[v].active) continue;
		Sound* snd = &g_Sounds[g_Voices[v].soundId];
		float vol = g_Voices[v].volume, speed = g_Voices[v].speed;
		if (speed <= 0.0f) speed = 1.0f;
		for (uint32_t i = 0; i < frameCount; i++) {
			if (g_Voices[v].cursor >= snd->totalFrames) {
				g_Voices[v].active = 0;
				break;
			}
			uint32_t srcIdx = g_Voices[v].cursor * 2, dstIdx = i * 2;
			int32_t sampleL = outputBuffer[dstIdx] + (int32_t)(snd->samples[srcIdx] * vol),
			sampleR = outputBuffer[dstIdx + 1] + (int32_t)(snd->samples[srcIdx + 1] * vol);
			if (sampleL > 32767)  sampleL = 32767;
			if (sampleL < -32768) sampleL = -32768;
			if (sampleR > 32767)  sampleR = 32767;
			if (sampleR < -32768) sampleR = -32768;
			outputBuffer[dstIdx] = (int16_t)sampleL;
			outputBuffer[dstIdx + 1] = (int16_t)sampleR;
			g_Voices[v].cursor += speed;
		}
	}
	pthread_mutex_unlock(&g_AudioMutex);
}
static void* AudioThreadFunc(void* arg) {
	(void)arg;
	int16_t buffer[BUFFER_FRAMES * AUDIO_CHANNELS];
	while (g_AudioRunning) {
		MixAudioBuffer(buffer, BUFFER_FRAMES);
		snd_pcm_sframes_t framesWritten = snd_pcm_writei(g_PcmHandle, buffer, BUFFER_FRAMES);
		if (framesWritten < 0) snd_pcm_prepare(g_PcmHandle);
	}
	return NULL;
}

int qsInit() {
	if (snd_pcm_open(&g_PcmHandle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) return 0;
	snd_pcm_set_params(g_PcmHandle, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, AUDIO_CHANNELS, AUDIO_SAMPLE_RATE, 1, 20000);
	g_AudioRunning = 1;
	pthread_create(&g_AudioThread, NULL, AudioThreadFunc, NULL);
	return 1;
}
int qsLoad(const QSoundNote* notes, int noteCount) {
	if (g_SoundCount >= MAX_SOUNDS || notes == NULL || noteCount <= 0) return -1;
	uint32_t totalFrames = 0;
	for (int i = 0; i < noteCount; i++) totalFrames += (uint32_t)((AUDIO_SAMPLE_RATE * notes[i].duration) / 1000);
	if (totalFrames == 0) return -1;
	int16_t* samples = (int16_t*)malloc(totalFrames * AUDIO_CHANNELS * sizeof(int16_t));
	if (!samples) return -1;
	uint32_t currentFrame = 0;
	for (int n = 0; n < noteCount; n++) {
		uint32_t noteFrames = (uint32_t)((AUDIO_SAMPLE_RATE * notes[n].duration) / 1000);
		float freq = (float)notes[n].frequency, noteVol = notes[n].volume / 255.0f;
		for (uint32_t i = 0; i < noteFrames; i++) {
			int16_t val = 0;
			if (notes[n].type == 4 || freq > 0.0f) {
				float t = (float)i / AUDIO_SAMPLE_RATE, envelope = 1.0f;
				if (i < 100) envelope = (float)i / 100.0f; else if (i > noteFrames - 100) envelope = (float)(noteFrames - i) / 100.0f;
				float sample = 0.0f, phase = 2.0f * M_PI * freq * t;
				switch (notes[n].type) {
					case 0: sample = sinf(phase); break;
					case 1: sample = (sinf(phase) >= 0.0f) ? 1.0f : -1.0f; break;
					case 2: sample = 2.0f * (t * freq - floorf(t * freq)) - 1.0f; break;
					case 3: sample = 4.0f * fabsf(t * freq - floorf(t * freq + 0.5f)) - 1.0f; break;
					case 4: sample = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f; break;
				}
				val = (int16_t)(sample * 18000.0f * envelope * noteVol);
			}
			uint32_t sampleIdx = (currentFrame + i) * 2;
			samples[sampleIdx]     = val;
			samples[sampleIdx + 1] = val;
		}
		currentFrame += noteFrames;
	}
	int id = g_SoundCount++;
	g_Sounds[id].samples = samples;
	g_Sounds[id].totalFrames = totalFrames;
	return id;
}
void qsPlay(int soundId, uint8_t volume, float speed) {
	if (soundId < 0 || soundId >= g_SoundCount) return;
	pthread_mutex_lock(&g_AudioMutex);
	for (int i = 0; i < MAX_VOICES; i++) {
		if (!g_Voices[i].active) {
			g_Voices[i].soundId = soundId;
			g_Voices[i].cursor = 0.0f;
			g_Voices[i].volume = volume / 255.0f;
			g_Voices[i].speed = speed;
			g_Voices[i].active = 1;
			break;
		}
	}
	pthread_mutex_unlock(&g_AudioMutex);
}
void qsClose() {
	g_AudioRunning = 0;
	pthread_join(g_AudioThread, NULL);
	for (int i = 0; i < g_SoundCount; i++) if (g_Sounds[i].samples) free(g_Sounds[i].samples);
	if (g_PcmHandle) snd_pcm_close(g_PcmHandle);
}
void qsConvert(const char* qsr_path, const char* qs_path) {
	FILE *qsr = fopen(qsr_path, "r");
	if (!qsr) return;
	FILE *qs = fopen(qs_path, "wb");
	if (!qs) { fclose(qsr); return; }
	char l[64];
	uint32_t i = 0;
	fwrite(&i, sizeof(uint32_t), 1, qs);
	while (fgets(l, sizeof(l), qsr)) {
		if (l[0] == '/') continue;
		unsigned int type, freq, ms, vol;
		if (sscanf(l, "%u.%u.%u.%u", &type, &freq, &ms, &vol) == 4) {
			uint8_t  t_val = (uint8_t)type;
			uint16_t f_val = (uint16_t)freq;
			uint16_t m_val = (uint16_t)ms;
			uint8_t  v_val = (uint8_t)vol;
			fwrite(&t_val, sizeof(uint8_t), 1, qs);
			fwrite(&f_val, sizeof(uint16_t), 1, qs);
			fwrite(&m_val, sizeof(uint16_t), 1, qs);
			fwrite(&v_val, sizeof(uint8_t), 1, qs);
			i++;
		}
	}
	fseek(qs, 0, SEEK_SET);
	fwrite(&i, sizeof(uint32_t), 1, qs);
	fclose(qsr);
	fclose(qs);
}
int qsOpen(const char* path) {
	FILE *qs = fopen(path, "rb");
	if (!qs) return -1;
	uint32_t c = 0;
	fread(&c, sizeof(uint32_t), 1, qs);
	QSoundNote l[c];
	for (uint32_t i = 0; i < c; i++) {
		fread(&l[i].type, sizeof(uint8_t), 1, qs);
		fread(&l[i].frequency, sizeof(uint16_t), 1, qs);
		fread(&l[i].duration, sizeof(uint16_t), 1, qs);
		fread(&l[i].volume, sizeof(uint8_t), 1, qs);
	}
	fclose(qs);
	return qsLoad(l, c);
}
