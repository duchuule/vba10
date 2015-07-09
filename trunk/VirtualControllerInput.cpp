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


		int dpad = EmulatorSettings::Current->DPadStyle;
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

		int dpad = EmulatorSettings::Current->DPadStyle;
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

		int dpad = EmulatorSettings::Current->DPadStyle;
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

	void VirtualControllerInput::CreateRenderRectangles(void)
	{
		float value = EmulatorSettings::Current->ControllerScale / 100.0f;
		float value2 = EmulatorSettings::Current->ButtonScale / 100.0f;

		EmulatorSettings ^settings = EmulatorSettings::Current;

		//get setting info
		if (this->emulator->GetHeight() > this->emulator->GetWidth())  //portrait
		{
			padCenterX = settings->PadCenterXP * this->emulator->GetWidth();
			padCenterY = settings->PadCenterYP * this->emulator->GetHeight();
			aLeft = settings->ALeftP * this->emulator->GetWidth();
			aTop = settings->ATopP * this->emulator->GetHeight();
			bLeft = settings->BLeftP * this->emulator->GetWidth();
			bTop = settings->BTopP * this->emulator->GetHeight();
			startLeft = settings->StartLeftP * this->emulator->GetWidth();
			startTop = settings->StartTopP * this->emulator->GetHeight();
			selectRight = settings->SelectRightP * this->emulator->GetWidth();
			selectTop = settings->SelectTopP * this->emulator->GetHeight();
			lLeft = settings->LLeftP * this->emulator->GetWidth();
			lTop = settings->LTopP * this->emulator->GetHeight();
			rRight = settings->RRightP * this->emulator->GetWidth();
			rTop = settings->RTopP * this->emulator->GetHeight();
			turboLeft = settings->TurboLeftP * this->emulator->GetWidth();
			turboTop = settings->TurboTopP * this->emulator->GetHeight();
			comboLeft = settings->ComboLeftP * this->emulator->GetWidth();
			comboTop = settings->ComboTopP * this->emulator->GetHeight();
		}
		else
		{
			padCenterX = settings->PadCenterXL * this->emulator->GetWidth();
			padCenterY = settings->PadCenterYL * this->emulator->GetHeight();
			aLeft = settings->ALeftL * this->emulator->GetWidth();
			aTop = settings->ATopL * this->emulator->GetHeight();
			bLeft = settings->BLeftL * this->emulator->GetWidth();
			bTop = settings->BTopL * this->emulator->GetHeight();
			startLeft = settings->StartLeftL * this->emulator->GetWidth();
			startTop = settings->StartTopL * this->emulator->GetHeight();
			selectRight = settings->SelectRightL * this->emulator->GetWidth();
			selectTop = settings->SelectTopL * this->emulator->GetHeight();
			lLeft = settings->LLeftL * this->emulator->GetWidth();
			lTop = settings->LTopL * this->emulator->GetHeight();
			rRight = settings->RRightL * this->emulator->GetWidth();
			rTop = settings->RTopL * this->emulator->GetHeight();
			turboLeft = settings->TurboLeftL * this->emulator->GetWidth();
			turboTop = settings->TurboTopL * this->emulator->GetHeight();
			comboLeft = settings->ComboLeftL * this->emulator->GetWidth();
			comboTop = settings->ComboTopL * this->emulator->GetHeight();
		}

		this->hscale = Windows::Graphics::Display::DisplayInformation::GetForCurrentView()->RawPixelsPerViewPixel;

		// Visible Rectangles
		this->padCrossRectangle.left = padCenterX - 105 * value * this->hscale;
		this->padCrossRectangle.right = padCenterX + 105 * value * this->hscale;
		this->padCrossRectangle.top = padCenterY - 105 * value * this->hscale;
		this->padCrossRectangle.bottom = padCenterY + 105 * value * this->hscale;

		this->aRectangle.left = aLeft;
		this->aRectangle.right = this->aRectangle.left + 120 * value2 * this->hscale;
		this->aRectangle.top = aTop;
		this->aRectangle.bottom = this->aRectangle.top + 120 * value2 * this->hscale;

		this->bRectangle.left = bLeft;
		this->bRectangle.right = this->bRectangle.left + 120 * value2 * this->hscale;
		this->bRectangle.top = bTop;
		this->bRectangle.bottom = this->bRectangle.top + 120 * value2 * this->hscale;


		this->startRectangle.left = startLeft;
		this->startRectangle.right = this->startRectangle.left + 100 * value2 * this->hscale;
		this->startRectangle.top = startTop;
		this->startRectangle.bottom = this->startRectangle.top + 50 * value2 * this->hscale;

		this->selectRectangle.right = selectRight;
		this->selectRectangle.left = this->selectRectangle.right - 100 * value2 * this->hscale;
		this->selectRectangle.top = selectTop;
		this->selectRectangle.bottom = this->selectRectangle.top + 50 * value2 * this->hscale;

		this->turboRectangle.left = turboLeft;
		this->turboRectangle.right = this->turboRectangle.left + 50 * value2 * this->hscale;
		this->turboRectangle.top = turboTop;
		this->turboRectangle.bottom = this->turboRectangle.top + 50 * value2 * this->hscale;

		this->comboRectangle.left = comboLeft;
		this->comboRectangle.right = this->comboRectangle.left + 50 * value2 * this->hscale;
		this->comboRectangle.top = comboTop;
		this->comboRectangle.bottom = this->comboRectangle.top + 50 * value2 * this->hscale;

		this->lRectangle.left = lLeft;
		this->lRectangle.right = this->lRectangle.left + 90 * value2 * this->hscale;
		this->lRectangle.top = lTop;
		this->lRectangle.bottom = this->lRectangle.top + 53 * value2 * this->hscale;


		this->rRectangle.right = rRight;
		this->rRectangle.left = this->rRectangle.right - 90 * value2 * this->hscale;
		this->rRectangle.top = rTop;
		this->rRectangle.bottom = this->rRectangle.top + 53 * value2 * this->hscale;



		this->visibleStickPos.x = (LONG)((this->padCrossRectangle.right + this->padCrossRectangle.left) / 2.0f);
		this->visibleStickPos.y = (LONG)((this->padCrossRectangle.top + this->padCrossRectangle.bottom) / 2.0f);

		this->visibleStickOffset.x = 0;
		this->visibleStickOffset.y = 0;
	}


	void VirtualControllerInput::CreateTouchRectangles(void)
	{
		//origin at top left corner
		EmulatorSettings ^settings = EmulatorSettings::Current;
		float touchVisualQuotient = Windows::Graphics::Display::DisplayInformation::GetForCurrentView()->RawPixelsPerViewPixel;


		this->lRect.Y = this->lRectangle.top / touchVisualQuotient;
		this->lRect.X = this->lRectangle.left / touchVisualQuotient;
		this->lRect.Height = (this->lRectangle.bottom - this->lRectangle.top) / touchVisualQuotient;
		this->lRect.Width = (this->lRectangle.right - this->lRectangle.left) / touchVisualQuotient;

		this->rRect.Y = this->rRectangle.top / touchVisualQuotient;
		this->rRect.X = this->rRectangle.left / touchVisualQuotient;
		this->rRect.Height = (this->rRectangle.bottom - this->rRectangle.top) / touchVisualQuotient;
		this->rRect.Width = (this->rRectangle.right - this->rRectangle.left) / touchVisualQuotient;

		// Cross		
		this->leftRect.X = this->padCrossRectangle.left / touchVisualQuotient;
		this->leftRect.Y = this->padCrossRectangle.top / touchVisualQuotient;
		this->leftRect.Width = ((this->padCrossRectangle.right - this->padCrossRectangle.left) / 3.0f) / touchVisualQuotient;
		this->leftRect.Height = (this->padCrossRectangle.bottom - this->padCrossRectangle.top) / touchVisualQuotient;

		this->rightRect.Width = ((this->padCrossRectangle.right - this->padCrossRectangle.left) / 3.0f) / touchVisualQuotient;
		this->rightRect.Y = this->padCrossRectangle.top / touchVisualQuotient;

		this->rightRect.Height = (this->padCrossRectangle.bottom - this->padCrossRectangle.top) / touchVisualQuotient;
		this->rightRect.X = (this->padCrossRectangle.left / touchVisualQuotient) + 2.0f * this->rightRect.Width;


		this->upRect.Y = this->padCrossRectangle.top / touchVisualQuotient;
		this->upRect.X = this->padCrossRectangle.left / touchVisualQuotient;
		this->upRect.Height = ((this->padCrossRectangle.bottom - this->padCrossRectangle.top) / 3.0f) / touchVisualQuotient;
		this->upRect.Width = (this->padCrossRectangle.right - this->padCrossRectangle.left) / touchVisualQuotient;

		this->downRect.Height = ((this->padCrossRectangle.bottom - this->padCrossRectangle.top) / 3.0f) / touchVisualQuotient;
		this->downRect.Y = (this->padCrossRectangle.top / touchVisualQuotient) + 2.0f * this->downRect.Height;
		this->downRect.X = this->padCrossRectangle.left / touchVisualQuotient;
		this->downRect.Width = (this->padCrossRectangle.right - this->padCrossRectangle.left) / touchVisualQuotient;

		// Buttons

		this->aRect.Y = this->aRectangle.top / touchVisualQuotient;
		this->aRect.X = this->aRectangle.left / touchVisualQuotient;
		this->aRect.Height = (this->aRectangle.bottom - this->aRectangle.top) / touchVisualQuotient;
		this->aRect.Width = (this->aRectangle.right - this->aRectangle.left) / touchVisualQuotient;

		this->bRect.Y = this->bRectangle.top / touchVisualQuotient;
		this->bRect.X = this->bRectangle.left / touchVisualQuotient;
		this->bRect.Height = (this->bRectangle.bottom - this->bRectangle.top) / touchVisualQuotient;
		this->bRect.Width = (this->bRectangle.right - this->bRectangle.left) / touchVisualQuotient;

		this->selectRect.Y = this->selectRectangle.top / touchVisualQuotient;
		this->selectRect.X = this->selectRectangle.left / touchVisualQuotient;
		this->selectRect.Height = (this->selectRectangle.bottom - this->selectRectangle.top) / touchVisualQuotient;
		this->selectRect.Width = (this->selectRectangle.right - this->selectRectangle.left) / touchVisualQuotient;

		this->startRect.Y = this->startRectangle.top / touchVisualQuotient;
		this->startRect.X = this->startRectangle.left / touchVisualQuotient;
		this->startRect.Height = (this->startRectangle.bottom - this->startRectangle.top) / touchVisualQuotient;
		this->startRect.Width = (this->startRectangle.right - this->startRectangle.left) / touchVisualQuotient;

		this->turboRect.Y = this->turboRectangle.top / touchVisualQuotient;
		this->turboRect.X = this->turboRectangle.left / touchVisualQuotient;
		this->turboRect.Height = (this->turboRectangle.bottom - this->turboRectangle.top) / touchVisualQuotient;
		this->turboRect.Width = (this->turboRectangle.right - this->turboRectangle.left) / touchVisualQuotient;

		this->comboRect.Y = this->comboRectangle.top / touchVisualQuotient;
		this->comboRect.X = this->comboRectangle.left / touchVisualQuotient;
		this->comboRect.Height = (this->comboRectangle.bottom - this->comboRectangle.top) / touchVisualQuotient;
		this->comboRect.Width = (this->comboRectangle.right - this->comboRectangle.left) / touchVisualQuotient;

		int dpad = EmulatorSettings::Current->DPadStyle;

		this->stickBoundaries.X = this->padCrossRectangle.left / touchVisualQuotient;
		this->stickBoundaries.Y = this->padCrossRectangle.top / touchVisualQuotient;
		this->stickBoundaries.Width = (this->padCrossRectangle.right - this->padCrossRectangle.left) / touchVisualQuotient;
		this->stickBoundaries.Height = (this->padCrossRectangle.bottom - this->padCrossRectangle.top) / touchVisualQuotient;

		if (dpad >= 2)
		{
			this->stickPos.X = this->stickBoundaries.X + this->stickBoundaries.Width / 2.0f;
			this->stickPos.Y = this->stickBoundaries.Y + this->stickBoundaries.Height / 2.0f;

			this->stickOffset.X = 0.0f;
			this->stickOffset.Y = 0.0f;

		}
	}



	void VirtualControllerInput::UpdateVirtualControllerRectangles(void)
	{

		CreateRenderRectangles();
		CreateTouchRectangles();
	}

	void VirtualControllerInput::GetCrossRectangle(RECT *rect)
	{
		*rect = this->padCrossRectangle;
	}

	void VirtualControllerInput::GetButtonsRectangle(RECT *rect)
	{
		*rect = this->aRectangle;
	}

	void VirtualControllerInput::GetStartSelectRectangle(RECT *rect)
	{
		*rect = this->startRectangle;
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

		int dpad = EmulatorSettings::Current->DPadStyle;

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
					float controllerScale = Windows::Graphics::Display::DisplayInformation::GetForCurrentView()->RawPixelsPerViewPixel;
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