#include <Game.h>

Game::Game()
{
	esl::Initialize();
}

Game::~Game()
{
	esl::Terminate();
}

void Game::init()
{
	mWindow = std::make_unique<esl::Window>(1280, 960, "Touhou 18 - UM", false, false);
	mWindow->setBackgroundColor(glm::vec4{ 1,1,1,1 });
	mWindow->setWindowPosition({ 400, 30 });
	mScriptSystem.initDialogueSystem(*mWindow);
	mScriptSystem.initAudioSystem(*mWindow);
	mScriptSystem.preloadSoundEffect("Assets/sound/");
	mScene = new TitleScene(*(mWindow.get()),mScriptSystem);
}

void Game::run()
{
	mWindow->showWindow();
}

void Game::update()
{
	double timeSinceLastUpdate = 0;
	double timePerFrame = 1.0 / 60.0;  // 60 FPS 更新频率
	
	while (mWindow->isOpen() && !mShouldQuit) {
		esl::Event e;
		mWindow->pollEvents(e);
		this->handle_event(e);
		
		double deltaTime = mMainClock.getElapsedTime();
		mMainClock.restart();
		timeSinceLastUpdate += deltaTime;
		
		// 固定时间步长更新游戏逻辑
		while (timeSinceLastUpdate >= timePerFrame) {
			timeSinceLastUpdate -= timePerFrame;
			mScene->update(timePerFrame);
			
			// 处理场景切换
			if (mScene->mSceneInfo.mSwitchToNextScene) {
				int index = mScene->mSceneInfo.mSwitchToSceneIndex;
				mScriptSystem.stopAudio();
				delete mScene;
				switch (index) {
				case -1: {
					mScene = new TitleScene(*(mWindow.get()), mScriptSystem);
					break;
				}
				case 0: {
					mScene = new MainGame(*(mWindow.get()), mScriptSystem);
					break;
				}
				case 1: {
					break;
				}
				case 2: {
					break;
				}
				case 3: {
					mShouldQuit = true;
					break;
				}
				default:
					break;
				}
				
			}
		}
		
		// 每帧都渲染，不受固定时间步长限制
		mScene->render();
	}
}

void Game::handle_event(esl::Event& e)
{
	mScene->process_input(e);
}

void Game::pause()
{
}

void Game::destory()
{
}
