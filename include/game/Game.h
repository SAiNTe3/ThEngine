#pragma once
#include <ESL.hpp>
#include <Window.hpp>
#include <iostream>
#include <Scene.h>

class Game {
	
	std::unique_ptr<esl::Window> mWindow;
	Scene* mScene=nullptr;
	bool mShouldQuit = false;
	esl::Clock mMainClock;
	ScriptSystem mScriptSystem;
public:
	Game();
	~Game();
	void init();
	void run();
	void update();
	void handle_event(esl::Event& e);
	void pause();
	void destory();
	
};