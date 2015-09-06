//
// PurchasePage.xaml.cpp
// Implementation of the PurchasePage class
//

#include "pch.h"
#include "PurchasePage.xaml.h"
#include "App.xaml.h"
#include "AdControl.xaml.h"

using namespace VBA10;

using namespace Platform;
using namespace Platform::Collections;
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
using namespace Windows::ApplicationModel::Store;
using namespace Concurrency;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

PurchasePage::PurchasePage()
{
	InitializeComponent();

	if (App::HasAds)
	{
		AdControl^ adControl = ref new AdControl();
		LayoutRoot->Children->Append(adControl);
		adControl->SetValue(Grid::RowProperty, 2);
	}
}

void PurchasePage::OnNavigatedTo(NavigationEventArgs^ /* e */)
{

	
}




void PurchasePage::ButtonBuyNow_Clicked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Button^ btn = (Button^)sender;

	String^ key = (String^)btn->Tag;


	if (!CurrentApp::LicenseInformation->ProductLicenses->Lookup(key)->IsActive)
	{

		create_task(CurrentApp::RequestProductPurchaseAsync(key))
			.then([key, this](task<PurchaseResults^> tresult) 
		{
			try
			{
				PurchaseResults^ result = tresult.get();
				//int test = result->Status;
				//reread license
				App::CheckProductLicense();

				if (CurrentApp::LicenseInformation->ProductLicenses->Lookup(key)->IsActive)
				{
					//prompt user to restart app if it's ad removal
					if (key == "removeads" || key == "noads_premium")
					{
						MessageDialog ^dialog = ref new MessageDialog("Purchase successful, ads will not be shown the next time you start the app.");
						dialog->ShowAsync();
					}

					if (key == "premiumfeatures")
					{
						MessageDialog ^dialog = ref new MessageDialog("Purchase successful.");
						dialog->ShowAsync();
					}

					this->OnNavigatedTo(nullptr);
				}
				
			}
			catch (Exception^)
			{ }
			
		}, task_continuation_context::use_current());



			



	}
}


void PurchasePage::loadProductsBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	this->txtLoading->Visibility = Windows::UI::Xaml::Visibility::Visible;

	create_task(CurrentApp::LoadListingInformationAsync())
		.then([this](task<ListingInformation^> tli)
	{
		try
		{
			ListingInformation^ li = tli.get();

			Vector<ProductItem^>^ picItems = ref new Vector<ProductItem^>();

			String^ key;
			String^ imageLink;
			String^ status = "";
			String^ pname = "";
			Windows::UI::Xaml::Visibility buyButtonVisibility = Windows::UI::Xaml::Visibility::Collapsed;

			//get premium + no ads
			key = "noads_premium";
			imageLink = "/Assets/noad_plus_icon.png";
			if (li->ProductListings->HasKey(key))
			{
				auto pListing = li->ProductListings->Lookup(key);
				status = CurrentApp::LicenseInformation->ProductLicenses->Lookup(key)->IsActive ? "Purchased, thank you!" : pListing->FormattedPrice;
				buyButtonVisibility = CurrentApp::LicenseInformation->ProductLicenses->Lookup(key)->IsActive ? Windows::UI::Xaml::Visibility::Collapsed : Windows::UI::Xaml::Visibility::Visible;
				pname = pListing->Name;
			}
			else
			{
				status = "Product is not available at the moment.";
				buyButtonVisibility = Windows::UI::Xaml::Visibility::Collapsed;
				pname = "No Ads + Premium Features";
			}

			ProductItem^ pitem1 = ref new ProductItem();
			pitem1->imgLink = imageLink;
			pitem1->Name = pname;
			pitem1->Status = status;
			pitem1->key = key;
			pitem1->BuyNowButtonVisible = buyButtonVisibility;
			picItems->Append(pitem1);


			//get no ads
			key = "removeads";
			imageLink = "/Assets/noad_icon.png";
			if (li->ProductListings->HasKey(key))
			{
				auto pListing = li->ProductListings->Lookup(key);
				status = CurrentApp::LicenseInformation->ProductLicenses->Lookup(key)->IsActive ? "Purchased, thank you!" : pListing->FormattedPrice;
				buyButtonVisibility = CurrentApp::LicenseInformation->ProductLicenses->Lookup(key)->IsActive ? Windows::UI::Xaml::Visibility::Collapsed : Windows::UI::Xaml::Visibility::Visible;
				pname = pListing->Name;
			}
			else
			{
				status = "Product is not available at the moment.";
				buyButtonVisibility = Windows::UI::Xaml::Visibility::Collapsed;
				pname = "Remove ads";
			}

			ProductItem^ pitem2 = ref new ProductItem();
			pitem2->imgLink = imageLink;
			pitem2->Name = pname;
			pitem2->Status = status;
			pitem2->key = key;
			pitem2->BuyNowButtonVisible = buyButtonVisibility;
			picItems->Append(pitem2);


			//get premium feature
			key = "premiumfeatures";
			imageLink = "/Assets/plus_sign.png";
			if (li->ProductListings->HasKey(key))
			{
				auto pListing = li->ProductListings->Lookup(key);
				status = CurrentApp::LicenseInformation->ProductLicenses->Lookup(key)->IsActive ? "Purchased, thank you!" : pListing->FormattedPrice;
				buyButtonVisibility = CurrentApp::LicenseInformation->ProductLicenses->Lookup(key)->IsActive ? Windows::UI::Xaml::Visibility::Collapsed : Windows::UI::Xaml::Visibility::Visible;
				pname = pListing->Name;
			}
			else
			{
				status = "Product is not available at the moment.";
				buyButtonVisibility = Windows::UI::Xaml::Visibility::Collapsed;
				pname = "Premium Features";
			}

			ProductItem^ pitem3 = ref new ProductItem();
			pitem3->imgLink = imageLink;
			pitem3->Name = pname;
			pitem3->Status = status;
			pitem3->key = key;
			pitem3->BuyNowButtonVisible = buyButtonVisibility;
			picItems->Append(pitem3);


			this->pics->ItemsSource = picItems;

		}
		catch (Exception^)
		{
			txtError->Visibility = Windows::UI::Xaml::Visibility::Visible;
		}

		txtLoading->Visibility = Windows::UI::Xaml::Visibility::Collapsed;

	}, task_continuation_context::use_current());
}
