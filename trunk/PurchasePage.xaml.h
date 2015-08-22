//
// PurchasePage.xaml.h
// Declaration of the PurchasePage class
//

#pragma once

#include "PurchasePage.g.h"


namespace VBA10
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class PurchasePage sealed
	{
	public:
		PurchasePage();
	protected:
		virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
	private:
		void Purchasebtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void ButtonBuyNow_Clicked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};

	[Windows::UI::Xaml::Data::BindableAttribute]
	public ref class ProductItem sealed
	{
	public:
		property Platform::String^ imgLink;
		property Platform::String^ Status;
		property Platform::String^ Name;
		property Platform::String^ key;
		property Windows::UI::Xaml::Visibility BuyNowButtonVisible;
	};
}
