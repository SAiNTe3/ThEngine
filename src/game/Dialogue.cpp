#include <Dialogue.h>
#include <fstream>
#include <string>
#include <filesystem>
#include <locale>
#include <codecvt>
void Dialogue_Illustration::init(const std::string& path,const std::string& index,bool reverse)
{
	full_illustration = std::make_unique<esl::Texture>(path + "face"+index+"bs.png");
	full_sprite = std::make_unique<esl::Sprite>(full_illustration.get());
	
	glm::vec2 size = glm::vec2(full_illustration->getSize().w, full_illustration->getSize().h);
	const glm::vec2 normal_size = glm::vec2(378, 728) * cutScale;
	glm::vec2 scale = normal_size / size;
	// printf("CutScale: %.2f\n", cutScale);
	full_sprite->setScale(reverse ? glm::vec2{ -scale.x,scale.y } : scale);

	ori_pos = reverse ? 
		glm::vec2{ 64+768,				normal_size.y/2 } : 
		glm::vec2{ normal_size.x/2,		normal_size.y/2 };
	dir_pos = reverse ? 
		glm::vec2{ 768,					normal_size.y/2 + 32 } : 
		glm::vec2{ normal_size.x/2 + 64,normal_size.y/2 + 32 };
	ori_pos.y += offsetY;
	dir_pos.y += offsetY;
	full_sprite->setPosition(ori_pos);
	
	
	std::string face_name[] = {
		"face"+index+"an","face" + index + "anl","face" + index + "dp","face" + index + "dpl","face" + index + "hp","face" + index + "hpl",
		"face" + index + "lo","face" + index + "lol","face" + index + "n2","face" + index + "n2l","face" + index + "no","face" + index + "nol",
		"face" + index + "pr","face" + index + "prl","face" + index + "sp","face" + index + "spl","face" + index + "sw","face" + index + "swl"
	};

	for (size_t i = 0; i < std::size(face_name); i++) {
		face_textures.emplace_back(
			std::move(
				std::make_unique<esl::Texture>(
					path + face_name[i] + ".png"
				)
			)
		);
		face_sprites.emplace_back(
			std::move(
				std::make_unique<esl::Sprite>(
					face_textures[i].get()
				)
			)
		);
		face_sprites[i]->setScale({ reverse ?glm::vec2{-scale.x,scale.y} : scale });
	}

	name_flavor = std::make_unique<esl::Texture>(path + "name.png");
	name_flavor_sprite = std::make_unique<esl::Sprite>(name_flavor.get());
}

void Dialogue::init(esl::Window& renderer)
{
	Dialogue::renderer = &renderer;
	Message::init(renderer);
	font = new esl::Font();
	font->loadFromFile("C:\\Windows\\Fonts\\simhei.ttf");
	font->setFontSize(24);
}

void Dialogue::render()
{
	// 修复：即使消息为空，如果在退出动画中也要继续渲染
	if (!Message::isEmpty() || mIsExiting) {
		if (player_visible) {
			renderer->draw(*player.full_sprite.get());
			if (player.face_index != -1)
				renderer->draw(*player.face_sprites[player.face_index].get());
		}
		if (boss_visible) {
			renderer->draw(*boss.full_sprite.get());
			if (boss.face_index != -1)
				renderer->draw(*boss.face_sprites[boss.face_index].get());
		}

		// 只在有消息时渲染消息
		if (!Message::isEmpty()) {
			Message::renderFirst();
		}
	}
}
void Dialogue::update(double deltaTime)
{
	// 如果正在退出，处理退出动画
	if (mIsExiting) {

		mExitTimer += deltaTime;
		float progress = static_cast<float>(glm::clamp(mExitTimer / mExitDuration, 0.0, 1.0));

		// 玩家向左下移动
		if (player.full_sprite) {
			glm::vec2 targetPos = player.ori_pos + glm::vec2(-300, -300) * progress;
			player.full_sprite->setPosition(targetPos);
			float alpha = 1.0f - progress;
			player.full_sprite->setColor(glm::vec4(1, 1, 1, alpha));
		}
		// 更新玩家表情位置和颜色
		if (player.face_index != -1 && player.face_sprites[player.face_index]) {
			glm::vec2 pos = player.full_sprite->getPosition();
			player.face_sprites[player.face_index]->setPosition(pos + player.face_pos * glm::vec2(0.7, 0.7));
			float alpha = 1.0f - progress;
			player.face_sprites[player.face_index]->setColor(glm::vec4(1, 1, 1, alpha));
		}

		// Boss 向右下移动
		if (boss.full_sprite) {
			glm::vec2 targetPos = boss.ori_pos + glm::vec2(300, -300) * progress;
			boss.full_sprite->setPosition(targetPos);
			float alpha = 1.0f - progress;
			boss.full_sprite->setColor(glm::vec4(1, 1, 1, alpha));
		}
		// 更新 Boss 表情位置和颜色
		if (boss.face_index != -1 && boss.face_sprites[boss.face_index]) {
			glm::vec2 pos = boss.full_sprite->getPosition();
			boss.face_sprites[boss.face_index]->setPosition(pos + boss.face_pos * glm::vec2(0.7, 0.7));
			float alpha = 1.0f - progress;
			boss.face_sprites[boss.face_index]->setColor(glm::vec4(1, 1, 1, alpha));
		}

		// 动画结束
		if (progress >= 1.0f) {
			mIsExiting = false;
			player_visible = false;
			boss_visible = false;			
		}
		return;
	}
	Message::MessageInfo info = Message::updateFirst(deltaTime);
	if (info.role == 0) {
		player_visible = true;
		player.face_index = info.face;
		player.full_sprite->setColor(glm::vec4(1, 1, 1, 1));
		player.moveToDir(static_cast<float>(deltaTime));
		boss.moveToOri(static_cast<float>(deltaTime));
		if (player.face_index != -1) {
			glm::vec2 pos = player.full_sprite->getPosition();
			player.face_sprites[player.face_index]->setPosition(pos+player.face_pos*glm::vec2(0.7,0.7));
			player.face_sprites[player.face_index]->setColor(glm::vec4(1, 1, 1, 1));
		}
		boss.full_sprite->setColor(glm::vec4(0.5, 0.5, 0.5, 1));
		if (boss.face_index != -1) {
			boss.face_sprites[boss.face_index]->setColor(glm::vec4(0.5, 0.5, 0.5, 1));
			boss.face_sprites[boss.face_index]->setPosition(boss.full_sprite->getPosition() + boss.face_pos * glm::vec2(0.7, 0.7));
		}
	}
	else if (info.role == 1) {
		boss_visible = true;
		boss.face_index = info.face;
		boss.full_sprite->setColor(glm::vec4(1, 1, 1, 1));
		boss.moveToDir(static_cast<float>(deltaTime));
		player.moveToOri(static_cast<float>(deltaTime));
		if (boss.face_index != -1) {
			glm::vec2 pos = boss.full_sprite->getPosition();
			boss.face_sprites[boss.face_index]->setPosition(pos + boss.face_pos * glm::vec2(0.7, 0.7));
			boss.face_sprites[boss.face_index]->setColor(glm::vec4(1, 1, 1, 1));
		}
		player.full_sprite->setColor(glm::vec4(0.5, 0.5, 0.5, 1));
		if (player.face_index != -1) {
			player.face_sprites[player.face_index]->setColor(glm::vec4(0.5, 0.5, 0.5, 1));
			player.face_sprites[player.face_index]->setPosition(player.full_sprite->getPosition() + player.face_pos * glm::vec2(0.7, 0.7));
		}
	}

}

void Dialogue::process_input(esl::Event& e)
{
	static bool respond = true;
	if (respond) {
		// 该布尔值用于记录上一次帧是否已经按下了上下键，防止长按时多次触发
		static bool key_pressed_last_frame = false;
		if (e.isKeyPressed(Keyboard::KEY_Z) && !key_pressed_last_frame) {
			Message::deleteFirst();
			key_pressed_last_frame = true;
		}
		if (e.isKeyReleased(Keyboard::KEY_Z)) {
			key_pressed_last_frame = false;
		}
	}
}

void Dialogue::reset() {
	// 重置可见性
	player_visible = false;
	boss_visible = false;
	// 重置退出状态
	mIsExiting = false;
	mExitTimer = 0.0;
	// 重置表情索引
	player.face_index = -1;
	boss.face_index = -1;

	// 重置位置到初始位置
	if (player.full_sprite) {
		player.full_sprite->setPosition(player.ori_pos);
		player.full_sprite->setColor(glm::vec4(1, 1, 1, 1));
	}
	for (auto& face_sprite : player.face_sprites) {
		if (face_sprite) {
			face_sprite->setColor(glm::vec4(1, 1, 1, 1));
		}
	}
	if (boss.full_sprite) {
		boss.full_sprite->setPosition(boss.ori_pos);
		boss.full_sprite->setColor(glm::vec4(1, 1, 1, 1));
	}
	for (auto& face_sprite : boss.face_sprites) {
		if (face_sprite) {
			face_sprite->setColor(glm::vec4(1, 1, 1, 1));
		}
	}
}

void Dialogue::startExit() {
	mIsExiting = true;
	mExitTimer = 0.0;
}