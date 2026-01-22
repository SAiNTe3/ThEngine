#pragma once
#include "AudioEngine.hpp"

namespace esl {
	class Audio {

	private:
		ma_engine* m_Engine = nullptr;
		ma_sound m_Sound;
		ma_result m_Result = MA_SUCCESS;
		ma_uint64 m_PausedCurser = 0;
		bool m_Paused = false;
		bool m_Initialized = false;
		std::string m_AudioPath;
	public:
		Audio();
		Audio(const std::string& audioPath, AudioEngine& engine);
		~Audio();
		void play();
		void pause();
		void stop();
		void loadFromFile(const std::string& path);
		void seekToFrame(ma_uint64 frame);
		ma_uint64 getCurrentFrame();
		void setLoop(bool isLooping);
		void setVolume(float volume);
		float getVolume()const;
		void setPan(float pan);
		float getPan()const;
		void setPitch(float pitch);
		float getPitch()const;
		void bindEngine(AudioEngine* engine);
		void setLoopInterval(ma_uint64 loopStartFrame, ma_uint64 loopEndFrame);
		void setLoopInterval(double loopStartSecond, double loopEndSecond);
		void setLoopInterval(double loopStartSecond, double loopEndSecond, ma_uint32 sampleRate);
		ma_uint32 getSampleRate();
	};
};
