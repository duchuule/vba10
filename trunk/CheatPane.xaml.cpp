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


	if (IsROMLoaded())
	{

		this->RefreshCheatList();

		addButton->IsEnabled = true;
		descLabel->Text = "This emulator supports Gameshark, CodeBreaker and GameGenie codes.";

//		create_task([this]()
//		{
//			return LoadCheats();  //we actually don't have to do this, we can just get the saved list
//		}).then([this](IVector<CheatData ^> ^cheats)
//		{
//			this->cheatCodes = (IObservableVector<CheatData ^> ^) cheats;
//
//			return this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
//			{
//				this->RefreshCheatList();
//			}));
//		}).then([this](task<void> t)
//		{
//			try
//			{
//				t.get();
//			}
//			catch (Platform::Exception ^ex)
//			{
//#if _DEBUG
//				String ^str = ex->Message;
//				wstring wstr(str->Begin(), str->End());
//
//				OutputDebugStringW(wstr.c_str());
//#endif
//			}
//		});
	}
	else
	{
		addButton->IsEnabled = false;
		descLabel->Text = "You need to load a ROM first before you can enter cheats.";
	}
}


void CheatPane::RefreshCheatList(void)
{
	//this->cheatList->ItemsSource = nullptr;
	//this->cheatList->ItemsSource = this->cheatCodes;
	cvsAllCheats->Source = ROMCheats;
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
			try
			{
				SaveCheats();
			}
			catch (InvalidCastException ^ex)
			{
#if _DEBUG
				Platform::String ^message = ex->Message;
				wstring wstr(message->Begin(), message->End());
				OutputDebugStringW(L"InvalidCastException");
#endif
			}

			this->RefreshCheatList();
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
	for (int i = 0; i < codes->Size; i++)
	{
		CheatData ^data = ref new CheatData();
		data->CheatCode = codes->GetAt(i);
		data->Description = this->descBox->Text;
		data->Enabled = true;

		ROMCheats->Append(data);
	}

	try
	{
		SaveCheats();
	}
	catch (InvalidCastException ^ex)
	{
#if _DEBUG
		Platform::String ^message = ex->Message;
		wstring wstr(message->Begin(), message->End());
		OutputDebugStringW(L"InvalidCastException");
#endif
	}

	this->RefreshCheatList();
	
	this->codeBox->Text = "";
	this->descBox->Text = "";
}

bool CheatPane::CheckCodeFormat(Platform::String ^codeText, void(*messageCallback)(Platform::String ^))
{
	if (codeText == nullptr || codeText->IsEmpty())
	{
		messageCallback("You must enter a cheat code first.");
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
				messageCallback("Invalid code format.");
				return false;
			}
		}
		if (line.length() != 6 && line.length() != 9 && line.length() != 8 && line.length() != 12 && line.length() != 16)
		{
			messageCallback("Invalid code format.");
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
}
