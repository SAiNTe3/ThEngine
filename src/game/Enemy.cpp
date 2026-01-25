#include <Enemy.h>
#include "Action.h"
#include "ActionFactory.h"
#include <BulletPool.h>
#include <cmath>
#include <Item.h>
#include "ScriptSystem.h"
std::string enemy_texture_path = ".\\Assets\\enemy\\";
esl::Window* Enemy::mRenderer = nullptr;
// 统一敌人类的静态纹理
pTexture EnemyUnit::sAnimalTexture = nullptr;
pTexture EnemyUnit::sNormalTexture = nullptr;
pTexture Enemy::sHPBar = nullptr;
ScriptSystem* Enemy::sScriptSystem = nullptr;
void Enemy::init(esl::Window* renderer)
{
	mRenderer = renderer;
}

void Enemy::updateActionQueue(std::deque<pAction>& actions, double delta)
{
	if (!actions.empty()) {
		auto& currentAction = actions.front();
		currentAction->apply(this);
		if (currentAction->update(delta)) {
			// 当前动作完成，移除并执行下一个
			actions.pop_front();
		}
		mFinished = false;
	}
	else mFinished = true;
}

void Enemy::update(double delta)
{
	if (mSpawnAction) {
		mHitable = false;
		static bool firstApply = true;
		if (firstApply) {
			firstApply = false;
			sScriptSystem->playSoundEffect("se_ch02.wav");
			
		}
		mSpawnAction->apply(this);
		if (mSpawnAction->update(delta)) {
			mSpawnAction = nullptr;
		}
		return;
	}
	mHitable = true;
	updateActionQueue(mMovementActions, delta);
	updateActionQueue(mDanmakuActions, delta);
	updateBullets(delta);
	if (mMovementActions.empty() && mDanmakuActions.empty() && !mDeathAction) {
		mSpriteAvailable = false;
		mHitable = false;
	}
	if (mBullets.empty()) mBulletsAvailable = false;
	else mBulletsAvailable = true;

	
	if (isHitByPlayer) {
		hitAnimationTimer += delta;
		if (hitAnimationTimer < 0.1) {
			mSprite->setColor(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
		}
		else {
			mSprite->setColor(glm::vec4(1.f, 1.f, 1.f, 1.0f));
			hitAnimationTimer = 0.0;
			isHitByPlayer = false;
		}
	}
	

	//优化：延迟删除，批量处理
	const float SCREEN_LEFT = 0.0f;
	const float SCREEN_RIGHT = 896.0f;
	const float SCREEN_TOP = 960.0f;
	const float SCREEN_BOTTOM = 0.0f;

	// 使用 erase-remove idiom 一次性删除所有失效子弹
	mBullets.erase(
		std::remove_if(mBullets.begin(), mBullets.end(),
			[&](pBullet& bullet) {
				if (!bullet) return true;

				glm::vec2 pos = bullet->getPosition();

				// 超出屏幕边界
				if (pos.x < SCREEN_LEFT || pos.x > SCREEN_RIGHT ||
					pos.y > SCREEN_TOP || pos.y < SCREEN_BOTTOM) {

					// 归还到对象池
					if (auto* bullet1 = dynamic_cast<Bullet_1*>(bullet.get())) {
						std::unique_ptr<Bullet_1> ptr(
							static_cast<Bullet_1*>(bullet.release())
						);
						BulletPoolHelper::returnBullet(std::move(ptr));
					}
					return true;
				}

				return false;
			}),
		mBullets.end()
	);
}

///PowerUp,Power,LifeUp,Life,SpellCard,FullPower,Point
void Enemy::setBonus(int powerUp, int power, int lifeUp, int life, int spellCard, int fullPower, int point, int radius)
{
	mBonusNum.powerUp = powerUp;
	mBonusNum.power = power;
	mBonusNum.lifeUp = lifeUp;
	mBonusNum.life = life;
	mBonusNum.spellCard = spellCard;
	mBonusNum.fullPower = fullPower;
	mBonusNum.point = point;
	mBonusNum.radius = radius;
}

void Enemy::updateBullets(double deltaTime)
{
	for (auto& bullet : mBullets) {
		if (bullet) bullet->update(deltaTime);
	}
}

void Enemy::render()
{
	if (mSprite && mSpriteAvailable) {
		mRenderer->draw(*mSprite);
	}

	// 优化：批量渲染子弹
	if (!mBullets.empty()) {
		Bullet_1::renderBatch(*mRenderer, mBullets);
	}
}

void Enemy::onBulletHit(int damage)
{
	if (mEnemyHP <= 0 || !mHitable) return;

	mEnemyHP -= damage;
	isHitByPlayer = true;

	// 如果敌人血量为0或以下，清空所有动作
	if (mEnemyHP <= 0) {
		mEnemyHP = 0;
		Item::generate_items(mBonusNum.powerUp, mBonusNum.power, mBonusNum.lifeUp, mBonusNum.life, mBonusNum.spellCard, mBonusNum.fullPower, mBonusNum.point, mSprite->getPosition(), mBonusNum.radius);
		this->setLifeBarVisiable(false);
		this->mMovementActions.clear();
		this->mDanmakuActions.clear();
		if (mClearBulletAfterDeath) {
			clearBullets();
		}
		DeathSoundEffect();
		if (!mDeathAction) {
			mSpriteAvailable = false;
			mHitable = false;
		}
		else {
			mMovementActions.push_back(std::move(mDeathAction));
			mDeathAction = nullptr;
		}
	}

}

pSprite& Enemy::getSprite()
{
	return this->mSprite;
}

void Enemy::addAction(pAction action, ActionType type)
{
	switch (type) {
	case ActionType::MOVEMENT:
		mMovementActions.push_back(std::move(action));
		break;
	case ActionType::DANMAKU:
		mDanmakuActions.push_back(std::move(action));
		break;
	case ActionType::DEATH:
		mDeathAction = std::move(action);
		break;
	case ActionType::SPAWN:
		mSpawnAction = std::move(action);
		break;
	}
}

void Enemy::addAwait(double awaitTime) {
	mMovementActions.push_back(std::move(std::make_unique<Await>(awaitTime)));
	mDanmakuActions.push_back(std::move(std::make_unique<Await>(awaitTime)));
}

void Enemy::clearAction()
{
	mMovementActions.clear();
	mDanmakuActions.clear();
}

void Enemy::DeathSoundEffect()
{
	sScriptSystem->playSoundEffect("se_enep00.wav");
}
void Enemy::setPlayerPosGetter(std::function<glm::vec2()> getter) {
	mGetPlayerPos = getter;
}

DanmakuAction& Enemy::shoot() {
	// 使用 thread_local 静态变量，避免内存问题
	static thread_local DanmakuAction builder;
	builder = DanmakuAction();  // 重置
	builder.setRenderer(mRenderer);
	builder.setPlayerPosGetter(mGetPlayerPos);

	return builder;
}

void Enemy::clearBullets()
{
	// 将所有子弹归还到对象池，然后清空容器
	for (auto& bullet : mBullets) {
		if (bullet) {
			Bullet::createEtBreakEffect(bullet->getPosition());

			if(dynamic_cast<Bullet_1*>(bullet.get())){
				// 释放 unique_ptr 的所有权并转换为 Bullet_1 类型，然后归还到对象池
				std::unique_ptr<Bullet_1> bullet1_ptr(static_cast<Bullet_1*>(bullet.release()));
				BulletPoolHelper::returnBullet(std::move(bullet1_ptr));
			}
		}
	}
	mBullets.clear();
}

void EnemyUnit::texture_init()
{
	if (!sAnimalTexture) {
		sAnimalTexture = std::make_unique<esl::Texture>(enemy_texture_path + "animal_spirits.png",esl::Texture::Wrap::CLAMP_TO_EDGE,esl::Texture::Filter::NEAREST);
	}
	if (!sNormalTexture) {
		sNormalTexture = std::make_unique<esl::Texture>(enemy_texture_path + "enemy.png", esl::Texture::Wrap::CLAMP_TO_EDGE, esl::Texture::Filter::NEAREST);
	}

}

// 动物精灵构造函数
EnemyUnit::EnemyUnit(AnimalType type, bool isHorizon, glm::vec2 pos, int HP)
{
	mIsAnimalSpirit = true;
	initAnimalSpirit(type, isHorizon, pos, HP);
}

// 普通敌人构造函数
EnemyUnit::EnemyUnit(NormalType type, glm::vec2 pos, int HP)
{
	mIsAnimalSpirit = false;
	initNormalEnemy(type, pos, HP);
}

void EnemyUnit::initAnimalSpirit(AnimalType type, bool isHorizon, glm::vec2 pos, int HP)
{
	texture_init();

	// 基准高度
	int base_height = 16;
	if (!isHorizon) {
		switch (type) {
		case AnimalType::BLUE: {
			break;
		}
		case AnimalType::GREEN: {
			base_height += 48;
			break;
		}
		case AnimalType::RED: {
			base_height += 48 * 2;
			break;
		}
		}
		mRect.resize(4);
		for (int i = 0; i < 4; i++) {
			mRect[i] = { 32 * i, base_height };
		}
		mSize = { 32, 48 };
		mSpriteNum = 4;
	}
	else {
		switch (type) {
		case AnimalType::BLUE: {
			base_height += 3 * 48;
			break;
		}
		case AnimalType::GREEN: {
			base_height += 3 * 48 + 32;
			break;
		}
		case AnimalType::RED: {
			base_height += 3 * 48 + 32 * 2;
			break;
		}
		}
		mRect.resize(4);
		for (int i = 0; i < 4; i++) {
			mRect[i] = { 48 * i, base_height };
		}
		mSize = { 48, 32 };
		mSpriteNum = 4;
	}

	mSprite = std::make_unique<esl::Sprite>(sAnimalTexture.get());
	mSprite->setTextureRect(mRect[0], mSize);
	mSprite->setPosition(pos);
	mSprite->setScale({ 2, 2 });
	//mSprite->setBorderVisiable(true);

	// 血量初始化
	mEnemyHP = HP;
	mMaxHP = HP;
}

void EnemyUnit::initNormalEnemy(NormalType type, glm::vec2 pos, int HP)
{
	texture_init();

	glm::vec2 start_pos{};
	glm::vec2 size{};
	size_t sprite_num = 0;

	switch (type) {
	case NormalType::TYPE1:
		start_pos = { 0, 0 };
		size = { 64, 64 };
		sprite_num = 5;
		break;
	case NormalType::TYPE2:
		start_pos = { 0, 64 };
		size = { 32, 32 };
		sprite_num = 5;
		break;
	case NormalType::TYPE3:
		start_pos = { 0, 64 + 32 };
		size = { 32, 32 };
		sprite_num = 5;
		break;
	case NormalType::TYPE4:
		start_pos = { 0, 64 + 32 * 2 };
		size = { 32, 32 };
		sprite_num = 5;
		break;
	case NormalType::TYPE5:
		start_pos = { 0, 64 + 32 * 3 };
		size = { 32, 32 };
		sprite_num = 5;
		break;
	case NormalType::TYPE6: // 跳过阴阳玉
		start_pos = { 0, 384 };
		size = { 48, 32 };
		sprite_num = 4;
		break;
	case NormalType::TYPE7:
		start_pos = { 0, 384 + 96 };
		size = { 48, 32 };
		sprite_num = 4;
		break;
	case NormalType::TYPE8:
		start_pos = { 320, 224 + 96 };
		size = { 48, 48 };
		sprite_num = 4;
		break;
	case NormalType::TYPE9:
		start_pos = { 320, 224 + 96 + 144 };
		size = { 48, 48 };
		sprite_num = 4;
		break;
	default:
		break;
	}

	mSize = size;
	mSpriteNum = sprite_num;
	mRect.resize(sprite_num);
	for (int i = 0; i < sprite_num; i++)
	{
		mRect[i] = start_pos + glm::vec2(i * size.x, 0);
	}

	mSprite = std::make_unique<esl::Sprite>(sNormalTexture.get());
	mSprite->setTextureRect(mRect[0], mSize);
	mSprite->setPosition(pos);
	mSprite->setScale({ 1.5, 1.5 });
	//mSprite->setBorderVisiable(true);

	// 血量初始化
	mEnemyHP = HP;
	mMaxHP = HP;
}

void EnemyUnit::update(double delta)
{
	// 基类事件更新
	Enemy::update(delta);

	// 帧动画
	if (++mFrame > 10) {
		mSpriteIndex = (mSpriteIndex + 1) % mSpriteNum;
		
		mSprite->setTextureRect(mRect[mSpriteIndex], mSize);

		mFrame = 0;
	}
}

void EnemyUnit::render()
{
	Enemy::render();
}

void Boss::initBossLifeBar()
{
	hp_back_texture = std::make_unique<esl::Texture>("Assets/front/e_life_bar_1.png", esl::Texture::Wrap::CLAMP_TO_EDGE, esl::Texture::Filter::NEAREST);
	hp_fore_texture = std::make_unique<esl::Texture>("Assets/front/e_life_bar_2.png", esl::Texture::Wrap::CLAMP_TO_EDGE, esl::Texture::Filter::NEAREST);
	hp_fore = std::make_unique<esl::ProgressSprite>(hp_fore_texture.get());
	hp_back = std::make_unique<esl::ProgressSprite>(hp_back_texture.get());
	hp_fore->setPosition(glm::vec2(400, 300));
	hp_fore->setScale(glm::vec2(-2.0f, 2.0f));
	hp_fore->setType(esl::ProgressSprite::Type::RADIAL);
	hp_fore->setPercentage(1.f);
	hp_fore->setReverseDirection(true);
	hp_fore->setRotation(90);

	hp_back->setPosition(glm::vec2(400, 300));
	hp_back->setScale(glm::vec2(-2.0f, 2.0f));
	hp_back->setType(esl::ProgressSprite::Type::RADIAL);
	hp_back->setPercentage(1.f);
	hp_back->setReverseDirection(true);
	hp_back->setRotation(90);
}

void Boss::initBoss(size_t boss,int hp,glm::vec2 pos)
{
	std::string texture_path = "Assets/stgenm/stage0"+ std::to_string(boss);
	texture_path += "/enm" + std::to_string(boss);
	texture_path += ".png";
	texture = std::make_unique<esl::Texture>(texture_path, esl::Texture::Wrap::CLAMP_TO_EDGE, esl::Texture::Filter::NEAREST);
	mSprite = std::make_unique<esl::Sprite>(texture.get());
	mSprite->setPosition(pos);
	mSprite->setScale({ 2,2 });
	mSize = { 48,80 };
	texture_path = "Assets/effect/eff_magicsquare.png";
	magic_square_texture = std::make_unique<esl::Texture>(texture_path);
	magic_square = std::make_unique<esl::Sprite>(magic_square_texture.get());
	magic_square->setAlpha(0.5);
	mEnemyHP = hp;
	mMaxHP = hp;
	mSprite->setTextureRect({ 0,272 + mSize.y * 2 }, mSize);
	initBossLifeBar();
	mEnemyType = EnemyType::BOSS;
}

void Boss::DeathSoundEffect()
{
	sScriptSystem->playSoundEffect("se_cat00.wav");
}

Boss::Boss(size_t boss, int hp,glm::vec2 pos)
{
	initBoss(boss, hp, pos);
}

void Boss::update(double deltaTime)
{
	totalTime += deltaTime;
	Enemy::update(deltaTime);
	switch (mStatus) 
	{
	case BossStatus::None:
		if (++mFrame > 10)
		{
			mSpriteIndex = (mSpriteIndex + 1) % 8;
			mSprite->setTextureRect({ mSpriteIndex * mSize.x,272 + mSize.y*2 }, mSize);
			mFrame = 0;
		}
		break;
	case BossStatus::Move:
		// 未实现
		break;
	case BossStatus::SpellCard:
		if (++mFrame > 10)
		{
			mSpriteIndex = (mSpriteIndex + 1) % 8;
			mSprite->setTextureRect({ mSpriteIndex * mSize.x,272 }, mSize);
			mFrame = 0;
		}
		break;
	}
	glm::vec2 boss_position = mSprite->getPosition();
	// 更新magic_square
	float rotation = magic_square->getRotation();
	rotation += 0.5;
	magic_square->setRotation(rotation);
	magic_square->setPosition(boss_position);
	double scaleX = 1.5 + 0.2 * sin(deltaTime);
	double scaleY = 1.5 + 0.2 * sin(deltaTime);
	magic_square->setScale(glm::vec2{scaleX,scaleY });
	hp_fore->setPosition(boss_position);
	hp_back->setPosition(boss_position);
	hp_fore->setPercentage(float(mEnemyHP) / float(mMaxHP));

}

void Boss::render()
{
	mRenderer->draw(*magic_square.get());
	mRenderer->draw(*hp_back.get());
	mRenderer->draw(*hp_fore.get());
	Enemy::render();
}

void Boss::loadFromFile(const std::string& path)
{
	const std::string prefix = "Assets/stgenm/stage";
}

DanmakuEmitter::DanmakuEmitter(glm::vec2 startPos){
	// 初始化基本属性
	mEnemyHP = 1;           // 设置为 1 以便正常工作
	mMaxHP = 1;
	mCollisionRadius = 0;   // 无碰撞体积

	// 创建不可见的精灵
	mSprite = std::make_unique<esl::Sprite>();
	mSprite->setPosition(startPos);
	mSprite->setAvailable(false);  // 关键：不可见
	mHitable = false;
	// 标记为不可交互
	mSpriteAvailable = false;
	mBulletsAvailable = true;
	mEnemyType = EnemyType::EMITTER;

	// 不掉落任何奖励
	setBonus(0, 0, 0, 0, 0, 0, 0, 0);
}

void DanmakuEmitter::render() {
	// 不渲染自身，只渲染子弹
	for (auto& bullet : mBullets) {
		if (bullet && bullet->getSprite()) {
			mRenderer->draw(*bullet->getSprite());
		}
	}
}
void DanmakuEmitter::update(double delta) {
	// 检查父 Enemy 是否还在 mEnemys 列表中（更安全）
	if (mParentEnemy && !mParentEnemy->mSpriteAvailable) {
		mSpriteAvailable = false;
		mBulletsAvailable = false;
	}

	Enemy::update(delta);
}

void DanmakuEmitter::setLifetime(double seconds) {
	addAction(
		std::make_unique<Await>(seconds),
		ActionType::DEATH
	);
}