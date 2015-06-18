#pragma once

#include <wrl.h>
#include <wrl/implements.h>
#include <windows.storage.streams.h>
#include <robuffer.h>
#include <vector>

// Modified from http://stackoverflow.com/questions/10520335/how-to-wrap-a-char-buffer-in-a-winrt-ibuffer-in-c 
namespace SQLiteWinRT
{
	class NativeBuffer : 
		public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::RuntimeClassType::WinRtClassicComMix>,
		ABI::Windows::Storage::Streams::IBuffer,
		Windows::Storage::Streams::IBufferByteAccess>
	{
	public:
		virtual ~NativeBuffer()
		{
		}

		STDMETHODIMP RuntimeClassInitialize(byte *buffer, UINT totalSize)
		{
			m_length = totalSize;
			//m_buffer = buffer;
			m_buffer = std::unique_ptr<byte[]>(new byte[totalSize]);
			memcpy(m_buffer.get(), buffer, totalSize);

			return S_OK;
		}

		STDMETHODIMP Buffer(byte **value)
		{
			//*value = m_buffer;
			*value = m_buffer.get();

			return S_OK;
		}

		STDMETHODIMP get_Capacity(UINT32 *value)
		{
			*value = m_length;

			return S_OK;
		}

		STDMETHODIMP get_Length(UINT32 *value)
		{
			*value = m_length;

			return S_OK;
		}

		STDMETHODIMP put_Length(UINT32 value)
		{
			m_length = value;

			return S_OK;
		}

	private:
		UINT32 m_length;
		//byte *m_buffer;
		std::unique_ptr<byte[]> m_buffer;
	};
}