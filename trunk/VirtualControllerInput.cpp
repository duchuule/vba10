#include "VirtualControllerInput.h"
#include "Emulator.h"
#include "EmulatorSettings.h"
#include <math.h>

//these positions are based on 1920x1080
#define CROSS_RECT_X		20   //distance from left side of screen to left side of button
#define CROSS_RECT_Y		100  //distance from top side of screen to top side of button
#define CROSS_RECT_WIDTH	230
#define CROSS_RECT_HEIGHT	230
#define BUTTONS_RECT_X		20 //distance from right side of screen to right side of button
#define BUTTONS_RECT_Y		84
#define BUTTONS_RECT_WIDTH	280
#define BUTTONS_RECT_HEIGHT	280
#define SS_RECT_X			0  //distance from center of screen to center of button
#define SS_RECT_Y			100
#define SS_RECT_WIDTH		400
#define SS_RECT_HEIGHT		70
#define L_RECT_X			0
#define L_RECT_Y			380
#define L_RECT_WIDTH		120
#define L_RECT_HEIGHT		70
#define R_RECT_X			L_RECT_X
#define R_RECT_Y			L_RECT_Y
#define R_RECT_WIDTH		L_RECT_WIDTH
#define R_RECT_HEIGHT		L_RECT_HEIGHT

#define CROSS_TOUCH_OVERLAP	0
#define CROSS_TOUCH_WIDTH	77 // vorher 80
#define BUTTONS_TOUCH_OVERLAP	0
#define BUTTONS_TOUCH_WIDTH	93	// vorher 85
#define START_TOUCH_WIDTH	200
#define SELECT_TOUCH_WIDTH	100
#define VCONTROLLER_Y_OFFSET 550

#define LEFT_RECT_X			CROSS_RECT_X - CROSS_TOUCH_OVERLAP
#define LEFT_RECT_Y			CROSS_RECT_Y - CROSS_TOUCH_OVERLAP
#define LEFT_RECT_WIDTH		CROSS_TOUCH_WIDTH + CROSS_TOUCH_OVERLAP
#define LEFT_RECT_HEIGHT	CROSS_RECT_HEIGHT + CROSS_TOUCH_OVERLAP * 2
#define UP_RECT_X			CROSS_RECT_X - CROSS_TOUCH_OVERLAP
#define UP_RECT_Y			CROSS_RECT_Y + CROSS_RECT_HEIGHT - CROSS_TOUCH_WIDTH
#define UP_RECT_WIDTH		CROSS_RECT_WIDTH + CROSS_TOUCH_OVERLAP * 2
#define UP_RECT_HEIGHT		CROSS_TOUCH_WIDTH + CROSS_TOUCH_OVERLAP
#define RIGHT_RECT_X		CROSS_RECT_X + CROSS_RECT_WIDTH - CROSS_TOUCH_WIDTH
#define RIGHT_RECT_Y		CROSS_RECT_Y - CROSS_TOUCH_OVERLAP
#define RIGHT_RECT_WIDTH	CROSS_TOUCH_WIDTH + CROSS_TOUCH_OVERLAP
#define RIGHT_RECT_HEIGHT	CROSS_RECT_HEIGHT + CROSS_TOUCH_OVERLAP * 2
#define DOWN_RECT_X			CROSS_RECT_X - CROSS_TOUCH_OVERLAP
#define DOWN_RECT_Y			CROSS_RECT_Y - CROSS_TOUCH_OVERLAP
#define DOWN_RECT_WIDTH		CROSS_RECT_WIDTH + CROSS_TOUCH_OVERLAP * 2
#define DOWN_RECT_HEIGHT	CROSS_TOUCH_WIDTH + CROSS_TOUCH_OVERLAP

#define SELECT_RECT_X		SS_RECT_X - SS_RECT_WIDTH / 4 
#define SELECT_RECT_Y		SS_RECT_Y
#define SELECT_RECT_WIDTH	SELECT_TOUCH_WIDTH
#define SELECT_RECT_HEIGHT	SS_RECT_HEIGHT
#define START_RECT_X		SS_RECT_X + SS_RECT_WIDTH / 4 
#define START_RECT_Y		SS_RECT_Y
#define START_RECT_WIDTH	START_TOUCH_WIDTH
#define START_RECT_HEIGHT	SS_RECT_HEIGHT

#define A_RECT_X_I			BUTTONS_RECT_X
#define A_RECT_Y_I			BUTTONS_RECT_Y + (BUTTONS_RECT_HEIGHT / 2)
#define A_RECT_WIDTH_I		BUTTONS_RECT_WIDTH / 2
#define A_RECT_HEIGHT_I		BUTTONS_RECT_HEIGHT / 2
#define B_RECT_X_I			BUTTONS_RECT_X + (BUTTONS_RECT_WIDTH / 2)
#define B_RECT_Y_I			BUTTONS_RECT_Y
#define B_RECT_WIDTH_I		BUTTONS_RECT_WIDTH / 2
#define B_RECT_HEIGHT_I		BUTTONS_RECT_HEIGHT / 2
//#define Y_RECT_X			BUTTONS_RECT_X + BUTTONS_RECT_WIDTH - BUTTONS_TOUCH_WIDTH
//#define Y_RECT_Y			BUTTONS_RECT_Y - CROSS_TOUCH_OVERLAP
//#define Y_RECT_WIDTH		BUTTONS_TOUCH_WIDTH + CROSS_TOUCH_OVERLAP
//#define Y_RECT_HEIGHT		BUTTONS_RECT_HEIGHT + CROSS_TOUCH_OVERLAP * 2
//#define X_RECT_X			BUTTONS_RECT_X - CROSS_TOUCH_OVERLAP
//#define X_RECT_Y			BUTTONS_RECT_Y + BUTTONS_RECT_HEIGHT - BUTTONS_TOUCH_WIDTH
//#define X_RECT_WIDTH		BUTTONS_RECT_WIDTH + CROSS_TOUCH_OVERLAP * 2
//#define X_RECT_HEIGHT		BUTTONS_TOUCH_WIDTH + CROSS_TOUCH_OVERLAP

//#define A_RECT_X_I			BUTTONS_RECT_X - CROSS_TOUCH_OVERLAP
//#define A_RECT_Y_I			BUTTONS_RECT_Y + BUTTONS_TOUCH_WIDTH
//#define A_RECT_WIDTH_I		BUTTONS_TOUCH_WIDTH + CROSS_TOUCH_OVERLAP
//#define A_RECT_HEIGHT_I		BUTTONS_TOUCH_WIDTH
//#define Y_RECT_X_I			BUTTONS_RECT_X + BUTTONS_RECT_WIDTH - BUTTONS_TOUCH_WIDTH
//#define Y_RECT_Y_I			BUTTONS_RECT_Y + BUTTONS_TOUCH_WIDTH
//#define Y_RECT_WIDTH_I		BUTTONS_TOUCH_WIDTH + CROSS_TOUCH_OVERLAP
//#define Y_RECT_HEIGHT_I		BUTTONS_TOUCH_WIDTH
//#define X_RECT_X_I			BUTTONS_RECT_X + BUTTONS_TOUCH_WIDTH
//#define X_RECT_Y_I			BUTTONS_RECT_Y + BUTTONS_RECT_HEIGHT - BUTTONS_TOUCH_WIDTH
//#define X_RECT_WIDTH_I		BUTTONS_TOUCH_WIDTH
//#define X_RECT_HEIGHT_I		BUTTONS_TOUCH_WIDTH + CROSS_TOUCH_OVERLAP
//#define B_RECT_X_I			BUTTONS_RECT_X + BUTTONS_TOUCH_WIDTH
//#define B_RECT_Y_I			BUTTONS_RECT_Y - CROSS_TOUCH_OVERLAP
//#define B_RECT_WIDTH_I		BUTTONS_TOUCH_WIDTH
//#define B_RECT_HEIGHT_I		BUTTONS_TOUCH_WIDTH + CROSS_TOUCH_OVERLAP

using namespace Windows::Foundation;

namespace VBA10
{
	VirtualControllerInput *VirtualControllerInput::instance = nullptr;

	VirtualControllerInput *VirtualControllerInput::GetInstance(void)
	{
		return instance;
	}

	VirtualControllerInput::VirtualControllerInput(void)
		: emulator(EmulatorGame::GetInstance()), 
		pointers(ref new Map<unsigned int, PointerPoint ^>()), 
		stickFingerDown(false)
	{ 
		ZeroMemory(&state, sizeof(ControllerState));
		InitializeCriticalSectionEx(&this->cs, 0, 0);
		this->pointers = ref new Platform::Collections::Map<unsigned int, PointerPoint ^>();
		instance = this;
	}

	VirtualControllerInput::~VirtualControllerInput(void)
	{
		DeleteCriticalSection(&this->cs);
	}

	void VirtualControllerInput::Reset(void)
	{
		this->stickFingerDown = false;
		this->stickFingerID = 0;

		this->stickOffset.X = 0;
		this->stickOffset.Y = 0;

		this->visibleStickOffset.x = 0;
		this->visibleStickOffset.y = 0;

		this->pointers->Clear();
	}

	const ControllerState *VirtualControllerInput::GetControllerState(void)
	{
		return &this->state;
	}

	void VirtualControllerInput::PointerPressed(PointerPoint ^point)
	{
		EnterCriticalSection(&this->cs);
		this->pointers->Insert(point->PointerId, point);
		LeaveCriticalSection(&this->cs);


		int dpad = GetDPadStyle();
		if(dpad >= 1)
		{
			Windows::Foundation::Point p = point->Position;
			this->ptest = p;

			if(this->stickBoundaries.Contains(p) && !stickFingerDown && !this->lRect.Contains(p))
			{
				float scale = (int) Windows::Graphics::Display::DisplayProperties::ResolutionScale / 100.0f;
				if(dpad == 2)
				{
					this->stickPos = p;
				}
				if(dpad == 2)
				{
					this->visibleStickPos.x = this->stickPos.X * scale;
					this->visibleStickPos.y = this->stickPos.Y * scale;
				}

				this->stickFingerID = point->PointerId;
				this->stickFingerDown = true;

				this->stickOffset.X = p.X - this->stickPos.X;
				this->stickOffset.Y = p.Y - this->stickPos.Y;

				this->visibleStickOffset.x = this->stickOffset.X * scale;
				this->visibleStickOffset.y = this->stickOffset.Y * scale;
			}
		}
	}

	void VirtualControllerInput::PointerMoved(PointerPoint ^point)
	{
		EnterCriticalSection(&this->cs);
		if(this->pointers->HasKey(point->PointerId))
		{
			this->pointers->Insert(point->PointerId, point);
		}
		LeaveCriticalSection(&this->cs);

		int dpad = GetDPadStyle();
		if(dpad >= 1)
		{
			if(this->stickFingerDown && point->PointerId == this->stickFingerID)
			{
				Windows::Foundation::Point p = point->Position;

				float scale = (int) Windows::Graphics::Display::DisplayProperties::ResolutionScale / 100.0f;

				stickOffset.X = p.X - this->stickPos.X;
				stickOffset.Y = p.Y - this->stickPos.Y;

				this->visibleStickOffset.x = this->stickOffset.X * scale;
				this->visibleStickOffset.y = this->stickOffset.Y * scale;
			}
		}
	}

	void VirtualControllerInput::PointerReleased(PointerPoint ^point)
	{
		EnterCriticalSection(&this->cs);
		if(this->pointers->HasKey(point->PointerId))
		{
			this->pointers->Remove(point->PointerId);
		}
		LeaveCriticalSection(&this->cs);

		int dpad = GetDPadStyle();
		if(dpad >= 1)
		{
			if(this->stickFingerDown && point->PointerId == this->stickFingerID)
			{
				this->stickFingerDown = false;
				this->stickFingerID = 0;

				this->stickOffset.X = 0;
				this->stickOffset.Y = 0;

				this->visibleStickOffset.x = 0;
				this->visibleStickOffset.y = 0;
			}
		}
	}

	void VirtualControllerInput::UpdateVirtualControllerRectangles(void)
	{
		int yOffset = 0;
		if(IsTouchControllerOnTop())
		{
			yOffset = VCONTROLLER_Y_OFFSET;
		}
		// 1920x1080 as reference value
		float resolutionScale = sqrt( this->emulator->GetHeight() / 1080.0f * this->emulator->GetWidth() / 1920.0f);

		yOffset *= resolutionScale;
		Windows::Foundation::Rect tmp;

		float controllerScale = GetControllerScaling() / 100.0f;

		this->CreateRectangleOnTheLeft(&tmp, CROSS_RECT_X, CROSS_RECT_Y, CROSS_RECT_WIDTH, CROSS_RECT_HEIGHT, resolutionScale);
		tmp.Width *= controllerScale;
		if(!IsTouchControllerOnTop())
		{
			tmp.Y += (1.0f - controllerScale) * tmp.Height;
		}
		tmp.Height *= controllerScale;

		//this->padCrossRectangle = RECT((int)tmp.X, (int)tmp.Y, (int)(tmp.X + tmp.Width), (int)(tmp.Y + tmp.Height));
		this->padCrossRectangle.left = (int) tmp.X;
		this->padCrossRectangle.top = (int) tmp.Y;
		this->padCrossRectangle.right = (int)(tmp.X + tmp.Width);
		this->padCrossRectangle.bottom = (int)(tmp.Y + tmp.Height);
		this->padCrossRectangle.top -= yOffset;
		this->padCrossRectangle.bottom -= yOffset;

		this->CreateRectangleCenter(&tmp, SS_RECT_X, SS_RECT_Y, SS_RECT_WIDTH, SS_RECT_HEIGHT, resolutionScale);

		tmp.Y += (1.0f - controllerScale) * (tmp.Height / 2.0f);
		tmp.Height *= controllerScale;
		tmp.X += (1.0f - controllerScale) * (tmp.Width / 2.0f);
		tmp.Width *= controllerScale;

		//this->startSelectRectangle = Engine::Rectangle((int)tmp.X, (int)tmp.Y, (int)tmp.Width, (int)tmp.Height);
		this->startSelectRectangle.left = (int) (tmp.X);
		this->startSelectRectangle.top = (int) tmp.Y;
		this->startSelectRectangle.right = (int)(tmp.X + tmp.Width);
		this->startSelectRectangle.bottom = (int)(tmp.Y + tmp.Height);
		//this->startSelectRectangle.Y -= yOffset;

		this->CreateRectangleOnTheRight(&tmp, BUTTONS_RECT_X, BUTTONS_RECT_Y, BUTTONS_RECT_WIDTH, BUTTONS_RECT_HEIGHT, resolutionScale);

		tmp.X += (1.0f - controllerScale) * tmp.Width;
		tmp.Width *= controllerScale;
		if(!IsTouchControllerOnTop())
		{
			tmp.Y += (1.0f - controllerScale) * tmp.Height;
		}
		tmp.Height *= controllerScale;

		/*this->buttonsRectangle = Engine::Rectangle((int)tmp.X, (int)tmp.Y, (int)tmp.Width, (int)tmp.Height);
		this->buttonsRectangle.Y -= yOffset;*/
		this->buttonsRectangle.left = (int) tmp.X;
		this->buttonsRectangle.top = (int) tmp.Y;
		this->buttonsRectangle.right = (int)(tmp.X + tmp.Width);
		this->buttonsRectangle.bottom = (int)(tmp.Y + tmp.Height);
		this->buttonsRectangle.top -= yOffset;
		this->buttonsRectangle.bottom -= yOffset;

		this->CreateRectangleOnTheLeft(&tmp, L_RECT_X, L_RECT_Y, L_RECT_WIDTH, L_RECT_HEIGHT, resolutionScale);

		tmp.Width *= controllerScale;
		if(!IsTouchControllerOnTop())
		{
			tmp.Y += (1.0f - controllerScale) * tmp.Height;
		}
		tmp.Height *= controllerScale;

		/*this->lRectangle = Engine::Rectangle((int)tmp.X, (int)tmp.Y, (int)tmp.Width, (int)tmp.Height);
		this->lRectangle.Y += (int)(yOffset * 0.7f);*/
		this->lRectangle.left = (int) tmp.X;
		this->lRectangle.top = (int) tmp.Y;
		this->lRectangle.right = (int)(tmp.X + tmp.Width);
		this->lRectangle.bottom = (int)(tmp.Y + tmp.Height);
		this->lRectangle.top -= yOffset;
		this->lRectangle.bottom -= yOffset;

		this->CreateRectangleOnTheRight(&tmp, R_RECT_X, R_RECT_Y, R_RECT_WIDTH, R_RECT_HEIGHT, resolutionScale);

		tmp.X += (1.0f - controllerScale) * tmp.Width;
		tmp.Width *= controllerScale;
		if(!IsTouchControllerOnTop())
		{
			tmp.Y += (1.0f - controllerScale) * tmp.Height;
		}
		tmp.Height *= controllerScale;

		/*this->rRectangle = Engine::Rectangle((int)tmp.X, (int)tmp.Y, (int)tmp.Width, (int)tmp.Height);
		this->rRectangle.Y += (int)(yOffset * 0.7f);*/
		this->rRectangle.left = (int) tmp.X;
		this->rRectangle.top = (int) tmp.Y;
		this->rRectangle.right = (int)(tmp.X + tmp.Width);
		this->rRectangle.bottom = (int)(tmp.Y + tmp.Height);
		this->rRectangle.top -= yOffset;
		this->rRectangle.bottom -= yOffset;

		float scale = (int)Windows::Graphics::Display::DisplayProperties::ResolutionScale / 100.0f;

		yOffset = 0;
		if(IsTouchControllerOnTop())
		{
			yOffset = VCONTROLLER_Y_OFFSET;
		}
		// 1920x1080 as reference value
		resolutionScale = sqrt(this->emulator->GetHeight() / scale / 1080.0f * this->emulator->GetWidth() / scale / 1920.0f);

		yOffset *= resolutionScale;

		this->CreateTouchRectangleOnTheLeft(&this->leftRect, LEFT_RECT_X, LEFT_RECT_Y, LEFT_RECT_WIDTH, LEFT_RECT_HEIGHT, resolutionScale);
		this->leftRect.Width *= controllerScale;
		if(!IsTouchControllerOnTop())
		{
			this->leftRect.Y += (1.0f - controllerScale) * (this->leftRect.Height);
		}
		this->leftRect.Height *= controllerScale;

		this->CreateTouchRectangleOnTheLeft(&this->rightRect, RIGHT_RECT_X, RIGHT_RECT_Y, RIGHT_RECT_WIDTH, RIGHT_RECT_HEIGHT, resolutionScale);
		this->rightRect.X = this->leftRect.X + 2.0f * this->leftRect.Width;
		this->rightRect.Width = this->leftRect.Width;
		this->rightRect.Y = this->leftRect.Y;
		this->rightRect.Height = this->leftRect.Height;
		
		this->CreateTouchRectangleOnTheLeft(&this->upRect, UP_RECT_X, UP_RECT_Y, UP_RECT_WIDTH, UP_RECT_HEIGHT, resolutionScale);
		this->CreateTouchRectangleOnTheLeft(&this->downRect, DOWN_RECT_X, DOWN_RECT_Y, DOWN_RECT_WIDTH, DOWN_RECT_HEIGHT, resolutionScale);
		if(IsTouchControllerOnTop())
		{
			this->upRect.Width *= controllerScale;
			this->upRect.Height *= controllerScale;
			this->downRect.Y = this->upRect.Y + 2.0f * this->upRect.Height;
			this->downRect.Height = this->upRect.Height;
			this->downRect.Width = this->upRect.Width;
		}else
		{
			this->downRect.Y += (1.0f - controllerScale) * this->downRect.Height;
			this->downRect.Height *= controllerScale;
			this->downRect.Width *= controllerScale;
			this->upRect.Y = this->downRect.Y - 2.0f * this->downRect.Height;
			this->upRect.Height = this->downRect.Height;
			this->upRect.Width = this->downRect.Width;
		}

		this->CreateTouchRectangleCenter(&this->startRect, START_RECT_X, START_RECT_Y, START_RECT_WIDTH, START_RECT_HEIGHT, resolutionScale);
		this->startRect.Width *= controllerScale;
		this->startRect.Y += (1.0f - controllerScale) * (this->startRect.Height / 2.0f);
		this->startRect.Height *= controllerScale;
		

		this->CreateTouchRectangleCenter(&this->selectRect, SELECT_RECT_X, SELECT_RECT_Y, SELECT_RECT_WIDTH, SELECT_RECT_HEIGHT, resolutionScale);
		this->selectRect.X += (1.0f - controllerScale) * this->selectRect.Width;
		this->selectRect.Width *= controllerScale;
		this->selectRect.Y += (1.0f - controllerScale) * (this->selectRect.Height / 2.0f);
		this->selectRect.Height *= controllerScale;

		this->CreateTouchRectangleOnTheLeft(&this->lRect, L_RECT_X, L_RECT_Y, L_RECT_WIDTH, L_RECT_HEIGHT, resolutionScale);
		this->lRect.Width *= controllerScale;
		if(!IsTouchControllerOnTop())
		{
			this->lRect.Y += (1.0f - controllerScale) * this->lRect.Height;
		}
		this->lRect.Height *= controllerScale;

		this->CreateTouchRectangleOnTheRight(&this->rRect, R_RECT_X, R_RECT_Y, R_RECT_WIDTH, R_RECT_HEIGHT, resolutionScale);
		this->rRect.X += (1.0f - controllerScale) * this->rRect.Width;
		this->rRect.Width *= controllerScale;
		if(!IsTouchControllerOnTop())
		{
			this->rRect.Y += (1.0f - controllerScale) * this->rRect.Height;
		}
		this->rRect.Height *= controllerScale;

		this->CreateTouchRectangleOnTheRight(&this->aRect, A_RECT_X_I, A_RECT_Y_I, A_RECT_WIDTH_I, A_RECT_HEIGHT_I, resolutionScale);
		this->CreateTouchRectangleOnTheRight(&this->bRect, B_RECT_X_I, B_RECT_Y_I, B_RECT_WIDTH_I, B_RECT_HEIGHT_I, resolutionScale);

		if(IsTouchControllerOnTop())
		{
			this->aRect.X += (1.0f - controllerScale) * this->aRect.Width;
			this->aRect.Width *= controllerScale;
			this->aRect.Height *= controllerScale;

			this->bRect.X = this->aRect.X - this->aRect.Width;
			this->bRect.Width = this->aRect.Width;
			this->bRect.Y = this->aRect.Y + this->aRect.Height;
			this->bRect.Height = this->aRect.Height;
		}else
		{
			this->aRect.X += (1.0f - controllerScale) * this->aRect.Width;
			this->aRect.Width *= controllerScale;

			this->bRect.X = this->aRect.X - this->aRect.Width;
			this->bRect.Width = this->aRect.Width;
			this->bRect.Y += (1.0f - controllerScale) * this->bRect.Height;
			this->bRect.Height *= controllerScale;

			this->aRect.Y = this->bRect.Y - this->bRect.Height;
			this->aRect.Height = this->bRect.Height;
		}

		this->leftRect.Y -= yOffset;
		this->rightRect.Y -= yOffset;
		this->downRect.Y -= yOffset;
		this->upRect.Y -= yOffset;
		//this->startRect.Y -= yOffset;
		//this->selectRect.Y -= yOffset;
		this->lRect.Y -= yOffset;
		this->rRect.Y -= yOffset;
		this->aRect.Y -= yOffset;
		this->bRect.Y -= yOffset;	

		this->visibleStickPos.x = (LONG) (this->padCrossRectangle.right + this->padCrossRectangle.left) / 2.0f;
		this->visibleStickPos.y = (LONG) (this->padCrossRectangle.top + this->padCrossRectangle.bottom ) / 2.0f;

		this->visibleStickOffset.x = 0;
		this->visibleStickOffset.y = 0;
		
		int dpad = GetDPadStyle();
		if(dpad >= 1)
		{
			int touchOffset = this->leftRect.Width * 0.0f;
			this->stickPos.X = this->leftRect.X + this->leftRect.Width * 1.5f + touchOffset;
			this->stickPos.Y = this->leftRect.Y + this->leftRect.Height / 2.0f;

			this->stickOffset.X = 0.0f;
			this->stickOffset.Y = 0.0f;

			if(dpad == 1)
			{
				float controllerScale = GetControllerScaling() / 100.0f;
				this->stickBoundaries.X = 0;//this->leftRect.X + touchOffset;
				this->stickBoundaries.Y = this->leftRect.Y - 50.0f * controllerScale;
				this->stickBoundaries.Width = this->leftRect.Width * 3 + touchOffset + 75.0f * controllerScale;
				this->stickBoundaries.Height = this->leftRect.Height + 100.0f * controllerScale;
			}else
			{
				this->stickBoundaries.Y = 0;
				this->stickBoundaries.X = 0;
				this->stickBoundaries.Width = this->selectRect.X;
				this->stickBoundaries.Height = this->emulator->GetHeight();
			}
		}
	}

	void VirtualControllerInput::GetCrossRectangle(RECT *rect)
	{
		*rect = this->padCrossRectangle;
	}

	void VirtualControllerInput::GetButtonsRectangle(RECT *rect)
	{
		*rect = this->buttonsRectangle;
	}

	void VirtualControllerInput::GetStartSelectRectangle(RECT *rect)
	{
		*rect = this->startSelectRectangle;
	}

	void VirtualControllerInput::GetLRectangle(RECT *rect)
	{
		*rect = this->lRectangle;
	}

	void VirtualControllerInput::GetRRectangle(RECT *rect)
	{
		*rect = this->rRectangle;
	}

	void VirtualControllerInput::CreateRectangleOnTheLeft(Windows::Foundation::Rect *rect, int x, int y, int width, int height, float scale)
	{
		float scaledWidth = width * scale;
		float scaledHeight = height * scale;
		float scaledX = (x * scale);
		float scaledY = this->emulator->GetHeight() - scaledHeight - (y * scale);

		rect->X = scaledX;
		rect->Y = scaledY;
		rect->Width = scaledWidth;
		rect->Height = scaledHeight;
	}

	void VirtualControllerInput::CreateRectangleCenter(Windows::Foundation::Rect *rect, int x, int y, int width, int height, float scale)
	{
		float scaledWidth = width * scale;
		float scaledHeight = height * scale;
		float scaledX = this->emulator->GetWidth() /2.0  - scaledWidth / 2.0f + x * scale;
		float scaledY = this->emulator->GetHeight() - scaledHeight - (y * scale);

		rect->X = scaledX;
		rect->Y = scaledY;
		rect->Width = scaledWidth;
		rect->Height = scaledHeight;
	}

	void VirtualControllerInput::CreateRectangleOnTheRight(Windows::Foundation::Rect *rect, int x, int y, int width, int height, float scale)
	{
		float scaledWidth = width * scale;
		float scaledHeight = height * scale;
		float scaledX = this->emulator->GetWidth() - scaledWidth - (x * scale);
		float scaledY = this->emulator->GetHeight() - scaledHeight - (y * scale);

		rect->X = scaledX;
		rect->Y = scaledY;
		rect->Width = scaledWidth;
		rect->Height = scaledHeight;
	}



	void VirtualControllerInput::CreateTouchRectangleOnTheLeft(Windows::Foundation::Rect *rect, int x, int y, int width, int height, float scale)
	{
		float windowscale = (int)Windows::Graphics::Display::DisplayProperties::ResolutionScale / 100.0f;
		float scaledWidth = width * scale;
		float scaledHeight = height * scale;
		float scaledX = (x * scale);
		float scaledY = (this->emulator->GetHeight()  / windowscale) - scaledHeight - (y * scale);

		rect->X = scaledX;
		rect->Y = scaledY;
		rect->Width = scaledWidth;
		rect->Height = scaledHeight;
	}

	void VirtualControllerInput::CreateTouchRectangleCenter(Windows::Foundation::Rect *rect, int x, int y, int width, int height, float scale)
	{
		float windowscale = (int)Windows::Graphics::Display::DisplayProperties::ResolutionScale / 100.0f;
		float scaledWidth = width * scale;
		float scaledHeight = height * scale;
		float scaledX = this->emulator->GetWidth() / windowscale / 2.0 - scaledWidth / 2.0f + x * scale;
		float scaledY = (this->emulator->GetHeight() / windowscale) - scaledHeight - (y * scale);

		rect->X = scaledX;
		rect->Y = scaledY;
		rect->Width = scaledWidth;
		rect->Height = scaledHeight;
	}

	void VirtualControllerInput::CreateTouchRectangleOnTheRight(Windows::Foundation::Rect *rect, int x, int y, int width, int height, float scale)
	{
		float windowscale = (int)Windows::Graphics::Display::DisplayProperties::ResolutionScale / 100.0f;
		float scaledWidth = width * scale;
		float scaledHeight = height * scale;
		float scaledX = (this->emulator->GetWidth() / windowscale) - scaledWidth - (x * scale);
		float scaledY = (this->emulator->GetHeight() / windowscale) - scaledHeight - (y * scale);

		rect->X = scaledX;
		rect->Y = scaledY;
		rect->Width = scaledWidth;
		rect->Height = scaledHeight;
	}

	void VirtualControllerInput::Update(void)
	{
		ZeroMemory(&this->state, sizeof(ControllerState));

		int dpad = GetDPadStyle();

		EnterCriticalSection(&this->cs);
		for (auto i = this->pointers->First(); i->HasCurrent; i->MoveNext())
		{
			PointerPoint ^p = i->Current->Value;
			Windows::Foundation::Point point = Windows::Foundation::Point(p->Position.X, p->Position.Y);
			bool stickFinger = false;

			if(dpad == 0)
			{
				if(this->leftRect.Contains(point))
				{
					state.LeftPressed = true;
				}
				if(this->upRect.Contains(point))
				{
					state.UpPressed = true;
				}
				if(this->rightRect.Contains(point))
				{
					state.RightPressed = true;
				}
				if(this->downRect.Contains(point))
				{
					state.DownPressed = true;
				}
			}else
			{
				if(this->stickFingerDown && p->PointerId == this->stickFingerID)
				{
					stickFinger = true;
					float deadzone = GetDeadzone();
					float controllerScale = (GetControllerScaling() / 100.0f);
					float length = (float) sqrt(this->stickOffset.X * this->stickOffset.X + this->stickOffset.Y * this->stickOffset.Y);
					float scale = (int) Windows::Graphics::Display::DisplayProperties::ResolutionScale / 100.0f;
					if(length >= deadzone * scale * controllerScale)
					{
						// Deadzone of 15
						float unitX = 1.0f;
						float unitY = 0.0f;
						float normX = this->stickOffset.X / length;
						float normY = this->stickOffset.Y / length;

						float dot = unitX * normX + unitY * normY;
						float rad = (float) acos(dot);

						if(normY > 0.0f)
						{
							rad = 6.28f - rad;
						}

						/*rad = (rad + 3.14f / 2.0f);
						if(rad > 6.28f)
						{
							rad -= 6.28f;
						}*/

						if((rad >= 0 && rad < 1.046f) || (rad > 5.234f && rad < 6.28f))
						{
							state.RightPressed = true;
						}
						if(rad >= 0.523f && rad < 2.626f)
						{
							state.UpPressed = true;
						}
						if(rad >= 2.093f && rad < 4.186f)
						{
							state.LeftPressed = true;
						}
						if(rad >= 3.663f && rad < 5.756f)
						{
							state.DownPressed = true;
						}
					}
				}
			}

			if(!stickFinger)
			{
				if(this->startRect.Contains(point))
				{
					this->state.StartPressed = true;
				}
				if(this->selectRect.Contains(point))
				{
					this->state.SelectPressed = true;
				}
				if(this->lRect.Contains(point))
				{
					this->state.LPressed = true;
				}
				if(this->rRect.Contains(point))
				{
					this->state.RPressed = true;
				}
				if(this->aRect.Contains(point))
				{
					this->state.APressed = true;
				}
				if(this->bRect.Contains(point))
				{
					this->state.BPressed = true;
				}
				/*if(this->xRect.Contains(point))
				{
					this->state.XPressed = true;
				}
				if(this->yRect.Contains(point))
				{
					this->state.YPressed = true;
				}*/
			}
		}
		LeaveCriticalSection(&this->cs);
	}
	
	void VirtualControllerInput::GetStickRectangle(RECT *rect)
	{
		int quarterWidth = (this->padCrossRectangle.right - this->padCrossRectangle.left) / 4;
		int quarterHeight = (this->padCrossRectangle.bottom - this->padCrossRectangle.top) / 4;

		rect->left = (this->visibleStickPos.x + this->visibleStickOffset.x) - quarterWidth;
		rect->right = rect->left + 2 * quarterWidth;
		rect->top = (this->visibleStickPos.y + this->visibleStickOffset.y) - quarterHeight;
		rect->bottom = rect->top + 2 * quarterHeight;		
	}
	
	void VirtualControllerInput::GetStickCenterRectangle(RECT *rect)
	{
		int quarterWidth = (this->padCrossRectangle.right - this->padCrossRectangle.left) / 16;
		int quarterHeight = (this->padCrossRectangle.bottom - this->padCrossRectangle.top) / 16;

		rect->left = this->visibleStickPos.x - quarterWidth;
		rect->right = rect->left + 2 * quarterWidth;
		rect->top = this->visibleStickPos.y - quarterHeight;
		rect->bottom = rect->top + 2 * quarterHeight;
	}

	bool VirtualControllerInput::StickFingerDown(void)
	{
		return this->stickFingerDown;
	}
}