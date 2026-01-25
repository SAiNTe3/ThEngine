#pragma once
#include<iostream>
namespace esl
{
	typedef unsigned int uint;
	typedef unsigned char uchar;
	class Texture 
	{
		uint m_Texture = 0;
		int m_Width = 0;
		int m_Height = 0;
		int m_Channel = 0;
		struct Size {
			int w;
			int h;
			Size(int x, int y) :w(x), h(y) {};
		};
		
	public:
		enum class Filter {
			LINEAR, //线性过滤
			NEAREST, //最近邻过滤
		};
		enum class Wrap {
			REPEAT,	//纹理坐标超出[0,1]范围时，重复纹理
			MIRRORED_REPEAT, //纹理坐标超出[0,1]范围时，镜像重复纹理
			CLAMP_TO_EDGE, //当纹理坐标超出[0,1]范围时，会使用纹理边缘的颜色
			CLAMP_TO_BORDER //纹理坐标超出部分取边界颜色
		};
		Texture(const char* path, Wrap wrap = Wrap::REPEAT, Filter filter = Filter::LINEAR);
		Texture(const std::string& path, Wrap wrap = Wrap::REPEAT, Filter filter = Filter::LINEAR);
		Texture(uint glfwTextureID, uint width, uint height);
		~Texture();
		Size getSize();
	private:
		void bind();
		int getWidth() const;
		int getHeight() const;
		int getChannel()const;
		friend class Sprite;
		friend class Sprite3D;
	};
}
