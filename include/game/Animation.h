#pragma once
#include <CircleShape.hpp>
#include <Sprite.hpp>
#include <Texture.hpp>
#include <array>
class Animation {
	
protected:
	using pSprite = std::unique_ptr<esl::Sprite>;
	using pTexture = std::unique_ptr<esl::Texture>;

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
		bigCircle.setPointCount(60);
		bigCircle2.setAsInversionLayer(true);
		bigCircle2.setRadius(2);
		bigCircle2.setPointCount(60);
		smallCircle.setAsInversionLayer(true);
		smallCircle.setRadius(1);
		smallCircle.setPointCount(60);
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
	std::unique_ptr<esl::Texture> mPauseTitleTexture;
	std::unique_ptr<esl::Texture> mPauseBackTexture;
	// 暂停菜单精灵
	std::vector<std::unique_ptr<esl::Sprite>> mPauseSprites;
	std::vector<std::unique_ptr<esl::Sprite>> mPauseTitleSprites;
	std::unique_ptr<esl::Sprite> mPauseBackSprite;

	int mSelectedIndex = 0;
	const glm::vec2 basePos = { 420.f, 570.f };
	const float moveSpeed = 50.f;
	const glm::vec2 size = { 512,64 };
	std::array<size_t,7> pauseOptionOrder = {0,1,2,9,3,8,7};
	size_t confirmOptionOrder[3] = { 10,11,12 };
public:
	enum class PauseMenuState {
		FADE_IN,
		WAITING,
		FADE_OUT,
		FINISHED
	}state = PauseMenuState::FINISHED;
	PauseMenu() {
		mPauseTexture = std::make_unique<esl::Texture>(".\\Assets\\ascii\\pause.png", esl::Texture::Wrap::CLAMP_TO_EDGE, esl::Texture::Filter::NEAREST);
		
		for (int i = 0; i < 13; i++) {
			std::unique_ptr<esl::Sprite> sprite = std::make_unique<esl::Sprite>(mPauseTexture.get());
			sprite->setTextureRectFlip({ 0.f, i * size.y }, size);
			sprite->setColor(glm::vec4{0.5,0.5,0.5,0});
			mPauseSprites.push_back(std::move(sprite));
		}

		mPauseTitleTexture = std::make_unique<esl::Texture>(".\\Assets\\ascii\\pause_title.png", esl::Texture::Wrap::CLAMP_TO_EDGE, esl::Texture::Filter::NEAREST);
		for (int i = 0; i < 4; i++) {
			std::unique_ptr<esl::Sprite> sprite = std::make_unique<esl::Sprite>(mPauseTitleTexture.get());
			sprite->setTextureRectFlip({ 0.f, i * 96.f }, { 256.f,64.f });
			sprite->setPosition(basePos - glm::vec2{225,-120});
			mPauseTitleSprites.push_back(std::move(sprite));
		}
		mPauseBackTexture = std::make_unique<esl::Texture>(".\\Assets\\ascii\\pause_back.png");
		mPauseBackSprite = std::make_unique<esl::Sprite>(mPauseBackTexture.get());
		mPauseBackSprite->setColor(glm::uvec4{ 30,120,80,255 });
		
	}
	void start(glm::vec2 pos) override {
		state = PauseMenuState::FADE_IN;
		animationTimer = 0;
		for (int i = 0; i < pauseOptionOrder.size();i++) {
			mPauseSprites[pauseOptionOrder[i]]->setPosition(basePos - glm::vec2{0, i * size.y});
			mPauseSprites[pauseOptionOrder[i]]->setAlpha(0.f);
			mPauseSprites[pauseOptionOrder[i]]->setColor(glm::vec4{ 0.5,0.5,0.5,0 });
		}
		mPauseTitleSprites[0]->setAlpha(0.f);
		mSelectedIndex = 0;
		for (auto& e : mPauseTitleSprites) {
			e->setPosition(basePos - glm::vec2{ 225,-120 });
		}
		mPauseBackSprite->setPosition(basePos+glm::vec2{ -375,-150 });
		mPauseBackSprite->setRotation(10);
	}
	void update(double delta) override {
		switch (state) {
		case PauseMenuState::FADE_IN: {
			animationTimer += delta;
			if (animationTimer < 0.25f) {
				float t = std::clamp(4 * animationTimer, 0.0, 1.0);
				float offset = moveSpeed * t;
				for (int i = 0; i < pauseOptionOrder.size(); i++) {
					mPauseSprites[pauseOptionOrder[i]]->setAlpha(static_cast<float>(t));
					mPauseSprites[pauseOptionOrder[i]]->move({ -moveSpeed * delta, 0 });
				}
				mPauseTitleSprites[0]->move({ 0, -moveSpeed * delta });
				mPauseTitleSprites[0]->setAlpha(static_cast<float>(t));
				mPauseBackSprite->move({ 10 * moveSpeed * delta, 0 });
				/*mPauseBackSprite->move({})*/
			}
			else {
				for (int i = 0; i < pauseOptionOrder.size(); i++) {
					mPauseSprites[pauseOptionOrder[i]]->setAlpha(static_cast<float>(1.0f));
				}
				mPauseTitleSprites[0]->setAlpha(1.f);
				state = PauseMenuState::WAITING;
				animationTimer = 0;
			}
			break;
		}
		case PauseMenuState::WAITING: {
			animationTimer += delta;
			float pulse = 0.5f + 0.5f * std::sin(animationTimer);
			for (int i = 0; i < pauseOptionOrder.size(); ++i) {
				if (i == mSelectedIndex) {
					float pulse = 0.75f + 0.25f * sin(5 * animationTimer);
					mPauseSprites[pauseOptionOrder[i]]->setColor(glm::vec4{ 1.0f, pulse, pulse, 1.0f });
				}
				else {
					mPauseSprites[pauseOptionOrder[i]]->setColor(glm::vec4{ 0.5f, 0.5f, 0.5f, 1.0f });
				}
			}
			mPauseBackSprite->setRotation(2 * std::sin(animationTimer * 3)+10);
			break;
		}
		case PauseMenuState::FADE_OUT: {
			animationTimer += delta;
			if (animationTimer < 0.25f) {
				float t = std::clamp(4 * animationTimer, 0.0, 1.0);
				float offset = moveSpeed * t;
				for (int i = 0; i < pauseOptionOrder.size(); i++) {
					mPauseSprites[pauseOptionOrder[i]]->setAlpha(static_cast<float>(1 - t));
					mPauseSprites[pauseOptionOrder[i]]->move({ moveSpeed * delta, 0 });
				}
				mPauseTitleSprites[0]->move({ 0, moveSpeed * delta });
				mPauseTitleSprites[0]->setAlpha(static_cast<float>(1 - t));
				mPauseBackSprite->move({ -10 * moveSpeed * delta, 0 });
			}
			else {
				for (int i = 0; i < pauseOptionOrder.size(); i++) {
					mPauseSprites[pauseOptionOrder[i]]->setAlpha(static_cast<float>(0.0f));
				}
				mPauseTitleSprites[0]->setAlpha(0.0f);
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
		if (state != PauseMenuState::FINISHED) {
			renderer.draw(*mPauseBackSprite);
			for (auto& sprite : mPauseSprites) {
				renderer.draw(*sprite);
			}
			renderer.draw(*mPauseTitleSprites[0]);
			
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

class SwitchScreenAnimation : public Animation {
	const std::string path = ".\\Assets\\effect\\eff_switchbg.png";
	const float speed = 100.f;
	pTexture mSwitchTexture;
	pSprite mSwitchSprite;
	bool finished = false;
public:
	SwitchScreenAnimation() {
		mSwitchTexture = std::make_unique<esl::Texture>(path);
		mSwitchSprite = std::make_unique<esl::Sprite>(mSwitchTexture.get());
		mSwitchSprite->setRepeat({ 4,4 });
	}
	void start(glm::vec2 pos = {}) override {
		finished = false;
		animationTimer = 0;
		mSwitchSprite->setPosition({ 1024,1024 });
	}
	void update(double delta) override {
		animationTimer += delta;
		float distance = delta * speed;
		mSwitchSprite->move({ -distance,-distance });
		if (mSwitchSprite->getPosition().x<-2048) {
			mSwitchSprite->move({ -1024,-1024 });
		}
		if (animationTimer > 5.0f) {
			finished = true;
		}
	}
	void draw(esl::Window& renderer) override {
		renderer.draw(*mSwitchSprite);
	}
	void finish() override {}
	bool isFinished() const { return finished; }
};