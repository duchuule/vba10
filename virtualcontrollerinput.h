#pragma once

#include "EmulatorInput.h"
#include <collection.h>

using namespace Windows::UI::Input;
using namespace Platform::Collections;

#define ARRAY_SIZE			1009
#define SMALL_ARRAY_SIZE	10

namespace VBA10
{
	class EmulatorGame;

	class VirtualControllerInput
		: public EmulatorInput
	{
	public:
		static VirtualControllerInput *GetInstance(void);

		VirtualControllerInput(void);
		~VirtualControllerInput(void);

		const ControllerState *GetControllerState(void);

		void GetStickRectangle(RECT *rect);
		void GetStickCenterRectangle(RECT *rect);
		bool StickFingerDown(void);
		void Reset(void);
		
		void PointerPressed(PointerPoint ^point);
		void PointerMoved(PointerPoint ^point);
		void PointerReleased(PointerPoint ^point);

		void UpdateVirtualControllerRectangles(void);

		void GetCrossRectangle(RECT *rect);
		void GetButtonsRectangle(RECT *rect);
		void GetStartSelectRectangle(RECT *rect);
		void GetLRectangle(RECT *rect);
		void GetRRectangle(RECT *rect);
		void Update(void);

	private:
		static VirtualControllerInput *instance;
		
		Map<unsigned int, Windows::UI::Input::PointerPoint ^> ^pointers;

		bool stickFingerDown;
		int stickFingerID;
		Windows::Foundation::Point stickPos;
		Windows::Foundation::Point stickOffset;
		Windows::Foundation::Point ptest;
		POINT visibleStickPos;
		POINT visibleStickOffset;
		Windows::Foundation::Rect stickBoundaries;

		CRITICAL_SECTION cs;
		EmulatorGame *emulator;
		ControllerState state;

		RECT padCrossRectangle;
		RECT startRectangle;
		RECT selectRectangle;
		RECT turboRectangle;
		RECT comboRectangle;
		RECT aRectangle;
		RECT bRectangle;
		RECT lRectangle;
		RECT rRectangle;

		Windows::Foundation::Rect leftRect;
		Windows::Foundation::Rect upRect;
		Windows::Foundation::Rect rightRect;
		Windows::Foundation::Rect downRect;
		Windows::Foundation::Rect startRect;
		Windows::Foundation::Rect selectRect;
		Windows::Foundation::Rect turboRect;
		Windows::Foundation::Rect comboRect;
		Windows::Foundation::Rect lRect;
		Windows::Foundation::Rect rRect;
		Windows::Foundation::Rect aRect;
		Windows::Foundation::Rect bRect;
		

		int padCenterX;
		int padCenterY;
		int aLeft;
		int aTop;
		int bLeft;
		int bTop;
		int startLeft;
		int startTop;
		int selectRight;
		int selectTop;
		int turboLeft;
		int turboTop;
		int comboLeft;
		int comboTop;
		int lLeft;
		int lTop;
		int rRight;
		int rTop;

		float hscale;

		void CreateRectangleOnTheLeft(Windows::Foundation::Rect *rect, int x, int y, int width, int height, float scale);
		void CreateRectangleOnTheRight(Windows::Foundation::Rect *rect, int x, int y, int width, int height, float scale);
		void CreateRectangleCenter(Windows::Foundation::Rect *rect, int x, int y, int width, int height, float scale);
		void CreateTouchRectangleOnTheLeft(Windows::Foundation::Rect *rect, int x, int y, int width, int height, float scale);
		void CreateTouchRectangleOnTheRight(Windows::Foundation::Rect *rect, int x, int y, int width, int height, float scale);
		void CreateTouchRectangleCenter(Windows::Foundation::Rect *rect, int x, int y, int width, int height, float scale);

		void CreateRenderRectangles(void);
		void CreateTouchRectangles(void);

	};
}