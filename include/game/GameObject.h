#pragma once
#include <glm/glm.hpp>
#include <Sprite.hpp>
#include <memory>

using pSprite = std::unique_ptr<esl::Sprite>;

/**
 * 游戏对象基类
 * Enemy 和 Bullet 都继承自这个类，使它们在结构上处于同一层次
 */
class GameObject {
public:
    virtual ~GameObject() = default;

    // 核心接口
    virtual void update(double delta) = 0;
    virtual void render() = 0;

    // 位置相关
    virtual glm::vec2 getPosition() const { 
        return mSprite ? mSprite->getPosition() : glm::vec2(0, 0); 
    }
    virtual void setPosition(glm::vec2 pos) { 
        if (mSprite) mSprite->setPosition(pos); 
    }

    // 可选：旋转、缩放等
    virtual float getRotation() const {
        return mSprite ? mSprite->getRotation() : 0.0f;
    }
    virtual void setRotation(float angle) {
        if (mSprite) mSprite->setRotation(angle);
    }

    // 获取精灵（供需要直接访问的地方使用）
    pSprite& getSprite() { return mSprite; }
    const pSprite& getSprite() const { return mSprite; }

protected:
    pSprite mSprite;  // 精灵（可视化表示）
};
