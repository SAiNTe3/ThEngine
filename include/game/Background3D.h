#include <iostream>
#include <Sprite3D.hpp>
#include <Window.hpp>

class Background3D {
	
protected:
	using pSprite3D = std::unique_ptr<esl::Sprite3D>;
	using pTexture = std::unique_ptr<esl::Texture>;
	using pSprite = std::unique_ptr<esl::Sprite>;
	esl::Camera mCamera;
	static esl::Window* mRenderer;
	static const std::string mTexturePath;
	static glm::vec2 mCenterPos;
	pSprite3D mBaseSprite = nullptr;
	pTexture mBaseTexture = nullptr;
	int mPhase = 0;
public:
static void init(esl::Window* renderer, glm::vec2 center);
static void cleanup();
Background3D();
	virtual void update(double deltaTime);
	virtual void render() = 0;
	void nextPhase() {
		mPhase++;
	}
};

class Stage01_Background : public Background3D {

	pSprite3D mCloudSprite = nullptr;
	pSprite mSkySprite = nullptr;
	pTexture mCloudTexture = nullptr;
	pTexture mSkyTexture = nullptr;

public:
	Stage01_Background();
	void update(double deltaTime);
	void render() override;
};