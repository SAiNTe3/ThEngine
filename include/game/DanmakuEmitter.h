#pragma once
#include "Action.h"
#include <vector>

/**
 * 独立的弹幕发射器
 * 用于从非 Enemy 位置发射弹幕（如随机位置、固定位置等）
 */
class DanmakuEmitter {
private:
    esl::Window* mRenderer;
    std::vector<std::unique_ptr<DanmakuAction>> mActions;
    std::vector<pBullet> mBullets;  // 发射的子弹
    glm::vec2 mPosition;  // 发射器位置

public:
    DanmakuEmitter(esl::Window* renderer, glm::vec2 position = {0, 0})
        : mRenderer(renderer), mPosition(position) {}
    
    // 设置位置
    void setPosition(glm::vec2 pos) { mPosition = pos; }
    glm::vec2 getPosition() const { return mPosition; }
    
    // 创建弹幕 Action
    DanmakuAction& shoot() {
        auto action = std::make_unique<DanmakuAction>();
        action->setRenderer(mRenderer);
        
        DanmakuAction* actionPtr = action.get();
        mActions.push_back(std::move(action));
        
        return *actionPtr;
    }
    
    // 更新
    void update(double deltaTime) {
        // 更新所有 Action
        for (auto it = mActions.begin(); it != mActions.end();) {
            (*it)->update(deltaTime);
            
            // 执行发射
            // 注意：这里需要一个假的 Enemy 对象，或者重构 apply 方法
            // 临时方案：直接在这里处理
            
            if ((*it)->update(deltaTime)) {
                it = mActions.erase(it);
            } else {
                ++it;
            }
        }
        
        // 更新子弹
        for (auto it = mBullets.begin(); it != mBullets.end();) {
            (*it)->update(deltaTime);
            
            // 检查子弹是否超出屏幕
            glm::vec2 pos = (*it)->getPosition();
            if (pos.x < 0 || pos.x > 896 || pos.y < 0 || pos.y > 1024) {
                // 归还对象池
                if (auto bullet1 = dynamic_cast<Bullet_1*>(it->get())) {
                    std::unique_ptr<Bullet_1> bullet1_ptr(static_cast<Bullet_1*>(it->release()));
                    BulletPoolHelper::returnBullet(std::move(bullet1_ptr));
                }
                it = mBullets.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    // 渲染
    void render() {
        for (auto& bullet : mBullets) {
            bullet->render();
        }
    }
    
    // 获取子弹列表（供碰撞检测使用）
    std::vector<pBullet>& getBullets() { return mBullets; }
    
    // 清空所有子弹
    void clear() {
        for (auto& bullet : mBullets) {
            if (auto bullet1 = dynamic_cast<Bullet_1*>(bullet.get())) {
                std::unique_ptr<Bullet_1> bullet1_ptr(static_cast<Bullet_1*>(bullet.release()));
                BulletPoolHelper::returnBullet(std::move(bullet1_ptr));
            }
        }
        mBullets.clear();
    }
};
