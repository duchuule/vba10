#include "GameTime.h"
#include <Windows.h>

namespace VBA10
{
	GameTime::GameTime(void)
	{
		this->frequency = Frequency();
		this->startTime = Counter();
		this->lastTime = this->startTime;
	}

	GameTime::~GameTime(void)
	{ }

	void GameTime::Update(void)
	{
		long long currentTime = Counter();
		this->total = (currentTime - startTime) / (float)frequency;
		this->elapsed = (currentTime - lastTime) / (float)frequency;
		lastTime = currentTime;	
	}
	
	void GameTime::SyncTime(void)
	{
		long long currentTime = Counter();
		float newtimeTotal = (currentTime - startTime) / (float)frequency;
		long long delta = (long long) ((newtimeTotal - this->total) * this->frequency);
		startTime += delta;
		lastTime += delta;
	}

	float GameTime::GetLastFrameElapsed(void)
	{
		return this->elapsed;
	}

	float GameTime::GetTotalElapsed(void)
	{
		return this->total;
	}
}