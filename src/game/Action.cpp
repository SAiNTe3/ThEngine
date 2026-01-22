#include <Action.h>
#include <Enemy.h>
#include <BulletPool.h>  // 添加 BulletPool 头文件


MovementAction::MovementAction(std::function<glm::vec2(double)> func, double duration):mTrajectoryFunc(func), mDuration(duration), mElapsedTime(0)
{
}

bool MovementAction::update(double deltaTime)
{
	mElapsedTime += deltaTime;
	return mElapsedTime >= mDuration;
}

void MovementAction::apply(Enemy* enemy) {
    if (enemy->getSprite()) {
        glm::vec2 newPos = mTrajectoryFunc(mElapsedTime / mDuration);
        enemy->getSprite()->setPosition(newPos);
    }
}

Await::Await(double awaitTime):mAwaitTime(awaitTime)
{
}

bool Await::update(double deltaTime)
{
    mElapsedTime += deltaTime;
    return mElapsedTime >= mAwaitTime;
}

void Await::apply(Enemy* enemy)
{
    enemy->mAngle = 0;
	enemy->mSpeed = 0;
}

void GenericMovementAction::apply(Enemy* enemy) {
    if (!mState.initialized) {
        mUpdater->initialize(mState, enemy->getSprite()->getPosition());
    }

    enemy->getSprite()->setPosition(mState.position);
    enemy->mSpeed = mState.speed;
    enemy->mAngle = mState.direction;
}

// ========== DanmakuAction 实现 ==========

bool DanmakuAction::update(double deltaTime) {
    mElapsedTime += deltaTime;
    // 达到结束条件
    if (mFinishCondition()) {
        return true;
    }
    // 检查是否需要发射新一轮
    // mRounds == -1 表示无限轮次
    if ((mRounds == -1 || mCurrentRound < mRounds) && 
        mElapsedTime >= mLastShootTime + mRoundInterval) {
        return false;  // 继续执行（在 apply 中发射）
    }
    
    // 所有轮次完成（只有非无限轮次才会完成）
    return (mRounds != -1 && mCurrentRound >= mRounds);
}

void DanmakuAction::apply(Enemy* enemy) {
    // 检查是否需要发射
    // mRounds == -1 表示无限轮次，永不停止
    if (mRounds != -1 && mCurrentRound >= mRounds) return;
    if (mElapsedTime < mLastShootTime + mRoundInterval) return;

    glm::vec2 enemyPos = enemy->getPosition();
    glm::vec2 shootPos = enemyPos + mPositionOffset;

    // 如果启用了朝向目标，在发射时计算一次角度
    if (mUsePlayerTarget) {
        // 优先使用 mTargetGetter，否则使用 mPlayerPosGetter（兼容旧代码）
        auto getter = mTargetGetter ? mTargetGetter : mPlayerPosGetter;

        if (getter) {
            glm::vec2 targetPos = getter();
            glm::vec2 toTarget = targetPos - enemyPos;

            // 计算朝向目标的角度（只计算一次）
            mBulletConfig.baseAngle = glm::degrees(std::atan2(toTarget.y, toTarget.x));
        }
    }

    // 应用每轮角度增量
    mBulletConfig.baseAngle += mAnglePerRound * mCurrentRound;
    if (mColorVectorUsed) {
        nextColor();
    }
    // 根据模式发射
    switch (mPattern) {
    case DanmakuPattern::LINEAR:
        shootLinear(enemy, shootPos);
        break;
    case DanmakuPattern::CIRCLE:
        shootCircle(enemy, shootPos);
        break;
    case DanmakuPattern::FAN:
        shootFan(enemy, shootPos);
        break;
    case DanmakuPattern::SPIRAL:
        shootSpiral(enemy, shootPos);
        break;
    }

    mCurrentRound++;
    mLastShootTime = mElapsedTime;
}

void DanmakuAction::shootLinear(Enemy* enemy, glm::vec2 startPos) {
    // 如果设置了发射半径，从圆周上发射
    if (mShootRadius > 0) {
        float radians = glm::radians(mBulletConfig.baseAngle);
        startPos += glm::vec2(
            mShootRadius * cos(radians),
            mShootRadius * sin(radians)
        );
    }
    auto bullet = BulletPoolHelper::getBullet(
        mBulletConfig.type,
        mBulletConfig.color,
        startPos,
        *mRenderer,
        mBulletConfig.baseAngle,
        mBulletConfig.baseSpeed
    );

    bullet->setRotation(mBulletConfig.baseAngle - 90);

    // 设置旋转同步标志
    if (auto bullet1 = dynamic_cast<Bullet_1*>(bullet.get())) {
        bullet1->mSyncRotation = mSyncRotationWithDirection;
    }

    // 应用运动参数（如果有加速度）
    if (mBulletAcceleration != 0.0f) {
        if (auto bullet1 = dynamic_cast<Bullet_1*>(bullet.get())) {
            if (mBulletNeverStop) {
                bullet1->addMovementAction(
                    BulletLinearMovement()
                        .direction(mBulletConfig.baseAngle)
                        .speed(mBulletConfig.baseSpeed)
                        .accelerate(mBulletAcceleration)
                        .accelerateDuring(mAccelerationDuration)  // 设置加速时间
                        .neverStop()
                        .buildBullet()
                );
            } else {
                bullet1->addMovementAction(
                    BulletLinearMovement()
                        .direction(mBulletConfig.baseAngle)
                        .speed(mBulletConfig.baseSpeed)
                        .accelerate(mBulletAcceleration)
                        .accelerateDuring(mAccelerationDuration)  // 设置加速时间
                        .stopAfter(mBulletMoveDuration)
                        .buildBullet()
                );
            }
        }
    }
    // 如果有自定义运动配置，应用它
    else if (!mMovementBuilders.empty()) {
        if (auto bullet1 = dynamic_cast<Bullet_1*>(bullet.get())) {
            for (const auto& builder : mMovementBuilders) {
                if (builder) {
                    bullet1->addMovementAction(builder());
                }
            }
        }
    }

    enemy->mBullets.push_back(std::move(bullet));
}

void DanmakuAction::shootCircle(Enemy* enemy, glm::vec2 startPos) {
    float angleStep = mAngleStep > 0 ? mAngleStep : (360.0f / mBulletCount);

    for (int i = 0; i < mBulletCount; i++) {
        float angle = mBulletConfig.baseAngle + angleStep * i;
        float speed = mBulletConfig.baseSpeed + mSpeedVariation * (i - mBulletCount / 2.0f);

        // 计算发射位置：从圆周上发射
        glm::vec2 shootPosForBullet = startPos;
        if (mShootRadius > 0) {
            float radians = glm::radians(angle);
            shootPosForBullet += glm::vec2(
                mShootRadius * cos(radians),
                mShootRadius * sin(radians)
            );
        }
        auto bullet = BulletPoolHelper::getBullet(
            mBulletConfig.type,
            mBulletConfig.color,
            shootPosForBullet,
            *mRenderer,
            angle,
            speed
        );

        bullet->setRotation(angle - 90);

        // 设置旋转同步标志
        if (auto bullet1 = dynamic_cast<Bullet_1*>(bullet.get())) {
            bullet1->mSyncRotation = mSyncRotationWithDirection;
        }

        // 应用运动参数（如果有加速度）
        if (mBulletAcceleration != 0.0f) {
            if (auto bullet1 = dynamic_cast<Bullet_1*>(bullet.get())) {
                if (mBulletNeverStop) {
                    bullet1->addMovementAction(
                        BulletLinearMovement()
                            .direction(angle)
                            .speed(speed)
                            .accelerate(mBulletAcceleration)
                            .accelerateDuring(mAccelerationDuration)
                            .neverStop()
                            .buildBullet()
                    );
                } else {
                    bullet1->addMovementAction(
                        BulletLinearMovement()
                            .direction(angle)
                            .speed(speed)
                            .accelerate(mBulletAcceleration)
                            .accelerateDuring(mAccelerationDuration)
                            .stopAfter(mBulletMoveDuration)
                            .buildBullet()
                    );
                }
            }
        }
        // 如果有自定义运动配置，应用它
        else if (!mMovementBuilders.empty()) {
            if (auto bullet1 = dynamic_cast<Bullet_1*>(bullet.get())) {
                for (const auto& builder : mMovementBuilders) {
                    if (builder) {
                        bullet1->addMovementAction(builder());
                    }
                }
            }
        }

        enemy->mBullets.push_back(std::move(bullet));
    }
}

void DanmakuAction::shootFan(Enemy* enemy, glm::vec2 startPos) {
    if (mBulletCount <= 1) {
        shootLinear(enemy, startPos);
        return;
    }

    float totalAngle = mAngleStep * (mBulletCount - 1);
    float startAngle = mBulletConfig.baseAngle - totalAngle / 2.0f;

    for (int i = 0; i < mBulletCount; i++) {
        float angle = startAngle + mAngleStep * i;
        float speed = mBulletConfig.baseSpeed + mSpeedVariation * (i - mBulletCount / 2.0f);
        // 计算发射位置：从圆周上发射
        glm::vec2 shootPosForBullet = startPos;
        if (mShootRadius > 0) {
            // 选项 1：所有子弹从同一个位置发射（扇形中心方向）
            if (!mAlignToRadius) {
                float radians = glm::radians(mBulletConfig.baseAngle);
                shootPosForBullet += glm::vec2(
                    mShootRadius * cos(radians),
                    mShootRadius * sin(radians)
                );
            }
            // 选项 2：每个子弹从对应角度的圆周位置发射
            else {
                float radians = glm::radians(angle);
                shootPosForBullet += glm::vec2(
                    mShootRadius * cos(radians),
                    mShootRadius * sin(radians)
                );
            }
        }
        auto bullet = BulletPoolHelper::getBullet(
            mBulletConfig.type,
            mBulletConfig.color,
            shootPosForBullet,
            *mRenderer,
            angle,
            speed
        );

        bullet->setRotation(angle - 90);

        // 设置旋转同步标志
        if (auto bullet1 = dynamic_cast<Bullet_1*>(bullet.get())) {
            bullet1->mSyncRotation = mSyncRotationWithDirection;
        }

        // 应用运动参数（如果有加速度）
        if (mBulletAcceleration != 0.0f) {
            if (auto bullet1 = dynamic_cast<Bullet_1*>(bullet.get())) {
                if (mBulletNeverStop) {
                    bullet1->addMovementAction(
                        BulletLinearMovement()
                            .direction(angle)
                            .speed(speed)
                            .accelerate(mBulletAcceleration)
                            .accelerateDuring(mAccelerationDuration)
                            .neverStop()
                            .buildBullet()
                    );
                } else {
                    bullet1->addMovementAction(
                        BulletLinearMovement()
                            .direction(angle)
                            .speed(speed)
                            .accelerate(mBulletAcceleration)
                            .accelerateDuring(mAccelerationDuration)
                            .stopAfter(mBulletMoveDuration)
                            .buildBullet()
                    );
                }
            }
        }
        // 如果有自定义运动配置，应用它
        else if (!mMovementBuilders.empty()) {
            if (auto bullet1 = dynamic_cast<Bullet_1*>(bullet.get())) {
                for (const auto& builder : mMovementBuilders) {
                    if (builder) {
                        bullet1->addMovementAction(builder());
                    }
                }
            }
        }

        enemy->mBullets.push_back(std::move(bullet));
    }
}

void DanmakuAction::shootSpiral(Enemy* enemy, glm::vec2 startPos) {
    // 螺旋模式：baseAngle 已经包含了每轮的增量
    float baseAngle = mBulletConfig.baseAngle;

    for (int i = 0; i < mBulletCount; i++) {
        float angle = baseAngle + (360.0f / mBulletCount) * i;
        // 计算发射位置：从圆周上发射
        glm::vec2 shootPosForBullet = startPos;
        if (mShootRadius > 0) {
            float radians = glm::radians(angle);
            shootPosForBullet += glm::vec2(
                mShootRadius * cos(radians),
                mShootRadius * sin(radians)
            );
        }
        auto bullet = BulletPoolHelper::getBullet(
            mBulletConfig.type,
            mBulletConfig.color,
            shootPosForBullet,
            *mRenderer,
            angle,
            mBulletConfig.baseSpeed
        );

        bullet->setRotation(angle - 90);

        // 设置旋转同步标志
        if (auto bullet1 = dynamic_cast<Bullet_1*>(bullet.get())) {
            bullet1->mSyncRotation = mSyncRotationWithDirection;
        }

        // 应用运动参数（如果有加速度）
        if (mBulletAcceleration != 0.0f) {
            if (auto bullet1 = dynamic_cast<Bullet_1*>(bullet.get())) {
                // 直接构建并传递，不使用中间变量（避免拷贝）
                if (mBulletNeverStop) {
                    bullet1->addMovementAction(
                        BulletLinearMovement()
                            .direction(angle)
                            .speed(mBulletConfig.baseSpeed)
                            .accelerate(mBulletAcceleration)
                            .accelerateDuring(mAccelerationDuration)
                            .neverStop()
                            .buildBullet()
                    );
                } else {
                    bullet1->addMovementAction(
                        BulletLinearMovement()
                            .direction(angle)
                            .speed(mBulletConfig.baseSpeed)
                            .accelerate(mBulletAcceleration)
                            .accelerateDuring(mAccelerationDuration)
                            .stopAfter(mBulletMoveDuration)
                            .buildBullet()
                    );
                }
            }
        }
        else if (!mMovementBuilders.empty()) {
            if (auto bullet1 = dynamic_cast<Bullet_1*>(bullet.get())) {
                for (const auto& builder : mMovementBuilders) {
                    if (builder) {
                        bullet1->addMovementAction(builder());
                    }
                }
            }
        }

        enemy->mBullets.push_back(std::move(bullet));
    }
}