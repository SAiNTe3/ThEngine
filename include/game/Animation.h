#pragma once
#include <CircleShape.hpp>
#include <Sprite.hpp>
#include <Texture.hpp>

class Animation {
protected:
	double animationTimer = 0;
	virtual void start(glm::vec2 pos) = 0;
	virtual void update(double delta) = 0;
	virtual void draw(esl::Window& renderer) = 0;
	virtual void finish() = 0;
};

// 死亡动画时的4+2个圆环
class DeathCircle : public Animation {
	// 大圆位于死亡位置
	esl::CircleShape bigCircle;
	esl::CircleShape bigCircle2;
	// 4个小圆位于正左右上下radius位置
	esl::CircleShape smallCircle;
	glm::vec2 deathPosition = { 0,0 };
	float radius = 48;
	
	bool started = false;
public:
	DeathCircle() {
		bigCircle.setAsInversionLayer(true);
		bigCircle.setRadius(2);
		bigCircle2.setAsInversionLayer(true);
		bigCircle2.setRadius(2);
		smallCircle.setAsInversionLayer(true);
		smallCircle.setRadius(1);
	}
	void start(glm::vec2 pos) override {
		started = true;
		deathPosition = pos;
		bigCircle.setPosition(pos);
		bigCircle2.setPosition(pos);
		animationTimer = 0;
	}
	void update(double delta) override {
		if (!started) return;
		animationTimer += delta;
		smallCircle.setScale(800.f * glm::vec2{ animationTimer, animationTimer });
		bigCircle.setScale(800.f * glm::vec2{ animationTimer, animationTimer });
		if (animationTimer > 1)
			bigCircle2.setScale(800.f * glm::vec2{ animationTimer - 1, animationTimer - 1 });
		else if (animationTimer > 2) started = false;
	}
	void draw(esl::Window& renderer) override {
		if (!started) return;
		renderer.draw(bigCircle);
		if (animationTimer > 1)
			renderer.draw(bigCircle2);
		const glm::vec2 offset[4] = {
			{ radius,	 0 } ,
			{ -radius,	 0 } ,
			{ 0,		 radius } ,
			{ 0,		 -radius } };
		for (int i = 0; i < 4; i++) {
			smallCircle.setPosition(offset[i] + deathPosition);
			renderer.draw(smallCircle);
		}
	}
	void finish() override { }
};

class PauseMenu : public Animation {
	// 暂停菜单纹理
	std::unique_ptr<esl::Texture> mPauseTexture;
	//========== 需要处理
	std::unique_ptr<esl::Texture> mPauseTitleTexture;
	// 暂停菜单精灵
	std::vector<std::unique_ptr<esl::Sprite>> mPauseSprites;

	int mSelectedIndex = 0;
	const glm::vec2 basePos = { 400.f, 570.f };
	const float moveSpeed = 50.f;
	const glm::vec2 size = { 512,64 };
	
public:
	enum class PauseMenuState {
		FADE_IN,
		WAITING,
		FADE_OUT,
		FINISHED
	}state = PauseMenuState::FINISHED;
	PauseMenu() {
		mPauseTexture = std::make_unique<esl::Texture>(".\\Assets\\effect\\pause.png", esl::Texture::Wrap::CLAMP_TO_EDGE, esl::Texture::Filter::NEAREST);
		
		for (int i = 0; i < 13; i++) {
			std::unique_ptr<esl::Sprite> sprite = std::make_unique<esl::Sprite>(mPauseTexture.get());
			sprite->setTextureRectFlip({ 0.f, i * size.y }, size);
			sprite->setPosition(basePos - glm::vec2{ 0, i * size.y });
			sprite->setColor(glm::vec4{0.5,0.5,0.5,0});
			mPauseSprites.push_back(std::move(sprite));
		}
	}
	void start(glm::vec2 pos) override {
		state = PauseMenuState::FADE_IN;
		animationTimer = 0;
		for (int i = 0; i < 13; i++) {
			mPauseSprites[i]->setPosition(basePos - glm::vec2{0, i * size.y});
			mPauseSprites[i]->setAlpha(0.f);
			mPauseSprites[i]->setColor(glm::vec4{ 0.5,0.5,0.5,0 });
		}
	}
	void update(double delta) override {
		switch (state) {
		case PauseMenuState::FADE_IN: {
			animationTimer += delta;
			if (animationTimer < 0.25f) {
				float t = std::clamp(4 * animationTimer, 0.0, 1.0);
				float offset = moveSpeed * t;
				for (int i = 0; i < 13; i++) {
					mPauseSprites[i]->setAlpha(static_cast<float>(t));
					mPauseSprites[i]->move({ moveSpeed * delta, 0 });
				}
			}
			else {
				for (auto& sprite : mPauseSprites) {
					sprite->setAlpha(static_cast<float>(1.0f));
				}
				state = PauseMenuState::WAITING;
				animationTimer = 0;
			}
			break;
		}
		case PauseMenuState::WAITING: {
			animationTimer += delta;
			float pulse = 0.5f + 0.5f * std::sin(animationTimer);
			for (int i = 0; i < mPauseSprites.size(); ++i) {
				if (i == mSelectedIndex) {
					float pulse = 0.75f + 0.25f * sin(5 * animationTimer);
					mPauseSprites[i]->setColor(glm::vec4{ 1.0f, pulse, pulse, 1.0f });
				}
				else {
					mPauseSprites[i]->setColor(glm::vec4{ 0.5f, 0.5f, 0.5f, 1.0f });
				}
			}
			break;
		}
		case PauseMenuState::FADE_OUT: {
			animationTimer += delta;
			if (animationTimer < 0.25f) {
				float t = std::clamp(animationTimer, 0.0, 1.0);
				float offset = moveSpeed * t;
				for (int i = 0; i < 13; i++) {
					mPauseSprites[i]->setAlpha(static_cast<float>(1 - 4 * animationTimer));
					mPauseSprites[i]->move({ -moveSpeed * delta, 0 });
				}
			}
			else {
				for (auto& sprite : mPauseSprites) {
					sprite->setAlpha(static_cast<float>(0.0f));
				}
				state = PauseMenuState::FINISHED;
				animationTimer = 0;
			}
			break;
		}	
		case PauseMenuState::FINISHED:
			break;
		}
	}
	void draw(esl::Window& renderer) override {
		if (state != PauseMenuState::FINISHED)
			for (auto& sprite : mPauseSprites) {
				renderer.draw(*sprite);
			}
	}
	void finish() override {}
	void nextOption() {
		if (state != PauseMenuState::WAITING)
			return;
		mSelectedIndex = (mSelectedIndex + 1) % mPauseSprites.size();
	}
	void previousOption() {
		if(state != PauseMenuState::WAITING)
			return;
		mSelectedIndex = (mSelectedIndex - 1 + mPauseSprites.size()) % mPauseSprites.size();
	}
	int confirmSelection() {
		if (state != PauseMenuState::WAITING)
			return -1;
		state = PauseMenuState::FADE_OUT;
		animationTimer = 0;
		return mSelectedIndex;
	}
	PauseMenuState getState() const { return state; }
	void restartTimer() { animationTimer = 0; }
};