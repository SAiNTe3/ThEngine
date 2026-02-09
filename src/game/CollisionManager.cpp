#include "CollisionManager.h"
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"
#include <glm/glm.hpp>
#include <cmath>

bool CollisionManager::checkEnemyBulletsVsPlayer(
    const std::vector<std::unique_ptr<Bullet>>& bullets,
    Player& player)
{
    // 使用 thread_local 避免频繁分配
    static thread_local std::vector<Bullet*> bulletPtrs;
    bulletPtrs.clear();
    bulletPtrs.reserve(bullets.size());

    for (const auto& bullet : bullets) {
        if (bullet) {
            bulletPtrs.push_back(bullet.get());
        }
    }

    // 调用优化版本
    return checkEnemyBulletsVsPlayer(bulletPtrs, player);
}
bool CollisionManager::checkEnemyBulletsVsPlayer(
    const std::vector<Bullet*>& bullets,
    Player& player)
{
    glm::vec2 playerPos = player.get_position();
    float playerRadius = player.mMissRadius;
    float grazeRadius = playerRadius + 64.0f;


    for (const auto& bullet : bullets) {
        if (!bullet || !bullet->getSprite()) continue;

        glm::vec2 bulletPos = bullet->getPosition();
        float bulletRadius = static_cast<float>(bullet->mCollisionRadius);

        // 优化：AABB 快速剔除（在精确检测前）
        float dx = playerPos.x - bulletPos.x;
        float dy = playerPos.y - bulletPos.y;

        // 使用保守估计的边界框
        float maxDist = playerRadius + bulletRadius + 64.0f;
        if (dx > maxDist || dx < -maxDist) continue;
        if (dy > maxDist || dy < -maxDist) continue;

        // 碰撞检测
        if (!player.isInvincible() && circleCollision(playerPos, playerRadius, bulletPos, bulletRadius)) {
            if (mEnemyBulletHitPlayerCallback) {
                mEnemyBulletHitPlayerCallback();
            }
            return true;
        }

        // 擦弹检测
        if (bullet->lastGrazeTime > 0.5f) {
            float grazeDist = grazeRadius + bulletRadius;

            if (std::abs(dx) <= grazeDist && std::abs(dy) <= grazeDist) {
                if (circleCollision(playerPos, grazeRadius, bulletPos, bulletRadius)) {
                    bullet->lastGrazeTime = 0.f;

                    if (mPlayerGrazeEnemyBulletCallback)
                        mPlayerGrazeEnemyBulletCallback();
                }
            }
        }
    }

    return false;
}
bool CollisionManager::checkPlayerBulletsVsEnemy(
    std::vector<std::unique_ptr<TraceBullet>>& bullets,
    Enemy& enemy)
{
    if (enemy.getHP() <= 0) return false;

    glm::vec2 enemyPos = enemy.getSprite()->getPosition();
    glm::vec2 enemySize = enemy.getSprite()->getGlobalSize();

    for (auto it = bullets.begin(); it != bullets.end(); ) {
        if (!(*it) || !(*it)->sprite) {
            ++it;
            continue;
        }

        glm::vec2 bulletPos = (*it)->sprite->getPosition();
        glm::vec2 bulletSize = (*it)->sprite->getGlobalSize();

        // 使用 rectCollision 统一处理
        if (rectCollision(bulletPos, bulletSize, enemyPos, enemySize) && enemy.getHP() > 0) {
            enemy.onBulletHit(5);
            it = bullets.erase(it);
            if (mPlayerBulletHitEnemyCallback) {
                mPlayerBulletHitEnemyCallback();
            }
            return true;
        }
        else {
            ++it;
        }
    }

    return false;
}
bool CollisionManager::checkPlayerBulletsVsEnemy(
    std::vector<pSprite>& bullets,
    Enemy& enemy)
{
    if (enemy.getHP() <= 0) return false;

    glm::vec2 enemyPos = enemy.getSprite()->getPosition();
    glm::vec2 enemySize = enemy.getSprite()->getGlobalSize();

    for (auto it = bullets.begin(); it != bullets.end(); ) {
        if (!(*it)->getAvailable()) {
            ++it;
            continue;
        }

        glm::vec2 bulletPos = (*it)->getPosition();
        glm::vec2 bulletSize = (*it)->getGlobalSize();

        // 使用 rectCollision 统一处理
        if (rectCollision(bulletPos, bulletSize, enemyPos, enemySize) && enemy.getHP() > 0) {
            enemy.onBulletHit(10);
			(*it)->setAvailable(false);
            if (mPlayerBulletHitEnemyCallback) {
                mPlayerBulletHitEnemyCallback();
            }
        }
        else {
            ++it;
        }
    }

    return false;
}
bool CollisionManager::checkPlayerVsEnemy(Player& player, Enemy& enemy)
{
    if (enemy.getHP() <= 0) return false;

    glm::vec2 playerPos = player.get_position();
    glm::vec2 enemyPos = enemy.getSprite()->getPosition();
    float playerRadius = player.mMissRadius;
    float enemyRadius = static_cast<float>(enemy.mCollisionRadius);

    // 使用 circleCollision 统一判定
    if (circleCollision(playerPos, playerRadius, enemyPos, enemyRadius)) {
        if (mPlayerHitEnemyCallback) {
            mPlayerHitEnemyCallback();
        }
        return true;
    }

    return false;
}
void CollisionManager::setEnemyBulletHitPlayerCallback(CollisionCallback callback)
{
    mEnemyBulletHitPlayerCallback = callback;
}

void CollisionManager::setPlayerBulletHitEnemyCallback(CollisionCallback callback)
{
    mPlayerBulletHitEnemyCallback = callback;
}

void CollisionManager::setPlayerHitEnemyCallback(CollisionCallback callback)
{
    mPlayerHitEnemyCallback = callback;
}

void CollisionManager::setPlayerGrazeEnemyBulletCallback(CollisionCallback callback)
{
	mPlayerGrazeEnemyBulletCallback = callback;
}

bool CollisionManager::circleCollision(const glm::vec2& pos1, float radius1, const glm::vec2& pos2, float radius2)
{
    glm::vec2 d = pos1 - pos2;
    return glm::dot(d, d) < (radius1 * radius1 + radius2 * radius2);
}

bool CollisionManager::rectCollision(const glm::vec2& pos1, const glm::vec2& size1, const glm::vec2& pos2, const glm::vec2& size2)
{
    // AABB 碰撞检测
    bool xOverlap = abs(pos1.x - pos2.x) < (size1.x + size2.x) / 2.0f;
    bool yOverlap = abs(pos1.y - pos2.y) < (size1.y + size2.y) / 2.0f;
    return xOverlap && yOverlap;
}