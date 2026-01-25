#pragma once
#include "Sprite.hpp"
#include <vector>

namespace esl
{
    class BlurEffect : public Sprite
    {
    public:
        BlurEffect(const glm::vec2& size = { 800, 600 });

        ~BlurEffect();

        // 重新调整缓冲区大小（例如窗口大小改变时）
        void resize(const glm::vec2& size);

        // 设置模糊迭代次数 (通常 4-10 次，偶数)
        // 次数越多越模糊，但也越慢
        void setIterations(int count) { m_iterations = count; }

        // 设置采样扩散范围 (默认 1.0)
        // 值越大，模糊范围越广，但过大可能会出现方块感
        void setSpread(float spread) { m_Spread = spread; }

        // 1. 抓取屏幕（或准备开始模糊流程）
        // 这一步是将当前屏幕内容复制到我们的第一个 FBO 中作为源
        void captureScreen(esl::Window& window, const glm::vec2& regionPos, const glm::vec2& regionSize);

        // 2. 执行模糊处理（Ping-Pong）
        void process();

        // 3. 绘制最终结果
        virtual void draw(float right, float top) override;

    private:
        glm::vec2 m_effectSize;
        int m_iterations = 10;
        float m_Spread = 1.0f;

        // Ping-Pong FBOs
        GLuint m_pingPongFBO[2] = { 0, 0 };
        GLuint m_pingPongColorbuffers[2] = { 0, 0 };
        GLuint m_processVAO = 0;
        GLuint m_processVBO = 0;
        void initProcessQuad();
        void initFrameBuffers();
        void initShader();
    };
}