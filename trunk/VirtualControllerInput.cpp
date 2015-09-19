#include "VirtualControllerInput.h"
#include "Emulator.h"
#include "EmulatorSettings.h"
#include <math.h>


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
		stickFingerDown(false), isEditMode(false)
	{ 
		ZeroMemory(&state, sizeof(ControllerState));
		InitializeCriticalSectionEx(&this->cs, 0, 0);
		this->pointers = new map<unsigned int, PointerInfo*>();
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

		this->pointers->clear();
	}

	const ControllerState *VirtualControllerInput::GetControllerState(void)
	{
		return &this->state;
	}

	void VirtualControllerInput::PointerPressed(PointerPoint ^point)
	{
		EnterCriticalSection(&this->cs);

		PointerInfo* pinfo = new PointerInfo();
		pinfo->point = point;
		pinfo->description = "";
		pinfo->IsMoved = false;
		this->pointers->insert(pair<unsigned int, PointerInfo*>(point->PointerId, pinfo));

		LeaveCriticalSection(&this->cs);

		this->ptest = point->Position;

		if (!isEditMode)
		{
			int dpad = EmulatorSettings::Current->DPadStyle;
			if (dpad >= 1)
			{
				Windows::Foundation::Point p = point->Position;


				if (this->stickBoundaries.Contains(p) && !stickFingerDown && !this->lRect.Contains(p))
				{
					float scale = (int)Windows::Graphics::Display::DisplayProperties::ResolutionScale / 100.0f;
					if (dpad == 2)
					{
						this->stickPos = p;
					}
					if (dpad == 2)
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
	}

	void VirtualControllerInput::PointerMoved(PointerPoint ^point)
	{
		EnterCriticalSection(&this->cs);
		std::map<unsigned int, PointerInfo*>::iterator iter = this->pointers->find(point->PointerId);
		

		if (isEditMode)  //edit position mode
		{
			if (iter != this->pointers->end())  //update the current pointer position
			{
				PointerInfo* pinfo = iter->second;
				pinfo->IsMoved = true;


				//move the control
				float dx = 0;
				float dy = 0;
				float scale = (int)Windows::Graphics::Display::DisplayProperties::ResolutionScale / 100.0f;

				dx = (point->Position.X - pinfo->point->Position.X) * scale;
				dy = - (point->Position.Y - pinfo->point->Position.Y) * scale;

				if (pinfo->description == "joystick")
				{
					this->PadLeft += dx;
					this->PadBottom += dy;
				}
				else if (pinfo->description == "l")
				{
					this->lLeft += dx;
					this->LCenterY += dy;
				}
				else if (pinfo->description == "r")
				{
					this->rRight -= dx;
					this->RCenterY += dy;
				}
				else if (pinfo->description == "a")
				{
					this->ACenterX -= dx;
					this->ACenterY += dy;
				}
				else if (pinfo->description == "b")
				{
					this->BCenterX -= dx;
					this->BCenterY += dy;
				}
				else if (pinfo->description == "select")
				{
					this->selectCenterX += dx;
					this->selectBottom += dy;
				}
				else if (pinfo->description == "start")
				{
					this->startCenterX += dx;
					this->startBottom += dy;
				}
				else if (pinfo->description == "turbo")
				{
					this->TurboCenterX -= dx;
					this->TurboCenterY += dy;
				}
				else if (pinfo->description == "combo")
				{
					this->ComboCenterX -= dx;
					this->ComboCenterY += dy;
				}


				//record new touch position
				pinfo->point = point;

				//update controller position on screen
				this->CreateRenderRectangles();

				//update touh region on screen
				this->CreateTouchRectangles();
			}
		}
		else  //regular mode
		{
			if (iter != this->pointers->end())  //update the current pointer position
			{
				PointerInfo* pinfo = new PointerInfo();
				pinfo->point = point;
				pinfo->description = "";
				pinfo->IsMoved = false;

				iter->second = pinfo;
			}
			int dpad = EmulatorSettings::Current->DPadStyle;
			if (dpad >= 1)
			{
				if (this->stickFingerDown && point->PointerId == this->stickFingerID)
				{
					Windows::Foundation::Point p = point->Position;

					float scale = (int)Windows::Graphics::Display::DisplayProperties::ResolutionScale / 100.0f;

					stickOffset.X = p.X - this->stickPos.X;
					stickOffset.Y = p.Y - this->stickPos.Y;

					this->visibleStickOffset.x = this->stickOffset.X * scale;
					this->visibleStickOffset.y = this->stickOffset.Y * scale;
				}
			}
		}

		LeaveCriticalSection(&this->cs);
	}

	void VirtualControllerInput::PointerReleased(PointerPoint ^point)
	{
		EnterCriticalSection(&this->cs);


		std::map<unsigned int, PointerInfo*>::iterator iter = this->pointers->find(point->PointerId);
		if (iter != this->pointers->end())
		{
			this->pointers->erase(iter);
		}
		LeaveCriticalSection(&this->cs);

		if (!isEditMode)
		{
			int dpad = EmulatorSettings::Current->DPadStyle;
			if (dpad >= 1)
			{
				if (this->stickFingerDown && point->PointerId == this->stickFingerID)
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
	}


	void VirtualControllerInput::Update(void)
	{
		ZeroMemory(&this->state, sizeof(ControllerState));

		int dpad = EmulatorSettings::Current->DPadStyle;

		EnterCriticalSection(&this->cs);

		for (auto i = this->pointers->begin(); i != this->pointers->end(); i++)
		{
			PointerPoint ^p = i->second->point;
			Windows::Foundation::Point point = Windows::Foundation::Point(p->Position.X, p->Position.Y);
			bool stickFinger = false;

			if (isEditMode)
			{
				if (this->stickBoundaries.Contains(point))
				{
					i->second->description = "joystick";
				}
				if (this->startRect.Contains(point))
				{
					i->second->description = "start";
				}
				if (this->selectRect.Contains(point))
				{
					i->second->description = "select";
				}
				if (this->lRect.Contains(point))
				{
					i->second->description = "l";
				}
				if (this->rRect.Contains(point))
				{
					i->second->description = "r";
				}
				if (this->aRect.Contains(point))
				{
					i->second->description = "a";
				}
				if (this->bRect.Contains(point))
				{
					i->second->description = "b";
				}
				if (this->turboRect.Contains(point))
				{
					i->second->description = "turbo";
				}
				if (this->comboRect.Contains(point))
				{
					i->second->description = "combo";
				}
			}
			else
			{


				if (dpad == 0)
				{
					if (this->leftRect.Contains(point))
					{
						state.LeftPressed = true;
					}
					if (this->upRect.Contains(point))
					{
						state.UpPressed = true;
					}
					if (this->rightRect.Contains(point))
					{
						state.RightPressed = true;
					}
					if (this->downRect.Contains(point))
					{
						state.DownPressed = true;
					}
				}
				else
				{
					if (this->stickFingerDown && p->PointerId == this->stickFingerID)
					{
						stickFinger = true;
						float deadzone = GetDeadzone();
						float controllerScale = Windows::Graphics::Display::DisplayInformation::GetForCurrentView()->RawPixelsPerViewPixel;
						float length = (float)sqrt(this->stickOffset.X * this->stickOffset.X + this->stickOffset.Y * this->stickOffset.Y);
						float scale = (int)Windows::Graphics::Display::DisplayProperties::ResolutionScale / 100.0f;
						if (length >= deadzone * scale * controllerScale)
						{
							// Deadzone of 15
							float unitX = 1.0f;
							float unitY = 0.0f;
							float normX = this->stickOffset.X / length;
							float normY = this->stickOffset.Y / length;

							float dot = unitX * normX + unitY * normY;
							float rad = (float)acos(dot);

							if (normY > 0.0f)
							{
								rad = 6.28f - rad;
							}

							/*rad = (rad + 3.14f / 2.0f);
							if(rad > 6.28f)
							{
							rad -= 6.28f;
							}*/

							if ((rad >= 0 && rad < 1.046f) || (rad > 5.234f && rad < 6.28f))
							{
								state.RightPressed = true;
							}
							if (rad >= 0.523f && rad < 2.626f)
							{
								state.UpPressed = true;
							}
							if (rad >= 2.093f && rad < 4.186f)
							{
								state.LeftPressed = true;
							}
							if (rad >= 3.663f && rad < 5.756f)
							{
								state.DownPressed = true;
							}
						}
					}
				}

				if (!stickFinger)
				{
					if (this->startRect.Contains(point))
					{
						this->state.StartPressed = true;
					}
					if (this->selectRect.Contains(point))
					{
						this->state.SelectPressed = true;
					}
					if (this->lRect.Contains(point))
					{
						this->state.LPressed = true;
					}
					if (this->rRect.Contains(point))
					{
						this->state.RPressed = true;
					}
					if (this->aRect.Contains(point))
					{
						this->state.APressed = true;
					}
					if (this->bRect.Contains(point))
					{
						this->state.BPressed = true;
					}
					if (this->turboRect.Contains(point))
					{
						if (EmulatorSettings::Current->TurboBehavior == 0)
							this->state.TurboTogglePressed = true;
						else 
							this->state.TurboPressed = true;
					}
					if (this->comboRect.Contains(point))
					{
						this->state.APressed = true;
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

			}  //isEditMode == false
		}
		LeaveCriticalSection(&this->cs);
	}

	void VirtualControllerInput::SetControllerPositionFromSettings()
	{
		EmulatorSettings ^settings = EmulatorSettings::Current;



		//IMPORTANT: hscale used to be 1.0 on 480p device. Now hscale is 1.5 on 480p devices, which makes the device effectively 320p
		// so now all the number are based on 320p
		float rawDpiX = Windows::Graphics::Display::DisplayInformation::GetForCurrentView()->RawDpiX;
		if (rawDpiX > 0 && rawDpiX < 1000) //if the monitor reports dimension
			this->physicalWidth = this->emulator->GetWidth() / rawDpiX;
		else
			this->physicalWidth = 8.0f;

		float rawDpiY = Windows::Graphics::Display::DisplayInformation::GetForCurrentView()->RawDpiY;
		if (rawDpiY > 0 && rawDpiY < 1000) //if the monitor reports dimension
			this->physicalHeight = this->emulator->GetHeight() / rawDpiY;
		else
			this->physicalHeight = 8.0f;




		//get setting info
		if (this->emulator->GetHeight() > this->emulator->GetWidth())  //portrait
		{
			//scale is the ratio between real physical size and design physical size
			this->hscale = pow(this->physicalWidth * 2.54f / 8.0f, 0.5f);
			this->vscale = pow(this->physicalHeight * 2.54f / 8.0f, 0.5f);

			PadLeft = settings->PadLeftP * vscale / 2.54f * rawDpiX;
			PadBottom = settings->PadBottomP * vscale / 2.54f * rawDpiY;
			ACenterX = settings->ACenterXP * vscale / 2.54f * rawDpiX;
			ACenterY = settings->ACenterYP * vscale / 2.54f * rawDpiY;
			BCenterX = settings->BCenterXP * vscale / 2.54f * rawDpiX;
			BCenterY = settings->BCenterYP * vscale / 2.54f * rawDpiY;
			startCenterX = settings->startCenterXP * hscale / 2.54f * rawDpiX;
			startBottom = settings->startBottomP * vscale / 2.54f * rawDpiY;
			selectCenterX = settings->selectCenterXP * hscale / 2.54f * rawDpiX;
			selectBottom = settings->selectBottomP * vscale / 2.54f * rawDpiY;
			lLeft = settings->LLeftP * vscale / 2.54f * rawDpiX;
			LCenterY = settings->LCenterYP * vscale / 2.54f * rawDpiY;
			rRight = settings->RRightP * vscale / 2.54f * rawDpiX;
			RCenterY = settings->RCenterYP * vscale / 2.54f * rawDpiY;
			TurboCenterX = settings->TurboCenterXP * vscale / 2.54f * rawDpiX;
			TurboCenterY = settings->TurboCenterYP * vscale / 2.54f * rawDpiY;
			ComboCenterX = settings->ComboCenterXP * vscale / 2.54f * rawDpiX;
			ComboCenterY = settings->ComboCenterYP * vscale / 2.54f * rawDpiY;
		}
		else
		{
			this->hscale = pow(this->physicalWidth * 2.54f / 8.0f, 0.75f);
			this->vscale = pow(this->physicalHeight * 2.54f / 8.0f, 0.75f);

			PadLeft = settings->PadLeftL * vscale / 2.54f * rawDpiX;
			PadBottom = settings->PadBottomL * vscale / 2.54f * rawDpiY;
			ACenterX = settings->ACenterXL * vscale / 2.54f * rawDpiX;
			ACenterY = settings->ACenterYL * vscale / 2.54f * rawDpiY;
			BCenterX = settings->BCenterXL * vscale / 2.54f * rawDpiX;
			BCenterY = settings->BCenterYL * vscale / 2.54f * rawDpiY;
			startCenterX = settings->startCenterXL * vscale / 2.54f * rawDpiX;
			startBottom = settings->startBottomL * vscale / 2.54f * rawDpiY;
			selectCenterX = settings->selectCenterXL * vscale / 2.54f * rawDpiX;
			selectBottom = settings->selectBottomL * vscale / 2.54f * rawDpiY;
			lLeft = settings->LLeftL * vscale / 2.54f * rawDpiX;
			LCenterY = settings->LCenterYL * vscale / 2.54f * rawDpiY;
			rRight = settings->RRightL * vscale / 2.54f * rawDpiX;
			RCenterY = settings->RCenterYL * vscale / 2.54f * rawDpiY;
			TurboCenterX = settings->TurboCenterXL * vscale / 2.54f * rawDpiX;
			TurboCenterY = settings->TurboCenterYL * vscale / 2.54f * rawDpiY;
			ComboCenterX = settings->ComboCenterXL * vscale / 2.54f * rawDpiX;
			ComboCenterY = settings->ComboCenterYL * vscale / 2.54f * rawDpiY;
		}
	}


	void VirtualControllerInput::SaveControllerPositionSettings()
	{
		EmulatorSettings ^settings = EmulatorSettings::Current;

		//IMPORTANT: hscale used to be 1.0 on 480p device. Now hscale is 1.5 on 480p devices, which makes the device effectively 320p
		// so now all the number are based on 320p
		float rawDpiX = Windows::Graphics::Display::DisplayInformation::GetForCurrentView()->RawDpiX;
		if (rawDpiX > 0 && rawDpiX < 1000) //if the monitor reports dimension
			this->physicalWidth = this->emulator->GetWidth() / rawDpiX;
		else
			this->physicalWidth = 8.0f;

		float rawDpiY = Windows::Graphics::Display::DisplayInformation::GetForCurrentView()->RawDpiY;
		if (rawDpiY > 0 && rawDpiY < 1000) //if the monitor reports dimension
			this->physicalHeight = this->emulator->GetHeight() / rawDpiY;
		else
			this->physicalHeight = 8.0f;




		//save setting
		if (this->emulator->GetHeight() > this->emulator->GetWidth())  //portrait
		{
			//scale is the ratio between real physical size and design physical size
			this->hscale = pow(this->physicalWidth * 2.54f / 8.0f, 0.5f);
			this->vscale = pow(this->physicalHeight * 2.54f / 8.0f, 0.5f);

			settings->PadLeftP = PadLeft / vscale * 2.54f / rawDpiX;
			settings->PadBottomP = PadBottom / vscale * 2.54f / rawDpiY;
			settings->ACenterXP = ACenterX / vscale * 2.54f / rawDpiX;
			settings->ACenterYP = ACenterY / vscale * 2.54f / rawDpiY;
			settings->BCenterXP = BCenterX / vscale * 2.54f / rawDpiX;
			settings->BCenterYP = BCenterY / vscale * 2.54f / rawDpiY;
			settings->startCenterXP = startCenterX / vscale * 2.54f / rawDpiX;
			settings->startBottomP = startBottom / vscale * 2.54f / rawDpiY;
			settings->selectCenterXP = selectCenterX / vscale * 2.54f / rawDpiX;
			settings->selectBottomP = selectBottom / vscale * 2.54f / rawDpiY;
			settings->LLeftP = lLeft / vscale * 2.54f / rawDpiX;
			settings->LCenterYP = LCenterY / vscale * 2.54f / rawDpiY;
			settings->RRightP = rRight / vscale * 2.54f / rawDpiX;
			settings->RCenterYP = RCenterY / vscale * 2.54f / rawDpiY;
			settings->TurboCenterXP = TurboCenterX / vscale * 2.54f / rawDpiX;
			settings->TurboCenterYP = TurboCenterY / vscale * 2.54f / rawDpiY;
			settings->ComboCenterXP = ComboCenterX / vscale * 2.54f / rawDpiX;
			settings->ComboCenterYP = ComboCenterY / vscale * 2.54f / rawDpiY;
		}
		else
		{
			this->hscale = pow(this->physicalWidth * 2.54f / 8.0f, 0.75f);
			this->vscale = pow(this->physicalHeight * 2.54f / 8.0f, 0.75f);

			settings->PadLeftL = PadLeft / vscale * 2.54f / rawDpiX;
			settings->PadBottomL = PadBottom / vscale * 2.54f / rawDpiY;
			settings->ACenterXL = ACenterX / vscale * 2.54f / rawDpiX;
			settings->ACenterYL = ACenterY / vscale * 2.54f / rawDpiY;
			settings->BCenterXL = BCenterX / vscale * 2.54f / rawDpiX;
			settings->BCenterYL = BCenterY / vscale * 2.54f / rawDpiY;
			settings->startCenterXL = startCenterX / vscale * 2.54f / rawDpiX;
			settings->startBottomL = startBottom / vscale * 2.54f / rawDpiY;
			settings->selectCenterXL = selectCenterX / vscale * 2.54f / rawDpiX;
			settings->selectBottomL = selectBottom / vscale * 2.54f / rawDpiY;
			settings->LLeftL = lLeft / vscale * 2.54f / rawDpiX;
			settings->LCenterYL = LCenterY / vscale * 2.54f / rawDpiY;
			settings->RRightL = rRight / vscale * 2.54f / rawDpiX;
			settings->RCenterYL = RCenterY / vscale * 2.54f / rawDpiY;
			settings->TurboCenterXL = TurboCenterX / vscale * 2.54f / rawDpiX;
			settings->TurboCenterYL = TurboCenterY / vscale * 2.54f / rawDpiY;
			settings->ComboCenterXL = ComboCenterX / vscale * 2.54f / rawDpiX;
			settings->ComboCenterYL = ComboCenterY / vscale * 2.54f / rawDpiY;
		}
	}

	void VirtualControllerInput::CreateRenderRectangles(void)
	{
		
		float rawDpiX = Windows::Graphics::Display::DisplayInformation::GetForCurrentView()->RawDpiX;
		float rawDpiY = Windows::Graphics::Display::DisplayInformation::GetForCurrentView()->RawDpiY;

		//setting to scale the size of the button
		float value = EmulatorSettings::Current->ControllerScale / 100.0f * rawDpiX / 122.0f; //122.0f is dpi of 14'' laptop
		float value2 = EmulatorSettings::Current->ButtonScale / 100.0f * rawDpiX / 122.0f;
		
		float pheight = this->emulator->GetHeight();
		float pwidth = this->emulator->GetWidth();
		// Visible Rectangles
		this->padCrossRectangle.left = PadLeft;
		this->padCrossRectangle.right = PadLeft + 120 * value * this->vscale;
		this->padCrossRectangle.bottom = pheight - PadBottom;
		this->padCrossRectangle.top = pheight - PadBottom - 120 * value * this->vscale;

		this->aRectangle.left = pwidth - ACenterX - 30 * value2 * this->vscale;
		this->aRectangle.right = pwidth - ACenterX + 30 * value2 * this->vscale;
		this->aRectangle.top = pheight - ACenterY - 30 * value2 * this->vscale;
		this->aRectangle.bottom = pheight - ACenterY + 30 * value2 * this->vscale;

		this->bRectangle.left = pwidth - BCenterX - 30 * value2 * this->vscale;
		this->bRectangle.right = pwidth - BCenterX + 30 * value2 * this->vscale;
		this->bRectangle.top = pheight - BCenterY - 30 * value2 * this->vscale;
		this->bRectangle.bottom = pheight - BCenterY + 30 * value2 * this->vscale;


		this->startRectangle.left = pwidth / 2.0f + startCenterX - 25 * value2 * this->vscale;
		this->startRectangle.right = pwidth / 2.0f + startCenterX + 25 * value2 * this->vscale;
		this->startRectangle.top = pheight - startBottom - 25 * value2 * this->vscale;
		this->startRectangle.bottom = pheight - startBottom;

		this->selectRectangle.left = pwidth / 2.0f + selectCenterX - 25 * value2 * this->vscale;
		this->selectRectangle.right = pwidth / 2.0f + selectCenterX + 25 * value2 * this->vscale;
		this->selectRectangle.top = pheight - selectBottom - 25 * value2 * this->vscale;
		this->selectRectangle.bottom = pheight - selectBottom;



		this->turboRectangle.left = pwidth - TurboCenterX - 12 * value2 * this->vscale;
		this->turboRectangle.right = pwidth - TurboCenterX + 12 * value2 * this->vscale;
		this->turboRectangle.top = pheight - TurboCenterY - 12 * value2 * this->vscale;
		this->turboRectangle.bottom = pheight - TurboCenterY + 12 * value2 * this->vscale;

		this->comboRectangle.left = pwidth - ComboCenterX - 12 * value2 * this->vscale;
		this->comboRectangle.right = pwidth - ComboCenterX + 12 * value2 * this->vscale;
		this->comboRectangle.top = pheight - ComboCenterY - 12 * value2 * this->vscale;
		this->comboRectangle.bottom = pheight - ComboCenterY + 12 * value2 * this->vscale;

		this->lRectangle.left = lLeft;
		this->lRectangle.right = this->lRectangle.left + 45 * value2 * this->vscale;
		this->lRectangle.top = pheight - LCenterY - 13 * value2 * this->vscale;
		this->lRectangle.bottom = pheight - LCenterY + 13 * value2 * this->vscale;


		this->rRectangle.right = pwidth - rRight;
		this->rRectangle.left = pwidth - rRight - 45 * value2 * this->vscale;
		this->rRectangle.top = pheight - RCenterY - 13 * value2 * this->vscale;
		this->rRectangle.bottom = pheight - RCenterY + 13 * value2 * this->vscale;



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
		SetControllerPositionFromSettings();
		CreateRenderRectangles();
		CreateTouchRectangles();
	}

	void VirtualControllerInput::GetCrossRectangle(RECT *rect)
	{
		*rect = this->padCrossRectangle;
	}

	void VirtualControllerInput::GetARectangle(RECT *rect)
	{
		*rect = this->aRectangle;
	}

	void VirtualControllerInput::GetBRectangle(RECT *rect)
	{
		*rect = this->bRectangle;
	}

	void VirtualControllerInput::GetStartRectangle(RECT *rect)
	{
		*rect = this->startRectangle;
	}

	void VirtualControllerInput::GetSelectRectangle(RECT *rect)
	{
		*rect = this->selectRectangle;
	}

	void VirtualControllerInput::GetTurboRectangle(RECT *rect)
	{
		*rect = this->turboRectangle;
	}

	void VirtualControllerInput::GetComboRectangle(RECT *rect)
	{
		*rect = this->comboRectangle;
	}

	void VirtualControllerInput::GetLRectangle(RECT *rect)
	{
		*rect = this->lRectangle;
	}

	void VirtualControllerInput::GetRRectangle(RECT *rect)
	{
		*rect = this->rRectangle;
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

	void VirtualControllerInput::EnterEditMode()
	{
		isEditMode = true;
	}

	bool VirtualControllerInput::IsEditMode()
	{
		return isEditMode;
	}

	void VirtualControllerInput::LeaveEditMode(bool accept)
	{
		if (accept) //accept, so store the settings
		{
			SaveControllerPositionSettings();
		}
		else //not accept, restore the old value
		{
			SetControllerPositionFromSettings();
			CreateRenderRectangles();
			CreateTouchRectangles();
		}

		isEditMode = false;
		
		
	}

	
}