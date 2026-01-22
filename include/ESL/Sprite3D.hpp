#pragma once
#include "Sprite.hpp"
#include "Camera.hpp"
namespace esl
{
	class Sprite3D : public Sprite
	{
		bool m_FogEnabled = false;
		float m_FogStart = 100.0f;
		float m_FogEnd = 500.0f;
		glm::vec4 m_FogColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		void setupFogShader();
	protected:
		glm::vec3 m_Rotation3D = glm::vec3(1.f, 1.f, 1.f);
		glm::quat m_Rotation3DQuat = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		Camera* m_Camera = nullptr;
		glm::vec3 m_Origin3D = glm::vec3(0.f, 0.f, 0.f);
	public:
		Sprite3D();
		Sprite3D(Texture* texture);
		void bindCamera(Camera* camera);
		void setRotation(glm::vec3 angles);
		void setPosition(glm::vec3 pos);
		void setOrigin(glm::vec3 pos);
		void move(glm::vec3 distance);
		glm::vec3 getRotation()const;
		glm::vec3 getPosition()const;
		glm::vec3 getOrigin()const;
		void setPosition(glm::vec2 pos) = delete;
		void setRotation(float) = delete;
		void setOrigin(glm::vec2) = delete;
		void move(glm::vec2) = delete;
		// 设置雾效开关
		void setFogEnabled(bool enabled);
		// 设置雾效距离范围 (start: 开始渐变距离, end: 完全雾化距离)
		void setFogDistance(float start, float end);
		// 设置雾效颜色 (支持 alpha)
		void setFogColor(glm::vec4 color);
	protected:
		virtual void draw(float right, float top)override;
		friend class Window;
	};
}
