#pragma once
#include <Dialogue.h>
#include <Audio.hpp>
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <deque>

class ScriptSystem
{
private:
	// 对话系统
	Dialogue* pDialogue = nullptr;
	std::string mCurrentSection;
	// 存储解析后的对话段落
	std::map<std::string, std::vector<Message*>> mDialogueSections;
	// 音频系统
	esl::Audio* pAudio = nullptr;
	esl::AudioEngine* pAudioEngine = nullptr;

	struct AudioInfo 
	{
		std::wstring musicTitle{};
		ma_uint64 start = 0;        // 起始时间
		ma_uint64 prelude = 0;      // 前奏长度
		ma_uint64 loopStart = 0;    // 循环开始
		ma_uint64 loopLength = 0;   // 循环长度
		esl::Sprite* titleSprite = nullptr;// 音乐标题
		esl::Texture* texture = nullptr;
	};
	int mCurrentAudio = 0;
	std::deque<AudioInfo*> mAudioFmts;
	const double AUDIO_DELAY_TIME = -1.0;
	double mAudioTimer = AUDIO_DELAY_TIME;
	bool mAudioTitleAnimation = false;
	esl::Window* mRenderer = nullptr;
	glm::vec2 mCenterPos{0,0};

	bool mStageCleared = false;
	bool mStageClearAnimationRev = false;
	esl::Texture* mStageClearTexture = nullptr;
	esl::Sprite* mStageClearSprite = nullptr;
	double mStageClearTimer = 0.0;

	// 音效表
	std::unordered_map<std::string,std::unique_ptr<esl::Audio>> mSoundEffects;
public:
	ScriptSystem();
	~ScriptSystem();
	// ========== 初始化系统 ==========
	void initDialogueSystem(esl::Window& renderer);
	void initAudioSystem(esl::Window& renderer);

	// ========== 音频系统 ==========
	void setAudio(int audio);
	void loadAudioScript(const std::string& path);
	void nextAudio();
	void stopAudio();
	void pauseAudio();
	void resumeAudio();
	void playAudio();
	void setCenterPos(glm::vec2 pos) { 
		mCenterPos = pos; 
		mStageClearSprite->setPosition(pos+glm::vec2{0,600});
	}
	// ========== 对话系统 ==========
	bool mDialogueActived = false;
	// 预加载整个对话文件，但不激活
	void preloadDialogueScript(const std::string& path);
	// 激活特定段落的对话
	void activateDialogueSection(const std::string& sectionName);

	// 检查是否有可用的对话段落
	bool hasDialogueSection(const std::string& sectionName) const;
	// ========== 音效系统 ==========
	bool preloadSoundEffect(const std::string& dirPath);
	void playSoundEffect(const std::string& soundName);
	void setSoundEffectVolume(float volume);
	// ========== 场景清理 ==========
	void stageClear();
	// ========== 符卡吟诵 ==========
	
	// ========== 更新和渲染 ==========
	void render();
	void update(double deltaTime);
	void processInput(esl::Event& e);

	// ========== 辅助方法 ==========
	void trim(std::wstring& str);
	std::string parseStrVal(std::wstring& wstr);
	std::wstring fileConvertToWideString(const std::string& path);
};