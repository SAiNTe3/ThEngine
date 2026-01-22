#include <Background3D.h>

esl::Window* Background3D::mRenderer = nullptr;
std::string const Background3D::mTexturePath = "Assets/background/";
glm::vec2 Background3D::mCenterPos = glm::vec2(0);
void Background3D::init(esl::Window* renderer, glm::vec2 pos)
{
	mRenderer = renderer;
	mCenterPos = pos;
}

Background3D::Background3D()
{
}

void Background3D::update(double deltaTime)
{
}

Stage01_Background::Stage01_Background()
{
	std::string texture_path = mTexturePath + "stage01/";
	mBaseTexture = std::make_unique<esl::Texture>(texture_path + "st01a.png");
	mCloudTexture = std::make_unique<esl::Texture>(texture_path + "stg4bg2.png");
	mSkyTexture = std::make_unique<esl::Texture>(texture_path + "A007.png");
	mBaseSprite = std::make_unique<esl::Sprite3D>(mBaseTexture.get());
	mCloudSprite = std::make_unique<esl::Sprite3D>(mCloudTexture.get());
	mSkySprite = std::make_unique<esl::Sprite>(mSkyTexture.get());

	mBaseSprite->setPosition({ 0,0,0});
	mCloudSprite->setPosition({ 0,0,50 });
	mSkySprite->setPosition({ 0,800});
	mSkySprite->setScale({ 2.0f,3.0f });
	mSkySprite->setTextureRect({ 0,1 }, { 1024,140 });
	mSkySprite->setRepeat({ 2,1 });
	

	mBaseSprite->setRepeat({ 2,6 });
	mCloudSprite->setRepeat({ 4,20 });
	
	mCloudSprite->setColor(glm::vec4{ 1,1,1,0.4 });
	mBaseSprite->setFogEnabled(true);
	mBaseSprite->setFogDistance(800.0f, 1200.0f);  
	mBaseSprite->setFogColor(glm::vec4(1.f, 1.f, 1.f, 0.0f)); 
	mCloudSprite->setFogEnabled(true);
	mCloudSprite->setFogDistance(800.0f, 1200.0f);
	mCloudSprite->setFogColor(glm::vec4(1.f, 1.f, 1.f, 0.0f));

	mCamera.m_Pos = glm::vec3(0, 0, 400);
	mCamera.m_Target = glm::vec3(0, 600, 0.f);
	mCamera.m_FieldOfView = 45.f;
	mCamera.m_ViewportOffset = glm::vec2(-192, 0);
	mCamera.m_Far = 1200;
	mBaseSprite->bindCamera(&mCamera);
	mCloudSprite->bindCamera(&mCamera);
}

void Stage01_Background::update(double deltaTime)
{
	switch (mPhase) {
	case 0: {
		static float last_frame_pos_y = 0;
		last_frame_pos_y = mCamera.m_Pos.y;
		float distance = 20 * deltaTime;
		mCamera.m_Pos.y += 20 * deltaTime;
		mCamera.m_Target.y += 20 * deltaTime;
		mCloudSprite->setPosition({ 0, -mCamera.m_Pos.y,0 });
		if (mCamera.m_Pos.y >= 512) {
			mCamera.m_Pos.y = last_frame_pos_y + distance - 512;
			mCamera.m_Target.y = last_frame_pos_y + distance - 512 + 600;
			mCloudSprite->setPosition({ 0, -mCamera.m_Pos.y,0 });
		}
		float sky_x = mSkySprite->getPosition().x;
		mSkySprite->setPosition({ sky_x + 10 * deltaTime, 800 });
		if (sky_x >= 2048) {
			mSkySprite->setPosition({ sky_x - 2048,800 });
		}
		break;
	}
	case 1: {
		break;
	}
	default: {
	}
	}
	
	return;
}

void Stage01_Background::render()
{
	mRenderer->draw(*mBaseSprite.get());
	mRenderer->draw(*mCloudSprite.get());
	mRenderer->draw(*mSkySprite.get());
}
