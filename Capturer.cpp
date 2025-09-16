#include "Capturer.h"
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#include <dxgi1_2.h>
#include <wrl/client.h>
#include <iostream>
#include <fstream>
#include <filesystem>

using Microsoft::WRL::ComPtr;

// save to BMP file
bool Capturer::SaveBitmapToFile(BYTE* pBitmapBits, LONG width, LONG height, WORD bitsPerPixel, const std::string& filePath)
{
	DWORD dwSizeofDIB = ((width * bitsPerPixel + 31) / 32) * 4 * height;
	BITMAPFILEHEADER bmfHeader = { 0 };
	BITMAPINFOHEADER bi = { 0 };

	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biHeight = -height; // top-down
	bi.biPlanes = 1;
	bi.biBitCount = bitsPerPixel;
	bi.biCompression = BI_RGB;

	bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmfHeader.bfSize = bmfHeader.bfOffBits + dwSizeofDIB;
	bmfHeader.bfType = 0x4D42; // BM
	std::filesystem::path path(filePath);
	std::filesystem::create_directories(path.parent_path());
	std::ofstream file(path, std::ios::out | std::ios::binary);
	if (!file) return false;

	file.write((char*)&bmfHeader, sizeof(bmfHeader));
	file.write((char*)&bi, sizeof(bi));
	file.write((char*)pBitmapBits, dwSizeofDIB);
	file.close();
	std::cout << "Saved bitmap to " << filePath << "\n";
	return true;
}


int Capturer::Capture(const std::string& ImagePath)
{
	HRESULT hr;

	ComPtr<ID3D11Device> d3dDevice;
	ComPtr<ID3D11DeviceContext> d3dContext;
	D3D_FEATURE_LEVEL featureLevel;

	hr = D3D11CreateDevice(
		nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
		nullptr, 0, D3D11_SDK_VERSION,
		&d3dDevice, &featureLevel, &d3dContext);
	if (FAILED(hr)) {
		std::cerr << "D3D11CreateDevice failed\n";
		return -1;
	}

	ComPtr<IDXGIDevice> dxgiDevice;
	d3dDevice.As(&dxgiDevice);

	ComPtr<IDXGIAdapter> dxgiAdapter;
	dxgiDevice->GetAdapter(&dxgiAdapter);

	ComPtr<IDXGIOutput> dxgiOutput;
	dxgiAdapter->EnumOutputs(0, &dxgiOutput);

	ComPtr<IDXGIOutput1> dxgiOutput1;
	dxgiOutput.As(&dxgiOutput1);

	ComPtr<IDXGIOutputDuplication> deskDupl;
	hr = dxgiOutput1->DuplicateOutput(d3dDevice.Get(), &deskDupl);
	if (FAILED(hr)) {
		std::cerr << "DuplicateOutput failed (need Win8+)\n";
		return -1;
	}

	DXGI_OUTDUPL_FRAME_INFO frameInfo;
	ComPtr<IDXGIResource> desktopResource;
	hr = deskDupl->AcquireNextFrame(500, &frameInfo, &desktopResource);
	if (FAILED(hr)) {
		std::cerr << "AcquireNextFrame failed\n";
		return -1;
	}

	ComPtr<ID3D11Texture2D> acquiredImage;
	desktopResource.As(&acquiredImage);

	D3D11_TEXTURE2D_DESC desc;
	acquiredImage->GetDesc(&desc);

	D3D11_TEXTURE2D_DESC descCopy = desc;
	descCopy.Usage = D3D11_USAGE_STAGING;
	descCopy.BindFlags = 0;
	descCopy.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	descCopy.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> cpuTex;
	hr = d3dDevice->CreateTexture2D(&descCopy, nullptr, &cpuTex);
	if (FAILED(hr)) {
		std::cerr << "CreateTexture2D failed\n";
		return -1;
	}

	d3dContext->CopyResource(cpuTex.Get(), acquiredImage.Get());

	D3D11_MAPPED_SUBRESOURCE mapped;
	hr = d3dContext->Map(cpuTex.Get(), 0, D3D11_MAP_READ, 0, &mapped);
	if (FAILED(hr)) {
		std::cerr << "Map failed\n";
		return -1;
	}

	int width = desc.Width;
	int height = desc.Height;
	int bpp = 32; // DXGI_FORMAT_B8G8R8A8_UNORM
	int rowPitch = width * 4;

	BYTE* pData = new BYTE[rowPitch * height];
	for (int y = 0; y < height; y++) {
		memcpy(pData + y * rowPitch,
			(BYTE*)mapped.pData + y * mapped.RowPitch,
			rowPitch);
	}
	
	SaveBitmapToFile(pData, width, height, bpp, ImagePath);

	delete[] pData;
	d3dContext->Unmap(cpuTex.Get(), 0);

	deskDupl->ReleaseFrame();

	return 0;
}

