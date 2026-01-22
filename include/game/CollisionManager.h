#pragma once
#include <vector>
#include <memory>
#include <functional>
#include <glm/glm.hpp>
#include <Sprite.hpp>
#include <Clock.hpp>
class Player;
class Enemy;
class Bullet;
struct TraceBullet;
// pSprite的类型定义
using pSprite = std::unique_ptr<esl::Sprite>;

// 碰撞检测管理器
class CollisionManager {
public:
    // 碰撞事件回调函数类型
    using CollisionCallback = std::function<void()>;
    
    CollisionManager() = default;
    ~CollisionManager() = default;
    // 检测Enemy子弹与Player的碰撞
    bool checkEnemyBulletsVsPlayer(const std::vector<Bullet*>& bullets, Player& player);
    bool checkEnemyBulletsVsPlayer(
        const std::vector<std::unique_ptr<Bullet>>& bullets,
        Player& player
    );
    // 检测Player子弹与Enemy的碰撞
    bool checkPlayerBulletsVsEnemy(std::vector<pSprite>& bullets, Enemy& enemy);
    bool checkPlayerBulletsVsEnemy(std::vector<std::unique_ptr<TraceBullet>>& bullets, Enemy& enemy);

    // 检测Player与Enemy的碰撞
    bool checkPlayerVsEnemy(Player& player, Enemy& enemy);

    // 设置碰撞事件回调
    void setEnemyBulletHitPlayerCallback(CollisionCallback callback);
    void setPlayerBulletHitEnemyCallback(CollisionCallback callback);
    void setPlayerHitEnemyCallback(CollisionCallback callback);
	void setPlayerGrazeEnemyBulletCallback(CollisionCallback callback);
    //新增：获取回调函数
    const CollisionCallback& getEnemyBulletHitPlayerCallback() const {
        return mEnemyBulletHitPlayerCallback;
    }

    const CollisionCallback& getPlayerGrazeEnemyBulletCallback() const {
        return mPlayerGrazeEnemyBulletCallback;
    }
private:
    // 基础的圆形碰撞检测
    bool circleCollision(const glm::vec2& pos1, float radius1, const glm::vec2& pos2, float radius2);

    // 基础的矩形碰撞检测
    bool rectCollision(const glm::vec2& pos1, const glm::vec2& size1, const glm::vec2& pos2, const glm::vec2& size2);

    // 碰撞事件回调
    CollisionCallback mEnemyBulletHitPlayerCallback;
    CollisionCallback mPlayerBulletHitEnemyCallback;
    CollisionCallback mPlayerHitEnemyCallback;
	CollisionCallback mPlayerGrazeEnemyBulletCallback;
};