#include <Message.h>

esl::Window* Message::renderer = nullptr;
esl::Texture* Message::balloon_texture = nullptr;
std::deque<Message*> Message::message_queue;

void Message::init(esl::Window& renderer)
{
	Message::renderer = &renderer;
	balloon_texture = new esl::Texture("Assets/face/balloon_1024.png");
}
void Message::renderFirst()
{
	if (message_queue.empty()) return;
	message_queue.front()->render();
}
Message::MessageInfo Message::updateFirst(double deltaTime)
{
	if (message_queue.empty()) return {};
	message_queue.front()->update(deltaTime);
	return { message_queue.front()->face, message_queue.front()->role };
}
void Message::deleteFirst()
{
	if (message_queue.empty()) return;
	delete message_queue.front();
	message_queue.pop_front();

}
Message::Message(int role)
{
	this->role = role;
	this->text[0] = new esl::WText();
	this->text[1] = new esl::WText();
}

Message::~Message()
{
	delete text[0];
	delete text[1];
}

void Message::update(double deltaTime)
{
	scaleX = text[0]->getScale().x;
	if (scaleX < 1.0f) {
		scaleX += static_cast<float>(deltaTime) * 4;
	}
	else scaleX = 1.0f;
	text[0]?text[0]->setScale({ scaleX,text[0]->getScale().y }):void(0);
	text[1]?text[1]->setScale({ scaleX,text[1]->getScale().y }):void(0);
}

void Message::setPosition(glm::vec2 pos)
{
	if (text_line_count == 0) return;
	else if (text_line_count == 1)
		text[0]->setPosition(pos);
	else if(text_line_count==2)
	{
		text[0]->setPosition(pos + glm::vec2(0, line_spacing/2));
		text[1]->setPosition(pos + glm::vec2(0, -line_spacing/2));
	}
	this->pos = pos;
}

void Message::setLineSpacing(float spacing)
{
	line_spacing = spacing;
}

void Message::textLine1(const std::wstring& text)
{
	this->text[0]->setText(text);
}

void Message::textLine2(const std::wstring& text)
{
	this->text[1]->setText(text);
}

void Message::addText(const std::wstring& text)
{
	this->text[text_line_count++]->setText(text);
	setPosition(this->pos);
}

void Message::setBalloonStyle(int style)
{

	int w = 206;
	int x = 0;
	int y[] = { 0,97,231,343,449,577,736,880};
	int h[] = { 96,128,96,96,128,148,144,144 };
	rect = { x,1024 - y[style] - h[style],w,h[style]};
	int offset[] = { 24,8,18,18,24,12,18,18 };
	y_offset = static_cast<float>(offset[style]);
}

void Message::setFont(esl::Font& font)
{
	this->font = &font;
}

void Message::clearText()
{
	text_line_count = 0;
}

void Message::endEdit()
{
	// 纹理坐标定义
	int x0 = 0, x1 = 90, x2 = 124;
	int left_width = x1;           // 左边缘宽度: 90
	int middle_width = x2 - x1;    // 中间部分宽度: 34
	int right_width = 206 - x2;    // 右边缘宽度: 82

	// 根据文本长度计算需要的中间部分数量
	auto len1 = text[0]?text[0]->getLength():0, len2 = text[1]?text[1]->getLength():0;
	size_t count = len1 > len2 ? len1 : len2;

	float current_x = pos.x;
	float scale = 0.6f;

	int direction = (role == 1) ? -1 : 1;

	esl::Sprite* left_sprite = new esl::Sprite(balloon_texture);
	esl::Sprite* right_sprite = new esl::Sprite(balloon_texture);
	// --- 第一部分：起始端 (Role 0:左/尖角, Role 1:右/尖角) ---
	if (role == 0) {
		// 正常模式：左边是尖角 (x0-x1)
		left_sprite->setTextureRect({ rect.x + x0, rect.y }, { left_width, rect.h });
		left_sprite->setScale({ 1,1 });
	}
	else {
		// 对称模式：起始点(右边)应该是尖角 (x0-x1)，但水平翻转
		left_sprite->setTextureRect({ rect.x + x0, rect.y }, { left_width, rect.h }); // 改回 x0
		left_sprite->setScale({ -1,1 });
	}
	
	left_sprite->setPosition({ current_x, pos.y + y_offset});
	balloons.push_back(left_sprite);
	current_x += direction * std::abs(left_sprite->getGlobalSize().x/2);

	// 中间
	for (size_t i = 0; i < count; i++) {
		esl::Sprite* middle_sprite = new esl::Sprite(balloon_texture);
		middle_sprite->setTextureRect({ rect.x + x1, rect.y }, { middle_width, rect.h });
		middle_sprite->setScale({ role?-scale:scale,1 });
		middle_sprite->setPosition({ current_x, pos.y + y_offset });
		balloons.push_back(middle_sprite);
		current_x += direction * std::abs(middle_sprite->getGlobalSize().x);
	}
	// --- 第三部分：结束端 (Role 0:右/圆角, Role 1:左/圆角) ---
	if (role == 1) {
		// 对称模式：结束点(左边)应该是圆角 (x2-End)，但水平翻转
		right_sprite->setTextureRect({ rect.x + x2, rect.y }, { right_width, rect.h }); // 改回 x2
		right_sprite->setScale({ -1,1 });
	}
	else {
		// 正常模式：右边是圆角 (x2-End)
		right_sprite->setTextureRect({ rect.x + x2, rect.y }, { right_width, rect.h });
		right_sprite->setScale({ 1,1 });
	}
	right_sprite->setPosition({ current_x, pos.y + y_offset });
	balloons.push_back(right_sprite);

	text[0]->setScale({ 0.1,1 });
	text[1]->setScale({ 0.1,1 });
	text[0]->setFont(*font);
	text[1]->setFont(*font);
	// 修改：根据 Role 决定文本对齐位置
	if (role == 1) {
		// 对于 Role 1，气泡向左生成，current_x 现在处于气泡最左端
		// 文本应该从这里开始
		glm::vec2 textPos = { current_x, pos.y };

		if (text_line_count == 1)
			text[0]->setPosition(textPos);
		else if (text_line_count == 2)
		{
			text[0]->setPosition(textPos + glm::vec2(0, line_spacing / 2));
			text[1]->setPosition(textPos + glm::vec2(0, -line_spacing / 2));
		}
	}
	else {
		this->setPosition(this->pos);
	}
	message_queue.push_back(this);
}

void Message::render()
{
	for (auto& b : balloons)
	{
		if (b) {  // 添加检查
			renderer->draw(*b);
		}
		renderer->draw(*b);
	}
	for (size_t i = 0; i < text_line_count; i++)
	{
		renderer->draw(*text[i]);
	}
}
