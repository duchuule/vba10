#ifndef DXSPRITEBATCH_H_
#define DXSPRITEBATCH_H_

#include <d3d11.h>
#include <queue>
#include <DirectXMath.h>
#include <collection.h>
#include "Color.h"
#include "Vector2.h"
#include "Rectangle.h"

using namespace DirectX;
using namespace std;
using namespace Microsoft::WRL;

namespace Engine
{
	struct SpriteInfo
	{
		XMFLOAT4 TargetArea;
		XMFLOAT4 SourceArea;
		Color Color;
		float Rotation;
		float Depth;
		ComPtr<ID3D11ShaderResourceView> TextureSRV;
		ComPtr<ID3D11Texture2D> Texture;
		/*ResourceID TextureID;
		ContentType ContentType;*/
	};

	struct SpriteVertex
	{
		XMFLOAT3 Position;
		Color Color;
		XMFLOAT2 TexCoord;

	public:
		SpriteVertex() { }

		SpriteVertex(float x, float y, float z, float tx, float ty)
			: Position(XMFLOAT3(x,y,z)), TexCoord(XMFLOAT2(tx, ty))
		{ }
	};

	class DXSpriteBatch
	{
	private:
		ComPtr<ID3D11Device1> device;
		ComPtr<ID3D11DeviceContext1> context;
		ComPtr<ID3D11PixelShader> ps;
		ComPtr<ID3D11PixelShader> customPS;
		ComPtr<ID3D11VertexShader> vs;
		ComPtr<ID3D11InputLayout> inputLayout;
		ComPtr<ID3D11Buffer> onFrameBuffer;
		ComPtr<ID3D11Buffer> onResizeBuffer;
		ComPtr<ID3D11Buffer> vertexBuffer;
		ComPtr<ID3D11Buffer> indexBuffer;

		ComPtr<ID3D11BlendState> alphaBlendState;
		ComPtr<ID3D11BlendState> additiveBlendState;
		ComPtr<ID3D11DepthStencilState> depthStencilState;
		ComPtr<ID3D11RasterizerState> rasterizerState;
		ComPtr<ID3D11SamplerState> samplerState;

		ComPtr<ID3D11ShaderResourceView> customSRV; //use for look up table or anything

		bool beginCalled;
		UINT batchedSprites;
		vector<SpriteInfo> queuedSprites;
		bool (*heapSort)(const SpriteInfo &, const SpriteInfo &);

		void LoadShaders(void);
		void UpdateProjectionMatrix(float width, float height);
		void InitializeBuffers(void);
		void CreateStates();
		void FlushBatch(void);
		void RenderBatch(UINT start, UINT end, SpriteInfo &spriteInfo);
		void QueueSprite(SpriteInfo &info);

		bool _useFilter;
	public:
		DXSpriteBatch(ID3D11Device1 *device, ID3D11DeviceContext1 *context, float width, float height);
		~DXSpriteBatch(void);

		void OnResize(float width, float height);
		void SetCustomPixelShader(void *customPS);
		void SetCustomShaderResourceView(void *customSRV);

		void Begin(XMMATRIX &world, bool filter);
		void Draw(const Rectangle &targetArea, const Rectangle *sourceArea, ID3D11ShaderResourceView *textureSRV, ID3D11Texture2D *texture, float depth, float rotation, Color &color);
		void Draw(const Rectangle &target, const Rectangle *source, ID3D11ShaderResourceView *textureSRV, ID3D11Texture2D *texture, float depth, Color &color);
		void Draw(const Rectangle &target, const Rectangle *source, ID3D11ShaderResourceView *textureSRV, ID3D11Texture2D *texture, Color &color);
		void Draw(const Rectangle &target, ID3D11ShaderResourceView *textureSRV, ID3D11Texture2D *texture, float depth, float rotation, Color &color);
		void Draw(const Rectangle &target, ID3D11ShaderResourceView *textureSRV, ID3D11Texture2D *texture, float depth, Color &color);
		void Draw(const Rectangle &target, ID3D11ShaderResourceView *textureSRV, ID3D11Texture2D *texture, Color &color);
		void Draw(const Vector2 &target, ID3D11ShaderResourceView *textureSRV, ID3D11Texture2D *texture, float depth, float rotation, Color &color);
		void Draw(const Vector2 &target, ID3D11ShaderResourceView *textureSRV, ID3D11Texture2D *texture, float depth, Color &color);
		void Draw(const Vector2 &target, ID3D11ShaderResourceView *textureSRV, ID3D11Texture2D *texture, Color &color);
		void End(void);
	};

	bool HeapCompareByTexture(const SpriteInfo &info1, const SpriteInfo &info2);
	bool HeapCompareBackToFront(const SpriteInfo &info1, const SpriteInfo &info2);
	bool HeapCompareFrontToBack(const SpriteInfo &info1, const SpriteInfo &info2);
}

#endif