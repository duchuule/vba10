#pragma once

#include <d3d11_1.h>
#include <collection.h>




namespace VBA10
{
	class ShaderManager
	{
	public: 
		

		ShaderManager(ID3D11Device1 *device);
		static ShaderManager *GetInstance();

		ID3D11PixelShader* GetCurrentShader() {return this->ps.Get();};
		void LoadShader(int selection);
	private:
		Microsoft::WRL::ComPtr<ID3D11Device1> device;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;
		static ShaderManager *instance;
	};
}
