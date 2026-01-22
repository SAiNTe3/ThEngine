#pragma once
#include "Sprite.hpp"

namespace esl
{
    class ProgressSprite : public Sprite
    {
    public:
        enum class Type
        {
            RADIAL,     // 径向进度条（圆形）
            HORIZONTAL, // 水平进度条
            VERTICAL    // 垂直进度条
        };

    private:
        Type m_Type = Type::RADIAL;
        float m_Percentage = 0.0f; // 0.0 - 1.0
        bool m_ReverseDirection = false; // 是否反向
        Shader* m_ProgressShader = nullptr;

        // 新增：静态共享资源
        static Shader* s_SharedProgressShader;
        static int s_InstanceCount;
        static bool s_ProgressShaderInitialized;

        // 新增：初始化和清理共享 Shader
        static void initializeProgressShader();
        static void cleanupProgressShader();
    public:
        ProgressSprite();
        ProgressSprite(Texture* texture);
        ~ProgressSprite();

        void setType(Type type);
        Type getType() const;

        void setPercentage(float percentage);
        float getPercentage() const;

        void setReverseDirection(bool reverse);
        bool getReverseDirection() const;

    protected:
        virtual void draw(float right, float top) override;
        void setupProgressShader();
    };
}