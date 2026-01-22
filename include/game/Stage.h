#pragma once
#include "Enemy.h"
#include <memory>
#include <vector>
#include <functional>
#include <queue>
class MainGame; // 前置声明

// ========== 任务基类 ==========
class Task {
protected:
    bool mStarted = false;
    bool mFinished = false;

public:
    virtual ~Task() = default;

    // 任务开始时调用（只执行一次）
    virtual void start(MainGame* game) { mStarted = true; }

    // 每帧更新
    virtual void update(double deltaTime, MainGame* game) = 0;

    // 检查是否完成
    virtual bool isFinished() const { return mFinished; }

    // 获取任务描述（调试用）
    virtual std::string getDescription() const = 0;
};

// ========== 等待任务 ==========
class AwaitTask : public Task {
private:
    double mDuration;
    double mElapsed = 0.0;

public:
    explicit AwaitTask(double duration) : mDuration(duration) {}

    void update(double deltaTime, MainGame* game) override {
        mElapsed += deltaTime;
        if (mElapsed >= mDuration) {
            mFinished = true;
        }
    }

    std::string getDescription() const override {
        return "Await " + std::to_string(mDuration) + "s";
    }
};

// ========== 生成敌人任务 ==========
class SpawnEnemyTask : public Task {
private:
    std::function<Enemy* ()> mSpawnFunc;
    Enemy* mSpawnedEnemy = nullptr;

public:
    explicit SpawnEnemyTask(std::function<Enemy* ()> spawnFunc)
        : mSpawnFunc(std::move(spawnFunc)) {
    }

    void start(MainGame* game) override;

    void update(double deltaTime, MainGame* game) override {
        mFinished = true; // 立即完成
    }

    Enemy* getSpawnedEnemy() const { return mSpawnedEnemy; }

    std::string getDescription() const override {
        return "Spawn Enemy";
    }
};

// ========== 自定义 Lambda 任务 ==========
class LambdaTask : public Task {
private:
    std::function<void(MainGame*)> mAction;

public:
    explicit LambdaTask(std::function<void(MainGame*)> action)
        : mAction(std::move(action)) {
    }

    void start(MainGame* game) override {
        Task::start(game);
        mAction(game);
        mFinished = true; // 立即完成
    }

    void update(double deltaTime, MainGame* game) override {}

    std::string getDescription() const override {
        return "Lambda Task";
    }
};

// ========== 条件触发任务 ==========
class ConditionalTask : public Task {
private:
    std::function<bool(MainGame*)> mCondition;
    std::unique_ptr<Task> mTask;

public:
    ConditionalTask(std::function<bool(MainGame*)> condition, std::unique_ptr<Task> task)
        : mCondition(std::move(condition)), mTask(std::move(task)) {
    }

    void start(MainGame* game) override {
        Task::start(game);
        if (mCondition(game) && mTask) {
            mTask->start(game);
        }
    }

    void update(double deltaTime, MainGame* game) override {
        if (mTask && !mTask->isFinished()) {
            mTask->update(deltaTime, game);
        }
        else {
            mFinished = true;
        }
    }

    std::string getDescription() const override {
        return "Conditional Task";
    }
};

// ========== 条件等待任务 ==========
class ConditionalAwaitTask : public Task {
private:
    std::function<bool()> mCondition;

public:
    explicit ConditionalAwaitTask(std::function<bool()> condition)
        : mCondition(std::move(condition)) {
    }

    void update(double deltaTime, MainGame* game) override {
        // 如果条件满足，标记为完成
        if (mCondition()) {
            mFinished = true;
        }
    }

    std::string getDescription() const override {
        return "Conditional Await Task";
    }
};

// ========== 关卡类 ==========
class Stage {
private:
    std::vector<std::unique_ptr<Task>> mTasks;
    size_t mCurrentTaskIndex = 0;
    double mElapsedTime = 0.0;
    bool mActive = false;
public:
    void addWaitUntil(std::function<bool()> condition) {
        addTask(std::make_unique<ConditionalAwaitTask>(std::move(condition)));
    }
    // 添加任务（移动语义）
    void addTask(std::unique_ptr<Task> task) {
        mTasks.push_back(std::move(task));
    }

    // 便捷方法：添加等待
    void addWait(double duration) {
        addTask(std::make_unique<AwaitTask>(duration));
    }

    // 便捷方法：添加 Lambda
    void addAction(std::function<void(MainGame*)> action) {
        addTask(std::make_unique<LambdaTask>(std::move(action)));
    }

    // 开始执行
    void start(MainGame* game) {
        mActive = true;
        if (!mTasks.empty() && mCurrentTaskIndex < mTasks.size()) {
            mTasks[mCurrentTaskIndex]->start(game);
        }
    }

    // 每帧更新
    void update(double deltaTime, MainGame* game) {
        if (!mActive || mTasks.empty()) return;
        mElapsedTime += deltaTime;

        // 更新当前任务
        if (mCurrentTaskIndex < mTasks.size()) {
            auto& currentTask = mTasks[mCurrentTaskIndex];

            currentTask->update(deltaTime, game);

            // 检查是否完成
            if (currentTask->isFinished()) {
                mCurrentTaskIndex++;

                // 启动下一个任务
                if (mCurrentTaskIndex < mTasks.size()) {
                    mTasks[mCurrentTaskIndex]->start(game);
                }
                else {
                    mActive = false; // 所有任务完成
                }
            }
        }
    }

    // 调试功能
    void skipToTask(size_t index) {
        if (index < mTasks.size()) {
            mCurrentTaskIndex = index;
        }
    }

    bool isFinished() const {
        return mCurrentTaskIndex >= mTasks.size();
    }

    size_t getCurrentTaskIndex() const {
        return mCurrentTaskIndex;
    }

    double getElapsedTime() const {
        return mElapsedTime;
    }
};