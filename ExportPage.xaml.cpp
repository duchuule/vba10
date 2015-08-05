//
// ExportPage.xaml.cpp
// Implementation of the ExportPage class
//
#include "pch.h"
#include "ExportPage.xaml.h"
#include "App.xaml.h"

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

}