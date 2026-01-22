#pragma once
#include <iostream>
#include <Window.hpp>
#include <Sprite.hpp>
#include <algorithm>
#include <random>
#include <ScriptSystem.h>
#ifndef PRINT_INFO
#define PRINT_INFO printf
#endif

using pSprite = std::unique_ptr<esl::Sprite>;
using pTexture = std::unique_ptr<esl::Texture>;

// Forward declaration
class Enemy;

// 追踪弹结构
struct TraceBullet {
	pSprite sprite;
	glm::vec2 velocity;
	Enemy* target = nullptr;  // 当前追踪目标
	bool hasTarget = false;
	float speed = 600.0f;
	float rotateSpeed = 5.0f;  // 转向速度（度/帧）
	
	TraceBullet(pTexture& texture, glm::vec2 position) {
		sprite = std::make_unique<esl::Sprite>(texture.get());
		sprite->setPosition(position);
		sprite->setScale({2, 2});
		velocity = {0, 1}; // 向上方向（正Y方向）
	}
};

class Player {

protected:
	static ScriptSystem* mScriptSystem;
	size_t mFrame = 0;
	pSprite mSprite;
	pSprite mSlowEffectSprite;
	int mSpriteIndex = 0;
	glm::uvec2 mRect[10]{};
	pTexture mTexture;
	pTexture mSlowEffectTexture;
	pTexture mYinYangOrbTexture;
	pTexture mTraceBulletTexture;
	// 子弹纹理
	std::vector<pTexture> mBulletTextures;
	// 子弹容器
	// 射击间隔
	size_t mShootInterval = 0;
	size_t mYinYangOrbCount = 1;
	std::vector<pSprite> mYinYangOrbs;
	
	// 敌机列表的引用
	std::vector<Enemy*>* mEnemyList = nullptr;
	// 无敌状态
	bool mInvincible = false;
	double mInvincibleTimer = 1.0;
public:
	std::vector<pSprite> mBullets;
	unsigned int& mPower;
	struct MoveDirection {
		float h, v;
	}mDirection{0};
	int mSpeed = 400;
	bool mEnableShoot = false;
	bool mHyperMode = true;
	float mMissRadius = 3;
	Player(unsigned int& power): mPower(power) {}
	void set_position(glm::vec2 pos);
	glm::vec2 get_position();
	void move(glm::vec2 distance);
	virtual void update(double delta);
	virtual void shoot();
	virtual void render();
	virtual void slowEffectRender();
	// 设置敌机列表引用
	void setEnemyList(std::vector<Enemy*>* enemyList) { mEnemyList = enemyList; }
	static void setSystem(ScriptSystem* system) { mScriptSystem = system; }
	static void getItemSoundEffect(){
		if (mScriptSystem) {
			mScriptSystem->playSoundEffect("se_item00.wav");
		}
	}
	int mCollectRadius = 64;
	void hitPlayer(glm::vec2 rebirthPos);
	bool isInvincible() const { return mInvincible; }
};

class Reimu :public Player {
	esl::Window& mRenderer;
protected:
	void update_bullets(double delta);
	void update_trace_bullets(double delta);
	// 寻找最近的敌机目标
	Enemy* findNearestEnemy(glm::vec2 bulletPos);
	// 检查目标是否仍然有效
	bool isTargetValid(Enemy* target);
public:
	// 追踪弹容器 - 移到public让CollisionManager访问
	std::vector<std::unique_ptr<TraceBullet>> mTraceBullets;
	
	Reimu(esl::Window& renderer, unsigned int& power);
	virtual void update(double delta);
	virtual void shoot();
	virtual void render();
	virtual void slowEffectRender();
};