#include "Game/Game.h"

using Microsoft::WRL::ComPtr;
namespace library
{
    /*--------------------------------------------------------------------
      Global Variables
    --------------------------------------------------------------------*/

    HINSTANCE               g_hInst = nullptr;
    HWND                    g_hWnd = nullptr;
    D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
    D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
    D3D11_VIEWPORT          g_viewport;

    ComPtr<ID3D11Device>           g_pd3dDevice;
    ComPtr<ID3D11Device1>           g_pd3dDevice1;
    ComPtr<ID3D11DeviceContext>    g_pImmediateContext;
    ComPtr<ID3D11DeviceContext1>    g_pImmediateContext1;
    ComPtr<ID3D11RenderTargetView> g_pRenderTargetView;
    ComPtr<IDXGISwapChain>         g_pSwapChain;
    ComPtr<IDXGISwapChain1>         g_pSwapChain1;
    ComPtr<ID3D11Texture2D>        pBackBuffer;

    /*--------------------------------------------------------------------
      Forward declarations
    --------------------------------------------------------------------*/
    HRESULT InitWindow(_In_ HINSTANCE hInstance, _In_ INT nCmdShow);
    HRESULT InitDevice();
    void CleanupDevice();
    void Render();

    /*F+F+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      Function: WindowProc

      Summary:  Defines the behavior of the window—its appearance, how
                it interacts with the user, and so forth

      Args:     HWND hWnd
                  Handle to the window
                UINT uMsg
                  Message code
                WPARAM wParam
                  Additional data that pertains to the message
                LPARAM lParam
                  Additional data that pertains to the message

      Returns:  LRESULT
                  Integer value that your program returns to Windows
    -----------------------------------------------------------------F-F*/
    LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

    HRESULT InitWindow(_In_ HINSTANCE hInstance, _In_ INT nCmdShow)
    {
        WNDCLASSEX wclass = {};
        wclass.cbSize = sizeof(WNDCLASSEX);
        wclass.style = CS_HREDRAW | CS_VREDRAW;
        wclass.lpfnWndProc = WindowProc;
        wclass.cbClsExtra = 0;
        wclass.cbWndExtra = 0;
        wclass.hInstance = hInstance;
        wclass.lpszClassName = L"Graphics";
        if (!RegisterClassEx(&wclass))
            return E_FAIL;

        g_hInst = hInstance;
        RECT wRect = { 0, 0, 800, 600 };
        AdjustWindowRect(&wRect, WS_OVERLAPPEDWINDOW, FALSE);

        g_hWnd = CreateWindow(L"Graphics", L"Game Graphics Programming Lab 01: Direct3D 11 Basics",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT, wRect.right - wRect.left, wRect.bottom - wRect.top, nullptr, nullptr, hInstance,
            nullptr);

        if (!g_hWnd)
            return E_FAIL;

        ShowWindow(g_hWnd, nCmdShow);

        return S_OK;
    }

    LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
    {
        PAINTSTRUCT ps;
        HDC hdc;
        switch (uMsg)
        {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
        return 0;
    }


    HRESULT InitDevice()
    {
        HRESULT hr = S_OK;

        RECT wRect;
        GetClientRect(g_hWnd, &wRect);
        UINT width = wRect.right - wRect.left;
        UINT height = wRect.bottom - wRect.top;

        UINT createDeviceFlags = 0;

#ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_DRIVER_TYPE driverTypes[] =
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };
        UINT numDriverTypes = ARRAYSIZE(driverTypes);

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_9_1,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_11_1
        };
        UINT numFeatureLevels = ARRAYSIZE(featureLevels);

        for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
        {
            g_driverType = driverTypes[driverTypeIndex];
            hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);

            if (hr == E_INVALIDARG)
            {
                // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
                hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                    D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
            }

            if (SUCCEEDED(hr))
                break;
        }
        if (FAILED(hr))
            return hr;

        ComPtr<IDXGIFactory1> dxgiFactory;
        {
            ComPtr<IDXGIDevice> dxgiDevice;
            // IDXGIDevice* dxgiDevice = nullptr;
            hr = g_pd3dDevice.As(&dxgiDevice);
            if (SUCCEEDED(hr))
            {
                ComPtr<IDXGIAdapter> adapter;

                hr = dxgiDevice->GetAdapter(adapter.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    hr = adapter->GetParent(__uuidof(IDXGIFactory1), (&dxgiFactory));
                }
            }
        }
        if (FAILED(hr))
            return hr;

        ComPtr<IDXGIFactory2> dxgiFactory2;
        hr = dxgiFactory.As(&dxgiFactory2);
        if (dxgiFactory2)
        {
            // DirectX 11.1 or later
            hr = g_pd3dDevice.As(&g_pd3dDevice1);
            if (SUCCEEDED(hr))
            {
                hr = g_pImmediateContext.As(&g_pImmediateContext1);
            }

            DXGI_SWAP_CHAIN_DESC1 sd = {};
            sd.Width = width;
            sd.Height = height;
            sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.BufferCount = 1;

            hr = dxgiFactory2->CreateSwapChainForHwnd(g_pd3dDevice.Get(), g_hWnd, &sd, nullptr, nullptr, g_pSwapChain1.GetAddressOf());
            if (SUCCEEDED(hr))
            {
                hr = g_pSwapChain1.As(&g_pSwapChain);
            }
        }
        else
        {
            // DirectX 11.0 systems
            DXGI_SWAP_CHAIN_DESC sd = {};
            sd.BufferCount = 1;
            sd.BufferDesc.Width = width;
            sd.BufferDesc.Height = height;
            sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.BufferDesc.RefreshRate.Numerator = 60;
            sd.BufferDesc.RefreshRate.Denominator = 1;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.OutputWindow = g_hWnd;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.Windowed = TRUE;

            hr = dxgiFactory->CreateSwapChain(g_pd3dDevice.Get(), &sd, g_pSwapChain.GetAddressOf());
        }

        // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
        dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);
        if (FAILED(hr))
            return hr;

        hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (&pBackBuffer));
        if (FAILED(hr))
            return hr;

        hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, g_pRenderTargetView.GetAddressOf());
        if (FAILED(hr))
            return hr;

        g_pImmediateContext->OMSetRenderTargets(1, g_pRenderTargetView.GetAddressOf(), nullptr);

        g_viewport.Width = 640;
        g_viewport.Height = 400;
        g_viewport.MinDepth = 0;
        g_viewport.MaxDepth = 1;
        g_viewport.TopLeftX = 0;
        g_viewport.TopLeftY = 0;
        g_pImmediateContext->RSSetViewports(1, &g_viewport);

        return S_OK;
    }


    void Render()
    {
        g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView.Get(), Colors::MidnightBlue);
        g_pSwapChain->Present(0, 0);
    }

    void CleanupDevice()
    {
        if (g_pd3dDevice) g_pd3dDevice->Release();
        if (g_pd3dDevice1) g_pd3dDevice1->Release();
        if (g_pImmediateContext) g_pImmediateContext->Release();
        if (g_pImmediateContext1) g_pImmediateContext1->Release();
        if (g_pSwapChain) g_pSwapChain->Release();
        if (g_pRenderTargetView) g_pRenderTargetView->Release();
        if (pBackBuffer) pBackBuffer->Release();

    }
}