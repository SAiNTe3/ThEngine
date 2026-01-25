#define STB_IMAGE_IMPLEMENTATION

#include "Texture.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "stbImage/stb_image.h"

namespace esl
{

	Texture::Texture(const char* path, Wrap wrap, Filter filter)
	{
		glGenTextures(1, &m_Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_Texture);
		uint _wrap,_filter;
		switch (filter) {
			case Filter::LINEAR:
				_filter = GL_LINEAR;
				break;
			case Filter::NEAREST:
				_filter = GL_NEAREST;
				break;
		}
		switch (wrap) {
			case Wrap::REPEAT:
				_wrap = GL_REPEAT;
				break;
			case Wrap::MIRRORED_REPEAT:
				_wrap = GL_MIRRORED_REPEAT;
				break;
			case Wrap::CLAMP_TO_EDGE:
				_wrap = GL_CLAMP_TO_EDGE;
				break;
			case Wrap::CLAMP_TO_BORDER:
				_wrap = GL_CLAMP_TO_BORDER;
				break;
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _filter);
		stbi_set_flip_vertically_on_load(true);
		uchar* data = stbi_load(path, &m_Width, &m_Height, &m_Channel, 0);
		if (data)
		{
			switch (m_Channel)
			{
			case 3:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_Width, m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				break;
			case 4:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
				break;
			default:
				std::cout << "Unsupported format!" << std::endl;
				stbi_image_free(data);
				return;
			}
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture:" <<path<<":" << stbi_failure_reason() << std::endl;
		}
		stbi_image_free(data);
	}
	Texture::Texture(const std::string& path, Wrap wrap, Filter filter):Texture(path.c_str(), wrap, filter)
	{

	}
	Texture::Texture(uint glfwTextureID, uint width, uint height)
	{
		this->m_Texture = glfwTextureID;
		this->m_Width = width;
		this->m_Height = height;
		this->m_Channel = 4;
	}
	Texture::~Texture()
	{
		glDeleteTextures(1, &m_Texture);
	}
	Texture::Size Texture::getSize()
	{
		return Size(m_Width, m_Height);
	}
	void Texture::bind()
	{
		glBindTexture(GL_TEXTURE_2D, m_Texture);
	}
	int Texture::getWidth() const
	{
		return m_Width;
	}
	int Texture::getHeight() const
	{
		return m_Height;
	}
	int Texture::getChannel() const
	{
		return m_Channel;
	}
}