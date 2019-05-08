#include <Windows.h>
#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <vector>

const int VERTEXCOUNT = 20;

using namespace DirectX;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

struct VERTEX {
	XMFLOAT3 V;
	XMFLOAT4 COLOR;
};

#define WIN_STYLE WS_OVERLAPPEDWINDOW
int CWIDTH;
int CHEIGHT;

HWND WHandle;
const char *ClassName = "Temp_Window";

int MouseX;
int MouseY;

IDXGISwapChain *pSwapChain;
ID3D11Device *pDevice;
ID3D11DeviceContext *pDeviceContext;
ID3D11RenderTargetView *pBackBuffer_RTV;
ID3D11InputLayout *pVertexLayout;
ID3D11VertexShader *pVertexShader;
ID3D11PixelShader *pPixelShader;
ID3D11Buffer *pVertexBuffer;
ID3D11RasterizerState *pRasterizerState;
ID3D11BlendState *pBlendState;

std::vector<VERTEX> vertices;

LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);
static float t;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hInstancePrev, LPSTR pCmdLine, int nCmdShow) {

	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WinProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = ClassName;
	RegisterClass(&wc);

	WHandle = CreateWindow(ClassName, "title", WIN_STYLE, CW_USEDEFAULT, CW_USEDEFAULT, 1920, 1080, NULL, NULL, hInstance, NULL);
	if (WHandle == NULL) return 0;
	ShowWindow(WHandle, nCmdShow);

	VERTEX v;
	v.V.x = ((float)MouseX / CWIDTH) * 2 - 1;
	v.V.y = ((float)MouseY / CHEIGHT) * -2 + 1;
	v.V.z = 0;
	v.COLOR.w = 0;
	for (int i = 0; i < VERTEXCOUNT; ++i) {
		vertices.push_back(v);
	}

	MSG msg = { 0 };
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			float clearColor[4] = { 0.1, 0.1, 0.1, 1 };
			pDeviceContext->ClearRenderTargetView(pBackBuffer_RTV, clearColor);

			VERTEX v;
			v.V.x = ((float)MouseX / CWIDTH) * 2 - 1;
			v.V.y = ((float)MouseY / CHEIGHT) * -2 + 1;
			v.V.z = 0;
			v.COLOR.w = 1;
			vertices.push_back(v);
			vertices.erase(vertices.begin());
			for (int i = 0; i < VERTEXCOUNT; i++) {
				vertices[i].COLOR.w -= 1.0f / VERTEXCOUNT;
			}
			t += 1.0f / 60;
			t -= (t > g_XMTwoPi.f[0] * 30 ? g_XMTwoPi.f[0] * 30 : 0);
			v.COLOR.x = sin(t / 5) * 0.5f + 0.5f;
			v.COLOR.y = sin(t / 2) * 0.5f + 0.5f;
			v.COLOR.z = sin(t / 3) * 0.5f + 0.5f;
			D3D11_MAPPED_SUBRESOURCE pdata;
			pDeviceContext->Map(pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pdata);
			memcpy_s(pdata.pData, pdata.RowPitch, &(vertices[0]), sizeof(VERTEX) * VERTEXCOUNT);
			pDeviceContext->Unmap(pVertexBuffer, 0);

			pDeviceContext->Draw(VERTEXCOUNT, 0);
			pSwapChain->Present(1, 0);
		}
	}

	return 0;
}


LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch (uMsg) {
	case WM_CREATE:
	{

		RECT csize;
		GetClientRect(hwnd, &csize);
		CWIDTH = csize.right;
		CHEIGHT = csize.bottom;

		DXGI_SWAP_CHAIN_DESC scd = { 0 };
		scd.BufferCount = 1;
		scd.BufferDesc.Width = CWIDTH;
		scd.BufferDesc.Height = CHEIGHT;
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.BufferDesc.RefreshRate.Numerator = 60;
		scd.BufferDesc.RefreshRate.Denominator = 1;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scd.OutputWindow = hwnd;
		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;
		scd.Windowed = TRUE;
		D3D_FEATURE_LEVEL fl = D3D_FEATURE_LEVEL_11_0;
		D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &fl, 1, D3D11_SDK_VERSION, &scd, &pSwapChain, &pDevice, NULL, &pDeviceContext);

		ID3D11Texture2D *pbbTex;
		pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pbbTex);
		pDevice->CreateRenderTargetView(pbbTex, NULL, &pBackBuffer_RTV);
		pbbTex->Release();

		D3D11_VIEWPORT vp;
		vp.Width = CWIDTH;
		vp.Height = CHEIGHT;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;

		ID3DBlob *pCompileVS = NULL;
		ID3DBlob *pCompilePS = NULL;
		D3DCompileFromFile(L"shader.hlsl", NULL, NULL, "VS", "vs_5_0", NULL, 0, &pCompileVS, NULL);
		pDevice->CreateVertexShader(pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), NULL, &pVertexShader);
		D3DCompileFromFile(L"shader.hlsl", NULL, NULL, "PS", "ps_5_0", NULL, 0, &pCompilePS, NULL);
		pDevice->CreatePixelShader(pCompilePS->GetBufferPointer(), pCompilePS->GetBufferSize(), NULL, &pPixelShader);

		D3D11_INPUT_ELEMENT_DESC layout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		pDevice->CreateInputLayout(layout, 2, pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), &pVertexLayout);
		pCompileVS->Release();
		pCompilePS->Release();

		D3D11_BUFFER_DESC cb;
		cb.ByteWidth = sizeof(VERTEX) * VERTEXCOUNT;
		cb.Usage = D3D11_USAGE_DYNAMIC;
		cb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cb.MiscFlags = 0;
		cb.StructureByteStride = sizeof(VERTEX);
		pDevice->CreateBuffer(&cb, NULL, &pVertexBuffer);

		D3D11_RASTERIZER_DESC rdc = {};
		rdc.FillMode = D3D11_FILL_SOLID;
		rdc.CullMode = D3D11_CULL_NONE;
		rdc.FrontCounterClockwise = TRUE;
		pDevice->CreateRasterizerState(&rdc, &pRasterizerState);

		D3D11_BLEND_DESC bd;
		ZeroMemory(&bd, sizeof(D3D11_BLEND_DESC));
		bd.RenderTarget[0].BlendEnable = TRUE;
		bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

		pDevice->CreateBlendState(&bd, &pBlendState);
		float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		UINT sampleMask = 0xffffffff;

		UINT stride = sizeof(VERTEX);
		UINT offset = 0;
		pDeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
		pDeviceContext->IASetInputLayout(pVertexLayout);
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
		pDeviceContext->OMSetRenderTargets(1, &pBackBuffer_RTV, NULL);
		pDeviceContext->RSSetViewports(1, &vp);
		pDeviceContext->VSSetShader(pVertexShader, NULL, 0);
		pDeviceContext->PSSetShader(pPixelShader, NULL, 0);
		pDeviceContext->RSSetState(pRasterizerState);
		pDeviceContext->OMSetBlendState(pBlendState, blendFactor, sampleMask);

		return 0;
	}
	case WM_DESTROY:

		pSwapChain->Release();
		pDeviceContext->Release();
		pDevice->Release();

		pBackBuffer_RTV->Release();

		pRasterizerState->Release();
		pVertexShader->Release();
		pVertexLayout->Release();
		pPixelShader->Release();
		pVertexBuffer->Release();
		pBlendState->Release();

		PostQuitMessage(0);
		return 0;
	case WM_LBUTTONDOWN:
		InvalidateRect(WHandle, NULL, TRUE);
		break;

	case WM_LBUTTONUP:
		InvalidateRect(WHandle, NULL, TRUE);
		break;

	case WM_MOUSEMOVE:
		MouseX = LOWORD(lParam);
		MouseY = HIWORD(lParam);
		InvalidateRect(WHandle, NULL, TRUE);
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}