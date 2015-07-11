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
		void GetARectangle(RECT *rect);
		void GetBRectangle(RECT *rect);
		void GetStartRectangle(RECT *rect);
		void GetSelectRectangle(RECT *rect);
		void GetTurboRectangle(RECT *rect);
		void GetComboRectangle(RECT *rect);
		void GetLRectangle(RECT *rect);
		void GetRRectangle(RECT *rect);
		void Update(void);

	private:
		static VirtualControllerInput *instance;
		float physicalWidth, physicalHeight; //in inch
		
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
		

		int PadLeft;
		int PadBottom;
		int ACenterX;
		int ACenterY;
		int BCenterX;
		int BCenterY;
		int startCenterX;
		int startBottom;
		int selectCenterX;
		int selectBottom;
		int TurboCenterX;
		int TurboCenterY;
		int ComboCenterX;
		int ComboCenterY;
		int lLeft;
		int LCenterY;
		int rRight;
		int RCenterY;

		float hscale, vscale;



		void CreateRenderRectangles(void);
		void CreateTouchRectangles(void);

	};
}