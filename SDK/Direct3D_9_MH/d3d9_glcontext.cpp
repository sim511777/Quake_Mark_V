/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/


#ifdef DIRECT3D9_WRAPPER
#include "d3d9_internal.h"

// link to d3d libraries this way so that we don't need to modify the project
#pragma comment (lib, "d3d9.lib")

globals_t d3d_Globals;


// d3dx crap - because Microsoft played musical chairs with d3dx versioning we can't just statically link and expect everything to work on different PCs,
// so instead we check d3dx dll versions and dynamically load from the best one we can find
HINSTANCE hInstD3DX = NULL;

D3DXLoadSurfaceFromMemoryProc QD3DXLoadSurfaceFromMemory = NULL;
D3DXSaveSurfaceToFileProc QD3DXSaveSurfaceToFile = NULL;
D3DXLoadSurfaceFromSurfaceProc QD3DXLoadSurfaceFromSurface = NULL;


void R_UnloadD3DX (void)
{
	// clear the procs
	QD3DXLoadSurfaceFromMemory = NULL;
	QD3DXSaveSurfaceToFile = NULL;
	QD3DXLoadSurfaceFromSurface = NULL;

	// and unload the library
	if (hInstD3DX)
	{
		FreeLibrary (hInstD3DX);
		hInstD3DX = NULL;
	}
}


BOOL R_TryLoadD3DX (char *libname)
{
	// because we don't have d3dx any more we load it this way - yuck!!!
	if ((hInstD3DX = LoadLibrary (libname)) != NULL)
	{
		// now try to load them load them
		if ((QD3DXLoadSurfaceFromMemory = (D3DXLoadSurfaceFromMemoryProc) GetProcAddress (hInstD3DX, "D3DXLoadSurfaceFromMemory")) == NULL) return FALSE;
		if ((QD3DXSaveSurfaceToFile = (D3DXSaveSurfaceToFileProc) GetProcAddress (hInstD3DX, "D3DXSaveSurfaceToFileA")) == NULL) return FALSE;
		if ((QD3DXLoadSurfaceFromSurface = (D3DXLoadSurfaceFromSurfaceProc) GetProcAddress (hInstD3DX, "D3DXLoadSurfaceFromSurface")) == NULL) return FALSE;

		// loaded OK
		return TRUE;
	}
	else
	{
		// didn't load at all
		return FALSE;
	}
}


void R_LoadD3DX (void)
{
	int i;

	// ensure that D3DX is unloaded before we begin
	R_UnloadD3DX ();

	// starting at 99 to future-proof things a little, 23 and previous were in static libs and
	// there was no plain old "d3dx9.dll"
	for (i = 99; i > 23; i--)
	{
		// try to load this version
		if (R_TryLoadD3DX (va ("d3dx9_%i.dll", i))) return;

		// unload if it didn't
		R_UnloadD3DX ();
	}

	// not sure if this even exists with some versions....
	if (R_TryLoadD3DX ("d3dx9.dll")) return;

	// the HINSTANCE for the library should be valid if it loaded or NULL if it didn't
	System_Error ("R_LoadD3DX : failed to load D3DX\nPlease update your installation of DirectX...");
}


globals_t::globals_t (void)
{
	this->RequestBPP = -1;
	this->RequestStencil = FALSE;
	this->Object = NULL;
}


void globals_t::CreateDirect3D (void)
{
	if (!this->Object)
	{
		if (!(this->Object = Direct3DCreate9 (D3D_SDK_VERSION)))
			System_Error ("Failed to create Direct3D Object");

		if (FAILED (this->Object->GetDeviceCaps (D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &this->DeviceCaps)))
			System_Error ("failed to get object caps");

		// get the format for the desktop mode
		if (FAILED (this->Object->GetAdapterDisplayMode (D3DADAPTER_DEFAULT, &this->DesktopMode)))
			System_Error ("Failed to get desktop mode");

		// now attempt to load d3dx; these pointers will hold good across mode changes so once only is sufficient
		R_LoadD3DX ();
	}
}


void context_t::InitializeStates (void)
{
	// clear
	this->State.Clear.Color = 0x00000000;
	this->State.Clear.Depth = 1.0f;
	this->State.Clear.Stencil = 0;
}


/*
========================
D3D_GetAdapterModeFormat

returns a usable adapter mode for the given width, height and bpp
========================
*/
D3DFORMAT context_t::GetAdapterModeFormat (int width, int height, int bpp)
{
	int i;

	// fill these in depending on bpp
	D3DFORMAT d3d_Formats[4];

	// these are the orders in which we prefer our formats
	if (bpp == -1)
	{
		// unspecified bpp uses the desktop mode format
		d3d_Formats[0] = d3d_Globals.DesktopMode.Format;
		d3d_Formats[1] = D3DFMT_UNKNOWN;
	}
	else if (bpp == 16)
	{
		d3d_Formats[0] = D3DFMT_R5G6B5;
		d3d_Formats[1] = D3DFMT_X1R5G5B5;
		d3d_Formats[2] = D3DFMT_A1R5G5B5;
		d3d_Formats[3] = D3DFMT_UNKNOWN;
	}
	else
	{
		d3d_Formats[0] = D3DFMT_X8R8G8B8;
		d3d_Formats[1] = D3DFMT_A8R8G8B8;
		d3d_Formats[2] = D3DFMT_UNKNOWN;
	}

	for (i = 0; ; i++)
	{
		UINT modecount;
		UINT m;

		// no more modes
		if (d3d_Formats[i] == D3DFMT_UNKNOWN) break;

		// get and validate the number of modes for this format; we expect that this will succeed first time
		modecount = d3d_Globals.Object->GetAdapterModeCount (D3DADAPTER_DEFAULT, d3d_Globals.DesktopMode.Format);

		if (!modecount) continue;

		// check each mode in turn to find a match
		for (m = 0; m < modecount; m++)
		{
			// get this mode
			D3DDISPLAYMODE mode;
			HRESULT hr = d3d_Globals.Object->EnumAdapterModes (D3DADAPTER_DEFAULT, d3d_Globals.DesktopMode.Format, m, &mode);

			// should never happen
			if (FAILED (hr)) continue;

			// d3d8 doesn't specify a format when enumerating so we need to restrict this to the correct format
			if (mode.Format != d3d_Formats[i]);

			// ensure that the texture formats we want to create exist
			if (!D3D_CheckTextureFormat (D3DFMT_L8, d3d_Formats[i])) continue;
			if (!D3D_CheckTextureFormat (D3DFMT_X8R8G8B8, d3d_Formats[i])) continue;
			if (!D3D_CheckTextureFormat (D3DFMT_A8R8G8B8, d3d_Formats[i])) continue;

			// check it against the requested mode
			if (mode.Width == width && mode.Height == height)
			{
				// copy it out and return the mode we got
				memcpy (&this->CurrentMode, &mode, sizeof (D3DDISPLAYMODE));
				return mode.Format;
			}
		}
	}

	// didn't find a format
	return D3DFMT_UNKNOWN;
}


void context_t::SetupPresentParams (int width, int height, int bpp, BOOL windowed)
{
	// clear present params to NULL
	memset (&this->PresentParams, 0, sizeof (D3DPRESENT_PARAMETERS));

	// popup windows are fullscreen always
	if (windowed)
	{
		// defaults for windowed mode - also need to store out clientrect.right and clientrect.bottom
		// (d3d_BPP is only used for fullscreen modes and is retrieved from our CDS override)
		this->CurrentMode.Format = d3d_Globals.DesktopMode.Format;
		this->CurrentMode.Width = width;
		this->CurrentMode.Height = height;
		this->CurrentMode.RefreshRate = 0;
	}
	else
	{
		// also fills in this->CurrentMode
		D3DFORMAT fmt = this->GetAdapterModeFormat (width, height, d3d_Globals.RequestBPP);

		// ensure that we got a good format
		if (fmt == D3DFMT_UNKNOWN)
			System_Error ("failed to get fullscreen mode");
	}

	// create with no vsync by default and we'll switch the mode when the cvars come online
	this->PresentParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	// fill in mode-dependent stuff
	this->PresentParams.BackBufferFormat = this->CurrentMode.Format;
	this->PresentParams.FullScreen_RefreshRateInHz = this->CurrentMode.RefreshRate;
	this->PresentParams.Windowed = windowed;

	// request 1 backbuffer
	this->PresentParams.BackBufferCount = 1;
	this->PresentParams.BackBufferWidth = width;
	this->PresentParams.BackBufferHeight = height;

	this->PresentParams.EnableAutoDepthStencil = TRUE;

	if (d3d_Globals.RequestStencil)
		this->PresentParams.AutoDepthStencilFormat = D3DFMT_D24S8;
	else this->PresentParams.AutoDepthStencilFormat = D3DFMT_D24X8;

	this->PresentParams.MultiSampleType = D3DMULTISAMPLE_NONE;
	this->PresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	this->PresentParams.hDeviceWindow = this->Window;
}


context_t::context_t (HDC hdc)
{
	// Baker didn't like this work being done in wglMakeCurrent so i moved it to wglCreateContext, which i guess it
	// was always appropriate for anyway.  in *theory* this would mean that the wrapper could be extended to support
	// multiple contexts if all the global state stuff/etc was pulled out into it's own context struct.

	// the object must be initialized one-time-only
	d3d_Globals.CreateDirect3D ();

	RECT clientrect;
	LONG winstyle;

	// we can't extern mainwindow as it may be called something else depending on the codebase used
	if ((this->Window = WindowFromDC (hdc)) == NULL) System_Error ("Direct3D9_wglCreateContext: could not determine application window");

	// get the dimensions of the window
	GetClientRect (this->Window, &clientrect);

	// see are we fullscreen
	winstyle = GetWindowLong (this->Window, GWL_STYLE);

	// setup our present parameters (popup windows are fullscreen always)
	this->SetupPresentParams (clientrect.right, clientrect.bottom, d3d_Globals.RequestBPP, !(winstyle & WS_POPUP));

	// here we use D3DCREATE_FPU_PRESERVE to maintain the resolution of Quake's timers (this is a serious problem)
	// and D3DCREATE_DISABLE_DRIVER_MANAGEMENT to protect us from rogue drivers (call it honest paranoia).  first
	// we attempt to create a hardware vp device.
	// --------------------------------------------------------------------------------------------------------
	// NOTE re pure devices: we intentionally DON'T request a pure device, EVEN if one is available, as we need
	// to support certain glGet functions that would be broken if we used one.
	// --------------------------------------------------------------------------------------------------------
	// NOTE re D3DCREATE_FPU_PRESERVE - this can be avoided if we use a timer that's not subject to FPU drift,
	// such as timeGetTime (with timeBeginTime (1)); by default Quake's times *ARE* prone to FPU drift as they
	// use doubles for storing the last time, which gradually creeps up to be nearer to the current time each
	// frame.  Not using doubles for the stored times (i.e. switching them all to floats) would also help here.
	if (SUCCEEDED (d3d_Globals.Object->CreateDevice (
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		this->Window,
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE | D3DCREATE_DISABLE_DRIVER_MANAGEMENT,
		&this->PresentParams,
		&this->Device
	)))
	{
		// attempt to create our vertex shader
		if (!this->CreateCommonVertexShader ())
		{
			// couldn't create it with a hardware VP device so destroy everything and try to create with software VP
			this->DestroyVertexShader ();
			SAFE_RELEASE (this->Device);
		}
	}

	if (!this->Device)
	{
		// it's OK, we may not have hardware vp available, so create a software vp device
		if (FAILED (d3d_Globals.Object->CreateDevice (
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			this->Window,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE | D3DCREATE_DISABLE_DRIVER_MANAGEMENT,
			&this->PresentParams,
			&this->Device
		))) System_Error ("failed to create Direct3D device");

		if (!this->CreateCommonVertexShader ()) System_Error ("failed to create Direct3D vertex shader with software VP device");
	}

	if (this->Device == NULL) System_Error ("created NULL Direct3D device");

	this->DeviceLost = FALSE;
	this->ClientActiveTexture = 0;

	// initialize textures
	this->InitTextures ();

	// initialize state
	this->InitStates ();

	// disable lighting
	this->SetRenderState (D3DRS_LIGHTING, FALSE);

	// set projection and world to dirty, beginning of stack and identity
	this->InitializeTransforms ();

	this->InitGeometry ();

	this->InitGammaAndContrast ();

	// states
	this->InitializeStates ();

	// clear the color buffer on creation
	this->Clear (D3DCLEAR_TARGET);
}


void context_t::PreReset (void)
{
	this->DestroyGammaAndContrast ();
	this->ReleaseDefaultPoolTextures ();
}


void context_t::PostReset (void)
{
	this->InitGammaAndContrast ();

	this->InitializeStates ();

	this->RecreateDefaultPoolTextures ();

	// set projection and world to dirty, beginning of stack and identity
	this->InitializeTransforms ();

	// force all states back to the way they were
	this->SetRenderStates ();
	this->SetTextureStates ();

	this->InitGeometry ();
}

void context_t::ResizeWindow (int width, int height, int bpp)
{
	// reset present params
	this->SetupPresentParams (width, height, bpp, true /*we have to be windowed to be calling Resize*/ );
	this->ResetDevice ();

	// repaint all windows
	InvalidateRect (NULL, NULL, TRUE); // Really?

//	Sleep (10); // Really?
}

void context_t::ResetMode (int width, int height, int bpp, BOOL windowed, int window_style, int window_ex_style)
{
	RECT winrect;
#if 0  // Baker ... let the engine tell us the window decoration, keep things flexible.
//	int winstyle;
	int winexstyle;
#endif 

	// reset present params
	this->SetupPresentParams (width, height, bpp, windowed);

	this->ResetDevice ();

	winrect.left = 0;
	winrect.right = width;
	winrect.top = 0;
	winrect.bottom = height;

//WS_OVERLAPPEDWINDOW
#if 0 // Baker: Let the engine decide on window decoration, keep things flexible
	winexstyle = 0;

#ifdef D3D_RESIZABLE_WINDOW
	winstyle = windowed ? WS_OVERLAPPEDWINDOW /* <--- this bastard just hides what it is*/ : WS_POPUP;

	//#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED     | \	// Want
	//                             WS_CAPTION        | \	// Want
	//                             WS_SYSMENU        | \	// Want
	//                             WS_THICKFRAME     | \	// MSDN: "Same as the WS_SIZEBOX style"
	//                             WS_MINIMIZEBOX    | \	// Want
	//                             WS_MAXIMIZEBOX)			// Do not want.

#else
	winstyle = windowed ? WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX : WS_POPUP;
#endif
#endif

	// reset stuff
	SetWindowLong (this->PresentParams.hDeviceWindow, GWL_EXSTYLE, window_ex_style);
	SetWindowLong (this->PresentParams.hDeviceWindow, GWL_STYLE, window_style);
	AdjustWindowRectEx (&winrect, window_style, FALSE, window_ex_style);

	// repaint all windows
	InvalidateRect (NULL, NULL, TRUE);

	// resize the window; also reposition it as we don't know if it's going to go down below the taskbar area
	// or not, so we just err on the side of caution here.
	SetWindowPos (
		this->PresentParams.hDeviceWindow,
#if 0 // Baker modification
		// See if we can make it non-topmost.  Problem: If window starts as fullscreen on engine start, it will always stay topmost 
		// even if we change to windowed mode.
		// I think this is not noticed in DirectFitz 9 because it starts out windowed.

		// FAIL
		windowed ? HWND_NOTOPMOST : NULL,   // <---------------------- this does NOT work, even though in theory it should
											// However, if we do it separately further below, it does work.
#else
		NULL,
#endif
		windowed ? (d3d_Globals.DesktopMode.Width - (winrect.right - winrect.left)) / 2 : 0,	// Baker: Review this centering calculation.
		windowed ? (d3d_Globals.DesktopMode.Height - (winrect.bottom - winrect.top)) / 2 : 0,	// Baker: Review this centering calculation.
		winrect.right - winrect.left,
		winrect.bottom - winrect.top,
		SWP_NOOWNERZORDER | SWP_NOREPOSITION | SWP_NOZORDER | SWP_SHOWWINDOW
	);

	// ensure
	SetForegroundWindow (this->PresentParams.hDeviceWindow);

#if 1 // Baker modification: See if we can make it non-topmost.  Problem: If window starts as fullscreen on engine start, it will always stay topmost even if we change to windowed mode.
	// THIS WORKS!
#ifndef _DEBUG // Baker: Hopefully this will allow me to debug better	
	if (windowed)
#endif // _DEBUG
		SetWindowPos (this->PresentParams.hDeviceWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);


#endif //

	// because a lot of things are now bouncing around the system we take a breather
//	Sleep (10);  // Neither seems to help nor hurt anything in any circustance I've encountered so far ...
}


void context_t::BeginScene (void)
{
	// check for a beginscene
	if (!this->State.SceneBegun)
	{
		// issue a beginscene (geometry needs this
		this->Device->BeginScene ();

		// bind our main d3d objects
		this->Geometry.Activate ();
		this->ActivateVertexShader ();

		// clear down bound textures
		for (int i = 0; i < D3D_MAX_TMUS; i++)
			this->State.BoundTextures[i] = NULL;

		this->State.DrawCount = 0;

		// we're in a scene now
		this->State.SceneBegun = TRUE;
	}
}


void context_t::EndScene (void)
{
	// if we lost the device (e.g. on a mode switch, alt-tab, etc) we must try to recover it
	if (this->DeviceLost)
	{
		// here we get the current status of the device
		HRESULT hr = this->Device->TestCooperativeLevel ();

		switch (hr)
		{
		case D3D_OK:
			// device is recovered
			this->DeviceLost = FALSE;
			this->PostReset ();
			break;

		case D3DERR_DEVICELOST:
			// device is still lost
			Sleep (1);
			break;

		case D3DERR_DEVICENOTRESET:
			// device is ready to be reset
			this->PreReset ();
			this->Device->Reset (&this->PresentParams);
			break;

		default:
			break;
		}

		// yield the CPU a little
		Sleep (10);

		// don't bother this frame
		return;
	}

	if (this->State.SceneBegun)
	{
		HRESULT hr;

		this->FlushGeometry ();

		/*
		if (this->State.DrawCount)
		{
			Con_Printf ("%i draw call\n", this->State.DrawCount);
			this->State.DrawCount = 0;
		}
		*/

		// finalize gamma and contrast adjustment
		this->FinishGammaAndContrast ();

		// endscene and present are only required if a scene was begun (i.e. if something was actually drawn)
		this->Device->EndScene ();
		this->State.SceneBegun = FALSE;

		// present the display
		hr = this->Device->Present (NULL, NULL, NULL, NULL);

		if (hr == D3DERR_DEVICELOST)
		{
			// flag a lost device
			this->DeviceLost = TRUE;
		}
		else if (FAILED (hr))
		{
			// something else bad happened
			System_Error ("FAILED (hr) on this->Device->Present");
		}
	}
}


void context_t::ResetDevice (void)
{
	this->FlushGeometry ();
	this->PreReset ();

	while (this->Device->TestCooperativeLevel () != D3D_OK)
		Sleep (1);

	// reset device
	if (FAILED (this->Device->Reset (&this->PresentParams)))
		System_Error ("context_t::ResetDevice : failed");

	while (this->Device->TestCooperativeLevel () != D3D_OK)
		Sleep (1);

	// clean up states/etc
	this->PostReset ();
}


void context_t::SetVSync (int interval)
{
	// don't change if it doesn't need to change
#if 1 // Baker test.  Disabling these didn't help windowed mode.
	if (!interval && this->PresentParams.PresentationInterval == D3DPRESENT_INTERVAL_IMMEDIATE) return;
	if (interval && this->PresentParams.PresentationInterval != D3DPRESENT_INTERVAL_IMMEDIATE) return;
#endif

	if (interval)
		this->PresentParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	else this->PresentParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	this->ResetDevice ();
}

void context_t::Sync (void)
{
	IDirect3DQuery9 *FinishEvent = NULL;
	
	if (SUCCEEDED (this->Device->CreateQuery (D3DQUERYTYPE_EVENT, &FinishEvent)))
	{
		this->FlushGeometry ();

		if (SUCCEEDED (FinishEvent->Issue (D3DISSUE_END)))
			while (FinishEvent->GetData (NULL, 0, D3DGETDATA_FLUSH) == S_FALSE);

		SAFE_RELEASE (FinishEvent);
	}
}


#endif

