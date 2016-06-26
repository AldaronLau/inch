/*
 * JL_lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLAU.c
 *	Audio
 * 		This library can play/record music/sound effects.
*/
#include "jl_pr.h"
#include "JLau.h"

#define JLAU_DEBUG_CHECK(jlau) jlau_checkthread__(jlau)
#define JLAU_CHANNEL_MUSIC -2
#define JLAU_CHANNEL_SOUND -3
#define JLAU_CHANNEL_LOCK -4

/** @cond **/
static void jlau_checkthread__(jlau_t* jlau) {
	uint8_t thread = jl_thread_current(jlau->jl);
	if(thread != 0) {
		jl_print(jlau->jl, "Audio fn is on the Wrong Thread: %d",
			thread);
		jl_print(jlau->jl, "Must be on thread 1!");
			jl_print_stacktrace(jlau->jl);
		exit(-1);
	}
}

static inline int32_t jlau_sec2ms__(float sec) {
	int32_t ms = ((int32_t)(sec * 1000.f));
	if(ms < 20) ms = 0;
	return ms;
}

/**
 * Load audio from data pointed to by "data" of length "dataSize" into "audio",
 * set volume of audio to "volumeChange"
 */
static inline void jlau_load(jlau_t* jlau, jlau_audio_t* audio,
	uint8_t volumeChange, const void *data, uint32_t dataSize,
	uint8_t isMusic)
{
	SDL_RWops *rw;

	JLAU_DEBUG_CHECK(jlau);
	jl_print_function(jlau->jl, "AU_Load");
	JL_PRINT_DEBUG(jlau->jl, "ausize: %d", dataSize);
	JL_PRINT_DEBUG(jlau->jl, "audata: \"%4s\"", data);
	rw = SDL_RWFromConstMem(data, dataSize);
	if(isMusic)
		audio->audio = Mix_LoadMUS_RW(rw, 1);
	else
		audio->audio = Mix_LoadWAV_RW(rw, 1);
	if(audio->audio == NULL) {
		jl_print(jlau->jl, ":Couldn't load audio because: %s",
			(char *)SDL_GetError());
		exit(-1);
	}
	audio->channel = isMusic ? JLAU_CHANNEL_MUSIC : JLAU_CHANNEL_SOUND;
	Mix_VolumeChunk(audio->audio, volumeChange);
//	SDL_RWclose(rw);
	JL_PRINT_DEBUG(jlau->jl, "Loaded audio!");
	jl_print_return(jlau->jl, "AU_Load");
}
/** @endcond **/

/** 
 * Fade in audio.
 * @param jlau: The jlau library context.
 * @param audio: The audio to play.
 * @param in: Seconds of fade-in.
 * @param vec: Where sound is coming from, or NULL for no position effect.
*/
void jlau_play(jlau_t* jlau, jlau_audio_t* audio, float in, jl_vec3_t* vec) {
	JLAU_DEBUG_CHECK(jlau);
	int32_t ms = jlau_sec2ms__(in);

	if(audio->channel == JLAU_CHANNEL_MUSIC) {
		Mix_HaltMusic();
		Mix_FadeInMusic(audio->audio, 1, 20);
	}else{
		JLAU_PLAY_MIX_PLAY_CHANNEL:
		if((audio->channel = Mix_FadeInChannel(-1, audio->audio, 0,
			ms + 1)) == -1)
		{
			jlau->num_channels++;
			Mix_ReserveChannels(jlau->num_channels);
			goto JLAU_PLAY_MIX_PLAY_CHANNEL;
		}
		// Calculate Position
		if(vec == NULL) {
			Mix_SetPosition(audio->channel, 0, 0);
		}else if(vec->x == 0 && vec->y == 0 && vec->z == 0) {
			Mix_SetPosition(audio->channel, 0, 0);
		}else{
			float distance = sqrtf((vec->x*vec->x) + (vec->y*vec->y)
				+ (vec->z*vec->z));
			float angle = atan2f(vec->z, vec->x) * (180.f / M_PI);
			if(angle < 0.f) angle += 360.f;

			Mix_SetPosition(audio->channel, angle, distance);
		}
	}
}

void jlau_lock(jlau_t* jlau, jlau_audio_t* audio, float in, jl_vec3_t* vec) {
	if(audio->channel != JLAU_CHANNEL_LOCK) jlau_play(jlau, audio, in, vec);
	audio->channel = JLAU_CHANNEL_LOCK;
}

void jlau_pause(jlau_t* jlau, jlau_audio_t* audio) {
	if(audio->channel == JLAU_CHANNEL_MUSIC)
		Mix_PauseMusic();
	else
		Mix_Pause(audio->channel);
}

void jlau_resume(jlau_t* jlau, jlau_audio_t* audio) {
	if(audio->channel == JLAU_CHANNEL_MUSIC)
		Mix_ResumeMusic();
	else
		Mix_Resume(audio->channel);
}

/**
 * Wait on an audio track to stop.  If it is stopped, fade in a new one.
 * @param jlau: The jlau library context.
 * @param w_audio: The audio to wait on, NULL to wait on music to finish.
 * @param n_audio: The audio to play.
 * @param in: Seconds of fade-in.
 * @param vec: Where sound is coming from, or NULL for no position effect.
 * @returns 1: Audio has finished, and new audio is playing
 * @returns 0: Audio is still playing.
**/
uint8_t jlau_wait(jlau_t* jlau, jlau_audio_t* w_audio, jlau_audio_t* n_audio,
	float in, jl_vec3_t* vec)
{
	// If audio is still playing, quit
	if(w_audio ? Mix_Playing(w_audio->channel) : Mix_PlayingMusic())
		return 0;
	//
	jlau_play(jlau, n_audio, in, vec);
	return 1;
}

/**
 * Fade out a currently playing audio (if there is any) for "out" seconds
 * @param audio: Audio to fade out, or NULL to fade out all audio.
 * @param out: How many seconds to fade audio.
*/
void jlau_stop(jlau_audio_t* audio, float out) {
	int32_t channel = (audio == NULL) ? -1 : audio->channel;
	int32_t ms = jlau_sec2ms__(out);

	if(channel == JLAU_CHANNEL_MUSIC)
		Mix_FadeOutMusic(ms + 1);
	else
		Mix_FadeOutChannel(channel, ms + 1);
}

/**
 * Load an audiotrack from a zipfile.
 * info: info is set to number of audiotracks loaded.
 * @param jlau: The library context
 * @param audio: Unintialized audio object.
 * @param zipdata: data for a zip file.
 * @param filename: Name of the audio file in the package.
 * @param music: Whether the audio is music or not.
*/
void jlau_add_audio(jlau_t* jlau, jlau_audio_t* audio, data_t* zipdata,
	const char* filename, uint8_t music)
{
	data_t aud;

	JLAU_DEBUG_CHECK(jlau);
	jl_file_pk_load_fdata(jlau->jl, &aud, zipdata, filename);
	jl_print_function(jlau->jl, "jlau_add_audio");
	JL_PRINT_DEBUG(jlau->jl, "Loading audiostuffs....");
	jlau_load(jlau, audio, 255, aud.data, aud.size, music);
	JL_PRINT_DEBUG(jlau->jl, "Loaded audiostuffs!");
	jl_print_return(jlau->jl, "jlau_add_audio");
}

/** @cond **/
static inline void _jlau_print_openblock(jl_t* jl) {
	jl_print_function(jl, "JLaudio");
}

static inline void _jlau_print_closeblock(jl_t* jl) {
	jl_print_return(jl, "JLaudio");
}
/** @endcond **/

jlau_t* jlau_init(jl_t* jl) {
	jlau_t* jlau = jl_memi(jl, sizeof(jlau_t));

	jlau->jl = jl;
	jl->jlau = jlau;
	JLAU_DEBUG_CHECK(jlau);

	jlau->num_channels = 0;
	//Open Block AUDI
	_jlau_print_openblock(jl);
	// Open the audio device
	JL_PRINT_DEBUG(jl, "initializing audio....");
	Mix_Init(MIX_INIT_OGG);
	if ( Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 1024) < 0 ) {
		jl_print(jl,
			":Couldn't set 11025 Hz 16-bit audio because: %s",
			(char *)SDL_GetError());
		exit(-1);
	}else{
		JL_PRINT_DEBUG(jl, "audio has been set.");
	}
	//Close Block AUDI
	_jlau_print_closeblock(jl);
	// Return the context.
	return jlau;
}

void jlau_kill(jlau_t* jlau) {
	JLAU_DEBUG_CHECK(jlau);
	//Open Block AUDI
	_jlau_print_openblock(jlau->jl);
	JL_PRINT_DEBUG(jlau->jl, "Quiting Audio....");
	//Free Everything
	Mix_CloseAudio();	
	JL_PRINT_DEBUG(jlau->jl, "Quit Audio Successfully!");
	//Close Block AUDI
	_jlau_print_closeblock(jlau->jl);
}
