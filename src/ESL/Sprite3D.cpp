#include"glad/glad.h"
#include"GLFW/glfw3.h"
#include "Sprite3D.hpp"

namespace esl
{
	Sprite3D::Sprite3D() :Sprite()
	{
		setupFogShader();
	}

	Sprite3D::Sprite3D(Texture* texture) :Sprite(texture)
	{
		setupFogShader();
	}
	void Sprite3D::setupFogShader()
	{
		// 用支持雾效的着色器替换默认着色器
		delete m_Shader;

		const std::string vstring = {
			"#version 460 core\n"
			"layout(location = 0) in vec3 aPos;\n"
			"layout(location = 2) in vec2 aUV;\n"
			"out vec2 uv;\n"
			"out vec3 fragWorldPos;\n"
			"uniform mat4 transform;\n"
			"uniform mat4 projection;\n"
			"uniform mat4 view;\n"
			"uniform vec2 uvScale;\n"
			"void main() {\n"
			"vec4 worldPos = transform * vec4(aPos, 1.0);\n"
			"fragWorldPos = worldPos.xyz;\n"
			"gl_Position = projection * view * worldPos;\n"
			"uv = aUV * uvScale;\n"
			"}\0" };

		const std::string fstring = {
			"#version 460 core\n"
			"in vec2 uv;\n"
			"in vec3 fragWorldPos;\n"
			"out vec4 fragColor;\n"
			"uniform sampler2D sampler;\n"
			"uniform vec4 spriteColor;\n"
			"uniform vec3 cameraPos;\n"
			"uniform float fogStart;\n"
			"uniform float fogEnd;\n"
			"uniform vec4 fogColor;\n"
			"uniform bool fogEnabled;\n"
			"void main() {\n"
			"vec4 texColor = texture(sampler, uv) * spriteColor;\n"
			"if (fogEnabled) {\n"
			"    float distance = length(fragWorldPos - cameraPos);\n"
			"    float fogFactor = clamp((distance - fogStart) / (fogEnd - fogStart), 0.0, 1.0);\n"
			"    fragColor = mix(texColor, fogColor, fogFactor);\n"
			"} else {\n"
			"    fragColor = texColor;\n"
			"}\n"
			"}\0" };

		m_Shader = new Shader(vstring, fstring);
		m_Shader->setInt("sampler", 0);

		// 默认禁用雾效
		m_FogEnabled = false;
		m_FogStart = 100.0f;
		m_FogEnd = 500.0f;
		m_FogColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}

	void Sprite3D::setFogEnabled(bool enabled)
	{
		m_FogEnabled = enabled;
	}

	void Sprite3D::setFogDistance(float start, float end)
	{
		m_FogStart = start;
		m_FogEnd = end;
	}

	void Sprite3D::setFogColor(glm::vec4 color)
	{
		m_FogColor = color;
	}

	void Sprite3D::bindCamera(Camera* camera)
	{
		m_Camera = camera;
	}

	void Sprite3D::setRotation(glm::vec3 angles)
	{
		m_Rotation3D = angles;
		m_Rotation3DQuat = glm::quat(glm::radians(angles));
	}

	void Sprite3D::setPosition(glm::vec3 pos)
	{
		m_Position = pos;
	}

	void Sprite3D::setOrigin(glm::vec3 pos)
	{
		m_Origin3D = pos;
	}

	void Sprite3D::move(glm::vec3 distance)
	{
		m_Position += distance;
	}

	glm::vec3 Sprite3D::getRotation() const
	{
		return m_Rotation3D;
	}

	glm::vec3 Sprite3D::getPosition()const
	{
		return m_Position;
	}
	glm::vec3 Sprite3D::getOrigin() const
	{
		return m_Origin3D;
	}
	void Sprite3D::draw(float right, float top)
	{
		m_Shader->load();
		m_Texture->bind();
		glm::mat4 projection;
		glm::mat4 view(1.f);
		if (m_Camera)
		{
			projection = glm::perspective(glm::radians(m_Camera->m_FieldOfView), right / top, m_Camera->m_Near, m_Camera->m_Far);
			view = glm::lookAt(m_Camera->m_Pos, m_Camera->m_Target, m_Camera->m_UpVector);
			if (m_Camera->m_ViewportOffset.x || m_Camera->m_ViewportOffset.y )
			{
				// 将像素坐标转换为标准化设备坐标(NDC)
				// NDC范围是[-1, 1]，屏幕中心为(0, 0)
				float ndcOffsetX = (m_Camera->m_ViewportOffset.x * 2.0f) / right;
				float ndcOffsetY = (m_Camera->m_ViewportOffset.y * 2.0f) / top;

				glm::mat4 offsetMatrix = glm::translate(glm::mat4(1.0f),
					glm::vec3(ndcOffsetX, ndcOffsetY, 0.0f));
				projection = offsetMatrix * projection;
			}
			// 传递摄像机位置用于雾效计算
			m_Shader->setVec3("cameraPos", m_Camera->m_Pos);
		}
		else
		{
			projection = glm::ortho(0.0f, right, 0.0f, top, -1.0f, 1.0f);
		}
		// 设置雾效参数
		m_Shader->setBool("fogEnabled", m_FogEnabled);
		m_Shader->setFloat("fogStart", m_FogStart);
		m_Shader->setFloat("fogEnd", m_FogEnd);
		m_Shader->setVec4("fogColor", m_FogColor);

		m_Shader->setMat4("projection", projection);
		m_Shader->setMat4("view", view);
		glm::mat4 transform = glm::mat4(1.0f);
		transform = glm::translate(transform, m_Position);
		transform = glm::translate(transform, m_Origin3D);

		glm::mat4 rotationMatrix = glm::mat4_cast(m_Rotation3DQuat);
		transform = transform * rotationMatrix;
		transform = glm::translate(transform, -m_Origin3D);
		glm::vec2 scaledSize = m_Size * m_Scale * m_RectScale * m_RepeatScale;
		transform = glm::scale(transform, glm::vec3(scaledSize.x, scaledSize.y, 1.0f));

		m_Shader->setMat4("transform", transform);
		m_Shader->setVec4("spriteColor", m_Color);
		m_Shader->setVec2("uvScale", m_RepeatScale);


		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		m_Shader->unload();
	}
}
