#pragma once
#include "../../Common.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include "d3dx12.h"

#include <wrl/client.h>

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

#define HR_VFY(hr) assert(SUCCEEDED(hr))

