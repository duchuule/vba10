//
// ExportPage.xaml.cpp
// Implementation of the ExportPage class
//
#include "pch.h"
#include "ExportPage.xaml.h"
#include "App.xaml.h"
#include "SelectFilePane.xaml.h"
#include "SelectFilesPane.xaml.h"

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



// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

ExportPage::ExportPage()
{
	InitializeComponent();
}





void ExportPage::Page_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//try re-sign in silently because access token expires every 1 hour
	if (EmulatorSettings::Current->SignedIn)
	{
		//live::live_client* LiveClient = new live::live_client();
		App::LiveClient->login(L"wl.skydrive_update wl.signin", true)
			.then([this](bool isLoggedIn)
		{
			signin_Completed(isLoggedIn);
		});
	}
}

void ExportPage::SignInbtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	App::LiveClient->login(L"wl.skydrive_update wl.signin", false)
		.then([this](bool isLoggedIn)
	{
		signin_Completed(isLoggedIn);

	});
}

void ExportPage::signin_Completed(bool isLoggedIn)
{
	if (isLoggedIn)
	{
		this->SignInbtn->Content = "Signed in";
		this->SignInbtn->IsEnabled = false;
		this->exportOneDrivebtn->IsEnabled = true;
		EmulatorSettings::Current->SignedIn = true;
	}
	else
	{
		this->SignInbtn->Content = "Sign in";
		this->SignInbtn->IsEnabled = true;
		this->exportOneDrivebtn->IsEnabled = false;
		EmulatorSettings::Current->SignedIn = false;
	}
}


void ExportPage::exportOneDrivebtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//get a list of rom
	Vector<Platform::String ^> ^romNames = ref new Vector<Platform::String ^>();
	for (int i = 0; i < App::ROMDB->AllROMDBEntries->Size; i++)
		romNames->Append(App::ROMDB->AllROMDBEntries->GetAt(i)->DisplayName);

	//open panel to let user select rom
	Popup ^statePopup = ref new Popup();
	statePopup->IsLightDismissEnabled = true;

	SelectFilePane ^pane = ref new SelectFilePane(romNames, "Select ROM");
	statePopup->Child = pane;
	pane->Width = titleBar->ActualWidth;//statePopup->Width;
	pane->MaxHeight = Window::Current->Bounds.Height - 48; //statePopup->MaxHeight;

	pane->FileSelectedCallback = ref new FileSelectedDelegate([=](int selectedIndex)
	{
		ROMDBEntry^ entry = App::ROMDB->AllROMDBEntries->GetAt(selectedIndex);

		//get list of save files
		Search::QueryOptions ^options = ref new Search::QueryOptions();
		options->FileTypeFilter->Append("*");
		options->IndexerOption = Search::IndexerOption::DoNotUseIndexer;
		options->UserSearchFilter = entry->DisplayName;
		create_task(entry->Folder->CreateFileQueryWithOptions(options)->GetFilesAsync())
			.then([this](IVectorView<StorageFile ^> ^files)
		{
			//open panel to let user select file
			Popup ^statePopup = ref new Popup();
			statePopup->IsLightDismissEnabled = true;

			Vector<Platform::String ^> ^fileNames = ref new Vector<Platform::String ^>();
			for (int i = 0; i < files->Size; i++)
				fileNames->Append(files->GetAt(i)->Name);

			SelectFilesPane ^pane = ref new SelectFilesPane(fileNames, "Select file(s) to export");
			statePopup->Child = pane;
			pane->Width = titleBar->ActualWidth;//statePopup->Width;
			pane->MaxHeight = Window::Current->Bounds.Height - 48; //statePopup->MaxHeight;

			pane->FilesSelectedCallback = ref new FilesSelectedDelegate([=](IVector<int>^ selectedIndices)
			{

			});

			auto transform = ((UIElement^)titleBar)->TransformToVisual(nullptr);

			Windows::Foundation::Point point = transform->TransformPoint(Windows::Foundation::Point());
			statePopup->HorizontalOffset = point.X + 1; //+ selectStateBtn->ActualWidth / 2.0f - pane->Width / 2.0f;
			statePopup->VerticalOffset = point.Y + titleBar->ActualHeight;

			statePopup->IsOpen = true;

		});



	});

	auto transform = ((UIElement^)titleBar)->TransformToVisual(nullptr);

	Windows::Foundation::Point point = transform->TransformPoint(Windows::Foundation::Point());
	statePopup->HorizontalOffset = point.X + 1; //+ selectStateBtn->ActualWidth / 2.0f - pane->Width / 2.0f;
	statePopup->VerticalOffset = point.Y + titleBar->ActualHeight;

	statePopup->IsOpen = true;
}

