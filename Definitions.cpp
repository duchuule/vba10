#include "Definitions.h"

using namespace VBA10;
using namespace Windows::UI::Xaml::Data;

void OneDriveFileItem::OnPropertyChanged(Platform::String^ propertyName)
{
	PropertyChangedEventArgs^ args =
		ref new PropertyChangedEventArgs(propertyName);
	PropertyChanged(this, args);
}