#pragma once
#include <memory>
#include <functional>
#include "Action.h"
#include "MovementBuilder.h"
class Enemy;

// Action 工厂类，负责创建所有类型的 Action
class ActionFactory {
public:
    using pAction = std::unique_ptr<Action>;

    // ========== 移动类 Action ==========

    // 直线运动 Builder
    static class LinearMovement createLinearMovement();

    // 圆周运动 Builder
    static class CircularMovement createCircularMovement();

    // 椭圆运动 Builder
    static class EllipticalMovement createEllipticalMovement();

    // 等待
    static pAction createAwait(double awaitTime);

    
};