#include "Renderer/Renderer.h"

namespace library
{
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Renderer

      Summary:  Constructor

      Modifies: [m_driverType, m_featureLevel, m_d3dDevice, m_d3dDevice1,
                 m_immediateContext, m_immediateContext1, m_swapChain,
                 m_swapChain1, m_renderTargetView, m_depthStencil,
                 m_depthStencilView, m_cbChangeOnResize, m_camera,
                 m_projection, m_renderables, m_vertexShaders, 
                 m_pixelShaders].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    Renderer::Renderer()
        : m_driverType(D3D_DRIVER_TYPE_NULL)
        , m_featureLevel(D3D_FEATURE_LEVEL_11_0)
        , m_d3dDevice(nullptr)
        , m_d3dDevice1(nullptr)
        , m_immediateContext(nullptr)
        , m_immediateContext1(nullptr)
        , m_swapChain(nullptr)
        , m_swapChain1(nullptr)
        , m_renderTargetView(nullptr)
        , m_depthStencil(nullptr)
        , m_depthStencilView(nullptr)
        , m_cbChangeOnResize(nullptr)
        , m_cbLights(nullptr)
        , m_pszMainSceneName(L"")
        , m_projection(XMMatrixIdentity())
        , m_camera(Camera(XMVectorSet( 0.f, -1.5f, 0.f, 0.f )))
        , m_renderables(std::unordered_map<std::wstring, std::shared_ptr<Renderable>>())
        , m_vertexShaders(std::unordered_map<std::wstring, std::shared_ptr<VertexShader>>())
        , m_pixelShaders(std::unordered_map<std::wstring, std::shared_ptr<PixelShader>>())
        , m_scenes(std::unordered_map<std::wstring, std::shared_ptr<Scene>>())
    {}

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Initialize

      Summary:  Creates Direct3D device and swap chain

      Args:     HWND hWnd
                  Handle to the window

      Modifies: [m_d3dDevice, m_featureLevel, m_immediateContext,
                 m_d3dDevice1, m_immediateContext1, m_swapChain1,
                 m_swapChain, m_renderTargetView, m_cbChangeOnResize, 
                 m_projection, m_cbLights, m_camera, m_vertexShaders, 
                 m_pixelShaders, m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::Initialize(_In_ HWND hWnd)
    {
        HRESULT hr = S_OK;
        RECT rc;

        GetClientRect(hWnd, &rc);

        UINT width = rc.right - static_cast<UINT>(rc.left);
        UINT height = rc.bottom - static_cast<UINT>(rc.top);

        UINT createDeviceFlags = 0;

        D3D_DRIVER_TYPE driverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
        UINT numDriverTypes = ARRAYSIZE(driverTypes);

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        UINT numFeatureLevels = ARRAYSIZE(featureLevels);

        for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
        {
            m_driverType = driverTypes[driverTypeIndex];
            hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());

            if (hr == E_INVALIDARG)
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, m_d3dDevice.GetAddressOf(), &m_featureLevel, m_immediateContext.GetAddressOf());
            }

            if (SUCCEEDED(hr))
            {
                break;
            }

        }
        if (FAILED(hr))
        {
            return hr;
        }

        // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
        ComPtr<IDXGIFactory1> dxgiFactory(nullptr);
        {
            ComPtr<IDXGIDevice> dxgiDevice(nullptr);
            hr = m_d3dDevice.As(&dxgiDevice);
            if (SUCCEEDED(hr))
            {
                ComPtr<IDXGIAdapter> adapter(nullptr);

                hr = dxgiDevice->GetAdapter(adapter.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = adapter->GetParent(__uuidof(IDXGIFactory1), (&dxgiFactory));
                }
            }
        }
        if (FAILED(hr))
        {
            return hr;
        }

        // Create swap chain
        ComPtr<IDXGIFactory2> dxgiFactory2(nullptr);
        hr = dxgiFactory.As(&dxgiFactory2);
        if (dxgiFactory2)
        {
            // DirectX 11.1 or later
            hr = m_d3dDevice.As(&m_d3dDevice1);
            if (SUCCEEDED(hr))
            {
                hr = m_immediateContext.As(&m_immediateContext1);
            }

            DXGI_SWAP_CHAIN_DESC1 sd =
            {
                .Width = width,
                .Height = height,
                .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                .SampleDesc = {.Count = 1, .Quality = 0},
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1
            };

            hr = dxgiFactory2->CreateSwapChainForHwnd(m_d3dDevice.Get(), hWnd, &sd, nullptr, nullptr, m_swapChain1.GetAddressOf());
            if (SUCCEEDED(hr))
            {
                hr = m_swapChain1.As(&m_swapChain);
            }
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd =
            {
                .BufferDesc =
                {
                    .Width = width,
                    .Height = height,
                    .RefreshRate = {.Numerator = 60, .Denominator = 1},
                    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
                },
                .SampleDesc = {.Count = 1, .Quality = 1},
                .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
                .BufferCount = 1,
                .OutputWindow = hWnd,
                .Windowed = TRUE,
            };

            hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(), &sd, m_swapChain.GetAddressOf());
        }

        dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);


        if (FAILED(hr))
        {
            return hr;
        }

        // Create a render target view
        ComPtr<ID3D11Texture2D> pBackBuffer(nullptr);

        hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (&pBackBuffer));
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_d3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Create a depth stencil texture 
        D3D11_TEXTURE2D_DESC descDepth =
        {
            .Width = width,
            .Height = height,
            .MipLevels = 1,
            .ArraySize = 1,
            .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
            .SampleDesc = {.Count = 1, .Quality = 0},
            .Usage = D3D11_USAGE_DEFAULT,
            .BindFlags = D3D11_BIND_DEPTH_STENCIL,
            .CPUAccessFlags = 0,
            .MiscFlags = 0
        };

        hr = m_d3dDevice->CreateTexture2D(&descDepth, nullptr, m_depthStencil.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV =
        {
            .Format = descDepth.Format,
            .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
            .Texture2D = {.MipSlice = 0 }
        };

        hr = m_d3dDevice->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, m_depthStencilView.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        m_immediateContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

        // Setup the viewport
        D3D11_VIEWPORT vp =
        {
            .TopLeftX = 0,
            .TopLeftY = 0,
            .Width = (FLOAT)width,
            .Height = (FLOAT)height,
            .MinDepth = 0.0f,
            .MaxDepth = 1.0f
        };

        m_immediateContext->RSSetViewports(1, &vp);

        m_camera.Initialize(m_d3dDevice.Get());
        if (FAILED(hr))
        {
            return hr;
        }

        

        // Create constant buffer
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(CBChangeOnResize);
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = 0;

        hr = m_d3dDevice->CreateBuffer(&bd, nullptr, m_cbChangeOnResize.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        CBChangeOnResize cr;
        m_projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, width / (FLOAT)height, 0.01f, 100.0f);
        cr.Projection = XMMatrixTranspose(m_projection);
        m_immediateContext->UpdateSubresource(m_cbChangeOnResize.Get(), 0, nullptr, &cr, 0, 0);

        D3D11_BUFFER_DESC bdl = {};
        bdl.ByteWidth = sizeof(CBLights);
        bdl.Usage = D3D11_USAGE_DEFAULT;
        bdl.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bdl.CPUAccessFlags = 0;

        hr = m_d3dDevice->CreateBuffer(&bdl, nullptr, m_cbLights.GetAddressOf());
        if (FAILED(hr))
        {
            return hr;
        }

        // Set primitive topology
        m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        for (auto it = m_renderables.begin(); it != m_renderables.end(); it++) {
            hr = it->second->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());

            if (FAILED(hr))
            {
                return hr;
            }
        }

        for (auto it = m_vertexShaders.begin(); it != m_vertexShaders.end(); it++) {
            hr = it->second->Initialize(m_d3dDevice.Get());

            if (FAILED(hr))
            {
                return hr;
            }
        }

        for (auto it = m_pixelShaders.begin(); it != m_pixelShaders.end(); it++) {
            hr = it->second->Initialize(m_d3dDevice.Get());

            if (FAILED(hr))
            {
                return hr;
            }
        }

        for (auto it = m_scenes.begin(); it != m_scenes.end(); ++it)
        {
            for (int i = 0; i < it->second->GetVoxels().size(); ++i)
            {
                it->second->GetVoxels()[i]->Initialize(m_d3dDevice.Get(), m_immediateContext.Get());
            }

        }

        return S_OK;
    }
    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddRenderable

      Summary:  Add a renderable object

      Args:     PCWSTR pszRenderableName
                  Key of the renderable object
                const std::shared_ptr<Renderable>& renderable
                  Shared pointer to the renderable object

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddRenderable(_In_ PCWSTR pszRenderableName, _In_ const std::shared_ptr<Renderable>& renderable)
    {
        // Checks if the key already exists in the renderables hash map
        // If the key already exists in hashmap, return E_FAIL
        if (m_renderables.contains(pszRenderableName))
        {
            return E_FAIL;
        }

        // Else, add the renderable and return S_OK
        else
        {
            m_renderables.insert(std::pair<std::wstring, std::shared_ptr<Renderable>>(pszRenderableName, renderable));
            return S_OK;
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddPointLight

      Summary:  Add a point light

      Args:     size_t index
                  Index of the point light
                const std::shared_ptr<PointLight>& pointLight
                  Shared pointer to the point light object

      Modifies: [m_aPointLights].

      Returns:  HRESULT
                  Status code.
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddPointLight(_In_ size_t index, _In_ const std::shared_ptr<PointLight>& pointLight)
    {
        if (index >= NUM_LIGHTS)
        {
            return E_FAIL;
        }
        else
        {
            m_aPointLights[index] = pointLight;
        }
        return S_OK;
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddVertexShader

      Summary:  Add the vertex shader into the renderer

      Args:     PCWSTR pszVertexShaderName
                  Key of the vertex shader
                const std::shared_ptr<VertexShader>&
                  Vertex shader to add

      Modifies: [m_vertexShaders].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddVertexShader(_In_ PCWSTR pszVertexShaderName, _In_ const std::shared_ptr<VertexShader>& vertexShader)
    {
        // Checks if the key already exists in the vertex shaders hash map
        // If the key already exists in hashmap, return E_FAIL
        if (m_vertexShaders.contains(pszVertexShaderName))
        {
            return E_FAIL;
        }

        // Else, add the vertex shaderand return S_OK*/
        else
        {
            m_vertexShaders.insert(std::pair<std::wstring, std::shared_ptr<VertexShader>>(pszVertexShaderName, vertexShader));
            return S_OK;
        }

    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddPixelShader

      Summary:  Add the pixel shader into the renderer

      Args:     PCWSTR pszPixelShaderName
                  Key of the pixel shader
                const std::shared_ptr<PixelShader>&
                  Pixel shader to add

      Modifies: [m_pixelShaders].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddPixelShader(_In_ PCWSTR pszPixelShaderName, _In_ const std::shared_ptr<PixelShader>& pixelShader)
    {

        // Checks if the key already exists in the pixel shaders hash map
        // If the key already exists in hashmap, return E_FAIL
        if (m_pixelShaders.contains(pszPixelShaderName))
        {
            return E_FAIL;
        }

        // Else, add the pixel shaderand return S_OK
        else
        {
            m_pixelShaders.insert(std::pair<std::wstring, std::shared_ptr<PixelShader>>(pszPixelShaderName, pixelShader));
            return S_OK;
        }

    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::AddScene

      Summary:  Add a scene

      Args:     PCWSTR pszSceneName
                  Key of a scene
                const std::filesystem::path& sceneFilePath
                  File path to initialize a scene

      Modifies: [m_scenes].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::AddScene(_In_ PCWSTR pszSceneName, const std::filesystem::path& sceneFilePath)
    {
        if (m_scenes.contains(pszSceneName))
        {
            return E_FAIL;
        }
        else
        {
            m_scenes.insert(std::make_pair(pszSceneName, std::make_shared<Scene>(sceneFilePath)));
            return S_OK;
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetMainScene

      Summary:  Set the main scene

      Args:     PCWSTR pszSceneName
                  Name of the scene to set as the main scene

      Modifies: [m_pszMainSceneName].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetMainScene(_In_ PCWSTR pszSceneName)
    {
        if (m_scenes.contains(pszSceneName))
        {
            m_pszMainSceneName = pszSceneName;
            return S_OK;
        }
        else
        {
            return E_FAIL;
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::HandleInput

      Summary:  Add the pixel shader into the renderer and initialize it

      Args:     const DirectionsInput& directions
                  Data structure containing keyboard input data
                const MouseRelativeMovement& mouseRelativeMovement
                  Data structure containing mouse relative input data

      Modifies: [m_camera].
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::HandleInput(_In_ const DirectionsInput& directions,
        _In_ const MouseRelativeMovement& mouseRelativeMovement, _In_ FLOAT deltaTime)
    {
        m_camera.HandleInput(
            directions,
            mouseRelativeMovement,
            deltaTime
        );
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Update

      Summary:  Update the renderables each frame

      Args:     FLOAT deltaTime
                  Time difference of a frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Update(_In_ FLOAT deltaTime)
    {
        // Update all renderable objects
        for (auto it = m_renderables.begin(); it != m_renderables.end(); it++)
        {
            it->second->Update(deltaTime);
        }

        for (int i = 0; i < NUM_LIGHTS; i++)
        {
            m_aPointLights[i]->Update(deltaTime);
        }


        for (UINT i = 0; i < m_scenes[m_pszMainSceneName]->GetVoxels().size(); i++)
        {
            m_scenes[m_pszMainSceneName]->GetVoxels()[i]->Update(deltaTime);
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::Render

      Summary:  Render the frame
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    void Renderer::Render()
    {
        // Clear the back buffer 
        m_immediateContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::MidnightBlue);

        // Clear the depth buffer to 1.0 (maximum depth)
        m_immediateContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

        CBChangeOnCameraMovement cm;
        cm.View = XMMatrixTranspose(m_camera.GetView());
        XMStoreFloat4(&cm.CameraPosition, m_camera.GetEye());
        m_immediateContext->UpdateSubresource(m_camera.GetConstantBuffer().Get(), 0, nullptr, &cm, 0, 0);

        CBLights cl;
        for (int i = 0; i < NUM_LIGHTS; i++)
        {
            cl.LightColors[i] = m_aPointLights[i]->GetColor();
            cl.LightPositions[i] = m_aPointLights[i]->GetPosition();
        }

        m_immediateContext->UpdateSubresource(m_cbLights.Get(), 0, nullptr, &cl, 0, 0);

        for (auto it = m_renderables.begin(); it != m_renderables.end(); it++)
        {
            // Set the vertex buffer
            UINT stride = sizeof(SimpleVertex);
            UINT offset = 0;
            m_immediateContext->IASetVertexBuffers(0, 1, it->second->GetVertexBuffer().GetAddressOf(), &stride, &offset);

            // Set the index buffer
            m_immediateContext->IASetIndexBuffer(it->second->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

            // set the input buffer
            m_immediateContext->IASetInputLayout(it->second->GetVertexLayout().Get());

            m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            // Update constant buffer for the first cube
            CBChangesEveryFrame cf;
            cf.World = XMMatrixTranspose(it->second->GetWorldMatrix());
            cf.OutputColor = it->second->GetOutputColor();
            m_immediateContext->UpdateSubresource(it->second->GetConstantBuffer().Get(), 0, nullptr, &cf, 0, 0);


            // Render the triangles (cube)
            m_immediateContext->VSSetShader(it->second->GetVertexShader().Get(), nullptr, 0);
            m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
            m_immediateContext->VSSetConstantBuffers(2, 1, it->second->GetConstantBuffer().GetAddressOf());

            m_immediateContext->PSSetShader(it->second->GetPixelShader().Get(), nullptr, 0);
            m_immediateContext->PSSetConstantBuffers(0u, 1u, m_camera.GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(2u, 1u, it->second->GetConstantBuffer().GetAddressOf());
            m_immediateContext->PSSetConstantBuffers(3u, 1u, m_cbLights.GetAddressOf());

            if (it->second->HasTexture())
            {
                for (UINT i = 0; i < it->second->GetNumMeshes(); ++i)
                {
                    UINT materialIndex = it->second->GetMesh(i).uMaterialIndex;
                    m_immediateContext->PSSetShaderResources(0, 1, it->second->GetMaterial(materialIndex).pDiffuse->GetTextureResourceView().GetAddressOf());
                    m_immediateContext->PSSetSamplers(0, 1, it->second->GetMaterial(materialIndex).pDiffuse->GetSamplerState().GetAddressOf());
                    m_immediateContext->DrawIndexed(it->second->GetMesh(i).uNumIndices, it->second->GetMesh(i).uBaseIndex, it->second->GetMesh(i).uBaseVertex);

                }
            }

            m_immediateContext->DrawIndexed(it->second->GetNumIndices(), 0, 0);

        }
        
        for (auto it = m_scenes.begin(); it != m_scenes.end(); it++)
        {
            for (UINT i = 0; i < it->second->GetVoxels().size(); i++)
            {
                UINT stride = sizeof(SimpleVertex);
                UINT offset = 0;
                m_immediateContext->IASetVertexBuffers(0, 1, it->second->GetVoxels()[i]->GetVertexBuffer().GetAddressOf(), &stride, &offset);
        
                UINT stride0 = sizeof(InstanceData);
                m_immediateContext->IASetVertexBuffers(1, 1, it->second->GetVoxels()[i]->GetInstanceBuffer().GetAddressOf(), &stride0, &offset);

                m_immediateContext->IASetIndexBuffer(it->second->GetVoxels()[i]->GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

                m_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

                m_immediateContext->IASetInputLayout(it->second->GetVoxels()[i]->GetVertexLayout().Get());

                CBChangesEveryFrame cf;
                cf.World = XMMatrixTranspose(it->second->GetVoxels()[i]->GetWorldMatrix());
                cf.OutputColor = it->second->GetVoxels()[i]->GetOutputColor();
                m_immediateContext->UpdateSubresource(it->second->GetVoxels()[i]->GetConstantBuffer().Get(), 0, nullptr, &cf, 0, 0);

                m_immediateContext->VSSetShader(it->second->GetVoxels()[i]->GetVertexShader().Get(), nullptr, 0);
                m_immediateContext->VSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(1, 1, m_cbChangeOnResize.GetAddressOf());
                m_immediateContext->VSSetConstantBuffers(2, 1, it->second->GetVoxels()[i]->GetConstantBuffer().GetAddressOf());

                m_immediateContext->PSSetShader(it->second->GetVoxels()[i]->GetPixelShader().Get(), nullptr, 0);
                m_immediateContext->PSSetConstantBuffers(0, 1, m_camera.GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(2, 1, it->second->GetVoxels()[i]->GetConstantBuffer().GetAddressOf());
                m_immediateContext->PSSetConstantBuffers(3, 1, m_cbLights.GetAddressOf());

                //draw
                m_immediateContext->DrawIndexedInstanced(it->second->GetVoxels()[i]->GetNumIndices(), it->second->GetVoxels()[i]->GetNumInstances(), 0, 0, 0);
            }
        }
        // Present the information rendered to the back buffer to the front buffer
        m_swapChain->Present(0, 0);
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfRenderable

      Summary:  Sets the vertex shader for a renderable

      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszVertexShaderName
                  Key of the vertex shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetVertexShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszVertexShaderName)
    {
        if (m_renderables.contains(pszRenderableName))
        {
            m_renderables.at(pszRenderableName)->SetVertexShader(m_vertexShaders.at(pszVertexShaderName));
            return S_OK;
        }
        else
        {
            return E_FAIL;
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfRenderable

      Summary:  Sets the pixel shader for a renderable

      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszPixelShaderName
                  Key of the pixel shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetPixelShaderOfRenderable(_In_ PCWSTR pszRenderableName, _In_ PCWSTR pszPixelShaderName)
    {
        if (m_renderables.contains(pszRenderableName))
        {
            m_renderables.at(pszRenderableName)->SetPixelShader(m_pixelShaders.at(pszPixelShaderName));
            return S_OK;
        }
        
        else
        {
            return E_FAIL;
        }
    }

    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetVertexShaderOfScene

      Summary:  Sets the vertex shader for the voxels in a scene

      Args:     PCWSTR pszSceneName
                  Key of the scene
                PCWSTR pszVertexShaderName
                  Key of the vertex shader

      Modifies: [m_scenes].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetVertexShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszVertexShaderName)
    {
        if (m_scenes.contains(pszSceneName))
        {
            for (int i = 0; i < m_scenes[pszSceneName]->GetVoxels().size(); i++)
            {
                m_scenes[pszSceneName]->GetVoxels()[i]->SetVertexShader(m_vertexShaders[pszVertexShaderName]);
            }
            return S_OK;
        }
        else
        {
            return E_FAIL;
        }
    }


    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::SetPixelShaderOfScene

      Summary:  Sets the pixel shader for the voxels in a scene

      Args:     PCWSTR pszRenderableName
                  Key of the renderable
                PCWSTR pszPixelShaderName
                  Key of the pixel shader

      Modifies: [m_renderables].

      Returns:  HRESULT
                  Status code
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    HRESULT Renderer::SetPixelShaderOfScene(_In_ PCWSTR pszSceneName, _In_ PCWSTR pszPixelShaderName)
    {
        if (m_scenes.contains(pszSceneName))
        {
            for (int i = 0; i < m_scenes[pszSceneName]->GetVoxels().size(); i++)
            {
                m_scenes[pszSceneName]->GetVoxels()[i]->SetPixelShader(m_pixelShaders[pszPixelShaderName]);
            }
            return S_OK;
        }
        else
        {
            return E_FAIL;
        }
    }



    /*M+M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M+++M
      Method:   Renderer::GetDriverType

      Summary:  Returns the Direct3D driver type

      Returns:  D3D_DRIVER_TYPE
                  The Direct3D driver type used
    M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M---M-M*/
    D3D_DRIVER_TYPE Renderer::GetDriverType() const
    {
        return m_driverType;
    }
}