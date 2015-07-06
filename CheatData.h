#pragma once

namespace VBA10
{
	[Windows::UI::Xaml::Data::BindableAttribute]
	public ref class CheatData sealed
	{
	public:
		property Platform::String^ CheatCode
		{
			Platform::String^ get()
			{
				return _cheatcode;
			}
			void set(Platform::String^ value)
			{
				_cheatcode = value;
			}
		}

		property Platform::String^ Description
		{
			Platform::String^ get()
			{
				return _description;
			}
			void set(Platform::String^ value)
			{
				_description = value;
			}
		}

		property bool Enabled
		{
			bool get()
			{
				return _enabled;
			}
			void set(bool value)
			{
				_enabled = value;
			}
		}

	private:
		Platform::String ^ _cheatcode;
		Platform::String ^ _description;
		bool _enabled;

	};
}

