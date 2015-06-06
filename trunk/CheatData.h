#ifndef CHEATDATA_H_
#define CHEATDATA_H_

namespace VBA10
{
	//[Windows::UI::Xaml::Data::BindableAttribute]
	public ref class CheatData sealed
	{
	public:
		property Platform::String ^CheatCode;
		property Platform::String ^Description;
		property bool Enabled;
	};
}

#endif