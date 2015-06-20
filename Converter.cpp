#include "Converter.h"
#include "Definitions.h"

using namespace Windows::UI::Xaml::Media::Imaging;
using namespace VBA10;

IsoImageConverter::IsoImageConverter()
{
}

IsoImageConverter::~IsoImageConverter()
{
}

Object^ IsoImageConverter::Convert(Object^ value, TypeName targetType, Object^ parameter, String^ language)
{
	try
	{
		BitmapImage^ bitmap = ref new BitmapImage();
		String^ path = (String^)value;
		if (path->Equals(DEFAULT_SNAPSHOT))
			return path;

		if (!path->IsEmpty())
		{
			/*using (var isoStore = IsolatedStorageFile.GetUserStoreForApplication())
			{
				using (IsolatedStorageFileStream fs = isoStore.OpenFile(path, System.IO.FileMode.Open, System.IO.FileAccess.Read))
				{
					bitmap.SetSource(fs);
				}
			}*/
		}
		return bitmap;
	}
	catch (Exception^ ex)
	{
		return nullptr;
	}


	//String^ _grade = "";
	//String^ _valueString = "";
	////try parsing the value to int
	//int _value = ((Windows::Foundation::IPropertyValue^)value)->GetInt32();
	//if (_value < 50)
	//	_grade = "F";
	//else if (_value < 60)
	//	_grade = "D";
	//else if (_value < 70)
	//	_grade = "C";
	//else if (_value < 80)
	//	_grade = "B";
	//else if (_value < 90)
	//	_grade = "A";
	//else if (_value < 100)
	//	_grade = "A+";
	//else if (_value == 100)
	//	_grade = "SUPER STAR!";
	//return _grade;
}

Object^ IsoImageConverter::ConvertBack(Object^ value, TypeName targetType, Object^ parameter, String^ language)
{
	return nullptr; //doing one-way binding so this is not required.
}

