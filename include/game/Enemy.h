#pragma once
#include <iostream>
#include <Player.h>
#include <Bullet.h>
#include <GameObject.h>  // 引入基类
#include <ProgressSprite.hpp>
#include <Sprite3D.hpp>
#include <deque>
#include <functional>
#include "ActionFactory.h"
#include "MovementBuilder.h"
#include <Animation.h>
#ifndef PRINT_INFO
#define PRINT_INFO printf
#endif

using pSprite = std::unique_ptr<esl::Sprite>;
using pTexture = std::unique_ptr<esl::Texture>;
extern std::string enemy_texture_path;
class Action;
class ScriptSystem;
using pAction = std::unique_ptr<Action>;
using pBullet = std::unique_ptr<Bullet>;

// Enemy 类 - 继承自 GameObject
class Enemy : public GameObject {
private:

protected:
	size_t mFrame = 0;
	size_t mSpriteIndex = 0;
	static esl::Window* mRenderer;
	std::function<glm::vec2()> mGetPlayerPos;

	// Enemy 的事件队列
	std::deque<pAction> mMovementActions;  // 敌人自己的移动事件
	std::deque<pAction> mDanmakuActions;   // 发射子弹的事件（每个事件发射一组子弹）
	pAction mDeathAction = nullptr;
	pAction mSpawnAction = nullptr;
	void updateActionQueue(std::deque<pAction>& actions, double delta);
	struct BonusNumber {
		int powerUp = 0;
		int power = 0;
		int lifeUp = 0;
		int life = 0;
		int spellCard = 0;
		int fullPower = 0;
		int point = 0;
		int radius = 0;
	}mBonusNum;
	static pTexture sHPBar;
	
	
	int mEnemyHP = 0;
	bool isHitByPlayer = false;
	double hitAnimationTimer = 0.0;

	static ScriptSystem* sScriptSystem;
public:	
	
	virtual void DeathSoundEffect();
	enum class EnemyType {
		NORMAL, BOSS, EMITTER
	}mEnemyType = EnemyType::NORMAL;
	std::vector<pBullet> mBullets;  // Enemy 发射的子弹列表
	double mCollisionRadius = 10;
	static void init(esl::Window* renderer);
	static void cleanup();
	static void setSystem(ScriptSystem* system) {
		sScriptSystem = system;
	}
	enum class ActionType {
		SPAWN,		// 出场
		MOVEMENT,  // 敌人自己的移动
		DANMAKU,   // 发射子弹（每个Action发射一组子弹）
		DEATH      // 死亡动作
	};
	
	int mMaxHP = 0;
	float mSpeed = 0;
	float mAngle = 0;
	bool mSpriteAvailable = true;
	bool mBulletsAvailable = true;
	bool mSpwaned = false;
	bool mFinished = false;
	bool mHitable = true;
	// 是否在敌人死亡后清除其弹幕
	bool mClearBulletAfterDeath = false;
	Enemy() = default;
	virtual ~Enemy() = default;
	int getHP() { return mEnemyHP; }
	// 实现 GameObject 接口
	void update(double delta) override;
	void render() override;
	float getHPRatio() { return float(mEnemyHP) / float(mMaxHP); }
	// Enemy 特有方法
	void setBonus(int powerUp, int power, int lifeUp, int life, int spellCard, int fullPower, int point, int radius = 16);
	void updateBullets(double deltaTime);
	void clearBullets();
	virtual void onBulletHit(int damage = 10);
	virtual void setLifeBarVisiable(bool visiable) {};
	pSprite& getSprite();

	// Action 管理
	void addAction(pAction action, ActionType type);
	void addAwait(double awaitTime);
	void clearAction();
	
	// 新的流式 API
	LinearMovement& moveTo() {
		static thread_local LinearMovement builder;
		builder = LinearMovement(); // 重置
		return builder;
	}

	CircularMovement& moveCircular() {
		static thread_local CircularMovement builder;
		builder = CircularMovement();
		return builder;
	}

	EllipticalMovement& moveElliptical() {
		static thread_local EllipticalMovement builder;
		builder = EllipticalMovement();
		return builder;
	}


	void setPlayerPosGetter(std::function<glm::vec2()> getter);

	// ========== 新的弹幕 API ==========
	// 创建弹幕 Action 构建器
	DanmakuAction& shoot();

};

// 统一的敌人类，整合了 AnimalSpirit 和 EnemyNormal 的功能
class EnemyUnit : public Enemy {
private:
	static pTexture sAnimalTexture;
	static pTexture sNormalTexture;
	
	friend class Enemy;  // 允许 Enemy::cleanup 访问私有静态成员

	std::vector<glm::vec2> mRect;
	glm::vec2 mSize;
	size_t mSpriteNum;
	bool mIsAnimalSpirit;

public:
	// 动物精灵类型枚举
	enum class AnimalType {
		BLUE, GREEN, RED
	};
	
	// 普通敌人类型枚举  
	enum class NormalType {
		TYPE1 = 1, TYPE2, TYPE3, TYPE4, TYPE5, TYPE6, TYPE7, TYPE8, TYPE9
	};

	void texture_init();

	// 创建动物精灵的构造函数
	EnemyUnit(AnimalType type, bool isHorizon, glm::vec2 pos = {0, 0}, int HP = 100);
	
	// 创建普通敌人的构造函数
	EnemyUnit(NormalType type, glm::vec2 pos = {0, 0}, int HP = 100);

	virtual void update(double delta) override;
	virtual void render() override;

private:
	void initAnimalSpirit(AnimalType type, bool isHorizon, glm::vec2 pos, int HP);
	void initNormalEnemy(NormalType type, glm::vec2 pos, int HP);
};

class Boss : public Enemy
{
	pTexture texture = nullptr;
	pTexture hp_fore_texture, hp_back_texture;
	pTexture magic_square_texture;
	using pProgress = std::unique_ptr<esl::ProgressSprite>;

	pProgress hp_fore, hp_back;
	pSprite magic_square;
	glm::vec2 mSize;
	size_t mSpriteIndex = 0;
	void initBossLifeBar();
	void initBoss(size_t boss,int hp,glm::vec2 pos);
	enum class BossStatus {
		None,Move,SpellCard
	}mStatus = BossStatus::None;
	long double totalTime = 0;

	DeathCircle mDeathCircle;

protected:
	
public:
	void DeathSoundEffect() override;
	Boss(size_t boss, int hp,glm::vec2 pos);
	virtual void update(double deltaTime)override;
	virtual void render()override;
	virtual void setLifeBarVisiable(bool visiable) { 
		this->hp_back->setAvailable(visiable);
		this->hp_fore->setAvailable(visiable);
	};
	void loadFromFile(const std::string& path);
};

// 在 Enemy.h 的末尾，Boss 类之后添加

// ========== 弹幕发射器：不可见且不可交互的敌人 ==========
// 用途：创建复杂的弹幕组合，发射器可以有自己的运动轨迹
class DanmakuEmitter : public Enemy {
private:
	Enemy* mParentEnemy = nullptr;
public:
	DanmakuEmitter(glm::vec2 startPos);

	// 重写 render 方法：什么都不绘制
	void render() override;
	void update(double delta) override;

	// 重写 onBulletHit：不可被击中
	void onBulletHit(int damage = 10) override {}

	// 便捷方法：设置自动销毁时间
	void setLifetime(double seconds);

	void bindLifetimeTo(Enemy* parent) {
		mParentEnemy = parent;
	}
};