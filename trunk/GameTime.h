#pragma once

#include <Windows.h>

using namespace std;

namespace VBA10
{
	inline long long Counter()
    {
        LARGE_INTEGER li;
        QueryPerformanceCounter(&li);
        return li.QuadPart;
    }

    inline long long Frequency()
    {
        LARGE_INTEGER li;
        QueryPerformanceFrequency(&li);
        return li.QuadPart;
    }

	ref class GameTime sealed
	{
	public:
		GameTime(void);
		virtual ~GameTime(void);

		float GetLastFrameElapsed(void);
		float GetTotalElapsed(void);
		void Update(void);
		void SyncTime(void);

	private:
		long long frequency;
		long long startTime;
		long long lastTime;
		float elapsed, total;
	};
}