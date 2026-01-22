#pragma once

#include <Window.hpp>
#include <Sprite.hpp>
#include <Clock.hpp>
#include <Player.h>
#include <Enemy.h>
#include <CollisionManager.h>  // 添加碰撞管理器头文件
#include <Front.h>
#include <Background3D.h>
#include <ScriptSystem.h>
#include <Stage.h>
#ifndef PRINT_INFO
#define PRINT_INFO printf
#endif

using pSprite = std::unique_ptr<esl::Sprite>;
using pTexture = std::unique_ptr<esl::Texture>;
// 场景基类
class Scene {

protected:
	esl::Clock mSceneClock;
	ScriptSystem& mScriptSystem;
public:
	struct SceneInfo {
		bool mSwitchToNextScene = false;
		int mSwitchToSceneIndex = -1;
	}mSceneInfo;
	Scene(ScriptSystem& scriptSystem):mScriptSystem(scriptSystem){}
	virtual void process_input(esl::Event& e);
	virtual void render();
	virtual void update(double deltaTime);
};

// 游戏主界面
class TitleScene :public Scene {
	esl::Window& mRender;
	// 背景图片
	pTexture mTitleBackTexture;
	pSprite mTitleBackgroundSprite;
	// 主界面选项
	std::vector<pTexture> mMenuSelectionTextures;
	std::vector<pSprite> mMenuSelections;
	int mSelectIndex = 0;
	bool mSelectRespond = true;
public:
	TitleScene(esl::Window& render, ScriptSystem& system);
	virtual void process_input(esl::Event& e);
	virtual void render();
	virtual void update(double deltaTime);
};
struct Data {
	unsigned int mPlayerLife = 3;
	unsigned int mPlayerSpellCard = 3;
	unsigned int mPlayerScore = 0;
	unsigned int mPlayerPower = 100;
	unsigned mHighScore = 100000000;
	unsigned mMoney = 0;
};
class MainGame :public Scene {
	const float LEFT = 64.0f;
	const float RIGHT = 64 + 768.0f;
	const float TOP = 896 + 32;
	const float BOTTOM = 32.0f;
	esl::Window& mRender;
	Player* mPlayer;
	double mDeltaTime = 0;
	std::vector<Enemy*> mEnemys;
	CollisionManager mCollisionManager;  // 添加碰撞管理器
	Front* mFront;  // 添加前景对象
	Stage mStage;
	Data mData;
	Background3D* mBackground;
	glm::vec2 mCenterPos { 768.0f / 2 + 64 ,128 };
	bool mPause = false;
	std::vector<Bullet*> mAllEnemyBullets;
public:
	MainGame(esl::Window& render, ScriptSystem& system);
	virtual void process_input(esl::Event& e);
	virtual void render();
	virtual void update(double deltaTime);
	void data_maintain();
	void pause();
	void resume();
	std::vector<Enemy*>& getEnemys() { return this->mEnemys; }
	void setupStage();
	glm::vec2 Position(glm::vec2 pos = {0,0}) {
		return mCenterPos + pos;
	}
	static int Random(int min, int max) {

		static std::random_device rd;
		static std::mt19937 gen(rd()); // Mersenne Twister 随机数生成器

		std::uniform_int_distribution<> distrib(min, max);

		// 生成一个随机数
		return distrib(gen);
	}
	static glm::vec2 RandomPos(int radius, glm::vec2 center) {
		if (radius <= 0) return center;
		float angle = static_cast<float>(Random(0, 360)) * 3.1415926f / 180.0f; // 随机角度，转换为弧度
		float r = static_cast<float>(Random(0, radius)); // 随机半径
		float x = center.x + r * cos(angle);
		float y = center.y + r * sin(angle);
		return { x,y };
	}
};