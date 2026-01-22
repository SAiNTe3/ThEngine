#define MINIAUDIO_IMPLEMENTATION
#include "Audio.hpp"
namespace esl
{
	Audio::Audio() = default;
	Audio::Audio(const std::string& audioPath, AudioEngine& engine)
	{
		this->bindEngine(&engine);
		this->loadFromFile(audioPath);
	}
	Audio::~Audio()
	{
		ma_sound_uninit(&m_Sound);
	}
	void Audio::play()
	{
		if (!m_Engine || !m_Initialized)
		{
			return;
		}
		if (m_Paused)
		{
			ma_sound_seek_to_pcm_frame(&m_Sound, m_PausedCurser);
			m_Paused = false;
		}
		ma_sound_start(&m_Sound);
	}
	void Audio::pause()
	{
		if (m_Paused)
		{
			return;
		}
		ma_sound_get_cursor_in_pcm_frames(&m_Sound, &m_PausedCurser);
		ma_sound_stop(&m_Sound);
		m_Paused = true;
	}
	void Audio::stop()
	{
		ma_sound_stop(&m_Sound);
		m_PausedCurser = 0;
		ma_sound_seek_to_pcm_frame(&m_Sound, 0);
		m_Paused = false;
	}
	void Audio::loadFromFile(const std::string& path)
	{
		m_Result = ma_sound_init_from_file(m_Engine, path.c_str(), 0, nullptr, nullptr, &m_Sound);
		if (m_Result != MA_SUCCESS)
		{
			std::cout << "Can't open audio file" << std::endl;
			return;
		}
		m_Initialized = true;
	}
	void Audio::seekToFrame(ma_uint64 frame)
	{
		if (m_Initialized) {
			ma_sound_seek_to_pcm_frame(&m_Sound, frame);
			// 更新暂停光标位置
			m_PausedCurser = frame;
		}
	}
	ma_uint64 Audio::getCurrentFrame()
	{
		ma_uint64 cursor = 0;
		if (m_Initialized) {
			ma_sound_get_cursor_in_pcm_frames(&m_Sound, &cursor);
		}
		return cursor;
	}
	void Audio::setLoop(bool isLooping)
	{
		ma_sound_set_looping(&m_Sound, isLooping);
	}
	void Audio::setVolume(float volume)
	{
		ma_sound_set_volume(&m_Sound, volume);
	}
	float Audio::getVolume() const
	{
		return ma_sound_get_volume(&m_Sound);
	}
	void Audio::setPan(float pan)
	{
		ma_sound_set_pan(&m_Sound, pan);
	}
	float Audio::getPan() const
	{
		return ma_sound_get_pan(&m_Sound);
	}
	void Audio::setPitch(float pitch)
	{
		ma_sound_set_pitch(&m_Sound, pitch);
	}
	float Audio::getPitch() const
	{
		return ma_sound_get_pitch(&m_Sound);
	}
	void Audio::bindEngine(AudioEngine* engine)
	{
		m_Engine = &(engine->m_Engine);
	}

	void Audio::setLoopInterval(ma_uint64 loopStartFrame, ma_uint64 loopEndFrame)
	{
		ma_data_source* pDataSource = ma_sound_get_data_source(&m_Sound);
		ma_result result = ma_data_source_set_loop_point_in_pcm_frames(pDataSource, loopStartFrame, loopEndFrame);
		if (result != MA_SUCCESS) {
			std::cout << "Failed: " << result << std::endl;
		}
		this->setLoop(true);
	}

	void Audio::setLoopInterval(double loopStartSecond, double loopEndSecond)
	{
		ma_uint64 sampleRate = ma_engine_get_sample_rate(this->m_Engine);
		ma_uint64 startFrame = static_cast<ma_uint64>(std::round(loopStartSecond * sampleRate));
		ma_uint64 endFrame = static_cast<ma_uint64>(std::round(loopEndSecond * sampleRate));
		this->setLoopInterval(startFrame, endFrame);
	}

	void Audio::setLoopInterval(double loopStartSecond, double loopEndSecond, ma_uint32 sampleRate)
	{
		ma_uint64 startFrame = static_cast<ma_uint64>(std::round(loopStartSecond * sampleRate));
		ma_uint64 endFrame = static_cast<ma_uint64>(std::round(loopEndSecond * sampleRate));
		this->setLoopInterval(startFrame, endFrame);
	}


}
