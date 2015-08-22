//
// PurchasePage.xaml.cpp
// Implementation of the PurchasePage class
//

#include "pch.h"
#include "PurchasePage.xaml.h"

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
using namespace Windows::ApplicationModel::Store;
using namespace Concurrency;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

PurchasePage::PurchasePage()
{
	InitializeComponent();
}

void PurchasePage::OnNavigatedTo(NavigationEventArgs^ /* e */)
{
	create_task(CurrentApp::LoadListingInformationAsync())
		.then([this](ListingInformation^ li)
	{
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
		pitem1->key = key ;
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

	}, task_continuation_context::use_current());
}

void PurchasePage::Purchasebtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	create_task(CurrentApp::RequestProductPurchaseAsync("removeads"));
}


void PurchasePage::ButtonBuyNow_Clicked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}
