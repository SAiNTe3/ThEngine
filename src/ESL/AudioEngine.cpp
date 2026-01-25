#include "AudioEngine.hpp"
namespace esl
{
	AudioEngine::AudioEngine()
	{
		ma_engine_init(nullptr, &m_Engine);
	}
	AudioEngine::AudioEngine(ma_uint64 sampleRate)
	{
		ma_engine_config engineConfig = ma_engine_config_init();
		engineConfig.sampleRate = static_cast<ma_uint32>(sampleRate);
		ma_engine_init(&engineConfig, &m_Engine);
	}
	AudioEngine::~AudioEngine()
	{
		ma_engine_uninit(&m_Engine);
	}

	ma_uint64 AudioEngine::getEngineSampleRate()
	{
		return ma_engine_get_sample_rate(&m_Engine);
	}

}
