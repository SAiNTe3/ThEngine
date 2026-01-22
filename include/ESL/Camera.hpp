#pragma once
#include"glm/glm.hpp"
#include"glm/gtc/matrix_transform.hpp"

namespace esl
{
	struct Camera
	{
		glm::vec3 m_Pos = glm::vec3(0.f, 0.f, 0.f);
		glm::vec3 m_Target = glm::vec3(0.f, 0.f, 0.f);
		glm::vec3 m_UpVector = glm::vec3(0.f, 1.f, 0.f);
		glm::vec2 m_ViewportOffset = glm::vec2(0.f, 0.f);
		//FOV field of vision
		float m_FieldOfView = 0.f;
		float m_Near = 0.1f;
		float m_Far = 1000.f;
	};
}
