#include <Player.h>
#include <Enemy.h>

ScriptSystem* Player::mScriptSystem = nullptr;

void Player::set_position(glm::vec2 pos)
{
	mSprite->setPosition(pos);
}

glm::vec2 Player::get_position()
{
	return mSprite->getPosition();
}

void Player::move(glm::vec2 distance)
{
	// 无敌状态下不可移动，自动复原位置
	if (mInvincible) return;
	mSprite->move(distance);
}

void Player::update(double delta)
{
}

void Player::shoot()
{
}

void Player::render()
{
	
}

void Player::slowEffectRender()
{
}

void Player::hitPlayer(glm::vec2 rebirthPos)
{
	mInvincible = true;
	mInvincibleTimer = 1.0;
	set_position(rebirthPos);
}

void Reimu::update_bullets(double delta)
{
	// 子弹更新
	for (auto& bullet : mBullets) {
		glm::vec2 bullet_position = bullet->getPosition();
		if (
			bullet_position.x<-10
			|| bullet_position.x>mRenderer.getWindowSize().x + 10
			|| bullet_position.y<-10
			|| bullet_position.y>mRenderer.getWindowSize().y + 10
			) {
			bullet->setAvailable(false);
			continue;
		}
		bullet->move({ 0,delta * 2000 });
	}
	// 清除失效子弹
	mBullets.erase(
		std::remove_if(
			mBullets.begin(),
			mBullets.end(),
			[](const pSprite& bullet)->bool {
				if (!bullet->getAvailable()) {
					return true;
				}
				return false;
			}
		),
		mBullets.end()
	);
}

void Reimu::update_trace_bullets(double delta)
{
	for (auto it = mTraceBullets.begin(); it != mTraceBullets.end();) {
		auto& traceBullet = *it;
		glm::vec2 bulletPos = traceBullet->sprite->getPosition();
		
		// 检查是否超出屏幕边界
		glm::ivec2 screenSize = mRenderer.getWindowSize();
		if (bulletPos.x < -32 || bulletPos.x > screenSize.x ||
			bulletPos.y < -64 || bulletPos.y > screenSize.y) {
			it = mTraceBullets.erase(it);
			continue;
		}
		
		// 1. 检查当前目标是否仍然有效
		if (traceBullet->hasTarget && !isTargetValid(traceBullet->target)) {
			traceBullet->hasTarget = false;
			traceBullet->target = nullptr;
		}
		
		// 2. 如果没有目标，寻找新目标
		if (!traceBullet->hasTarget) {
			traceBullet->target = findNearestEnemy(bulletPos);
			if (traceBullet->target) {
				traceBullet->hasTarget = true;
			}
		}
		
		// 3. 根据是否有目标来更新速度方向
		if (traceBullet->hasTarget && traceBullet->target) {
			// 有目标：追踪目标
			glm::vec2 targetPos = traceBullet->target->getSprite()->getPosition();
			glm::vec2 toTarget = glm::normalize(targetPos - bulletPos);
			
			// 使用插值平滑转向，避免过于急剧的转向
			float lerpFactor = traceBullet->rotateSpeed * static_cast<float>(delta);
			lerpFactor = std::min(lerpFactor, 1.0f); // 限制最大转向速度
			
			// 计算新的速度方向
			glm::vec2 newVelocity = glm::normalize(
				traceBullet->velocity * (1.0f - lerpFactor) + toTarget * lerpFactor
			);
			traceBullet->velocity = newVelocity;
		} else {
			// 无目标：保持当前方向（向前移动）
			// 速度方向不变，继续按原方向移动
		}
		
		// 4. 更新位置
		glm::vec2 movement = traceBullet->velocity * traceBullet->speed * static_cast<float>(delta);
		traceBullet->sprite->move(movement);
		
		// 5. 更新精灵旋转（让子弹朝向移动方向）
		float angle = atan2(traceBullet->velocity.y, traceBullet->velocity.x) * 180.0f / 3.14159f;
		traceBullet->sprite->setRotation(angle + 90.0f); // +90度调整精灵朝向
		
		++it;
	}
}

Enemy* Reimu::findNearestEnemy(glm::vec2 bulletPos)
{
	if (!mEnemyList || mEnemyList->empty()) {
		return nullptr;
	}
	
	Enemy* nearestEnemy = nullptr;
	float nearestDistance = std::numeric_limits<float>::max();
	
	for (Enemy* enemy : *mEnemyList) {
		if (!enemy || !enemy->mSpriteAvailable || enemy->mEnemyType == Enemy::EnemyType::EMITTER) {
			continue;
		}
		
		glm::vec2 enemyPos = enemy->getSprite()->getPosition();
		float distance = glm::length(enemyPos - bulletPos);
		
		// 只追踪屏幕内的敌机
		glm::ivec2 screenSize = mRenderer.getWindowSize();
		if (enemyPos.x >= 0 && enemyPos.x <= screenSize.x &&
			enemyPos.y >= 0 && enemyPos.y <= screenSize.y) {
			
			if (distance < nearestDistance) {
				nearestDistance = distance;
				nearestEnemy = enemy;
			}
		}
	}
	
	return nearestEnemy;
}

bool Reimu::isTargetValid(Enemy* target)
{
	if (!target || !mEnemyList) {
		return false;
	}
	
	// 检查目标是否还在敌机列表中且可用
	auto it = std::find(mEnemyList->begin(), mEnemyList->end(), target);
	if (it == mEnemyList->end() || !target->mHitable) {
		return false;
	}
	
	// 检查目标是否还在屏幕范围内（给一定缓冲区域）
	glm::vec2 enemyPos = target->getSprite()->getPosition();
	glm::ivec2 screenSize = mRenderer.getWindowSize();
	
	return (enemyPos.x >= 0 && enemyPos.x <= screenSize.x &&
			enemyPos.y >= 0 && enemyPos.y <= screenSize.y);
}

Reimu::Reimu(esl::Window& renderer, unsigned int& power) :mRenderer(renderer), Player(power)
{
	mTexture = std::make_unique<esl::Texture>(".\\Assets\\player\\reimu.png");
	mSlowEffectTexture = std::make_unique<esl::Texture>(".\\Assets\\effect\\eff_sloweffect.png");
	for (int i = 0; i < 8; i++) {
		mRect[i]={ i*32,48*2 };
	}
	// 左移
	mRect[8]={ 7 * 32,48 * 1 };
	// 右移
	mRect[9]={ 7 * 32,48 * 0 };
	mSprite = std::make_unique<esl::Sprite>(mTexture.get());
	mSprite->setTextureRect(mRect[0],{32,48});
	mSprite->setScale({ 2,2 });
	// 子弹加载
	std::string bullet_dir_path = "Assets\\bullet\\reimu\\";
	for (int i = 0; i < 11; i++) {
		std::string path = bullet_dir_path + std::to_string(i + 1);
		this->mBulletTextures.push_back(std::make_unique<esl::Texture>(path + ".png"));
	}
	mSlowEffectSprite = std::make_unique<esl::Sprite>(mSlowEffectTexture.get());
	mSlowEffectSprite->setTextureRect({ 0,0 }, { 64,64 });
	mSlowEffectSprite->setScale({ 2,2 });
	mMissRadius = 3*2;
	mSlowEffectSprite->setPosition(mSprite->getPosition());

	mYinYangOrbTexture = std::make_unique<esl::Texture>(".\\Assets\\bullet\\reimu\\9.png");
	mYinYangOrbs.resize(1);
	mYinYangOrbs[0] = std::make_unique<esl::Sprite>(mYinYangOrbTexture.get());
	mYinYangOrbs[0]->setScale({1.5,1.5});
	mYinYangOrbs[0]->setPosition(mSprite->getPosition()+glm::vec2{0,-72});
	
	// 追踪弹纹理加载
	mTraceBulletTexture = std::make_unique<esl::Texture>(".\\Assets\\bullet\\reimu\\trace.png");
}

void Reimu::update(double delta)
{
	// 角色动画更新
	mFrame++;
	if(mInvincible){
		mInvincibleTimer -= delta;
		if (mInvincibleTimer <= 0.0) {
			mInvincible = false;
			mInvincibleTimer = 0.0;
		}
		mSprite->move({ 0, delta * 128 });
	}
	if (mDirection.h < 0) {
		mSpriteIndex = 8;
	}
	else if (mDirection.h > 0) {
		mSpriteIndex = 9;
	}
	else {
		if (mFrame > 10) {
			mSpriteIndex = (mSpriteIndex + 1) % 8;
			mFrame = 0;
		}
	}
	mSprite->setTextureRect(mRect[mSpriteIndex],{32,48});
	// 射击子弹
	this->shoot();
	
	// 更新普通子弹
	update_bullets(delta);
	
	// 更新追踪弹
	update_trace_bullets(delta);

	mSlowEffectSprite->setPosition(mSprite->getPosition());
	mSlowEffectSprite->setRotation(mSlowEffectSprite->getRotation() + 1);

	// 动态调整阴阳玉数量
	size_t requiredOrbs = mPower / 100;
	if (requiredOrbs > mYinYangOrbs.size()) {
		// 需要添加更多阴阳玉
		while (mYinYangOrbs.size() < requiredOrbs) {
			mYinYangOrbs.push_back(std::make_unique<esl::Sprite>(mYinYangOrbTexture.get()));
			mYinYangOrbs.back()->setScale({ 1.5,1.5 });
		}
	}
	// 阴阳玉动画和位置更新
	glm::vec2 playerPos = mSprite->getPosition();
	for (size_t i = 0; i < requiredOrbs && i < mYinYangOrbs.size(); i++) {
		// 计算水平偏移：居中排列，间隔36px
		float totalWidth = (requiredOrbs - 1) * 36.0f;  // 总宽度
		float startX = -totalWidth / 2.0f;              // 起始X偏移
		float offsetX = startX + i * 36.0f;             // 当前阴阳玉的X偏移
		mYinYangOrbs[i]->setPosition(playerPos + glm::vec2{ offsetX, -72 });
		mYinYangOrbs[i]->setRotation(mYinYangOrbs[i]->getRotation() + 4);
	}

	// 更新阴阳玉计数
	mYinYangOrbCount = requiredOrbs;
}

void Reimu::shoot()
{
	// 射击间隔帧
	mShootInterval++;
	if (!mEnableShoot) return;
	if (mShootInterval < 4) return;
	mScriptSystem->playSoundEffect("se_plst00.wav");
	mShootInterval = 0;
	switch (mHyperMode)
	{
	case true: {
		glm::vec2 player_position = mSprite->getPosition();
		pSprite bullet = std::make_unique<esl::Sprite>(mBulletTextures[0].get());
		bullet->setPosition({ player_position.x - 15,player_position.y });
		bullet->setScale({ 2,2 });
		mBullets.push_back(std::move(bullet));
		bullet = std::make_unique<esl::Sprite>(mBulletTextures[0].get());
		bullet->setPosition({ player_position.x + 15,player_position.y });
		bullet->setScale({ 2,2 });
		mBullets.push_back(std::move(bullet));
		
		// 从每个阴阳玉发射追踪弹
		for (int i = 0; i < mYinYangOrbCount; i++) {
			auto traceBullet = std::make_unique<TraceBullet>(mTraceBulletTexture, mYinYangOrbs[i]->getPosition());
			mTraceBullets.push_back(std::move(traceBullet));
		}
		break;
	}
	case false: {
		glm::vec2 player_position = mSprite->getPosition();
		for (size_t i = 1; i<=mPower/100; i++) {
			pSprite bullet = std::make_unique<esl::Sprite>(mBulletTextures[3].get());
			bullet->setPosition({ player_position.x - 15 * i +5,player_position.y });
			bullet->setScale({ 2,2 });
			mBullets.push_back(std::move(bullet));
			bullet = std::make_unique<esl::Sprite>(mBulletTextures[3].get());
			bullet->setPosition({ player_position.x + 15 * i -5,player_position.y });
			bullet->setScale({ 2,2 });
			mBullets.push_back(std::move(bullet));
			bullet = std::make_unique<esl::Sprite>(mBulletTextures[0].get());
			bullet->setPosition({ player_position.x - 15,player_position.y });
			bullet->setScale({ 2,2 });
			mBullets.push_back(std::move(bullet));
			bullet = std::make_unique<esl::Sprite>(mBulletTextures[0].get());
			bullet->setPosition({ player_position.x + 15,player_position.y });
			bullet->setScale({ 2,2 });
			mBullets.push_back(std::move(bullet));
		}
		break;
	}
	}
}

void Reimu::render()
{
	// 绘制角色
	mRenderer.draw(*mSprite.get());
	// 绘制普通子弹
	for (auto& bullet : mBullets) {
		mRenderer.draw(*bullet.get());
	}
	// 绘制追踪弹
	for (auto& traceBullet : mTraceBullets) {
		mRenderer.draw(*traceBullet->sprite.get());
	}
	// 绘制阴阳玉
	for(int i=0;i<mPower/100 && i<mYinYangOrbs.size();i++){
		mRenderer.draw(*mYinYangOrbs[i].get());
	}
}

void Reimu::slowEffectRender()
{
	if (!mHyperMode) mRenderer.draw(*mSlowEffectSprite.get());
}
