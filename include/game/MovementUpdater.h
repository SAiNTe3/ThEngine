#pragma once
#include "MovementState.h"
#include <glm/glm.hpp>

// ========== 运动更新器接口 ==========
class IMovementUpdater {
public:
    virtual ~IMovementUpdater() = default;

    // 初始化状态（在第一帧调用）
    virtual void initialize(MovementState& state, const glm::vec2& currentPos) = 0;

    // 更新状态（每帧调用）
    virtual void update(MovementState& state, double deltaTime) = 0;

    virtual std::unique_ptr<IMovementUpdater> clone() const = 0;
};

// ========== 直线运动更新器 ==========
class LinearMovementUpdater : public IMovementUpdater {
public:
    void initialize(MovementState& state, const glm::vec2& currentPos) override {
        state.position = currentPos;

        // 动态目标：如果设置了 Getter，在初始化时至少求值一次
        // 这样即使 useDynamicTarget 为 false (Static Target 模式)，也能正确获取初始目标位置
        if (state.targetGetter) {
            state.targetPosition = state.targetGetter();
        }

        // 如果设置了目标点，计算方向和速度
        if (state.useTargetMode) {
            glm::vec2 direction = glm::normalize(state.targetPosition - currentPos);
            state.velocity = direction * state.speed;

            // 即使速度为0(等待发射)，也应该更新朝向
            if (glm::length(direction) > 0.001f) {
                state.direction = std::atan2(direction.y, direction.x) * 180.0f / glm::pi<float>();
            }
        }
        else {
            // 否则使用方向角度和速度
            float radians = glm::radians(state.direction);
            state.velocity = glm::vec2(std::cos(radians), std::sin(radians)) * state.speed;
        }

        state.initialized = true;
    }

    void update(MovementState& state, double deltaTime) override {
        // 如果启用了动态目标，每一帧更新速度方向
        if (state.useDynamicTarget && state.targetGetter) {
            state.targetPosition = state.targetGetter();
            glm::vec2 direction = glm::normalize(state.targetPosition - state.position);
            
            // 如果速度标量大于0，更新速度向量
            if (state.speed > 0.001f) {
                state.velocity = direction * state.speed;
            }
            // 更新朝向
            if (glm::length(direction) > 0.001f) {
                state.direction = std::atan2(direction.y, direction.x) * 180.0f / glm::pi<float>();
            }
        }

        if (state.accelerationDuration >= 0 ) {
            state.accelerationDuration -= deltaTime;
            // 更新速度：v = v₀ + at
            state.velocity += state.acceleration * static_cast<float>(deltaTime);
            
            // 沿当前方向应用标量加速度
            if (std::abs(state.accelerationScalar) > 0.001f) {
                float radians = glm::radians(state.direction);
                state.velocity += glm::vec2(std::cos(radians), std::sin(radians)) * state.accelerationScalar * static_cast<float>(deltaTime);
            }
        }
        
        state.speed = glm::length(state.velocity);

        // 更新方向
        if (state.speed > 0.01f) {
            state.direction = std::atan2(state.velocity.y, state.velocity.x) * 180.0f / glm::pi<float>();
        }

        // 更新位置：p = p₀ + vt
        state.position += state.velocity * static_cast<float>(deltaTime);

        state.elapsedTime += deltaTime;
    }

    std::unique_ptr<IMovementUpdater> clone() const override {
        return std::make_unique<LinearMovementUpdater>();
    }
};

// ========== 目标点运动更新器（带缓动）==========
class TargetMovementUpdater : public IMovementUpdater {
private:
    std::function<float(float)> mEaseFunc; // 缓动函数 t ∈ [0,1] -> [0,1]
    glm::vec2 mStartPos{0,0};
    float mTotalDistance = 0;
    float mCalculatedDuration = 0;  // 记录计算出的持续时间
public:
    explicit TargetMovementUpdater(std::function<float(float)> easeFunc = nullptr)
        : mEaseFunc(easeFunc ? easeFunc : [](float t) { return t; })
        , mCalculatedDuration(0.0f) {
    }

    void initialize(MovementState& state, const glm::vec2& currentPos) override {
        state.position = currentPos;
        mStartPos = currentPos;

        // 动态目标：在初始化时求值
        if (state.useDynamicTarget && state.targetGetter) {
            state.targetPosition = state.targetGetter();
        }

        mTotalDistance = glm::distance(currentPos, state.targetPosition);
        mCalculatedDuration = state.speed > 0 ? mTotalDistance / state.speed : 1.0f;

        state.elapsedTime = 0;
        state.initialized = true;
    }
    
    void update(MovementState& state, double deltaTime) override {
        state.elapsedTime += deltaTime;

        // 使用初始化时计算的持续时间
        float t = std::min(1.0f, static_cast<float>(state.elapsedTime / mCalculatedDuration));
        float easedT = mEaseFunc(t);

        // 插值计算位置
        state.position = mStartPos + (state.targetPosition - mStartPos) * easedT;

        // 更新速度
        if (t < 1.0f && deltaTime > 0) {
            glm::vec2 direction = glm::normalize(state.targetPosition - mStartPos);
            state.velocity = direction * state.speed;
        }
        else {
            state.velocity = { 0, 0 };
        }

        state.speed = glm::length(state.velocity);
        if (state.speed > 0.01f) {
            state.direction = std::atan2(state.velocity.y, state.velocity.x) * 180.0f / glm::pi<float>();
        }
    }

    std::unique_ptr<IMovementUpdater> clone() const override {
        return std::make_unique<TargetMovementUpdater>(mEaseFunc);
    }
};

// ========== 圆周运动更新器 ==========
class CircularMovementUpdater : public IMovementUpdater {
public:
    void initialize(MovementState& state, const glm::vec2& currentPos) override {
        state.position = currentPos;

        if (state.useTangentMode) {
            // 模式3：使用切线速度和方向

            if (state.radius < 0.01f) {
                state.radius = 100.0f;  // 默认半径
            }

            // 1. 计算角速度：ω = v / r
            state.angleVelocity = state.tangentVelocity / state.radius;

            // 如果是逆时针，角速度为正；顺时针为负
            if (state.clockwise) {
                state.angleVelocity = -state.angleVelocity;
            }

            // 2. 计算法线方向（垂直于切线）
            // 顺时针：法线在切线右侧（切线方向 - 90°）
            // 逆时针：法线在切线左侧（切线方向 + 90°）
            float normalDirection = state.tangentDirection + (state.clockwise ? -90.0f : 90.0f);
            float normalRadians = glm::radians(normalDirection);

            // 3. 计算圆心位置：圆心 = 当前位置 + 半径 * 法线方向
            glm::vec2 normalVec = glm::vec2(std::cos(normalRadians), std::sin(normalRadians));
            state.center = currentPos + normalVec * state.radius;

            // 4. 计算初始角度（从圆心指向当前位置）
            glm::vec2 offset = currentPos - state.center;
            state.angle = std::atan2(offset.y, offset.x);

            // 验证：重新计算位置应该等于当前位置
            glm::vec2 verifyPos = state.center + glm::vec2(
                state.radius * std::cos(state.angle),
                state.radius * std::sin(state.angle)
            );

            state.position = currentPos;  // 保持当前位置

        }
        else if (state.useCenterMode) {
            // 模式2：以指定位置为中心
            if (state.autoCalculateRadius) {
                glm::vec2 offset = currentPos - state.center;
                state.radius = glm::length(offset);

                if (state.radius > 0.01f) {
                    state.angle = std::atan2(offset.y, offset.x);
                }
                else {
                    state.angle = 0;
                    state.radius = 1.0f;
                }
            }

            state.position = state.center + glm::vec2(
                state.radius * std::cos(state.angle),
                state.radius * std::sin(state.angle)
            );
        }
        else {
            // 模式1：以当前位置为中心
            state.center = currentPos;
            state.position = state.center + glm::vec2(state.radius, 0);
        }
        state.initialized = true;
    }
    void update(MovementState& state, double deltaTime) override {
        // 更新角度
        state.angle += state.angleVelocity * deltaTime;

        // 更新半径
        state.radius += state.radiusVelocity * deltaTime;

        // 计算新位置
        state.position = state.center + glm::vec2(
            state.radius * std::cos(state.angle),
            state.radius * std::sin(state.angle)
        );

        // 更新速度和方向
        state.speed = state.radius * std::abs(state.angleVelocity);

        state.elapsedTime += deltaTime;
    }

    std::unique_ptr<IMovementUpdater> clone() const override {
        return std::make_unique<CircularMovementUpdater>();
    }

};

// ========== 椭圆运动更新器 ==========
class EllipticalMovementUpdater : public IMovementUpdater {
public:
    void initialize(MovementState& state, const glm::vec2& currentPos) override {
        state.position = currentPos;

        if (state.useCenterMode) {
            if (state.autoCalculateRadius) {
                glm::vec2 offset = currentPos - state.center;

                // 需要考虑椭圆的旋转，反向变换得到局部坐标
                float cosDir = std::cos(state.ellipseDirection);
                float sinDir = std::sin(state.ellipseDirection);

                // 反向旋转到椭圆局部坐标系
                float localX = offset.x * cosDir + offset.y * sinDir;
                float localY = -offset.x * sinDir + offset.y * cosDir;

                // 计算基础半径（取短轴）
                state.radius = std::sqrt(localX * localX + localY * localY / (state.axisRatio * state.axisRatio));

                if (state.radius > 0.01f) {
                    // 计算角度
                    state.angle = std::atan2(localY / state.axisRatio, localX);
                }
                else {
                    state.angle = 0;
                    state.radius = 1.0f;
                }
            }

            // 立即同步 position
            float a = state.radius * state.axisRatio;
            float b = state.radius;

            float localX = a * std::cos(state.angle);
            float localY = b * std::sin(state.angle);

            float cosDir = std::cos(state.ellipseDirection);
            float sinDir = std::sin(state.ellipseDirection);
            float rotatedX = localX * cosDir - localY * sinDir;
            float rotatedY = localX * sinDir + localY * cosDir;

            state.position = state.center + glm::vec2(rotatedX, rotatedY);
        }
        else {
            state.center = currentPos;
            state.position = currentPos + glm::vec2(state.radius * state.axisRatio, 0);
        }

        state.initialized = true;
    }

    void update(MovementState& state, double deltaTime) override {
        // 更新角度
        state.angle += state.angleVelocity * deltaTime;

        // 更新基础半径
        state.radius += state.radiusVelocity * deltaTime;

        // 计算椭圆的长短轴
        float a = state.radius * state.axisRatio;  // 长半轴
        float b = state.radius;                     // 短半轴

        // 局部坐标系中的位置
        float localX = a * std::cos(state.angle);
        float localY = b * std::sin(state.angle);

        // 应用椭圆旋转
        float cosDir = std::cos(state.ellipseDirection);
        float sinDir = std::sin(state.ellipseDirection);
        float rotatedX = localX * cosDir - localY * sinDir;
        float rotatedY = localX * sinDir + localY * cosDir;

        // 世界坐标
        state.position = state.center + glm::vec2(rotatedX, rotatedY);

        // 计算速度（简化版本）
        float avgRadius = (a + b) / 2.0f;
        state.speed = avgRadius * std::abs(state.angleVelocity);

        state.elapsedTime += deltaTime;
    }

    std::unique_ptr<IMovementUpdater> clone() const override {
        return std::make_unique<EllipticalMovementUpdater>();
    }
};

// ========== 波动运动更新器 ==========
class WaveMovementUpdater : public IMovementUpdater {
private:
    float mAmplitude;      // 波动幅度
    float mFrequency;      // 波动频率
    glm::vec2 mBaseDir{0,0};    // 基础移动方向（单位向量）
    glm::vec2 mWaveDir{0,0};    // 波动方向（垂直于基础方向）
    float mBaseSpeed;      // 基础速度
    float mPhase;          // 相位偏移

public:
    WaveMovementUpdater(float amplitude = 50.0f, float frequency = 2.0f, float phase = 0.0f)
        : mAmplitude(amplitude), mFrequency(frequency), mPhase(phase), mBaseSpeed(0) {
    }

    void initialize(MovementState& state, const glm::vec2& currentPos) override {
        state.position = currentPos;

        // 计算基础方向（使用state.direction）
        float radians = glm::radians(state.direction);
        mBaseDir = glm::vec2(std::cos(radians), std::sin(radians));

        // 计算波动方向（垂直于基础方向，逆时针旋转90度）
        mWaveDir = glm::vec2(-mBaseDir.y, mBaseDir.x);

        mBaseSpeed = state.speed;
        state.velocity = mBaseDir * mBaseSpeed;
        state.elapsedTime = 0;
        state.initialized = true;
    }

    void update(MovementState& state, double deltaTime) override {
        // 更新时间
        state.elapsedTime += deltaTime;

        // 计算波动偏移量：offset = amplitude * sin(2π * frequency * time + phase)
        float waveOffset = mAmplitude * std::sin(2.0f * glm::pi<float>() * mFrequency * state.elapsedTime + mPhase);

        // 计算瞬时速度的波动分量（对正弦函数求导）
        float waveVelocity = mAmplitude * 2.0f * glm::pi<float>() * mFrequency *
            std::cos(2.0f * glm::pi<float>() * mFrequency * state.elapsedTime + mPhase);

        // 基础位置：沿基础方向移动
        glm::vec2 baseMovement = mBaseDir * mBaseSpeed * static_cast<float>(deltaTime);

        // 波动位移：垂直方向的瞬时速度
        glm::vec2 waveMovement = mWaveDir * waveVelocity * static_cast<float>(deltaTime);

        // 更新位置
        state.position += baseMovement + waveMovement;

        // 更新速度向量（用于显示或其他用途）
        state.velocity = mBaseDir * mBaseSpeed + mWaveDir * waveVelocity;
        state.speed = glm::length(state.velocity);

        // 更新方向
        if (state.speed > 0.01f) {
            state.direction = std::atan2(state.velocity.y, state.velocity.x) * 180.0f / glm::pi<float>();
        }
    }

    std::unique_ptr<IMovementUpdater> clone() const override {
        return std::make_unique<WaveMovementUpdater>(mAmplitude, mFrequency, mPhase);
    }
};

// ========== 贝塞尔曲线运动更新器 ==========
class BezierMovementUpdater : public IMovementUpdater {
private:
    std::vector<glm::vec2> mControlPoints;  // 控制点
    float mDuration;                         // 持续时间
    std::function<float(float)> mEaseFunc;  // 缓动函数

public:
    BezierMovementUpdater(const std::vector<glm::vec2>& controlPoints,
        float duration,
        std::function<float(float)> easeFunc = nullptr)
        : mControlPoints(controlPoints)
        , mDuration(duration)
        , mEaseFunc(easeFunc ? easeFunc : [](float t) { return t; }) {
    }

    void initialize(MovementState& state, const glm::vec2& currentPos) override {
        state.position = currentPos;
        // 将当前位置作为起点
        if (mControlPoints.empty() || glm::distance(mControlPoints[0], currentPos) > 1.0f) {
            mControlPoints.insert(mControlPoints.begin(), currentPos);
        }
        state.elapsedTime = 0;
        state.initialized = true;
    }

    void update(MovementState& state, double deltaTime) override {
        state.elapsedTime += deltaTime;

        // 计算进度 t ∈ [0, 1]
        float t = std::min(1.0f, static_cast<float>(state.elapsedTime / mDuration));
        float easedT = mEaseFunc(t);

        // 根据控制点数量选择贝塞尔曲线类型
        if (mControlPoints.size() == 3) {
            // 二次贝塞尔曲线: B(t) = (1-t)²P0 + 2(1-t)tP1 + t²P2
            state.position = quadraticBezier(mControlPoints[0], mControlPoints[1], mControlPoints[2], easedT);

            // 计算切线方向（一阶导数）
            glm::vec2 tangent = quadraticBezierDerivative(mControlPoints[0], mControlPoints[1], mControlPoints[2], easedT);
            if (glm::length(tangent) > 0.01f) {
                state.velocity = tangent;
                state.speed = glm::length(tangent);
                state.direction = std::atan2(tangent.y, tangent.x) * 180.0f / glm::pi<float>();
            }
        }
        else if (mControlPoints.size() == 4) {
            // 三次贝塞尔曲线: B(t) = (1-t)³P0 + 3(1-t)²tP1 + 3(1-t)t²P2 + t³P3
            state.position = cubicBezier(mControlPoints[0], mControlPoints[1], mControlPoints[2], mControlPoints[3], easedT);

            // 计算切线方向
            glm::vec2 tangent = cubicBezierDerivative(mControlPoints[0], mControlPoints[1], mControlPoints[2], mControlPoints[3], easedT);
            if (glm::length(tangent) > 0.01f) {
                state.velocity = tangent;
                state.speed = glm::length(tangent);
                state.direction = std::atan2(tangent.y, tangent.x) * 180.0f / glm::pi<float>();
            }
        }
    }

    std::unique_ptr<IMovementUpdater> clone() const override {
        return std::make_unique<BezierMovementUpdater>(mControlPoints, mDuration, mEaseFunc);
    }

private:
    // 二次贝塞尔曲线计算
    glm::vec2 quadraticBezier(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2, float t) const {
        float u = 1.0f - t;
        return u * u * p0 + 2.0f * u * t * p1 + t * t * p2;
    }

    // 二次贝塞尔曲线导数（切线方向）
    glm::vec2 quadraticBezierDerivative(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2, float t) const {
        return 2.0f * (1.0f - t) * (p1 - p0) + 2.0f * t * (p2 - p1);
    }

    // 三次贝塞尔曲线计算
    glm::vec2 cubicBezier(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, float t) const {
        float u = 1.0f - t;
        return u * u * u * p0 + 3.0f * u * u * t * p1 + 3.0f * u * t * t * p2 + t * t * t * p3;
    }

    // 三次贝塞尔曲线导数
    glm::vec2 cubicBezierDerivative(const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, float t) const {
        float u = 1.0f - t;
        return 3.0f * u * u * (p1 - p0) + 6.0f * u * t * (p2 - p1) + 3.0f * t * t * (p3 - p2);
    }
};

// ========== 通用贝塞尔曲线运动更新器（支持任意阶数）==========
class GeneralBezierMovementUpdater : public IMovementUpdater {
private:
    std::vector<glm::vec2> mStaticPoints;// 静态控制点
    std::vector<std::function<glm::vec2()>> mDynamicPoints;// 动态控制点获取器
    std::vector<glm::vec2> mResolvedPoints;// 求值后的实际控制点
    float mDuration;
    std::function<float(float)> mEaseFunc;

public:
    GeneralBezierMovementUpdater(
        const std::vector<glm::vec2>& staticPoints,
        const std::vector<std::function<glm::vec2()>>& dynamicPoints,
        float duration,
        std::function<float(float)> easeFunc = nullptr)
        : mStaticPoints(staticPoints)
        , mDynamicPoints(dynamicPoints)
        , mDuration(duration)
        , mEaseFunc(easeFunc ? easeFunc : [](float t) { return t; }) {
    }

    void initialize(MovementState& state, const glm::vec2& currentPos) override {
        state.position = currentPos;

        // 求值所有控制点（动态 + 静态）
        mResolvedPoints.clear();
        mResolvedPoints.reserve(mStaticPoints.size());

        for (size_t i = 0; i < mStaticPoints.size(); i++) {
            if (mDynamicPoints[i]) {
                // 动态点：调用 getter
                mResolvedPoints.push_back(mDynamicPoints[i]());
            }
            else {
                // 静态点：直接使用
                mResolvedPoints.push_back(mStaticPoints[i]);
            }
        }

        // 添加起点
        if (mResolvedPoints.empty() || glm::distance(mResolvedPoints[0], currentPos) > 1.0f) {
            mResolvedPoints.insert(mResolvedPoints.begin(), currentPos);
        }

        state.elapsedTime = 0;
        state.initialized = true;
    }

    void update(MovementState& state, double deltaTime) override {
        state.elapsedTime += deltaTime;
        float t = std::min(1.0f, static_cast<float>(state.elapsedTime / mDuration));
        float easedT = mEaseFunc(t);

        // 使用求值后的控制点
        state.position = deCasteljau(mResolvedPoints, easedT);

        // 计算切线
        if (t < 1.0f) {
            float dt = 0.001f;
            glm::vec2 p1 = deCasteljau(mResolvedPoints, std::max(0.0f, easedT - dt));
            glm::vec2 p2 = deCasteljau(mResolvedPoints, std::min(1.0f, easedT + dt));
            glm::vec2 tangent = p2 - p1;

            if (glm::length(tangent) > 0.01f) {
                state.velocity = tangent;
                state.speed = glm::length(tangent);
                state.direction = std::atan2(tangent.y, tangent.x) * 180.0f / glm::pi<float>();
            }
        }
    }

    std::unique_ptr<IMovementUpdater> clone() const override {
        return std::make_unique<GeneralBezierMovementUpdater>(mStaticPoints, mDynamicPoints, mDuration, mEaseFunc);
    }

private:
    glm::vec2 deCasteljau(const std::vector<glm::vec2>& points, float t) const {
        if (points.size() == 1) {
            return points[0];
        }

        std::vector<glm::vec2> reducedPoints;
        reducedPoints.reserve(points.size() - 1);

        for (size_t i = 0; i < points.size() - 1; i++) {
            glm::vec2 interpolated = (1.0f - t) * points[i] + t * points[i + 1];
            reducedPoints.push_back(interpolated);
        }

        return deCasteljau(reducedPoints, t);
    }
};

// ========== 复合通用贝塞尔曲线运动更新器 ==========
class CompositeGeneralBezierMovementUpdater : public IMovementUpdater {
public:
    struct BezierSegment {
        std::vector<glm::vec2> controlPoints;
        float duration = 0;
        std::function<float(float)> easeFunc;
    };

private:
    std::vector<BezierSegment> mSegments;
    float mTotalDuration;
    int mCurrentSegment;
    bool mControlPointsInitialized;

public:
    CompositeGeneralBezierMovementUpdater(const std::vector<BezierSegment>& segments)
        : mSegments(segments), mTotalDuration(0.0f), mCurrentSegment(0), mControlPointsInitialized(false) {
        for (const auto& seg : mSegments) {
            mTotalDuration += seg.duration;
        }
    }

    void initialize(MovementState& state, const glm::vec2& currentPos) override {
        state.position = currentPos;

        if (!mControlPointsInitialized && !mSegments.empty()) {
            // 第一段：使用当前位置作为起点
            if (!mSegments[0].controlPoints.empty()) {
                mSegments[0].controlPoints.insert(mSegments[0].controlPoints.begin(), currentPos);
            }

            // 后续段：使用上一段的终点作为起点
            for (size_t i = 1; i < mSegments.size(); i++) {
                if (!mSegments[i].controlPoints.empty() && !mSegments[i - 1].controlPoints.empty()) {
                    glm::vec2 prevEndPoint = mSegments[i - 1].controlPoints.back();
                    mSegments[i].controlPoints.insert(mSegments[i].controlPoints.begin(), prevEndPoint);
                }
            }

            mControlPointsInitialized = true;
        }

        state.elapsedTime = 0;
        mCurrentSegment = 0;
        state.initialized = true;
    }

    void update(MovementState& state, double deltaTime) override {
        state.elapsedTime += deltaTime;

        // 找到当前段
        float accumulatedTime = 0.0f;
        int targetSegment = 0;

        for (size_t i = 0; i < mSegments.size(); i++) {
            if (state.elapsedTime < accumulatedTime + mSegments[i].duration) {
                targetSegment = i;
                break;
            }
            accumulatedTime += mSegments[i].duration;
        }

        if (state.elapsedTime >= mTotalDuration) {
            targetSegment = mSegments.size() - 1;
            accumulatedTime = mTotalDuration - mSegments[targetSegment].duration;
        }

        mCurrentSegment = targetSegment;
        const auto& segment = mSegments[mCurrentSegment];

        // 计算进度
        float segmentTime = state.elapsedTime - accumulatedTime;
        float t = std::min(1.0f, segmentTime / segment.duration);
        float easedT = segment.easeFunc ? segment.easeFunc(t) : t;

        // 使用德卡斯特里奥算法计算位置
        state.position = deCasteljau(segment.controlPoints, easedT);

        // 计算切线
        if (t < 1.0f) {
            float dt = 0.001f;
            glm::vec2 p1 = deCasteljau(segment.controlPoints, std::max(0.0f, easedT - dt));
            glm::vec2 p2 = deCasteljau(segment.controlPoints, std::min(1.0f, easedT + dt));
            glm::vec2 tangent = p2 - p1;

            if (glm::length(tangent) > 0.01f) {
                state.velocity = tangent;
                state.speed = glm::length(tangent);
                state.direction = std::atan2(tangent.y, tangent.x) * 180.0f / glm::pi<float>();
            }
        }
    }

    std::unique_ptr<IMovementUpdater> clone() const override {
        auto cloned = std::make_unique<CompositeGeneralBezierMovementUpdater>(mSegments);
        cloned->mControlPointsInitialized = mControlPointsInitialized;
        return cloned;
    }

private:
    glm::vec2 deCasteljau(const std::vector<glm::vec2>& points, float t) const {
        if (points.size() == 1) {
            return points[0];
        }

        std::vector<glm::vec2> reducedPoints;
        reducedPoints.reserve(points.size() - 1);

        for (size_t i = 0; i < points.size() - 1; i++) {
            glm::vec2 interpolated = (1.0f - t) * points[i] + t * points[i + 1];
            reducedPoints.push_back(interpolated);
        }

        return deCasteljau(reducedPoints, t);
    }
};

// ========== 缓动函数库 ==========
namespace EaseFunctions {
    inline float linear(float t) { return t; }
    inline float easeIn(float t) { return t * t; }
    inline float easeOut(float t) { return 1.0f - (1.0f - t) * (1.0f - t); }
    inline float easeInOut(float t) { return t * t * (3.0f - 2.0f * t); }
    inline float easeInCubic(float t) { return t * t * t; }
    inline float easeOutCubic(float t) { return 1.0f - std::pow(1.0f - t, 3.0f); }
}