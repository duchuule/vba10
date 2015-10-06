//
// CheatPane.xaml.cpp
// Implementation of the CheatPane class
//

#include "pch.h"
#include "CheatPane.xaml.h"
#include "EmulatorFileHandler.h"
#include "CheatData.h"
#include "stringhelper.h"
#include <string>
#include <sstream>
#include "AdControl.xaml.h"
#include "App.xaml.h"

using namespace std;

using namespace VBA10;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Popups;

#define IS_HEX(a) (\
	(a >= '0' && a <= '9') || (a >= 'a' && a <= 'f') || (a >= 'A' && a <= 'F'))

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

CheatPane::CheatPane()
{
	InitializeComponent();
	
	//create ad control
	if (App::HasAds)
	{
		AdControl^ adControl = ref new AdControl();
		LayoutRoot->Children->Append(adControl);
		adControl->SetValue(Grid::RowProperty, 2);
	}

	if (IsROMLoaded())
	{

		

		svMain->Visibility = Windows::UI::Xaml::Visibility::Visible;
		txtNoROM->Visibility = Windows::UI::Xaml::Visibility::Collapsed;

		

		this->RefreshCheatList();


	}
	else
	{
		svMain->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		txtNoROM->Visibility = Windows::UI::Xaml::Visibility::Visible;
	}
}


void CheatPane::RefreshCheatList(void)
{
	//this->cheatList->ItemsSource = nullptr;
	//this->cheatList->ItemsSource = this->cheatCodes;
	cvsAllCheats->Source = ROMCheats;

	if (ROMCheats->Size > 0)
	{
		cheatList->Visibility = Windows::UI::Xaml::Visibility::Visible;
		txtNoCheat->Visibility = Windows::UI::Xaml::Visibility::Collapsed;

	}
	else
	{
		cheatList->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		txtNoCheat->Visibility = Windows::UI::Xaml::Visibility::Visible;
	}
}


void CheatPane::DeleteCheatButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Button ^button = safe_cast<Button ^>(sender);
	CheatData ^data = safe_cast<CheatData ^>(button->DataContext);

	for (int i = 0; i < ROMCheats->Size; i++)
	{
		if(ROMCheats->GetAt(i) == data)
		{
			ROMCheats->RemoveAt(i);

			//save cheats
//			try
//			{
//				SaveCheats();
//			}
//			catch (InvalidCastException ^ex)
//			{
//#if _DEBUG
//				Platform::String ^message = ex->Message;
//				wstring wstr(message->Begin(), message->End());
//				OutputDebugStringW(L"InvalidCastException");
//#endif
//			}

			this->RefreshCheatList();
			ShouldApplyNewCheats = true;
			break;
		}
	}
}




void CheatPane::addButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if(!this->CheckCodeFormat(this->codeBox->Text, 
		[](Platform::String ^message){
			MessageDialog ^dialog = ref new MessageDialog(message);
			dialog->ShowAsync();
		}))
	{
		return;
	}
	Vector<Platform::String ^> ^codes = this->GetCodes(this->codeBox->Text);
	
	CheatData ^data = ref new CheatData();
	data->Description = this->descBox->Text;
	data->Enabled = true;

	if (codes->Size > 0)
		data->CheatCode = codes->GetAt(0);


	for (int i = 1; i < codes->Size; i++)
	{
		data->CheatCode += "\n" + codes->GetAt(i);
	}

	ROMCheats->Append(data);

//	try
//	{
//		SaveCheats();
//	}
//	catch (InvalidCastException ^ex)
//	{
//#if _DEBUG
//		Platform::String ^message = ex->Message;
//		wstring wstr(message->Begin(), message->End());
//		OutputDebugStringW(L"InvalidCastException");
//#endif
//	}

	ShouldApplyNewCheats = true;
	this->RefreshCheatList();
	
	this->codeBox->Text = "";
	this->descBox->Text = "";
}

bool CheatPane::CheckCodeFormat(Platform::String ^codeText, void(*messageCallback)(Platform::String ^))
{
	auto loader =  Windows::ApplicationModel::Resources::ResourceLoader::GetForViewIndependentUse();


	if (codeText == nullptr || codeText->IsEmpty())
	{
		messageCallback(loader->GetString("EmptyCheatCodeError"));
		return false;
	}

	vector<string> codeParts;
	string code(codeText->Begin(), codeText->End());

	strreplace(code, '\n', '\r');
	strSplitLines(code, codeParts);

	for (int i = 0; i < codeParts.size(); i++)
	{
		string line = codeParts.at(i);
		StrToUpper(line);
		replaceAll(line, "\t", "");
		replaceAll(line, " ", "");
		for (int i = 0; i < line.length(); i++)
		{
			if (!IS_HEX(line.at(i)))
			{
				messageCallback(loader->GetString("InvalidCheatCodeError"));
				return false;
			}
		}
		if (line.length() != 6 && line.length() != 9 && line.length() != 8 && line.length() != 12 && line.length() != 16)
		{
			messageCallback(loader->GetString("InvalidCheatCodeError"));
			return false;
		}
	}

	return true;
}




Vector<Platform::String ^>^ CheatPane::GetCodes(Platform::String ^codeText)
{


	vector<string> codeParts;
	string code(codeText->Begin(), codeText->End());

	strreplace(code, '\n', '\r');
	strSplitLines(code, codeParts);

	Vector<Platform::String ^> ^codes = ref new Vector<Platform::String ^>();
	bool continuedFromLast = false;
	stringstream ss;

	

	for (int i = 0; i < codeParts.size(); i++)
	{
		string line = codeParts.at(i);
		StrToUpper(line);
		replaceAll(line, "\t", "");
		replaceAll(line, "-", "");
		line = trim(line);

		if (continuedFromLast == false) //reset the string builder if not continued from last time
			ss.str("");  //clear the stringstream

		if (line.size() == 6)
		{
			ss << line.substr(0, 3);
			ss << '-';
			ss << line.substr(3, 3);
		}
		else if (line.size() == 9)
		{
			ss << line.substr(0, 3);
			ss << '-';
			ss << line.substr(3, 3);
			ss << '-';
			ss << line.substr(6, 3);
		}
		else if (line.size() == 8) // 12345678
		{
			if (IsGBAROMLoaded())  //convert to 12345678 12345678 format for gameboy advance game
			{
				if (continuedFromLast) //this is the second part
				{
					ss << ' ';
					ss << line;
					continuedFromLast = false;
				}
				else
				{
					ss << line;
					continuedFromLast = true;
				}
			}
			else
				ss << line;
			
		}
		else if(line.size() == 12)  //123456781234
		{
			ss << line.substr(0, 8);
			ss << ' ';
			ss << line.substr(8, 4);
		}
		else if (line.size() == 13)  //12345678 1234
		{
			ss << line;
		}
		else if(line.size() == 16 || line.size() == 17)  //two version of gameshark codes for GBA, 1234567812345678 OR 12345678 12345678
		{
			ss << line;
		}

		if (continuedFromLast == false)
		{
			string finalCode = ss.str();
			wstring wstr(finalCode.begin(), finalCode.end());
			codes->Append(ref new Platform::String(wstr.c_str()));
		}
	}

	return codes;
}



void CheatPane::enableCheatBox_Checked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	CheckBox ^box = safe_cast<CheckBox ^>(sender);
	CheatData ^data = safe_cast<CheatData ^>(box->DataContext);

	data->Enabled = box->IsChecked->Value;

	ShouldApplyNewCheats = true;
}
