#pragma once

#include <d3d11_1.h>
#include <collection.h>




namespace VBA10
{
	class ShaderManager
	{
	public: 
		

		ShaderManager(ID3D11Device1 *device, ID3D11DeviceContext1 *context);
		static ShaderManager *GetInstance();

		ID3D11PixelShader* GetCurrentShader() {return this->ps.Get();};
		ID3D11ShaderResourceView* GetLookUpTable() { return this->lutSRV.Get(); };
		void LoadShader(int selection);
	private:
		Microsoft::WRL::ComPtr<ID3D11Device1> device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext1> context;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	lutSRV;
		Microsoft::WRL::ComPtr<ID3D11Resource>				lutResource;

		static ShaderManager *instance;
	};
}
