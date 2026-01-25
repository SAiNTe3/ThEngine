#pragma once
#include <iostream>
#include <functional>
#include <vector>
#include <memory>
#include <Window.hpp>
#include "MovementState.h"
#include "MovementUpdater.h"
#include "Bullet.h"  // 引入 Bullet 头文件以使用 BulletLinearMovement

class Enemy;
class Action {
public:
    virtual ~Action() = default;
    virtual bool update(double deltaTime) = 0; // 返回true表示事件完成
    virtual void apply(class Enemy* enemy) = 0;
};


class MovementAction : public Action {
    std::function<glm::vec2(double)> mTrajectoryFunc; // 轨迹函数
    double mDuration;
    double mElapsedTime = 0;
public:
    MovementAction(std::function<glm::vec2(double)> func, double duration);
    bool update(double deltaTime);

    void apply(Enemy* enemy);
};

class Await :public Action {
    double mAwaitTime;
    double mElapsedTime = 0;
public:
    Await(double awaitTime);
    bool update(double deltaTime);
    void apply(Enemy* enemy);
};

// ========== 弹幕系统 ==========

// 子弹配置
struct BulletConfig {
    int type = 2;           // 子弹类型
    int color = 1;          // 子弹颜色
    float baseAngle = 0;    // 基础角度
    float baseSpeed = 200;  // 基础速度
};

// 发射模式枚举
enum class DanmakuPattern {
    LINEAR,     // 直线（单发或连发）
    CIRCLE,     // 环形
    FAN,        // 扇形
    RANDOM,     // 随机
    SPIRAL      // 螺旋
};

// 前向声明
class Bullet;
class BulletMovementAction {
public:
    virtual ~BulletMovementAction() = default;
    virtual bool update(double deltaTime) = 0;
    virtual void apply(Bullet* bullet) = 0;
};
using pBulletMovementAction = std::unique_ptr<BulletMovementAction>;

class GenericBulletMovementAction : public BulletMovementAction {
private:
    MovementState mState;
    std::unique_ptr<IMovementUpdater> mUpdater;
    std::unique_ptr<IStopCondition> mStopCondition;
    bool mInitialized = false;

public:
    GenericBulletMovementAction(MovementState initialState,
        std::unique_ptr<IMovementUpdater> updater,
        std::unique_ptr<IStopCondition> stopCondition)
        : mState(initialState)
        , mUpdater(std::move(updater))
        , mStopCondition(std::move(stopCondition)) {
    }

    bool update(double deltaTime) override;
    void apply(Bullet* bullet) override;
};


// 弹幕 Action - 负责发射一组子弹
class DanmakuAction : public Action {
private:
    // 基础配置
    BulletConfig mBulletConfig;
    DanmakuPattern mPattern = DanmakuPattern::LINEAR;

    // 发射参数
    int mBulletCount = 1;           // 每次发射的子弹数量
    int mRounds = 1;                // 发射轮数（-1表示无限）
    double mRoundInterval = 1.0;    // 轮次间隔
    double mElapsedTime = 0;
    int mCurrentRound = 0;
    double mLastShootTime = 0;
    bool mUsePlayerTarget = false;  // 是否朝向玩家（发射时计算一次）
    std::vector<int> mColorVector;  // 每次发射子弹的颜色列表（循环）
    // 模式特定参数
    float mAngleStep = 0;           // 环形/扇形：角度间隔
    float mAnglePerRound = 0;       // 每轮角度增量（螺旋）
    float mSpeedVariation = 0;      // 速度变化
    glm::vec2 mPositionOffset{0,0}; // 发射位置偏移
    float mShootRadius = 0.0f;      // 发射半径（0表示从中心发射）
    bool mAlignToRadius = true;     // 是否将子弹方向对齐到半径方向
    bool mColorVectorUsed = false;  // 是否使用子弹颜色列表
    int mColorIndex = -1;            // 若使用列表的序号

    // 子弹运动配置列表
    std::vector<std::function<pBulletMovementAction()>> mMovementBuilders;

    float mBulletAcceleration = 0.0f;
    float mAccelerationDuration = -1.0f;  // 加速持续时间（-1表示永久）
    float mBulletAngularVelocity = 0.0f;
    float mBulletAngularAccel = 0.0f;
    double mBulletMoveDuration = 10.0;
    bool mBulletNeverStop = false;
    bool mSyncRotationWithDirection = false;  // 子弹精灵是否跟随运动方向旋转

    // 目标获取器
    std::function<glm::vec2()> mTargetGetter;

    // 渲染器引用
    esl::Window* mRenderer = nullptr;
    std::function<glm::vec2()> mPlayerPosGetter;  // 保留兼容性
    std::function<bool()> mFinishCondition = []()->bool{ return false; };
    // 内部方法
    void shootLinear(class Enemy* enemy, glm::vec2 startPos);
    void shootCircle(class Enemy* enemy, glm::vec2 startPos);
    void shootFan(class Enemy* enemy, glm::vec2 startPos);
    void shootSpiral(class Enemy* enemy, glm::vec2 startPos);
    void nextColor() {
        mColorIndex = (mColorIndex + 1) % mColorVector.size();
        mBulletConfig.color = mColorVector[mColorIndex];
    }

public:
    DanmakuAction() = default;

    bool update(double deltaTime) override;
    void apply(class Enemy* enemy) override;

    // ========== 构建器 API ==========

    // 设置子弹类型和颜色
    DanmakuAction& bullet(int type, int color) {
        mBulletConfig.type = type;
        mBulletConfig.color = color;
        return *this;
    }
    // 设置发射模式
    DanmakuAction& pattern(DanmakuPattern p) {
        mPattern = p;
        return *this;
    }
    // 设置子弹数量
    DanmakuAction& count(int n) {
        mBulletCount = n;
        return *this;
    }
    // 设置每轮子弹颜色
    DanmakuAction& colors(std::vector<int> colors) {
        mColorVectorUsed = true;
        mColorVector = colors;
        return *this;
    }
    // 设置发射轮数和间隔
    DanmakuAction& rounds(int n, double interval = 1.0) {
        mRounds = n;
        mRoundInterval = interval;
        return *this;
    }
    // 设置无限轮次发射
    DanmakuAction& infiniteRounds(double interval = 1.0) {
        mRounds = -1;  // -1 表示无限轮次
        mRoundInterval = interval;
        return *this;
    }
    // 只设置间隔（保持当前轮数）
    DanmakuAction& interval(double interval) {
        mRoundInterval = interval;
        return *this;
    }
    // 设置每轮角度增量（用于螺旋等）
    DanmakuAction& rotatePerRound(float angleDelta) {
        mAnglePerRound = angleDelta;
        return *this;
    }
    // 设置方向
    DanmakuAction& direction(float angle) {
        mBulletConfig.baseAngle = angle;
        return *this;
    }
    // 朝向目标（发射时计算方向）
    DanmakuAction& toTarget(std::function<glm::vec2()> targetGetter) {
        mTargetGetter = targetGetter;
        mUsePlayerTarget = true;
        return *this;
    }
    // 朝向玩家（便捷方法，已弃用 - 使用 toTarget 代替）
    DanmakuAction& towardPlayer() {
        // 注意：需要先调用 setPlayerPosGetter
        mUsePlayerTarget = true;
        return *this;
    }
    // 设置速度
    DanmakuAction& speed(float s) {
        mBulletConfig.baseSpeed = s;
        return *this;
    }
    // 设置角度间隔（环形/扇形）
    DanmakuAction& angleStep(float step) {
        mAngleStep = step;
        return *this;
    }
    // 设置速度变化
    DanmakuAction& speedVariation(float variation) {
        mSpeedVariation = variation;
        return *this;
    }
    // 设置位置偏移
    DanmakuAction& offset(glm::vec2 off) {
        mPositionOffset = off;
        return *this;
    }
    DanmakuAction& acceleration(float accel) {
        mBulletAcceleration = accel;
        return *this;
    }
    // 设置加速持续时间（秒）
    DanmakuAction& accelerateDuring(float duration) {
        mAccelerationDuration = duration;
        return *this;
    }
    DanmakuAction& angularVelocity(float angularVel) {
        mBulletAngularVelocity = angularVel;
        return *this;
    }
    DanmakuAction& angularAcceleration(float angularAccel) {
        mBulletAngularAccel = angularAccel;
        return *this;
    }
    DanmakuAction& neverStop() {
        mBulletNeverStop = true;
        return *this;
    }
    // 设置子弹精灵跟随运动方向旋转
    DanmakuAction& syncRotation(bool enable = true) {
        mSyncRotationWithDirection = enable;
        return *this;
    }

    // 设置子弹运动配置
    DanmakuAction& withMovement(std::function<pBulletMovementAction()> builder) {
        mMovementBuilders.push_back(builder);
        return *this;
    }
    // 添加：清除所有运动配置
    DanmakuAction& clearMovements() {
        mMovementBuilders.clear();
        return *this;
    }
    // 设置发射半径（从敌人中心半径R的圆周上发射）
    DanmakuAction& shootFromRadius(float radius) {
        mShootRadius = radius;
        return *this;
    }

    DanmakuAction& finishWhen(std::function<bool()> finishCondition) {
        mFinishCondition = finishCondition;
        return *this;
    }

    // 设置是否将子弹方向对齐到半径（默认true）
    DanmakuAction& alignToRadius(bool align = true) {
        mAlignToRadius = align;
        return *this;
    }

    // 构建并返回 unique_ptr（用于 addAction）
    std::unique_ptr<DanmakuAction> build() {
        auto action = std::make_unique<DanmakuAction>();
        // 复制所有配置
        *action = *this;
        return action;
    }

    // 设置必要的引用
    void setRenderer(esl::Window* renderer) { mRenderer = renderer; }
    void setPlayerPosGetter(std::function<glm::vec2()> getter) { mPlayerPosGetter = getter; }
};



// ========== 通用运动 Action ==========
class GenericMovementAction : public Action {
private:
    MovementState mState;
    std::unique_ptr<IMovementUpdater> mUpdater;
    std::unique_ptr<IStopCondition> mStopCondition;

public:
    GenericMovementAction(MovementState initialState,
        std::unique_ptr<IMovementUpdater> updater,
        std::unique_ptr<IStopCondition> stopCondition)
        : mState(initialState)
        , mUpdater(std::move(updater))
        , mStopCondition(std::move(stopCondition)) {
    }

    bool update(double deltaTime) override {
        mUpdater->update(mState, deltaTime);
        return mStopCondition->shouldStop(mState);
    }

    void apply(Enemy* enemy) override;
};

// ========== 距离停止条件（已移动的总距离）==========
class DistanceTraveledStop : public IStopCondition {
private:
    float mTargetDistance;
    glm::vec2 mStartPosition{0,0};
    bool mInitialized = false;

public:
    explicit DistanceTraveledStop(float distance)
        : mTargetDistance(distance) {
    }

    bool shouldStop(const MovementState& state) const override {
        // 第一次调用时记录起始位置
        if (!mInitialized) {
            const_cast<DistanceTraveledStop*>(this)->mStartPosition = state.position;
            const_cast<DistanceTraveledStop*>(this)->mInitialized = true;
            return false;
        }

        // 计算已移动的距离
        float distanceTraveled = glm::distance(mStartPosition, state.position);
        return distanceTraveled >= mTargetDistance;
    }

    std::unique_ptr<IStopCondition> clone() const override {
        auto cloned = std::make_unique<DistanceTraveledStop>(mTargetDistance);
        cloned->mStartPosition = mStartPosition;
        cloned->mInitialized = mInitialized;
        return cloned;
    }
};

// 基于动画完成度的停止条件

class CompletionStop : public IStopCondition {
public:
    bool shouldStop(const MovementState& state) const override {
        // 检查是否到达目标点（误差小于1像素）
        if (state.useTargetMode) {
            return glm::distance(state.position, state.targetPosition) < 1.0f;
        }
        return false;
    }

    std::unique_ptr<IStopCondition> clone() const override {
        return std::make_unique<CompletionStop>();
    }
};