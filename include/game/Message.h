#pragma once
#include <iostream>
#include <vector>
#include <ModernText.hpp>
#include <Sprite.hpp>
#include <Window.hpp>
#include <deque>
class Message {
	static esl::Window* renderer;
	static esl::Texture* balloon_texture;
	static std::deque<Message*> message_queue;
	esl::Font* font = nullptr;
	glm::vec2 pos{};
	float line_spacing = 32.0f;
	float scaleX = 1;
	esl::WText* text[2]{ nullptr };
	size_t text_line_count = 0;
	std::vector<esl::Sprite*> balloons;
	struct Rect {
		int x, y, w, h;
	};
	Rect rect;
	float y_offset = 0;
	int role = 0;
	int face = -1;
public:
	struct MessageInfo {
		int face;
		int role;
	};
	static void init(esl::Window& renderer);
	static bool isEmpty() { return message_queue.empty(); }
	static void renderFirst();
	static MessageInfo updateFirst(double deltaTime);
	static void deleteFirst();
	Message(int role);
	~Message();
	void setFace(int index) { face = index; }
	int getFace() const { return face; }
	void update(double deltaTime);
	void setPosition(glm::vec2 pos);
	void setLineSpacing(float spacing);// 设置行间距
	void textLine1(const std::wstring& text);// 设置第一行文本
	void textLine2(const std::wstring& text);// 设置第二行文本
	void addText(const std::wstring& text);// 设置下一行文本
	void setBalloonStyle(int style);
	void setBalloonLength(float length);
	void setFont(esl::Font& font);
	void clearText();// 清除文本
	void endEdit();
	void render();
};