#pragma once
#include <unordered_map>
#include <string>
#include <AL/al.h>
#include <AL/alc.h>
#include <glm/glm.hpp>
#include "importfile.hpp"

namespace audio {
	//Sound device singleton
	class SoundDevice {
		SoundDevice();
		~SoundDevice();
		ALCdevice* device;
		ALCcontext* ctx;
	public:
		static SoundDevice* get();
	};

	struct SfxMetaData {
		std::string name;
		std::string path;
		float gain;
		float pitch;
	};

	SfxMetaData entryToSfxMetaData(const impfile::Entry &entry);

	struct Sfx {
		ALuint buffer;
		float gain;
		float pitch;
	};

	//Sfx manager, it is assumed that all sound effect files are .wav files
	class SfxManager {
		std::unordered_map<std::string, Sfx> sfx;
		SfxManager() {}
	public:
		static SfxManager* get();
		//Returns the buffer of the sound effect added
		ALuint addSfx(const std::string &name, const SfxMetaData &metadata);
		void importFromFile(const char *path);
		//SfxManager must contain sfxid otherwise the program will crash
		ALuint getBuffer(const std::string &sfxid);
		const Sfx& getSfx(const std::string &sfxid);
	};

	class SoundSource {
		ALuint source;
		ALuint buffer;
	public:
		SoundSource(ALuint bufferToPlay);
		SoundSource(ALuint bufferToPlay, float gain, float pitch);
		SoundSource(
			ALuint bufferToPlay,
			float gain,
			float pitch,
			const glm::vec3 &position
		);
		ALint getState();
		void play();
		void pause();
		void stop();
		ALuint getSrc();
	};

	class SoundSourceManager {
		std::vector<SoundSource> sources;
		SoundSourceManager() {}
	public:
		static SoundSourceManager* get();
		void play(const Sfx &sfx);
		void play(const Sfx &sfx, const glm::vec3 &position);
		void playid(const std::string &id);
		void playid(const std::string &id, const glm::vec3 &position);
		void playid(const std::string &id, const glm::vec3 &position, float gainfactor);
		void pauseAll();
		void unpauseAll();
		void stopAll();
		void clearSources();
	};

	void updateListener(
		const glm::vec3 &listenerpos,
		const glm::vec3 &direction
	);
	void resetListener();
}

#define SFX audio::SfxManager::get()
#define SNDSRC audio::SoundSourceManager::get()
