#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "ProgressSprite.hpp"

namespace esl
{
    ProgressSprite::ProgressSprite() : Sprite()
    {
        setupProgressShader();
    }

    ProgressSprite::ProgressSprite(Texture* texture) : Sprite(texture)
    {
        setupProgressShader();
    }

    ProgressSprite::~ProgressSprite()
    {
        if (m_ProgressShader)
        {
            delete m_ProgressShader;
            m_ProgressShader = nullptr;
        }
    }

    void ProgressSprite::setType(Type type)
    {
        m_Type = type;
    }

    ProgressSprite::Type ProgressSprite::getType() const
    {
        return m_Type;
    }

    void ProgressSprite::setPercentage(float percentage)
    {
        m_Percentage = glm::clamp(percentage, 0.0f, 1.0f);
    }

    float ProgressSprite::getPercentage() const
    {
        return m_Percentage;
    }

    void ProgressSprite::setReverseDirection(bool reverse)
    {
        m_ReverseDirection = reverse;
    }

    bool ProgressSprite::getReverseDirection() const
    {
        return m_ReverseDirection;
    }

    void ProgressSprite::setupProgressShader()
    {
        const std::string vertexShader = R"(
            #version 460 core
            layout(location = 0) in vec3 aPos;
            layout(location = 2) in vec2 aUV;
            out vec2 uv;
            uniform mat4 transform;
            uniform mat4 projection;
            uniform mat4 view;
            uniform vec2 uvScale;
            void main() {
                gl_Position = projection * view * transform * vec4(aPos, 1.0);
                uv = aUV * uvScale;
            }
        )";

        const std::string fragmentShader = R"(
            #version 460 core
            in vec2 uv;
            out vec4 fragColor;
            uniform sampler2D sampler;
            uniform vec4 spriteColor;
            uniform float percentage;
            uniform int progressType;
            uniform bool reverseDirection;

            void main() {
                vec4 texColor = texture(sampler, uv) * spriteColor;
                
                if (progressType == 0) { // RADIAL
                    // 将UV坐标转换为圆形坐标
                    vec2 center = vec2(0.5, 0.5);
                    vec2 coord = uv - center;
                    float angle = atan(coord.y, coord.x);
                    
                    // 将角度从 [-PI, PI] 映射到 [0, 1]
                    float normalizedAngle = (angle + 3.14159265359) / (2.0 * 3.14159265359);
                    
                    if (reverseDirection) {
                        normalizedAngle = 1.0 - normalizedAngle;
                    }
                    
                    // 检查当前角度是否在进度范围内
                    if (normalizedAngle > percentage) {
                        discard;
                    }
                    
                    // 检查是否在圆形区域内
                    float radius = length(coord);
                    if (radius > 0.5) {
                        discard;
                    }
                    
                    fragColor = texColor;
                }
                else if (progressType == 1) { // HORIZONTAL
                    float progress = reverseDirection ? (1.0 - uv.x) : uv.x;
                    if (progress > percentage) {
                        discard;
                    }
                    fragColor = texColor;
                }
                else if (progressType == 2) { // VERTICAL
                    float progress = reverseDirection ? uv.y : (1.0 - uv.y);
                    if (progress > percentage) {
                        discard;
                    }
                    fragColor = texColor;
                }
                else {
                    fragColor = texColor;
                }
            }
        )";

        m_ProgressShader = new Shader(vertexShader, fragmentShader);
        m_ProgressShader->setInt("sampler", 0);
    }

    void ProgressSprite::draw(float right, float top)
    {
        // 先绘制边框（如果有）
        drawBorder(right, top);

        m_ProgressShader->load();
        bindTexture();

        glm::mat4 projection;
        glm::mat4 view(1.f);
        projection = glm::ortho(0.0f, right, 0.0f, top, -1.0f, 1.0f);

        m_ProgressShader->setMat4("projection", projection);
        m_ProgressShader->setMat4("view", view);

        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, m_Position);
        transform = glm::translate(transform, glm::vec3(m_Origin, 0.0f));
        transform = glm::rotate(transform, glm::radians(m_Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
        transform = glm::translate(transform, glm::vec3(-m_Origin, 0.0f));

        glm::vec2 scaledSize = m_Size * m_Scale * m_RectScale * m_RepeatScale;
        transform = glm::scale(transform, glm::vec3(scaledSize.x, scaledSize.y, 1.0f));

        m_ProgressShader->setMat4("transform", transform);
        m_ProgressShader->setVec4("spriteColor", m_Color);
        m_ProgressShader->setVec2("uvScale", m_RepeatScale);

        // 设置进度条相关参数
        m_ProgressShader->setFloat("percentage", m_Percentage);
        m_ProgressShader->setInt("progressType", static_cast<int>(m_Type));
        m_ProgressShader->setBool("reverseDirection", m_ReverseDirection);

        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        m_ProgressShader->unload();
    }
}