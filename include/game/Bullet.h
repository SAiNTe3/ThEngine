#pragma once
#include <iostream>
#include <memory>
#include <deque>
#include <functional>
#include <glm/glm.hpp>
#include <Sprite.hpp>
#include <Texture.hpp>
#include <Window.hpp>
#include <GameObject.h> 
#include "MovementState.h"    
#include "MovementUpdater.h"
extern std::string bullet_texture_path;
class BulletMovementAction; // Forward declaration
class Bullet;
// 前向声明
using pSprite = std::unique_ptr<esl::Sprite>;
using pTexture = std::unique_ptr<esl::Texture>;
using pBullet = std::unique_ptr<Bullet>;
using pBulletMovementAction = std::unique_ptr<BulletMovementAction>; // Added alias

class Player;


// ========== 子弹基类 ==========
class Bullet : public GameObject {
protected:
	struct EtBreakEffect {
		glm::vec2 position = { 0,0 };
		double lifetime = 0;
		size_t current_index = 0;
	};
	static pTexture etbreakTexture;
	static pSprite etbreakSprite;
	static std::vector<EtBreakEffect> etbreaks;
	static const glm::vec2 etbreakFrames[8];

public:
static void createEtBreakEffect(glm::vec2 pos);
static void updateEtBreaks(double deltaTime);
static void drawEtBreaks(esl::Window& renderer);
static void initEtBreak();
static void cleanupEtBreak();

	double mCollisionRadius = 10;
	std::deque<pBulletMovementAction> mMovementActions; // Changed type
	double lastGrazeTime = 0.0;

	Bullet() = default;
	virtual ~Bullet();

	void update(double delta) override {};

	void render() override {};
	virtual void addMovementAction(pBulletMovementAction action) = 0; // Changed signature
};

// ========== Bullet_1 实现 ==========
class Bullet_1 : public Bullet {
	static pTexture sTexture[6];
	esl::Window& mRenderer;
	void updateMovementActions(double deltaTime);

public:
	static void init();
	static void cleanup();
	float mAngle = 0;
	float mSpeed = 0;
	bool mSyncRotation = false;

	static void setupBulletProperties(
		esl::Sprite* sprite,
		double& collisionRadius,
		int type,
		int color
	);
	static esl::Texture* selectTexture(int type);

	Bullet_1(int type, int color, glm::vec2 pos, esl::Window& render, float angle, float speed);

	void update(double delta) override;
	void render() override;
	void addMovementAction(pBulletMovementAction action) override;
	void clearMovementActions();

	static void renderBatch(esl::Window& renderer, const std::vector<pBullet>& bullets);

	friend class BulletPool;
};


