#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <functional>

// ========== 运动状态结构体 ==========
struct MovementState {
    // 位置相关
    glm::vec2 position = { 0, 0 };
    glm::vec2 targetPosition = { 0, 0 };
    bool useTargetMode = false;

    // 线性运动
    glm::vec2 velocity = { 0, 0 };          // 速度向量（像素/秒）
    glm::vec2 acceleration = { 0, 0 };      // 加速度向量（像素/秒²）
    float accelerationScalar = 0.0f;        // 标量加速度（沿当前方向，像素/秒²）
    float speed = -1;                       // 速度标量（像素/秒）
    float direction = 0;                   // 方向角度（度）
    float accelerationDuration = -1.0f;// 加速度持续时间（-1表示为无限）
    // 动态目标支持
    std::function<glm::vec2()> targetGetter;  // 动态目标位置获取器
    bool useDynamicTarget = false;             // 是否使用动态目标

    // 角度运动
    float angularVelocity = 0;             // 角速度（度/秒）
    float angularAcceleration = 0;         // 角加速度（度/秒²）

    // 圆周/椭圆运动
    glm::vec2 center = { 0, 0 };             // 圆心/椭圆中心
    float radius = 0;                      // 半径
    float radiusVelocity = 0;              // 半径变化速度
    float angle = 0;                       // 当前角度（弧度）
    float angleVelocity = 0;               // 角度变化速度（弧度/秒）

    bool useCenterMode = false;  // true: 使用指定的 center，false: 使用当前位置为中心
    bool autoCalculateRadius = true;  // true: 根据当前位置自动计算半径和起始角度
    bool useTangentMode = false;       // 使用切线速度模式

    // 切线模式参数
    float tangentVelocity = 0;         // 切线速度（线速度）
    float tangentDirection = 0;        // 切线方向（度）
    bool clockwise = true;             // 顺时针还是逆时针
    // 椭圆特有
    float axisRatio = 1.0f;                // 长短轴比例
    float ellipseDirection = 0;            // 椭圆旋转角度

    // 辅助状态
    double elapsedTime = 0;                // 已运行时间
    bool initialized = false;              // 是否已初始化

    // 重置方法
    void reset() {
        position = { 0, 0 };
        targetPosition = { 0, 0 };
        velocity = { 0, 0 };
        acceleration = { 0, 0 };
        accelerationScalar = 0.0f;
        speed = 0;
        direction = 0;
        angularVelocity = 0;
        angularAcceleration = 0;
        center = { 0, 0 };
        radius = 0;
        radiusVelocity = 0;
        angle = 0;
        angleVelocity = 0;
        axisRatio = 1.0f;
        ellipseDirection = 0;
        elapsedTime = 0;
        initialized = false;
    }
};

// ========== 终止条件接口 ==========
class IStopCondition {
public:
    virtual ~IStopCondition() = default;
    // 检查是否应该停止（const 保证不修改状态）
    virtual bool shouldStop(const MovementState& state) const = 0;
    virtual std::unique_ptr<IStopCondition> clone() const = 0;
};

// 时间终止条件
class TimeStop : public IStopCondition {
    double mDuration;
public:
    explicit TimeStop(double duration) : mDuration(duration) {}

    bool shouldStop(const MovementState& state) const override {
        return state.elapsedTime >= mDuration;
    }

    std::unique_ptr<IStopCondition> clone() const override {
        return std::make_unique<TimeStop>(mDuration);
    }
};

// 距离终止条件
class DistanceStop : public IStopCondition {
    float mThreshold;
public:
    explicit DistanceStop(float threshold = 5.0f) : mThreshold(threshold) {}

    bool shouldStop(const MovementState& state) const override {
        float dist = glm::distance(state.position, state.targetPosition);
        return dist <= mThreshold;
    }

    std::unique_ptr<IStopCondition> clone() const override {
        return std::make_unique<DistanceStop>(mThreshold);
    }
};

// 速度归零终止条件
class VelocityStop : public IStopCondition {
    float mThreshold;
public:
    explicit VelocityStop(float threshold = 0.1f) : mThreshold(threshold) {}

    bool shouldStop(const MovementState& state) const override {
        return state.speed <= mThreshold;
    }

    std::unique_ptr<IStopCondition> clone() const override {
        return std::make_unique<VelocityStop>(mThreshold);
    }
};

// 半径终止条件（用于圆周运动）
class RadiusStop : public IStopCondition {
    float mMinRadius;
    float mMaxRadius;
public:
    RadiusStop(float minRadius = 0, float maxRadius = 1e6f)
        : mMinRadius(minRadius), mMaxRadius(maxRadius) {
    }

    bool shouldStop(const MovementState& state) const override {
        return state.radius < mMinRadius || state.radius > mMaxRadius;
    }

    std::unique_ptr<IStopCondition> clone() const override {
        return std::make_unique<RadiusStop>(mMinRadius, mMaxRadius);
    }
};

// 角度范围终止条件（用于圆弧运动）
class AngleRangeStop : public IStopCondition {
    float mTotalAngle; // 总共旋转的角度（弧度）
    mutable float mStartAngle;
    mutable bool mStartAngleSet = false;
public:
    explicit AngleRangeStop(float totalAngle) : mTotalAngle(totalAngle), mStartAngle(0) {}

    bool shouldStop(const MovementState& state) const override {
        if (!mStartAngleSet) {
            mStartAngle = state.angle;
            mStartAngleSet = true;
        }
        float traveled = std::abs(state.angle - mStartAngle);
        return traveled >= mTotalAngle;
    }

    std::unique_ptr<IStopCondition> clone() const override {
        return std::make_unique<AngleRangeStop>(mTotalAngle);
    }
};

// 组合终止条件（任意一个满足即停止）
class AnyStop : public IStopCondition {
    std::vector<std::unique_ptr<IStopCondition>> mConditions;
public:
    void add(std::unique_ptr<IStopCondition> condition) {
        mConditions.push_back(std::move(condition));
    }

    bool shouldStop(const MovementState& state) const override {
        for (const auto& cond : mConditions) {
            if (cond->shouldStop(state)) return true;
        }
        return false;
    }

    std::unique_ptr<IStopCondition> clone() const override {
        auto cloned = std::make_unique<AnyStop>();
        for (const auto& cond : mConditions) {
            cloned->add(cond->clone());
        }
        return cloned;
    }
};

// 组合终止条件（所有条件都满足才停止）
class AllStop : public IStopCondition {
    std::vector<std::unique_ptr<IStopCondition>> mConditions;
public:
    void add(std::unique_ptr<IStopCondition> condition) {
        mConditions.push_back(std::move(condition));
    }

    bool shouldStop(const MovementState& state) const override {
        if (mConditions.empty()) return false;
        for (const auto& cond : mConditions) {
            if (!cond->shouldStop(state)) return false;
        }
        return true;
    }

    std::unique_ptr<IStopCondition> clone() const override {
        auto cloned = std::make_unique<AllStop>();
        for (const auto& cond : mConditions) {
            cloned->add(cond->clone());
        }
        return cloned;
    }
};

// 永不停止条件
class NeverStop : public IStopCondition {
public:
    bool shouldStop(const MovementState& state) const override {
        return false;
    }

    std::unique_ptr<IStopCondition> clone() const override {
        return std::make_unique<NeverStop>();
    }
};