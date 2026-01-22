#include "ESL.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Text.hpp"
#include "Font.hpp"
namespace esl
{
	void Initialize()
	{
		static bool isInitialized = false;
		if (isInitialized)
		{
			return;
		}
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		Font::init();
		isInitialized = true;
	}
	void Terminate()
	{
		glfwTerminate();
	}

	int DebugLog(const std::string& message)
	{
#ifdef ESL_DEBUG
		std::cout << "[ESL Debug] " << message << std::endl;
#endif
		return 0;
	}

}