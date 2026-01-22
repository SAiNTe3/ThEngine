#pragma once
#include "MovementState.h"
#include "MovementUpdater.h"
#include "Action.h"
#include <memory>

// ========== 直线运动 Builder ==========
class LinearMovement {
private:
    MovementState mState;
    std::unique_ptr<IStopCondition> mStopCondition;
    std::function<float(float)> mEaseFunc = EaseFunctions::linear;
    bool mUseEasing = false;

public:
    // 设置目标点
    LinearMovement& to(glm::vec2 target) {
        mState.targetPosition = target;
        mState.useTargetMode = true;
        return *this;
    }
    // 新增：动态目标（跟踪目标物体）
    LinearMovement& toTarget(std::function<glm::vec2()> targetGetter) {
        mState.targetGetter = targetGetter;
        mState.useDynamicTarget = true;
        mState.useTargetMode = true;
        return *this;
    }

    LinearMovement& enableTracking() {
        mState.useDynamicTarget = true;
        return *this;
    }
    LinearMovement& disableTracking() {
        mState.useDynamicTarget = false;
        return *this;
    }
    LinearMovement& alwaysTracking() {
        return enableTracking();
    }

    // 便捷方法：跟踪目标 + 偏移
    LinearMovement& toTarget(std::function<glm::vec2()> targetGetter, glm::vec2 offset) {
        mState.targetGetter = [targetGetter, offset]() {
            return targetGetter() + offset;
            };
        mState.useDynamicTarget = true;
        mState.useTargetMode = true;
        return *this;
    }
    // 设置方向和速度
    LinearMovement& direction(float angleDegrees) {
        mState.direction = angleDegrees;
        mState.useTargetMode = false;
        return *this;
    }

    LinearMovement& speed(float speed) {
        mState.speed = speed;
        return *this;
    }

    // 设置加速度
    LinearMovement& accelerate(float acc) {
        // 如果已经设置了方向，使用方向计算向量
        // 注意：这里我们只记录标量加速度，在更新器中结合方向使用
        // 或者我们可以在这里暂时不计算向量，留给Updater处理
        // 但目前的State结构acceleration是向量
        
        // 方案：存储加速度标量，让Updater去计算方向
        // 但State没有accelerationScalar字段
        // 临时方案：如果direction已知，就用direction；如果未知（比如依赖Target），这就麻烦了
        
        // 修正：我们应该修改Updater，使其能够处理标量加速度沿着当前速度方向（或Target方向）
        // 但State里只有acceleration向量。
        
        // 让我们改变策略：这里只设置一个标记或特殊值，或者修改State结构
        // 为了最小改动，我们假定direction已经设置（对于非Target模式）
        // 对于Target模式，direction会在initialize时计算
        
        // 更好的方案：在MovementState中添加 accelerationScalar
        // 但这需要修改State结构。
        
        // 替代方案：在Updater中，如果发现acceleration向量为0但有accelerationDuration，那也不对
        
        // 让我们修改MovementBuilder，使其在构建时尽可能推迟向量计算
        // 遗憾的是State是直接传递的。
        
        // 观察 LinearMovementUpdater::update
        // state.velocity += state.acceleration * deltaTime;
        // 它是直接加向量。这意味着 acceleration 必须是正确的向量。
        
        // 如果是 .toTarget() ... .accelerate(100)，此时 direction 未知（依赖 targetGetter）
        // 因此 acceleration 向量无法在这里正确计算。
        
        // 必须修改 MovementState 和 Updater 来支持 "沿当前方向加速"
        mState.accelerationScalar = acc;
        return *this;
    }

    LinearMovement& accelerate(glm::vec2 acc) {
        mState.acceleration = acc;
        return *this;
    }

    // 新增: 设置加速持续时间
    LinearMovement& accelerateDuring(float duration) {
        mState.accelerationDuration = duration;
        return *this;
    }

    // 设置缓动函数
    LinearMovement& ease(std::function<float(float)> func) {
        mEaseFunc = func;
        mUseEasing = true;
        return *this;
    }

    LinearMovement& easeIn() { return ease(EaseFunctions::easeIn); }
    LinearMovement& easeOut() { return ease(EaseFunctions::easeOut); }
    LinearMovement& easeInOut() { return ease(EaseFunctions::easeInOut); }

    // 设置终止条件
    LinearMovement& stopAfter(double seconds) {
        mStopCondition = std::make_unique<TimeStop>(seconds);
        return *this;
    }

    LinearMovement& stopWhenReached(float threshold = 5.0f) {
        mStopCondition = std::make_unique<DistanceStop>(threshold);
        return *this;
    }

    LinearMovement& stopWhenSlow(float threshold = 0.1f) {
        mStopCondition = std::make_unique<VelocityStop>(threshold);
        return *this;
    }
    // 设置终止条件：移动指定距离后停止
    LinearMovement& stopWhenReachDistance(float distance) {
        mStopCondition = std::make_unique<DistanceTraveledStop>(distance);
        return *this;
    }

    // 新增: 永不停止
    LinearMovement& neverStop() {
        mStopCondition = std::make_unique<NeverStop>();
        return *this;
    }
    
    // 构建 Action
    std::unique_ptr<Action> build() {
        // 默认终止条件：时间或距离
        if (!mStopCondition) {
            if (mUseEasing && mState.useTargetMode) {
                // 使用缓动时，应该等到动画完成
                auto anyStop = std::make_unique<AnyStop>();
                anyStop->add(std::make_unique<CompletionStop>());
                anyStop->add(std::make_unique<TimeStop>(300.0)); // 设置一个很大的时间作为安全网
                mStopCondition = std::move(anyStop);
            }
            else {
                // 原有逻辑
                auto anyStop = std::make_unique<AnyStop>();
                anyStop->add(std::make_unique<TimeStop>(10.0));
                if (mState.useTargetMode) {
                    anyStop->add(std::make_unique<DistanceStop>(5.0f));
                }
                mStopCondition = std::move(anyStop);
            }
        }

        // 选择合适的更新器
        std::unique_ptr<IMovementUpdater> updater;
        if (mUseEasing) {
            updater = std::make_unique<TargetMovementUpdater>(mEaseFunc);
        }
        else {
            updater = std::make_unique<LinearMovementUpdater>();
        }

        return std::make_unique<GenericMovementAction>(
            mState, std::move(updater), std::move(mStopCondition)
        );
    }

    std::unique_ptr<BulletMovementAction> buildBullet() {
        // 默认终止条件：时间或距离
        if (!mStopCondition) {
            if (mUseEasing && mState.useTargetMode) {
                // 使用缓动时，应该等到动画完成
                auto anyStop = std::make_unique<AnyStop>();
                anyStop->add(std::make_unique<CompletionStop>());
                anyStop->add(std::make_unique<TimeStop>(300.0)); // 设置一个很大的时间作为安全网
                mStopCondition = std::move(anyStop);
            }
            else {
                // 原有逻辑
                auto anyStop = std::make_unique<AnyStop>();
                anyStop->add(std::make_unique<TimeStop>(10.0));
                if (mState.useTargetMode) {
                    anyStop->add(std::make_unique<DistanceStop>(5.0f));
                }
                mStopCondition = std::move(anyStop);
            }
        }

        // 选择合适的更新器
        std::unique_ptr<IMovementUpdater> updater;
        if (mUseEasing) {
            updater = std::make_unique<TargetMovementUpdater>(mEaseFunc);
        }
        else {
            updater = std::make_unique<LinearMovementUpdater>();
        }

        return std::make_unique<GenericBulletMovementAction>(
            mState, std::move(updater), std::move(mStopCondition)
        );
    }
};

// ========== 圆周运动 Builder ==========
class CircularMovement {
private:
    MovementState mState;
    std::unique_ptr<IStopCondition> mStopCondition;

public:
    CircularMovement& radius(float r) {
        mState.radius = r;
        mState.useCenterMode = false;
        mState.useTangentMode = false;
        mState.autoCalculateRadius = false;
        return *this;
    }
    // ========== 模式2：以指定点为中心（新增）==========
    CircularMovement& centerAt(glm::vec2 centerPos) {
        mState.center = centerPos;
        mState.useCenterMode = true;  // 标记为"以外部点为中心"
        mState.useTangentMode = false;
        mState.autoCalculateRadius = true;  // 自动计算半径和角度
        return *this;
    }

    // ========== 模式3：指定中心和半径（完全手动）==========
    CircularMovement& centerAt(glm::vec2 centerPos, float r) {
        mState.center = centerPos;
        mState.radius = r;
        mState.useCenterMode = true;
        mState.useTangentMode = false;
        mState.autoCalculateRadius = false;  // 不自动计算，使用指定值
        return *this;
    }
    // 设置切线速度（线速度）
    CircularMovement& tangentVelocity(float v) {
        mState.tangentVelocity = v;
        mState.useTangentMode = true;
        mState.useCenterMode = false;
        return *this;
    }

    // 设置切线方向（当前运动方向，度）
    CircularMovement& tangentDirection(float degrees) {
        mState.tangentDirection = degrees;
        mState.useTangentMode = true;
        mState.useCenterMode = false;
        return *this;
    }

    // 设置半径（必需）
    CircularMovement& withRadius(float r) {
        mState.radius = r;
        return *this;
    }

    // 设置旋转方向
    CircularMovement& clockwise(bool cw = true) {
        mState.clockwise = cw;
        return *this;
    }

    CircularMovement& counterClockwise() {
        mState.clockwise = false;
        return *this;
    }
    // ========== 通用方法 ==========
    CircularMovement& radiusSpeed(float v) {
        mState.radiusVelocity = v;
        return *this;
    }

    CircularMovement& angleSpeed(float radPerSec) {
        mState.angleVelocity = radPerSec;
        return *this;
    }

    CircularMovement& startAngle(float radians) {
        mState.angle = radians;
        return *this;
    }

    // 终止条件
    CircularMovement& stopAfter(double seconds) {
        mStopCondition = std::make_unique<TimeStop>(seconds);
        return *this;
    }

    CircularMovement& stopAtRadius(float minR, float maxR) {
        mStopCondition = std::make_unique<RadiusStop>(minR, maxR);
        return *this;
    }

    CircularMovement& stopAfterRotation(float radians) {
        mStopCondition = std::make_unique<AngleRangeStop>(radians);
        return *this;
    }

    CircularMovement& circles(float numCircles) {
        mStopCondition = std::make_unique<AngleRangeStop>(numCircles * 2.0f * glm::pi<float>());
        return *this;
    }

    std::unique_ptr<Action> build() {
        if (!mStopCondition) {
            // 默认：1圈或10秒
            auto anyStop = std::make_unique<AnyStop>();
            anyStop->add(std::make_unique<TimeStop>(10.0));
            anyStop->add(std::make_unique<AngleRangeStop>(2.0f * glm::pi<float>()));
            mStopCondition = std::move(anyStop);
        }

        return std::make_unique<GenericMovementAction>(
            mState,
            std::make_unique<CircularMovementUpdater>(),
            std::move(mStopCondition)
        );
    }

    std::unique_ptr<BulletMovementAction> buildBullet() {
        if (!mStopCondition) {
            // 默认：1圈或10秒
            auto anyStop = std::make_unique<AnyStop>();
            anyStop->add(std::make_unique<TimeStop>(10.0));
            anyStop->add(std::make_unique<AngleRangeStop>(2.0f * glm::pi<float>()));
            mStopCondition = std::move(anyStop);
        }

        return std::make_unique<GenericBulletMovementAction>(
            mState,
            std::make_unique<CircularMovementUpdater>(),
            std::move(mStopCondition)
        );
    }
};

// ========== 椭圆运动 Builder ==========
class EllipticalMovement {
private:
    MovementState mState;
    std::unique_ptr<IStopCondition> mStopCondition;

public:
    EllipticalMovement& radius(float r) {
        mState.radius = r;
        mState.useCenterMode = false;
        mState.autoCalculateRadius = false;
        return *this;
    }
    // 模式2：以指定点为中心
    EllipticalMovement& centerAt(glm::vec2 centerPos) {
        mState.center = centerPos;
        mState.useCenterMode = true;
        mState.autoCalculateRadius = true;
        return *this;
    }

    // 模式3：指定中心和半径
    EllipticalMovement& centerAt(glm::vec2 centerPos, float r) {
        mState.center = centerPos;
        mState.radius = r;
        mState.useCenterMode = true;
        mState.autoCalculateRadius = false;
        return *this;
    }
    EllipticalMovement& axisRatio(float ratio) {
        mState.axisRatio = ratio;
        return *this;
    }

    EllipticalMovement& rotate(float radians) {
        mState.ellipseDirection = radians;
        return *this;
    }

    EllipticalMovement& angleSpeed(float radPerSec) {
        mState.angleVelocity = radPerSec;
        return *this;
    }

    EllipticalMovement& stopAfter(double seconds) {
        mStopCondition = std::make_unique<TimeStop>(seconds);
        return *this;
    }

    EllipticalMovement& circles(float numCircles) {
        mStopCondition = std::make_unique<AngleRangeStop>(numCircles * 2.0f * glm::pi<float>());
        return *this;
    }

    std::unique_ptr<Action> build() {
        if (!mStopCondition) {
            mStopCondition = std::make_unique<TimeStop>(10.0);
        }

        return std::make_unique<GenericMovementAction>(
            mState,
            std::make_unique<EllipticalMovementUpdater>(),
            std::move(mStopCondition)
        );
    }

    std::unique_ptr<BulletMovementAction> buildBullet() {
        if (!mStopCondition) {
            mStopCondition = std::make_unique<TimeStop>(10.0);
        }

        return std::make_unique<GenericBulletMovementAction>(
            mState,
            std::make_unique<EllipticalMovementUpdater>(),
            std::move(mStopCondition)
        );
    }
};

// ========== 波动运动构建器 ==========
class WaveMovement {
private:
    MovementState mState;
    float mAmplitude = 50.0f;
    float mFrequency = 2.0f;
    float mPhase = 0.0f;
    std::unique_ptr<IStopCondition> mStopCondition;

public:
    WaveMovement() {
        mState.speed = 100.0f;
        mState.direction = 270.0f;  // 默认向下
    }

    // 设置波动幅度
    WaveMovement& amplitude(float amp) {
        mAmplitude = amp;
        return *this;
    }

    // 设置波动频率（每秒振荡次数)
    WaveMovement& frequency(float freq) {
        mFrequency = freq;
        return *this;
    }

    // 设置相位偏移（弧度）
    WaveMovement& phase(float ph) {
        mPhase = ph;
        return *this;
    }

    // 设置基础移动方向（角度）
    WaveMovement& direction(float deg) {
        mState.direction = deg;
        return *this;
    }

    // 设置基础速度
    WaveMovement& speed(float spd) {
        mState.speed = spd;
        return *this;
    }

    // 停止条件：时间
    WaveMovement& stopAfter(double seconds) {
        mStopCondition = std::make_unique<TimeStop>(seconds);
        return *this;
    }

    // 停止条件：距离
    WaveMovement& stopAfterDistance(float distance) {
        mStopCondition = std::make_unique<DistanceStop>(distance);
        return *this;
    }

    std::unique_ptr<Action> build() {
        if (!mStopCondition) {
            mStopCondition = std::make_unique<TimeStop>(10.0);
        }
        return std::make_unique<GenericMovementAction>(
            mState,
            std::make_unique<WaveMovementUpdater>(mAmplitude, mFrequency, mPhase),
            std::move(mStopCondition)
        );
    }

    std::unique_ptr<BulletMovementAction> buildBullet() {
        if (!mStopCondition) {
            mStopCondition = std::make_unique<TimeStop>(10.0);
        }
        return std::make_unique<GenericBulletMovementAction>(
            mState,
            std::make_unique<WaveMovementUpdater>(mAmplitude, mFrequency, mPhase),
            std::move(mStopCondition)
        );
    }
};

// ========== 贝塞尔曲线运动构建器 ==========
class BezierMovement {
private:
    MovementState mState;
    std::vector<glm::vec2> mControlPoints;
    float mDuration = 1.0f;
    std::function<float(float)> mEaseFunc = EaseFunctions::linear;
    std::unique_ptr<IStopCondition> mStopCondition;

public:
    BezierMovement() = default;

    // 添加控制点（相对于起点的偏移）
    BezierMovement& controlPoint(glm::vec2 point) {
        mControlPoints.push_back(point);
        return *this;
    }

    // 添加控制点（使用偏移量）
    BezierMovement& controlPointOffset(float x, float y) {
        mControlPoints.push_back(glm::vec2(x, y));
        return *this;
    }

    // 设置终点（相对于起点）
    BezierMovement& to(glm::vec2 endpoint) {
        mControlPoints.push_back(endpoint);
        return *this;
    }

    // 快速创建：二次贝塞尔（起点 -> 控制点 -> 终点）
    BezierMovement& quadratic(glm::vec2 controlPoint, glm::vec2 endpoint) {
        mControlPoints.clear();
        mControlPoints.push_back(controlPoint);
        mControlPoints.push_back(endpoint);
        return *this;
    }

    // 快速创建：三次贝塞尔（起点 -> 控制点1 -> 控制点2 -> 终点）
    BezierMovement& cubic(glm::vec2 cp1, glm::vec2 cp2, glm::vec2 endpoint) {
        mControlPoints.clear();
        mControlPoints.push_back(cp1);
        mControlPoints.push_back(cp2);
        mControlPoints.push_back(endpoint);
        return *this;
    }

    // 设置持续时间
    BezierMovement& duration(float seconds) {
        mDuration = seconds;
        return *this;
    }

    // 设置缓动函数
    BezierMovement& ease(std::function<float(float)> func) {
        mEaseFunc = func;
        return *this;
    }

    BezierMovement& easeIn() { return ease(EaseFunctions::easeIn); }
    BezierMovement& easeOut() { return ease(EaseFunctions::easeOut); }
    BezierMovement& easeInOut() { return ease(EaseFunctions::easeInOut); }

    std::unique_ptr<Action> build() {
        if (!mStopCondition) {
            mStopCondition = std::make_unique<TimeStop>(mDuration);
        }

        return std::make_unique<GenericMovementAction>(
            mState,
            std::make_unique<BezierMovementUpdater>(mControlPoints, mDuration, mEaseFunc),
            std::move(mStopCondition)
        );
    }

    std::unique_ptr<BulletMovementAction> buildBullet() {
        if (!mStopCondition) {
            mStopCondition = std::make_unique<TimeStop>(mDuration);
        }

        return std::make_unique<GenericBulletMovementAction>(
            mState,
            std::make_unique<BezierMovementUpdater>(mControlPoints, mDuration, mEaseFunc),
            std::move(mStopCondition)
        );
    }
};

// ========== 通用贝塞尔曲线运动构建器（支持任意阶数）==========
class GeneralBezierMovement {
private:
    MovementState mState;
    std::vector<glm::vec2> mControlPoints;//静态
    std::vector<std::function<glm::vec2()>> mDynamicPoints;//动态
    float mDuration = 1.0f;
    std::function<float(float)> mEaseFunc = EaseFunctions::linear;
    std::unique_ptr<IStopCondition> mStopCondition;

public:
    GeneralBezierMovement() = default;

    // 静态控制点
    GeneralBezierMovement& point(glm::vec2 p) {
        mControlPoints.push_back(p);
        mDynamicPoints.push_back(nullptr);  // 占位
        return *this;
    }

    // 动态控制点（跟踪目标）
    GeneralBezierMovement& pointAtTarget(std::function<glm::vec2()> getter) {
        mControlPoints.push_back(glm::vec2(0, 0));  // 占位
        mDynamicPoints.push_back(getter);
        return *this;
    }

    // 动态控制点 + 偏移
    GeneralBezierMovement& pointAtTarget(std::function<glm::vec2()> getter, glm::vec2 offset) {
        mControlPoints.push_back(glm::vec2(0, 0));
        mDynamicPoints.push_back([getter, offset]() { return getter() + offset; });
        return *this;
    }

    // 批量添加（混合静态和动态）
    GeneralBezierMovement& points(std::initializer_list<glm::vec2> pts) {
        for (const auto& p : pts) {
            point(p);
        }
        return *this;
    }

    // 设置持续时间
    GeneralBezierMovement& duration(float seconds) {
        mDuration = seconds;
        return *this;
    }

    // 缓动
    GeneralBezierMovement& ease(std::function<float(float)> func) {
        mEaseFunc = func;
        return *this;
    }

    GeneralBezierMovement& easeIn() { return ease(EaseFunctions::easeIn); }
    GeneralBezierMovement& easeOut() { return ease(EaseFunctions::easeOut); }
    GeneralBezierMovement& easeInOut() { return ease(EaseFunctions::easeInOut); }

    std::unique_ptr<Action> build() {
        if (!mStopCondition) {
            mStopCondition = std::make_unique<TimeStop>(mDuration + 0.1f);
        }

        return std::make_unique<GenericMovementAction>(
            mState,
            std::make_unique<GeneralBezierMovementUpdater>(
                mControlPoints,      // 参数1
                mDynamicPoints,      // 参数2
                mDuration,           // 参数3
                mEaseFunc            // 参数4
            ),
            std::move(mStopCondition)
        );
    }

    std::unique_ptr<BulletMovementAction> buildBullet() {
        if (!mStopCondition) {
            mStopCondition = std::make_unique<TimeStop>(mDuration + 0.1f);
        }

        return std::make_unique<GenericBulletMovementAction>(
            mState,
            std::make_unique<GeneralBezierMovementUpdater>(
                mControlPoints,      // 参数1
                mDynamicPoints,      // 参数2
                mDuration,           // 参数3
                mEaseFunc            // 参数4
            ),
            std::move(mStopCondition)
        );
    }
};

// ========== 复合通用贝塞尔曲线构建器 ==========
class CompositeGeneralBezierMovement {
private:
    MovementState mState;
    std::vector<CompositeGeneralBezierMovementUpdater::BezierSegment> mSegments;
    CompositeGeneralBezierMovementUpdater::BezierSegment mCurrentSegment;
    std::unique_ptr<IStopCondition> mStopCondition;

public:
    CompositeGeneralBezierMovement() {
        mCurrentSegment.duration = 1.0f;
        mCurrentSegment.easeFunc = EaseFunctions::linear;
    }

    // 添加控制点到当前段
    CompositeGeneralBezierMovement& point(glm::vec2 p) {
        mCurrentSegment.controlPoints.push_back(p);
        return *this;
    }

    CompositeGeneralBezierMovement& point(float x, float y) {
        return point(glm::vec2(x, y));
    }

    // 一次性添加多个控制点
    CompositeGeneralBezierMovement& points(std::initializer_list<glm::vec2> pts) {
        mCurrentSegment.controlPoints.insert(
            mCurrentSegment.controlPoints.end(), pts.begin(), pts.end()
        );
        return *this;
    }

    // 设置当前段持续时间
    CompositeGeneralBezierMovement& duration(float seconds) {
        mCurrentSegment.duration = seconds;
        return *this;
    }

    // 设置当前段缓动
    CompositeGeneralBezierMovement& ease(std::function<float(float)> func) {
        mCurrentSegment.easeFunc = func;
        return *this;
    }

    CompositeGeneralBezierMovement& easeIn() { return ease(EaseFunctions::easeIn); }
    CompositeGeneralBezierMovement& easeOut() { return ease(EaseFunctions::easeOut); }
    CompositeGeneralBezierMovement& easeInOut() { return ease(EaseFunctions::easeInOut); }
    CompositeGeneralBezierMovement& linear() { return ease(EaseFunctions::linear); }

    // 开始新段
    CompositeGeneralBezierMovement& newSegment() {
        if (!mCurrentSegment.controlPoints.empty()) {
            mSegments.push_back(mCurrentSegment);
            mCurrentSegment.controlPoints.clear();
            mCurrentSegment.duration = 1.0f;
            mCurrentSegment.easeFunc = EaseFunctions::linear;
        }
        return *this;
    }

    std::unique_ptr<Action> build() {
        // 保存最后一段
        if (!mCurrentSegment.controlPoints.empty()) {
            mSegments.push_back(mCurrentSegment);
        }

        float totalDuration = 0.0f;
        for (const auto& seg : mSegments) {
            totalDuration += seg.duration;
        }

        if (!mStopCondition) {
            mStopCondition = std::make_unique<TimeStop>(totalDuration + 0.1f);
        }

        return std::make_unique<GenericMovementAction>(
            mState,
            std::make_unique<CompositeGeneralBezierMovementUpdater>(mSegments),
            std::move(mStopCondition)
        );
    }

    std::unique_ptr<BulletMovementAction> buildBullet() {
        // 保存最后一段
        if (!mCurrentSegment.controlPoints.empty()) {
            mSegments.push_back(mCurrentSegment);
        }

        float totalDuration = 0.0f;
        for (const auto& seg : mSegments) {
            totalDuration += seg.duration;
        }

        if (!mStopCondition) {
            mStopCondition = std::make_unique<TimeStop>(totalDuration + 0.1f);
        }

        return std::make_unique<GenericBulletMovementAction>(
            mState,
            std::make_unique<CompositeGeneralBezierMovementUpdater>(mSegments),
            std::move(mStopCondition)
        );
    }
};

// ========== 别名定义，方便使用 buildBullet() ==========
using BulletLinearMovement = LinearMovement;
using BulletCircularMovement = CircularMovement;
using BulletEllipticalMovement = EllipticalMovement;
using BulletWaveMovement = WaveMovement;
using BulletBezierMovement = BezierMovement;
using BulletGeneralBezierMovement = GeneralBezierMovement;
using BulletCompositeGeneralBezierMovement = CompositeGeneralBezierMovement;