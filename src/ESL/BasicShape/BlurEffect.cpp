#include "BlurEffect.hpp"
#include <iostream>
#include <glad/glad.h>
namespace esl
{
    // 定义全屏 Quad 数据 (process 阶段专用)
    // 覆盖 NDC (-1, -1) 到 (1, 1)，UV (0, 0) 到 (1, 1)
    static const float g_quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    BlurEffect::BlurEffect(const glm::vec2& size) : m_effectSize(size)
    {
        initFrameBuffers();
        initShader();
        initProcessQuad(); // 新增：初始化处理用的 Quad
        // 默认全白，保证混合时不染色
        m_Color = glm::vec4(1.0f);
    }

    BlurEffect::~BlurEffect()
    {
        glDeleteFramebuffers(2, m_pingPongFBO);
        glDeleteTextures(2, m_pingPongColorbuffers);

        if (m_processVAO) glDeleteVertexArrays(1, &m_processVAO);
        if (m_processVBO) glDeleteBuffers(1, &m_processVBO);
    }

    void BlurEffect::resize(const glm::vec2& size)
    {
        m_effectSize = size;
        glDeleteFramebuffers(2, m_pingPongFBO);
        glDeleteTextures(2, m_pingPongColorbuffers);
        initFrameBuffers();
    }

    void BlurEffect::initFrameBuffers()
    {
        glGenFramebuffers(2, m_pingPongFBO);
        glGenTextures(2, m_pingPongColorbuffers);

        for (unsigned int i = 0; i < 2; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, m_pingPongFBO[i]);
            glBindTexture(GL_TEXTURE_2D, m_pingPongColorbuffers[i]);

            // 分配内存
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (int)m_effectSize.x, (int)m_effectSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pingPongColorbuffers[i], 0);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "Framebuffer not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // 新增：初始化全屏 Quad
    void BlurEffect::initProcessQuad()
    {
        glGenVertexArrays(1, &m_processVAO);
        glGenBuffers(1, &m_processVBO);
        glBindVertexArray(m_processVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_processVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_quadVertices), &g_quadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindVertexArray(0);
    }

    void BlurEffect::initShader()
    {
        // 简化 Vertex Shader
        const std::string vertexCode = R"(
            #version 460 core
            layout (location = 0) in vec3 aPos;
            layout (location = 2) in vec2 aTexCoord;
            out vec2 TexCoord;

            uniform mat4 model;
            uniform mat4 projection;
            uniform bool isProcess; 

            void main()
            {
                if(isProcess) {
                    
                    gl_Position = vec4(aPos.xy, 0.0, 1.0); 
                } else {
                    
                    gl_Position = projection * model * vec4(aPos, 1.0);
                }
                TexCoord = aTexCoord;
            }
        )";

        // Fragment Shader 保持不变
        const std::string fragmentCode = R"(
            #version 460 core
            out vec4 FragColor;
            in vec2 TexCoord;

            uniform sampler2D image;
            uniform bool horizontal;
            uniform bool isProcess; 
            uniform vec4 spriteColor; // [新增] 接收 Sprite 的颜色/透明度
            uniform float spread;     // [新增] 扩散系数
            
            uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

            void main()
            {             
                if (!isProcess) {
                    // [修改] 混合原始颜色 * spriteColor
                    vec4 texColor = texture(image, TexCoord);
                    FragColor = texColor * spriteColor;
                    return;
                }

                // Process 阶段：高斯模糊 (这部分不需要乘以 spriteColor，因为处理过程应该是纯数据的)
                // 只有最终 draw 阶段才需要应用透明度
                vec2 tex_offset = spread / textureSize(image, 0); 
                vec3 result = texture(image, TexCoord).rgb * weight[0]; 
                
                if(horizontal)
                {
                    for(int i = 1; i < 5; ++i)
                    {
                        result += texture(image, TexCoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
                        result += texture(image, TexCoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
                    }
                }
                else
                {
                    for(int i = 1; i < 5; ++i)
                    {
                        result += texture(image, TexCoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
                        result += texture(image, TexCoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
                    }
                }
                FragColor = vec4(result, 1.0); // 过程纹理保持 Alpha = 1.0
            }
        )";

        if (m_Shader) delete m_Shader;
        m_Shader = new Shader(vertexCode, fragmentCode);
    }

    void BlurEffect::captureScreen(esl::Window& window, const glm::vec2& regionPos, const glm::vec2& regionSize) {

        glReadBuffer(GL_BACK);
        glActiveTexture(GL_TEXTURE0);
        
        // 读取到 buffer [0]
        glBindTexture(GL_TEXTURE_2D, m_pingPongColorbuffers[0]);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (GLint)regionPos.x, (GLint)regionPos.y, (GLint)regionSize.x, (GLint)regionSize.y);
        glBindTexture(GL_TEXTURE_2D, 0);

        m_Size = regionSize;
        // 设置 Sprite 位置为区域中心，确保 draw 时位置正确
        // 假设 regionPos 是左下角
        // 假设 Sprite 绘制是以中心为原点的，或者你需要根据 Sprite 设置调整
        // 简单假设: Sprite 的原点 m_Origin 是 (0,0) (Quad中心)
        glm::vec2 centerPos = regionPos + regionSize * 0.5f;
        m_Position = glm::vec3(centerPos.x, centerPos.y, 0.0f);
    }

    void BlurEffect::process()
    {
        if (!m_Shader) return;
        
        GLint lastViewport[4];
        glGetIntegerv(GL_VIEWPORT, lastViewport);
        glViewport(0, 0, (GLsizei)m_effectSize.x, (GLsizei)m_effectSize.y);

        m_Shader->load();
        glActiveTexture(GL_TEXTURE0);

        m_Shader->setInt("isProcess", 1);
        m_Shader->setInt("image", 0);
        m_Shader->setFloat("spread", m_Spread);

        bool horizontal = true;
        bool first_iteration = true;

        // 使用专用的全屏 Quad VAO
        glBindVertexArray(m_processVAO);

        for (int i = 0; i < m_iterations; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, m_pingPongFBO[horizontal]);
            m_Shader->setInt("horizontal", horizontal);

            // 绑定源: 第一次是 buffer[0] (capture内容)，之后是 buffer[!horizontal]
            // 注意：第一次写入 buffer[1] (Horiz: true)，也就是将 buffer[0] 处理后写入 buffer[1]
            glBindTexture(GL_TEXTURE_2D, first_iteration ? m_pingPongColorbuffers[0] : m_pingPongColorbuffers[!horizontal]);

            // 全屏绘制（6个点，因为是 triangles）
            glDrawArrays(GL_TRIANGLES, 0, 6);

            horizontal = !horizontal;
            if (first_iteration) first_iteration = false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0); // 恢复屏幕 FBO
        m_Shader->setInt("isProcess", 0);

        m_Shader->unload();

        glViewport(lastViewport[0], lastViewport[1], lastViewport[2], lastViewport[3]);
    }

    void BlurEffect::draw(float right, float top)
    {
        if (!m_Shader) return;

        m_Shader->load();
        m_Shader->setInt("isProcess", 0); // 绘制阶段
        m_Shader->setVec4("spriteColor", m_Color);
        glActiveTexture(GL_TEXTURE0);

        bool isEven = (m_iterations % 2 == 0);
        glBindTexture(GL_TEXTURE_2D, isEven ? m_pingPongColorbuffers[0] : m_pingPongColorbuffers[1]);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, m_Position);
        model = glm::rotate(model, glm::radians(m_Rotation), glm::vec3(0.0f, 0.0f, 1.0f));

        model = glm::scale(model, glm::vec3(m_Size, 1.0f));

        glm::mat4 projection = glm::ortho(0.0f, right, 0.0f, top, -1.0f, 1.0f);

        m_Shader->setMat4("projection", projection);
        m_Shader->setMat4("model", model);

        if (m_VAO == 0) setup();
        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        m_Shader->unload();
    }
}