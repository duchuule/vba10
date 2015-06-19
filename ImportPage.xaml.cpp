//
// ImportPage.xaml.cpp
// Implementation of the ImportPage class
//

#include "pch.h"
#include "ImportPage.xaml.h"
#include "Database\ROMDBEntry.h"
#include "Database\ROMDatabase.h"
#include "SelectFilePane.xaml.h"


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
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage;
using namespace Windows::Storage::AccessCache;
using namespace Concurrency;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

ImportPage::ImportPage()
{
	InitializeComponent();
}


void ImportPage::chooseFolderbtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	FolderPicker ^picker = ref new FolderPicker();

	picker->FileTypeFilter->Append(".gba");
	picker->FileTypeFilter->Append(".gbc");
	picker->FileTypeFilter->Append(".gb");
	picker->FileTypeFilter->Append(".sav");
	picker->FileTypeFilter->Append(".sgm");
	picker->FileTypeFilter->Append(".gb");
	picker->FileTypeFilter->Append(".zip");
	picker->FileTypeFilter->Append(".rar");
	picker->FileTypeFilter->Append(".7z");

	picker->ViewMode = PickerViewMode::List;
	picker->CommitButtonText = "Select ROM Directory";

	task<void> t = create_task(picker->PickSingleFolderAsync()).then([this](StorageFolder ^folder)
	{
		if (folder)
		{
			//remove special char in path so that we can use path as token
			Platform::String ^ptoken = folder->Path;

			wstring token(ptoken->Begin(), ptoken->End());
			replace(token.begin(), token.end(), ':', '_');
			replace(token.begin(), token.end(), '/', '_');
			replace(token.begin(), token.end(), '\\', '_');
			ptoken = ref new Platform::String(token.c_str());

			//add folder to future accesslist
			if (!StorageApplicationPermissions::FutureAccessList->ContainsItem(ptoken))
				StorageApplicationPermissions::FutureAccessList->AddOrReplace(ptoken, folder);

			//get list of qualified file in folder
			Search::QueryOptions ^options = ref new Search::QueryOptions();
			options->FileTypeFilter->Append(".gba");
			options->FileTypeFilter->Append(".gbc");
			options->FileTypeFilter->Append(".gb");
			//TODO: add support for other file types
			return create_task(folder->CreateFileQueryWithOptions(options)->GetFilesAsync())
				.then([this](IVectorView<StorageFile ^> ^files)
			{

				//open panel to let user select file
				Popup ^statePopup = ref new Popup();
				statePopup->IsLightDismissEnabled = true;

				SelectFilePane ^pane = ref new SelectFilePane(files);
				statePopup->Child = pane;
				pane->Width = titleBar->ActualWidth;//statePopup->Width;
				pane->MaxHeight = Window::Current->Bounds.Height - 48; //statePopup->MaxHeight;

				pane->FileSelectedCallback = ref new FileSelectedDelegate([=](StorageFile ^file)
				{
					ROMDBEntry^ entry = ref new ROMDBEntry(1, file->DisplayName, file->Name, file->Path);
					App::ROMDB->AllROMDBEntries->Append(entry);
					create_task(App::ROMDB->AddAsync(entry)).then([](task<void> t)
					{
						try
						{
							t.get();
							// .get() didn't throw, so we succeeded, print out success message
							MessageDialog ^dialog = ref new MessageDialog("File imported successfully.");
							dialog->ShowAsync();
						}
						catch (Platform::Exception ^ex)
						{
						}
					});
					
				});

				//auto transform = ((UIElement^)sender)->TransformToVisual(nullptr); //nullptr to get position related to windows
				auto transform = ((UIElement^)titleBar)->TransformToVisual(nullptr);

				Windows::Foundation::Point point = transform->TransformPoint(Windows::Foundation::Point());
				statePopup->HorizontalOffset = point.X; //+ selectStateBtn->ActualWidth / 2.0f - pane->Width / 2.0f;
				statePopup->VerticalOffset = point.Y + titleBar->ActualHeight;

				statePopup->IsOpen = true;
			});
		}
		else
			return create_task([] {});

	});

}

void ImportPage::ImportFile(StorageFile^ file)
{

}



