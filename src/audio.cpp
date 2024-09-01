#include "audio.hpp"
#include "app.hpp"
#include <algorithm>
#include <dr_wav/dr_wav.h>

namespace audio {
	SoundDevice::~SoundDevice()
	{
		if(!alcMakeContextCurrent(NULL))
			die("Failed to unset OpenAL context!\n");
		alcDestroyContext(ctx);
		if(ctx)
			die("Failed to destroy OpenAL Context!\n");
		if(!alcCloseDevice(device))
			die("Failed to close device!\n");
		fprintf(stderr, "Closed sound device.\n");
	}

	SoundDevice::SoundDevice()
	{
		//Get the default device
		device = alcOpenDevice(NULL);
		if(device) {
			//Create OpenAL context
			ctx = alcCreateContext(device, NULL);
			if(!ctx)
				fprintf(stderr, "E: Failed to create OpenAL Context!\n");
			if(!alcMakeContextCurrent(ctx))
				fprintf(stderr, "E: failed to set OpenAL Context!\n");

			//Output sound device opened
			const ALchar* name = NULL;
			if(alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT"))
				name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
			if(!name || alcGetError(device) != AL_NO_ERROR)
				name = alcGetString(device, ALC_DEVICE_SPECIFIER);
			fprintf(stderr, "Opened: %s\n", name);
		}
		else
			fprintf(stderr, "E: can not open sound device!\n");
	}

	SoundDevice* SoundDevice::get()
	{
		static SoundDevice* dev = new SoundDevice;
		return dev;
	}

	SfxMetaData entryToSfxMetaData(const impfile::Entry &entry)
	{
		SfxMetaData sfx;
		sfx.name = entry.name;
		sfx.path = entry.getVar("path");
		
		if(entry.getVar("gain").empty())
			sfx.gain = 1.0f;
		else
			//If "gain" is not a valid float, this should just set the gain to be 0.0
			sfx.gain = atof(entry.getVar("gain").c_str());
	
		if(entry.getVar("pitch").empty())
			sfx.pitch = 1.0f;
		else
			sfx.pitch = atof(entry.getVar("pitch").c_str());

		return sfx;
	}

	SfxManager* SfxManager::get()
	{
		static SfxManager* sfxmanager = new SfxManager;
		return sfxmanager;
	}

	ALuint SfxManager::addSfx(const std::string &name, const SfxMetaData &metadata)
	{
		drwav wav;
		if(!drwav_init_file(&wav, metadata.path.c_str(), NULL)) {
			fprintf(stderr, "Failed to open: %s\n", metadata.path.c_str());
			return 0;
		}

		size_t decodedsz = wav.totalPCMFrameCount * wav.channels * sizeof(drwav_int32);
		drwav_int32* decoded = (drwav_int32*)malloc(decodedsz);
		size_t numSamplesDecoded = 
			drwav_read_pcm_frames_s32(&wav, wav.totalPCMFrameCount, decoded);
		ALuint buffer = 0;
		alGenBuffers(1, &buffer);

		//Get format
		ALenum format = AL_NONE;
		if(wav.channels == 1)
			format = AL_FORMAT_MONO16;
		else if(wav.channels == 2)
			format = AL_FORMAT_STEREO16;

		if(!format) {
			fprintf(stderr, "Unrecognized channel count: %d\n", wav.channels);
			free(decoded);
			drwav_uninit(&wav);
			return 0;
		}

		if(wav.totalPCMFrameCount < 1) {
			fprintf(stderr, "Failed to read samples in %s\n", metadata.path.c_str()); 
			free(decoded);
			drwav_uninit(&wav);
			return 0;
		}

		alBufferData(buffer, format, decoded, decodedsz, wav.sampleRate);

		//Check for errors
		ALenum err = alGetError();
		if(err != AL_NO_ERROR) {
			fprintf(stderr, "OpenAL Error: %s\n", alGetString(err));
			if(buffer && alIsBuffer(buffer))
				alDeleteBuffers(1, &buffer);
			//Clean up
			free(decoded);
			drwav_uninit(&wav);
			return 0;
		}
	
		Sfx s = {
			.buffer = buffer,
			.gain = metadata.gain,
			.pitch = metadata.pitch,
		};

		sfx.insert({ name, s });

		//Clean up
		free(decoded);
		drwav_uninit(&wav);

		return buffer;
	}

	void SfxManager::importFromFile(const char *path)
	{
		std::vector<impfile::Entry> entries = impfile::parseFile(path);
		for(const auto &entry : entries) {
			SfxMetaData sfxmetadata = entryToSfxMetaData(entry);
			//Add sound effect
			addSfx(sfxmetadata.name, sfxmetadata);
		}
	}

	ALuint SfxManager::getBuffer(const std::string &sfxid)
	{
		return sfx.at(sfxid).buffer;
	}

	const Sfx& SfxManager::getSfx(const std::string &sfxid)
	{
		return sfx.at(sfxid);
	}

	SoundSource::SoundSource(ALuint bufferToPlay)
	{
		buffer = bufferToPlay;
		alGenSources(1, &source);
		alSourcef(source, AL_ROLLOFF_FACTOR, 0.0f);
		alSourcef(source, AL_PITCH, 1.0f);
		alSourcef(source, AL_GAIN, 1.0f);
		alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
		alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
		alSourcei(source, AL_LOOPING, false);
		alSourcei(source, AL_BUFFER, buffer);

		ALenum err = alGetError();
		if(err != AL_NO_ERROR)
			fprintf(stderr, "OpenAL Error: %s\n", alGetString(err));
	}

	SoundSource::SoundSource(ALuint bufferToPlay, float gain, float pitch)
	{
		buffer = bufferToPlay;
		alGenSources(1, &source);
		alSourcef(source, AL_ROLLOFF_FACTOR, 0.0f);
		alSourcef(source, AL_PITCH, pitch);
		alSourcef(source, AL_GAIN, gain);
		alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
		alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
		alSourcei(source, AL_LOOPING, false);
		alSourcei(source, AL_BUFFER, buffer);

		ALenum err = alGetError();
		if(err != AL_NO_ERROR)
			fprintf(stderr, "OpenAL Error: %s\n", alGetString(err));
	}

	SoundSource::SoundSource(
		ALuint bufferToPlay,
		float gain,
		float pitch,
		const glm::vec3 &position
	) {
		buffer = bufferToPlay;
		alGenSources(1, &source);
		alSourcef(source, AL_ROLLOFF_FACTOR, 1.0f);
		alSourcef(source, AL_PITCH, pitch);
		alSourcef(source, AL_GAIN, gain);
		alSource3f(source, AL_POSITION, position.x, position.y, position.z);
		alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
		alSourcei(source, AL_LOOPING, false);
		alSourcei(source, AL_BUFFER, buffer);
	}

	void SoundSource::play()
	{
		alSourcePlay(source);
	}

	ALint SoundSource::getState()
	{
		ALint state;
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		return state;
	}

	ALuint SoundSource::getSrc()
	{
		return source;
	}

	void SoundSource::pause()
	{
		alSourcePause(source);
	}

	void SoundSource::stop()
	{
		alSourceStop(source);
	}

	SoundSourceManager* SoundSourceManager::get()
	{
		static SoundSourceManager* sourcemanager = new SoundSourceManager;
		return sourcemanager;
	}

	void SoundSourceManager::play(const Sfx &sfx)
	{
		SoundSource src = SoundSource(sfx.buffer, sfx.gain, sfx.pitch);
		src.play();
		sources.push_back(src);
	}

	void SoundSourceManager::play(const Sfx &sfx, const glm::vec3 &position)
	{
		SoundSource src = SoundSource(sfx.buffer, sfx.gain, sfx.pitch, position);
		src.play();
		sources.push_back(src);
	}

	void SoundSourceManager::playid(const std::string &id)
	{
		play(SFX->getSfx(id));
	}

	void SoundSourceManager::playid(const std::string &id, const glm::vec3 &position)
	{
		play(SFX->getSfx(id), position);
	}

	void SoundSourceManager::playid(
		const std::string &id,
		const glm::vec3 &position,
		float gainfactor
	) {
		Sfx sfx = SFX->getSfx(id);
		sfx.gain *= gainfactor;
		play(sfx, position);
	}

	void SoundSourceManager::clearSources()
	{
		if(sources.empty())
			return;

		std::vector<ALuint> sourcesToDelete;
		for(auto &src : sources)
			if(src.getState() != AL_PLAYING && src.getState() != AL_PAUSED)
				sourcesToDelete.push_back(src.getSrc());

		sources.erase(std::remove_if(
			sources.begin(),
			sources.end(),
			[](SoundSource &src) {
				return src.getState() != AL_PLAYING && src.getState() != AL_PAUSED;
			}
		), sources.end());

		alDeleteSources(sourcesToDelete.size(), &sourcesToDelete[0]);
	}

	void SoundSourceManager::pauseAll()
	{
		for(auto &source : sources)
			source.pause();
	}

	void SoundSourceManager::unpauseAll()
	{
		for(auto &source : sources)
			source.play();
	}

	void SoundSourceManager::stopAll()
	{
		for(auto &source : sources)
			source.stop();
	}

	void updateListener(
		const glm::vec3 &listenerpos,
		const glm::vec3 &direction
	) {
		alListener3f(AL_POSITION, listenerpos.x, listenerpos.y, listenerpos.z);

		float orientation[6] = {
			direction.x,
			direction.y,
			direction.z,
			0.0f,
			1.0f,
			0.0f,
		};
		alListenerfv(AL_ORIENTATION, orientation);
	}

	void resetListener()
	{
		alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
		alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
		float orientation[6] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
		alListenerfv(AL_ORIENTATION, orientation);
	}
}
