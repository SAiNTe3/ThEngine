#include <Bullet.h>
#include <Player.h>
#include <cmath>
#include "Action.h"

Bullet::~Bullet() = default;

std::string bullet_texture_path = ".\\Assets\\bullet\\";

pTexture Bullet::etbreakTexture = nullptr;
pSprite Bullet::etbreakSprite = nullptr;
const glm::vec2 Bullet::etbreakFrames[8] = {
		{0,64}, {64,64}, {128,64}, { 192,64 }, {0,0}, {64,0}, {128,0},{192,0}
};
std::vector<Bullet::EtBreakEffect> Bullet::etbreaks;
pTexture Bullet_1::sTexture[6];
// 静态辅助函数：设置子弹属性（纹理坐标和碰撞半径）
void Bullet_1::setupBulletProperties(
	esl::Sprite* sprite,
	double& collisionRadius,
	int type,
	int color
) {
	glm::vec2 size = { 0,0 };
	glm::vec2 start_pos = { 0,0 };

	// 根据 type 选择纹理
	sprite->setTexture(selectTexture(type));

	// 设置纹理坐标和碰撞半径
	switch (type) {
	case 1:
	{
		start_pos = { 0,0 };
		size = { 8,8 };
		sprite->setTextureRect(start_pos + glm::vec2{ 8 * (color % 8), (color / 8) * 8 }, size);
		collisionRadius = 2.4;
	}
		break;
	case 2:
	{
		start_pos = { 64,0 };
		size = { 16,16 };
		sprite->setTextureRect(start_pos + glm::vec2{ color * 16,0 }, size);
		collisionRadius = 4;
		break;
	}
	case 3:
	{
		start_pos = { 0,16 };
		size = { 32,32 };
		sprite->setTextureRect(start_pos + glm::vec2{ color * 32,0 }, size);
		collisionRadius = 12;
		break;
	}
	case 4:
	{
		start_pos = { 0,32 };
		size = { 8,8 };
		sprite->setTextureRect(start_pos + glm::vec2{ 8 * (color % 8), (color / 8) * 8 }, size);
		collisionRadius = 2.4;
		break;
	}
	case 5:
	{
		start_pos = { 64,32 };
		size = { 8,8 };
		sprite->setTextureRect(start_pos + glm::vec2{ 8 * (color % 8), (color / 8) * 8 }, size);
		collisionRadius = 2;
		break;
	}
	case 6:
	case 8:
	case 9:
	case 11:
	case 12:
	case 16:
	{
		collisionRadius = 2.4;
		start_pos = { 0,64 + 16 * (type - 6) };
		size = { 16,16 };
		sprite->setTextureRect(start_pos + glm::vec2{ color * 16, 0 }, size);
		break;
	}
	case 10:
	{
		collisionRadius = 2.8;
		start_pos = { 0,64 + 16 * (type - 6) };
		size = { 16,16 };
		sprite->setTextureRect(start_pos + glm::vec2{ color * 16, 0 }, size);
		break;
	}
	case 13:
	{
		collisionRadius = 3.2;
		start_pos = { 0,64 + 16 * (type - 6) };
		size = { 16,16 };
		sprite->setTextureRect(start_pos + glm::vec2{ color * 16, 0 }, size);
		break;
	}
	case 7:
	case 14:
	case 15:
	case 17:
	{
		start_pos = { 0,64 + 16 * (type - 6) };
		size = { 16,16 };
		sprite->setTextureRect(start_pos + glm::vec2{ color * 16, 0 }, size);
		collisionRadius = 4;
		break;
	}
	case 18:
	{
		start_pos = { 13 * 16,3 * 16 };
		size = { 16,16 };
		sprite->setTextureRect(start_pos + glm::vec2{ color * 16, 0 }, size);
		collisionRadius = 4;
		break;
	}
	case 19:
	{
		start_pos = { 0,0 };
		size = { 64,64 };
		sprite->setTextureRect(start_pos + glm::vec2{ color * 64, 0 }, size);
		collisionRadius = 14;
		break;
	}
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	{
		start_pos = { 0,64 + (type - 20) * 32 };
		size = { 32,32 };
		sprite->setTextureRect(start_pos + glm::vec2{ color * 32, 0 }, size);
		collisionRadius = 7;
		break;
	}
	case 26:
		break;
	default:
		collisionRadius = 2.4;
		break;
	}
}
esl::Texture* Bullet_1::selectTexture(int type) {
	if (type <= 18) return sTexture[0].get();
	if (type <= 25) return sTexture[1].get();
	if (type <= 31) return sTexture[2].get();
	if (type == 32) return sTexture[3].get();
	if (type <= 36) return sTexture[4].get();
	if (type <= 38) return sTexture[5].get();
	return sTexture[0].get();  // 默认
}
void Bullet_1::init() {
	if (!sTexture[0]) {
		for (int i = 0; i < 6; i++) {
			char index = '1' + i;
			sTexture[i] = std::make_unique<esl::Texture>(bullet_texture_path + "bullet" + index + ".png");
		}
	}
}

Bullet_1::Bullet_1(int type, int color, glm::vec2 pos, esl::Window& render, float angle, float speed):mRender(render), mAngle(angle), mSpeed(speed)
{
	static bool firstTime = true;
	if (firstTime) {
		firstTime = false;
		printf("[Bullet_1] Object size: %zu bytes\n", sizeof(Bullet_1));
		printf("[Bullet_1] Sprite size: %zu bytes\n", sizeof(esl::Sprite));
		printf("[Bullet_1] Expected 5000 bullets memory: %.2f MB\n",
			sizeof(Bullet_1) * 5000 / 1024.0 / 1024.0);
	}
	// 创建精灵（使用临时纹理，会在 setupBulletProperties 中正确设置）
	mSprite = std::make_unique<esl::Sprite>(sTexture[0].get());

	// 调用公共函数设置属性
	setupBulletProperties(mSprite.get(), mCollisionRadius, type, color);

	// 设置位置和缩放
	mSprite->setPosition(pos);
	mSprite->setScale({ 2,2 });
	mCollisionRadius *= 2;
	//mSprite->setBorderVisiable(true);
}

void Bullet_1::renderBatch(
	esl::Window& renderer,
	const std::vector<pBullet>& bullets)
{
	// 优化：按纹理分组，减少纹理切换
	static thread_local std::unordered_map<esl::Texture*, std::vector<Bullet_1*>> batches;
	batches.clear();

	// 预留空间
	for (auto& [_, vec] : batches) {
		vec.reserve(bullets.size() / 6);  // 假设平均分布
	}

	// 分组
	for (const auto& bullet : bullets) {
		if (!bullet) continue;

		auto* bullet1 = dynamic_cast<Bullet_1*>(bullet.get());
		if (!bullet1 || !bullet1->mSprite) continue;

		esl::Texture* tex = bullet1->mSprite->getTexture();
		if (tex) {
			batches[tex].push_back(bullet1);
		}
	}

	// 批量绘制（减少纹理切换）
	for (auto& [texture, group] : batches) {
		// 可以在这里添加纹理绑定优化
		for (auto* bullet : group) {
			renderer.draw(*bullet->mSprite);
		}
	}
}
void Bullet_1::update(double delta)
{
	lastGrazeTime += delta;
	// 更新运动Action系统
	updateMovementActions(delta);

	// 如果没有Action，使用基础移动
	if (mMovementActions.empty()) {
		glm::vec2 velocity = {
			mSpeed * cos(glm::radians(mAngle)) * static_cast<float>(delta),
			mSpeed * sin(glm::radians(mAngle)) * static_cast<float>(delta)
		};
		mSprite->setPosition(mSprite->getPosition() + velocity);
	}

	// 如果启用了旋转同步，更新精灵旋转角度
	if (mSyncRotation) {
		mSprite->setRotation(mAngle - 90);  // -90 是因为精灵图片默认朝上
	}
}

void Bullet_1::render()
{
	mRender.draw(*mSprite);
}

void Bullet_1::updateMovementActions(double deltaTime)
{
	// 更新运动Action队列
	for (auto it = mMovementActions.begin(); it != mMovementActions.end();) {
		// 先应用Action效果（确保初始化）
		(*it)->apply(this);

		// 然后更新状态
		if ((*it)->update(deltaTime)) {
			// Action完成，移除
			it = mMovementActions.erase(it);
		} else {
			// 只执行第一个Action
			break;
		}
	}
}

void Bullet::createEtBreakEffect(glm::vec2 pos)
{
	EtBreakEffect effect;
	effect.position = pos;
	effect.lifetime = 0;
	effect.current_index = 0;
	etbreaks.push_back(effect);
}

void Bullet::updateEtBreaks(double deltaTime)
{
	for (auto it = etbreaks.begin(); it != etbreaks.end();) {
		it->lifetime += deltaTime;
		// 每0.1秒切换一帧
		size_t frameIndex = static_cast<size_t>(it->lifetime / 0.1);
		if (frameIndex >= 8) {
			it = etbreaks.erase(it);
		} else {
			it->current_index = frameIndex;
			++it;
		}
	}
}

void Bullet::drawEtBreaks(esl::Window& renderer)
{
	for (const auto& effect : etbreaks) {
		if (etbreakSprite && etbreakTexture) {
			etbreakSprite->setTextureRect(
				etbreakFrames[effect.current_index],
				glm::vec2{ 64,64 }
			);
			etbreakSprite->setPosition(effect.position);
			renderer.draw(*etbreakSprite);
		}
	}
}

void Bullet::initEtBreak()
{
	etbreakTexture = std::make_unique<esl::Texture>("./Assets/effect/etbreak.png");
	etbreakSprite = std::make_unique<esl::Sprite>(etbreakTexture.get());
}

void Bullet_1::addMovementAction(pBulletMovementAction action)
{
	mMovementActions.push_back(std::move(action));
}

void Bullet_1::clearMovementActions()
{
	mMovementActions.clear();
}

// 在文件末尾添加这两个函数的实现

bool GenericBulletMovementAction::update(double deltaTime) {
	mState.elapsedTime += deltaTime;
	mUpdater->update(mState, deltaTime);
	return mStopCondition->shouldStop(mState);
}

void GenericBulletMovementAction::apply(Bullet* bullet) {
	
	if (!mInitialized) {
		// 只有当 NOT useTargetMode 时，才从子弹继承方向
		// 因为如果使用了 TargetMode，方向是由目标位置决定的，不应该被子弹初始角度覆盖
		if (!mState.useTargetMode) {
			if (auto b1 = dynamic_cast<Bullet_1*>(bullet)) {
				if (mState.direction == 0) {
					mState.direction = b1->mAngle;
				}
			}
		}
		// 速度继承同理，如果速度未设置，从子弹继承
		if (auto b1 = dynamic_cast<Bullet_1*>(bullet)) {
			if (mState.speed < 0) {
				mState.speed = b1->mSpeed;
			}
		}
		mUpdater->initialize(mState, bullet->getPosition());
		mInitialized = true;
	}

	// 更新子弹位置
	bullet->setPosition(mState.position);

	// 如果是 Bullet_1，同步方向和速度
	if (auto b1 = dynamic_cast<Bullet_1*>(bullet)) {
		b1->mAngle = mState.direction;
		b1->mSpeed = mState.speed;

		// 如果启用了旋转同步
		if (b1->mSyncRotation) {
			b1->getSprite()->setRotation(mState.direction);
		}
	}
}