#pragma once

namespace esl
{
	class Renderable
	{
	public:
		virtual void draw(float right, float top) {};

		virtual ~Renderable() = default;
	};

	class RenderTarget
	{
	public:
		virtual void clear() {};
		virtual void display() {};
		virtual void draw(Renderable& renderable) {};
		virtual ~RenderTarget() = default;

	};
}