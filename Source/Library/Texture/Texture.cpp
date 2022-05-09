#include "Texture.h"

#include "Texture/WICTextureLoader.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Texture::Texture

      Summary:  Constructor

      Args:     const std::filesystem::path& textureFilePath
                  Path to the texture to use

      Modifies: [m_filePath, m_textureRV, m_samplerLinear].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    Texture::Texture(_In_ const std::filesystem::path& filePath)
        : m_filePath(filePath)
        , m_textureRV(nullptr)
        , m_samplerLinear(nullptr)
    {}

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Texture::Initialize

      Summary:  Initializes the texture

      Args:     ID3D11Device* pDevice
                  The Direct3D device to create the buffers
                ID3D11DeviceContext* pImmediateContext
                  The Direct3D context to set buffers

      Modifies: [m_textureRV, m_samplerLinear].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Texture::Initialize(_In_ ID3D11Device* pDevice, ID3D11DeviceContext* pImmediateContext)
    {
        HRESULT hr = S_OK;
        hr = CreateWICTextureFromFile(
            pDevice,
            pImmediateContext,
            m_filePath.c_str(),
            nullptr,
            &m_textureRV
        );

        if (FAILED(hr))
        {
            return hr;
        }

        // Create the Sampler State
        D3D11_SAMPLER_DESC sd = {};
        sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sd.MinLOD = 0;
        sd.MaxLOD = D3D11_FLOAT32_MAX;
        hr = pDevice->CreateSamplerState(&sd, m_samplerLinear.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Texture::GetTextureResourceView

      Summary:  Constructor

      Returns:  ComPtr<ID3D11ShaderResourceView>&
                  Shader resource view
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11ShaderResourceView>& Texture::GetTextureResourceView()
    {
        return m_textureRV;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Texture::GetSamplerState

      Summary:  Constructor

      Returns:  ComPtr<ID3D11SamplerState>&
                  Sampler state
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    ComPtr<ID3D11SamplerState>& Texture::GetSamplerState()
    {
        return m_samplerLinear;
    }
}