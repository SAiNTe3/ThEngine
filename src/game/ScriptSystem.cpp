#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include <ScriptSystem.h>
#include <fstream>
#include <string>
#include <filesystem>
#include <locale>
#include <codecvt>
#include <sstream>
#include <Scene.h>
#include <Enemy.h>
#include <Action.h>
ScriptSystem::ScriptSystem()
{

}
ScriptSystem::~ScriptSystem()
{
	delete pDialogue;
	delete pAudio;
	delete mStageClearTexture;
	delete mStageClearSprite;

}
void ScriptSystem::initDialogueSystem(esl::Window& renderer)
{
	pDialogue = new Dialogue();
	pDialogue->init(renderer);
	
}
void ScriptSystem::initAudioSystem(esl::Window& renderer)
{
	pAudioEngine = new esl::AudioEngine(44100);
	pAudio = new esl::Audio("Assets/audio/thbgm.dat", *pAudioEngine);
	loadAudioScript("Assets/audio/thbgm.fmt");
	pAudio->setLoop(true);
	setAudio(mCurrentAudio);
	
	for (int i = 1; i < mAudioFmts.size(); i++) {
		bool isStage = i % 2;
		if (isStage)
			mAudioFmts[i]->texture = new esl::Texture("Assets\\front\\logo\\st0" + std::to_string(i) + "logo.png");
		else mAudioFmts[i]->texture = mAudioFmts[i - 1]->texture;
		mAudioFmts[i]->titleSprite = new esl::Sprite(mAudioFmts[i]->texture);
		mAudioFmts[i]->titleSprite->setTextureRect({ 0,isStage ? 32 : 0, }, { 768,32 });
	}
	mRenderer = &renderer;
	mStageClearTexture = new esl::Texture("Assets/front/front01.png");
	mStageClearSprite = new esl::Sprite(mStageClearTexture);
}
void ScriptSystem::nextAudio()
{
	setAudio(++mCurrentAudio);
	playAudio();
	mAudioTitleAnimation = true;
	mAudioTimer = AUDIO_DELAY_TIME;
	mAudioFmts[mCurrentAudio]->titleSprite->setPosition(mCenterPos + glm::vec2{0,-120});
}
void ScriptSystem::playAudio()
{
	pAudio->play();
}
void ScriptSystem::stopAudio()
{
	pAudio->stop();
}
void ScriptSystem::pauseAudio()
{
	pAudio->pause();
}
void ScriptSystem::resumeAudio()
{
	pAudio->play();
}
void ScriptSystem::setAudio(int audio)
{
	int channels = 2;      // 默认立体声
	int sampleRate = 44100; // 默认采样率
	int bitsPerSample = 16; // 默认16位
	auto bytesToFrames = [](ma_uint64 bytes) -> ma_uint64 {
		int bytesPerFrame = 2 * (16 / 8);
		return (bytesPerFrame > 0) ? bytes / bytesPerFrame : 0;
	};
	mCurrentAudio = audio;
	AudioInfo fmt = *mAudioFmts[mCurrentAudio];
	pAudio->setLoopInterval(bytesToFrames(fmt.loopStart), bytesToFrames(fmt.loopStart + fmt.loopLength));
	pAudio->seekToFrame(bytesToFrames(fmt.start));

}
void ScriptSystem::loadAudioScript(const std::string& path)
{
	std::wistringstream stream(fileConvertToWideString(path));
	std::wstring line;
	while (std::getline(stream, line))
	{
		auto parseAudioInfo=[this, &stream, &line]() {
			AudioInfo* fmt = nullptr;
			while(std::getline(stream, line))
			{
				if (line.find(L"@a") != std::wstring::npos)
				{
					fmt = new AudioInfo();
				}
				else if (line.find(L"title") != std::wstring::npos)
				{
					trim(line);
					if (line.find(L"\"") != std::wstring::npos) {
						size_t first_quote = line.find_first_of(L"\"");
						size_t last_quote = line.find_last_of(L"\"");
						if (first_quote != std::wstring::npos && last_quote != std::wstring::npos && first_quote < last_quote) {
							fmt->musicTitle = line.substr(first_quote + 1, last_quote - first_quote - 1);
						}
					}
				}
				else if (line.find(L"start") != std::wstring::npos)
				{
					fmt->start = std::stoull(parseStrVal(line), nullptr, 0);
				}
				else if (line.find(L"prelude") != std::wstring::npos)
				{
					fmt->prelude = std::stoull(parseStrVal(line), nullptr, 0);
				}
				else if (line.find(L"loopStart") != std::wstring::npos)
				{
					fmt->loopStart = std::stoull(parseStrVal(line), nullptr, 0);
				}
				else if (line.find(L"loopLength") != std::wstring::npos)
				{
					fmt->loopLength = std::stoull(parseStrVal(line), nullptr, 0);
				}
				else if (line.find(L"@end") != std::wstring::npos)
				{
					mAudioFmts.emplace_back(fmt);
					fmt = nullptr;
				}
				else if (line.find(L"@#") != std::wstring::npos)
				{
					break;
				}
			}
		};
		trim(line);
		if (line.empty()) continue;
		if (line.find(L"@BGM") != std::wstring::npos)
		{
			parseAudioInfo();
		}
	}
}

void ScriptSystem::render()
{
	if(pDialogue && mDialogueActived)
		pDialogue->render();
	if (mAudioTitleAnimation) {
		mRenderer->draw(*mAudioFmts[mCurrentAudio]->titleSprite);
	}
	if(mStageCleared)
		mRenderer->draw(*mStageClearSprite);
}

void ScriptSystem::update(double deltaTime)
{
	//对话更新
	if (pDialogue && mDialogueActived) {
		pDialogue->update(deltaTime);

		// 调试：打印当前状态
		static bool hasStartedExit = false;

		// 检查对话是否结束
		if (Message::isEmpty()) {
			// 触发退出动画（只触发一次）
			if (!hasStartedExit && !pDialogue->isExiting()) {
				pDialogue->startExit();
				hasStartedExit = true;

			}

			// 等待退出动画完成
			if (pDialogue->mIsExiting == false && hasStartedExit) {
				// 退出动画已完成
				mDialogueActived = false;
				hasStartedExit = false;  // 重置标志
			}
		}
		else {
			// 重置标志（如果消息不为空）
			hasStartedExit = false;
		}
	}
	//音乐标题更新
	if (mAudioTitleAnimation) {
		mAudioTimer += deltaTime;
		float alpha = 1.0f;
		if (mAudioTimer < 0) {
			
		}
		else if (mAudioTimer >= 0 && mAudioTimer < 1) {
			mAudioFmts[mCurrentAudio]->titleSprite->move({ 0 ,40 * deltaTime });
		}
		else if (mAudioTimer >= 1 && mAudioTimer < 2) {
			alpha = 1;
		}
		else if (mAudioTimer >= 2 && mAudioTimer < 3) {
			alpha = static_cast<float>( - mAudioTimer + 3.0f);
		}
		else {
			alpha = 0.0f;
			mAudioTimer = AUDIO_DELAY_TIME;
			mAudioTitleAnimation = false;
		}
		mAudioFmts[mCurrentAudio]->titleSprite->setAlpha(alpha);
	}

	if (mStageCleared) {
		mStageClearTimer += deltaTime;
		if (mStageClearTimer <= 0.2) {
			mStageClearAnimationRev = false;
		}
		else if (mStageClearTimer > 0.2 && mStageClearTimer <= 1.2) {
			
		}
		else if (mStageClearTimer > 1.2 && mStageClearTimer <= 1.4) {
			mStageClearAnimationRev = true;
		}
		else {
			mStageClearAnimationRev = false;
			mStageCleared = false;
			mStageClearTimer = 0.0;
		}
		float scaley = mStageClearSprite->getScale().y;
		scaley +=4*static_cast<float>(mStageClearAnimationRev ? -deltaTime : deltaTime);
		if (scaley > 1) scaley = 1;
		mStageClearSprite->setScale({ 1,scaley });
		
	}
	
}

void ScriptSystem::processInput(esl::Event& e)
{
	if(pDialogue && mDialogueActived)
		pDialogue->process_input(e);
}

void ScriptSystem::trim(std::wstring& str)
{
	size_t start = str.find_first_not_of(L" \t\r\n");
	if (start == std::wstring::npos) {
		str.clear();
		return;
	}
	size_t end = str.find_last_not_of(L" \t\r\n");
	str = str.substr(start, end - start + 1);
}

std::string ScriptSystem::parseStrVal(std::wstring& line)
{
	trim(line);
	std::wstring wstr = line.substr(line.find(L":") + 1);
	trim(wstr);
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(wstr);
}

std::wstring ScriptSystem::fileConvertToWideString(const std::string& path)
{
	std::filesystem::path filePath = path;
	if (!std::filesystem::exists(filePath)) {
		throw std::runtime_error("File not found: " + path);
	}
	// 使用ifstream读取UTF-8文件
	std::ifstream file(filePath, std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file: " + path);
	}
	// 读取整个文件内容
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();
	// 转换UTF-8到wstring
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.from_bytes(content);
}

void ScriptSystem::preloadDialogueScript(const std::string& path)
{
	std::wistringstream stream(fileConvertToWideString(path));
	std::wstring line;

	// 1. 解析角色信息（和原来一样）
	auto parseRole = [this, &stream, &line](size_t role) {
		std::getline(stream, line);
		std::string face_index_str = parseStrVal(line);
		float faceX, faceY, offsetY, cutScale;
		std::getline(stream, line);
		faceX = std::stof(parseStrVal(line));
		std::getline(stream, line);
		faceY = std::stof(parseStrVal(line));
		std::getline(stream, line);
		offsetY = std::stof(parseStrVal(line));
		std::getline(stream, line);
		cutScale = std::stof(parseStrVal(line));
		switch (role) {
		case 0:
			
			pDialogue->player.face_pos = { faceX, faceY };
			pDialogue->player.offsetY = offsetY;
			pDialogue->player.cutScale = cutScale;
			pDialogue->player.init("Assets/face/pl" + face_index_str + "/", face_index_str, false);
			break;
		case 1:
			
			pDialogue->boss.face_pos = { faceX, faceY };
			pDialogue->boss.offsetY = offsetY;
			pDialogue->boss.cutScale = cutScale;
			pDialogue->boss.init("Assets/face/enemy" + face_index_str + "/", face_index_str, true);
			break;
		}
		};

	// 2. 修改：解析对话段落（支持分段）
	auto parseDialogueSection = [this, &stream, &line](const std::string& sectionName) {
		std::vector<Message*> messages;  // 临时存储这一段的消息

		while (std::getline(stream, line)) {
			trim(line);
			if (line.empty()) continue;

			// 如果遇到 @#，当前段落结束
			if (line.find(L"@#") != std::wstring::npos) {
				break;
			}

			// 如果遇到新的段落标记，停止解析
			if (line.find(L"@DialogueSection:") != std::wstring::npos) {
				break;
			}

			size_t text_line_count = 0;
			if (line.find(L"@1") != std::wstring::npos) {
				text_line_count = 1;
			}
			else if (line.find(L"@2") != std::wstring::npos) {
				text_line_count = 2;
			}
			else {
				continue;
			}

			// 读取属性
			int role = 0, face = 0, balloon = 0;
			for (int i = 0; i < 3; i++) {
				if (!std::getline(stream, line)) break;
				trim(line);

				if (line.find(L"role:") != std::wstring::npos) {
					std::wstring role_wstr = line.substr(line.find(L":") + 1);
					trim(role_wstr);
					role = std::stoi(std::string(role_wstr.begin(), role_wstr.end()));
				}
				else if (line.find(L"face:") != std::wstring::npos) {
					std::wstring face_wstr = line.substr(line.find(L":") + 1);
					trim(face_wstr);
					face = std::stoi(std::string(face_wstr.begin(), face_wstr.end()));
				}
				else if (line.find(L"balloon:") != std::wstring::npos) {
					std::wstring balloon_wstr = line.substr(line.find(L":") + 1);
					trim(balloon_wstr);
					balloon = std::stoi(std::string(balloon_wstr.begin(), balloon_wstr.end()));
				}
			}

			// 创建消息
			Message* message = new Message(role);

			for (size_t i = 0; i < text_line_count; i++) {
				if (!std::getline(stream, line)) break;
				trim(line);

				if (line.find(L"\"") != std::wstring::npos) {
					size_t first_quote = line.find_first_of(L"\"");
					size_t last_quote = line.find_last_of(L"\"");
					if (first_quote != std::wstring::npos && last_quote != std::wstring::npos && first_quote < last_quote) {
						std::wstring text = line.substr(first_quote + 1, last_quote - first_quote - 1);
						message->addText(text);
					}
				}
			}

			message->setFont(*pDialogue->font);
			message->setFace(face);
			message->setPosition(mCenterPos + glm::vec2{ role == 0 ? -200:200,200});
			message->setBalloonStyle(balloon);

			// 不立即 endEdit，而是存储起来
			messages.push_back(message);
		}

		// 保存这一段的消息
		mDialogueSections[sectionName] = messages;
	};

	// 3. 解析文件
	std::string currentSection;
	while (std::getline(stream, line)) {
		trim(line);
		if (line.empty()) continue;

		if (line.find(L"@Player") != std::wstring::npos) {
			parseRole(0);
		}
		else if (line.find(L"@Enemy") != std::wstring::npos) {
			parseRole(1);
		}
		else if (line.find(L"@DialogueSection:") != std::wstring::npos) {
			// 提取段落名称
			size_t colonPos = line.find(L":");
			if (colonPos != std::wstring::npos) {
				std::wstring sectionNameW = line.substr(colonPos + 1);
				trim(sectionNameW);
				currentSection = std::string(sectionNameW.begin(), sectionNameW.end());
				parseDialogueSection(currentSection);
			}
		}
		else if (line.find(L"@#") != std::wstring::npos) {
			break;
		}
	}
}
// 新增：激活特定段落
void ScriptSystem::activateDialogueSection(const std::string& sectionName)
{
	if (mDialogueSections.find(sectionName) == mDialogueSections.end()) {
		//std::cerr << "[ScriptSystem] Dialogue section not found: " << sectionName << std::endl;
		return;
	}
	if (pDialogue) {
		pDialogue->reset();
		//std::cout << "[ScriptSystem] Dialogue state reset" << std::endl;
	}
	// 将该段落的消息添加到队列
	for (auto* message : mDialogueSections[sectionName]) {
		if (!message) {
			//std::cerr << "[ScriptSystem] NULL message in section: " << sectionName << std::endl;
			continue;
		}
		message->endEdit();  // 现在才真正添加到消息队列
	}

	mCurrentSection = sectionName;
	mDialogueActived = true;

	//std::cout << "[ScriptSystem] Activated dialogue section: " << sectionName << std::endl;
}

// 检查段落是否存在
bool ScriptSystem::hasDialogueSection(const std::string& sectionName) const
{
	return mDialogueSections.find(sectionName) != mDialogueSections.end();
}

bool ScriptSystem::preloadSoundEffect(const std::string& dirPath)
{
	if (!std::filesystem::exists(dirPath)) {
		std::cerr << "Directory not found: " << dirPath << std::endl;
		return false;
	}
	try {
		for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
			if (entry.is_regular_file()) {
				std::string extension = entry.path().extension().string();
				// 检查是否为音频文件
				if (extension == ".wav" || extension == ".mp3" || extension == ".ogg") {
					std::string filePath = entry.path().string();
					std::string fileName = entry.path().filename().string();

					// 音效加载逻辑
					mSoundEffects[fileName] = std::move(std::make_unique<esl::Audio>(filePath,*pAudioEngine));

					std::cout << "Loaded sound effect: " << fileName << std::endl;
				}
			}
		}
		return true;
	}
	catch (const std::filesystem::filesystem_error& e) {
		std::cerr << "Failed to preload sound effects: " << e.what() << std::endl;
		return false;
	}
}

void ScriptSystem::playSoundEffect(const std::string& soundName)
{
	auto it = mSoundEffects.find(soundName);
	if (it != mSoundEffects.end()) {
		it->second->stop();
		it->second->play();
	}
	else {
		std::cerr << "Sound effect not found: " << soundName << std::endl;
	}
}

void ScriptSystem::setSoundEffectVolume(float volume)
{
	for (auto& [name, audio] : mSoundEffects) {
		audio->setVolume(volume);
	}
}

void ScriptSystem::stageClear()
{
	mStageCleared = true;
	mStageClearSprite->setScale({ 1,0 });
	//this->playSoundEffect("");
}
