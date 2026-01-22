#include <BulletPool.h>
#include <Bullet.h>
#include <Action.h> // Include Action.h to ensure BulletMovementAction definition is available

// 静态成员初始化
std::unique_ptr<BulletPool> BulletPool::s_instance = nullptr;
std::mutex BulletPool::s_mutex;

BulletPool::BulletPool(size_t initialSize, size_t maxSize) 
    : m_maxPoolSize(maxSize), m_initialPoolSize(initialSize) {
    // 不在构造函数中预分配，因为需要 Window 引用
}

BulletPool& BulletPool::getInstance() {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_instance) {
        s_instance = std::unique_ptr<BulletPool>(new BulletPool(2500,2500));
    }
    return *s_instance;
}

void BulletPool::reinitializeBullet(Bullet_1* bullet, int type, int color, glm::vec2 pos, esl::Window& render, float angle, float speed) {
    // 重置基本属性
    bullet->mAngle = angle;
    bullet->mSpeed = speed;
    bullet->mMovementActions.clear();
    bullet->mSyncRotation = false;

    Bullet_1::setupBulletProperties(bullet->mSprite.get(), bullet->mCollisionRadius, type, color);

    // 设置位置、旋转和缩放
    bullet->mSprite->setPosition(pos);
    bullet->mSprite->setRotation(angle - 90);
    bullet->mSprite->setScale({ 2,2 });
    bullet->mCollisionRadius *= 2;
}
std::unique_ptr<Bullet_1> BulletPool::getBullet(
    int type, int color, glm::vec2 pos,
    esl::Window& render, float angle, float speed)
{
    std::lock_guard<std::mutex> lock(m_poolMutex);

    if (m_availableBullets.empty()) {
        // 修复：不再扩容，直接创建临时对象
        static size_t warningCount = 0;
        if (warningCount++ % 100 == 0) {
            printf("[BulletPool Warning] Pool exhausted %zu times! "
                "Pool size: %zu, MaxSize: %zu\n",
                warningCount, m_initialPoolSize, m_maxPoolSize);
        }

        return std::make_unique<Bullet_1>(type, color, pos, render, angle, speed);
    }

    auto bullet = std::move(m_availableBullets.front());
    m_availableBullets.pop();
    reinitializeBullet(bullet.get(), type, color, pos, render, angle, speed);
    return bullet;
}
void BulletPool::returnBullet(std::unique_ptr<Bullet_1> bullet) {
    if (!bullet) return;
    
    std::lock_guard<std::mutex> lock(m_poolMutex);

    // 修复：移除 maxPoolSize 限制
    bullet->mMovementActions.clear();
    bullet->mAngle = 0.0f;
    bullet->mSpeed = 0.0f;
    bullet->mSprite->setPosition(glm::vec2(-1000, -1000));

    m_availableBullets.push(std::move(bullet));
}

void BulletPool::preallocate(size_t count, esl::Window& render) {
    std::lock_guard<std::mutex> lock(m_poolMutex);
    
    // 修复：移除 maxPoolSize 限制，或者先扩大 maxPoolSize
    if (count > m_maxPoolSize) {
        m_maxPoolSize = count * 2;  // 自动扩大最大容量
        printf("[BulletPool] Auto-expanded maxPoolSize to %zu\n", m_maxPoolSize);
    }

    printf("[BulletPool] Preallocating %zu bullets...\n", count);

    for (size_t i = 0; i < count; ++i) {
        auto bullet = std::make_unique<Bullet_1>(1, 0, glm::vec2(-1000, -1000), render, 0.0f, 0.0f);
        m_availableBullets.push(std::move(bullet));
    }
    printf("[BulletPool] Preallocated %zu bullets successfully!\n", m_availableBullets.size());
}

size_t BulletPool::getAvailableCount() const {
    std::lock_guard<std::mutex> lock(m_poolMutex);
    return m_availableBullets.size();
}

void BulletPool::clear() {
    std::lock_guard<std::mutex> lock(m_poolMutex);
    while (!m_availableBullets.empty()) {
        m_availableBullets.pop();
    }
}

// BulletPoolHelper 命名空间的实现
namespace BulletPoolHelper {
    std::unique_ptr<Bullet_1> getBullet(int type, int color, glm::vec2 pos, esl::Window& render, float angle, float speed) {
        return BulletPool::getInstance().getBullet(type, color, pos, render, angle, speed);
    }
    
    void returnBullet(std::unique_ptr<Bullet_1> bullet) {
        BulletPool::getInstance().returnBullet(std::move(bullet));
    }
    
    void preallocateBullets(size_t count, esl::Window& render) {
        BulletPool::getInstance().preallocate(count, render);
    }
    
    size_t getAvailableBulletCount() {
        return BulletPool::getInstance().getAvailableCount();
    }
}