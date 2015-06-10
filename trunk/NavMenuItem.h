#pragma once
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Interop;

namespace VBA10
{
	/// <summary>
	/// Data to represent an item in the nav menu.
	/// </summary>
	public ref class NavMenuItem sealed
	{
	public:
		NavMenuItem(Platform::String^, Symbol, TypeName);
		NavMenuItem(Platform::String^, Symbol, TypeName, Object^);
		property Platform::String^ Label;
		property Symbol Symbol;
		property TypeName DestPage;
		property Platform::String^ SymbolAsChar
		{
			Platform::String^ get();
		}
		property Object^ Arguments;

	};
}
