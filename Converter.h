#pragma once
#include "pch.h"
#include <ppltasks.h>

//IMPORTANT NOTE: need to include this header file in both the .h and .cpp of an xaml page

using namespace Platform;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Interop;
using namespace Concurrency;
using namespace Windows::UI::Xaml::Media::Imaging;

namespace VBA10
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class IsoImageConverter sealed : Windows::UI::Xaml::Data::IValueConverter
	{
	public:
		IsoImageConverter();
		virtual Object^ Convert(Object^ value, TypeName targetType, Object^ parameter, String^ language);
		virtual Object^ ConvertBack(Object^ value, TypeName targetType, Object^ parameter, String^ language);
	private:
		~IsoImageConverter();

		task<void> LoadImageFromPath(BitmapImage^ bitmap, String^ filepath);
	};


}
