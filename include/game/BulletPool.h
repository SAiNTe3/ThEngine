#pragma once
#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include <Bullet.h>

// 子弹对象池，用于复用子弹对象，减少内存分配开销
class BulletPool {
private:
    static std::unique_ptr<BulletPool> s_instance;
    static std::mutex s_mutex;
    
    std::queue<std::unique_ptr<Bullet_1>> m_availableBullets;
    mutable std::mutex m_poolMutex;
    size_t m_maxPoolSize;
    size_t m_initialPoolSize;
    
    // 私有构造函数，实现单例模式
    BulletPool(size_t initialSize = 100, size_t maxSize = 500);
    
public:
    // 获取单例实例
    static BulletPool& getInstance();
    
    // 禁用拷贝构造和赋值
    BulletPool(const BulletPool&) = delete;
    BulletPool& operator=(const BulletPool&) = delete;
    
    // 从对象池获取子弹对象
    std::unique_ptr<Bullet_1> getBullet(int type, int color, glm::vec2 pos, esl::Window& render, float angle, float speed);
    
    // 将子弹对象归还到对象池
    void returnBullet(std::unique_ptr<Bullet_1> bullet);
    
    // 预分配子弹对象到池中（需要窗口引用）
    void preallocate(size_t count, esl::Window& render);
    
    // 获取池的状态信息
    size_t getAvailableCount() const;
    size_t getMaxPoolSize() const { return m_maxPoolSize; }
    
    // 清理对象池
    void clear();
    
    // 析构函数
    ~BulletPool() = default;
    
private:
    // 重新初始化子弹对象
    void reinitializeBullet(Bullet_1* bullet, int type, int color, glm::vec2 pos, esl::Window& render, float angle, float speed);
};

// 便于使用的全局函数
namespace BulletPoolHelper {
    // 获取子弹
    std::unique_ptr<Bullet_1> getBullet(int type, int color, glm::vec2 pos, esl::Window& render, float angle, float speed);
    
    // 归还子弹
    void returnBullet(std::unique_ptr<Bullet_1> bullet);
    
    // 预分配
    void preallocateBullets(size_t count, esl::Window& render);
    
    // 获取池状态
    size_t getAvailableBulletCount();
}