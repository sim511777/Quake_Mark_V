/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2012 John Fitzgibbons and others
Copyright (C) 2009-2014 Baker and others

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// in_win.c -- windows 95 mouse and joystick code
// 02/21/97 JCB Added extended DirectInput code to support external controllers.


#include "quakedef.h"
#include "sdlquake.h"

int Input_Local_Capture_Mouse (cbool bDoCapture)
{
	static cbool captured = false;

	if (bDoCapture && !captured)
	{
//	    SDL_WM_GrabInput(SDL_GRAB_ON);
	    SDL_SetRelativeMouseMode(SDL_TRUE);
//		ShowCursor (FALSE); // Hides mouse cursor
//		SetCapture (sysplat.mainwindow);	// Captures mouse events
		Con_DPrintf ("Mouse Captured\n");
		captured = true;
	}

	if (!bDoCapture && captured)
	{
//	    SDL_WM_GrabInput(SDL_GRAB_OFF);
	    SDL_SetRelativeMouseMode(SDL_FALSE);
		//ShowCursor (TRUE); // Hides mouse cursor
		//ReleaseCapture ();
		//ClipCursor (NULL); // Can't hurt
		Con_DPrintf ("Mouse Released\n");
		captured = false;
	}

	return 1; // Accepted
}


cbool Input_Local_Update_Mouse_Clip_Region_Think (mrect_t* mouseregion)
{
#if 0
	mrect_t oldregion = *mouseregion;
	WINDOWINFO windowinfo;
	windowinfo.cbSize = sizeof (WINDOWINFO);
	GetWindowInfo (sysplat.mainwindow, &windowinfo);	// client_area screen coordinates

	// Fill in top left, bottom, right, center
	mouseregion->left = windowinfo.rcClient.left;
	mouseregion->right = windowinfo.rcClient.right;
	mouseregion->bottom = windowinfo.rcClient.bottom;
	mouseregion->top = windowinfo.rcClient.top;

	if (memcmp (mouseregion, &oldregion, sizeof(mrect_t) ) != 0)
	{  // Changed!
		mouseregion->width = mouseregion->right - mouseregion->left;
		mouseregion->height = mouseregion->bottom - mouseregion->top;
		mouseregion->center_x = (mouseregion->left + mouseregion->right) / 2;
		mouseregion->center_y = (mouseregion->top + mouseregion->bottom) / 2;
		ClipCursor (&windowinfo.rcClient);
		return true;
	}
#endif
	return false;
}

void Input_Local_Mouse_Cursor_SetPos (int x, int y)
{
//	SetCursorPos (x, y);
}

void Input_Local_Mouse_Cursor_GetPos (int *x, int *y)
{
//	POINT current_pos;
//	GetCursorPos (&current_pos);
//
//	*x = current_pos.x;
//	*y = current_pos.y;
}


#ifdef _WIN32

STICKYKEYS StartupStickyKeys = {sizeof (STICKYKEYS), 0};
TOGGLEKEYS StartupToggleKeys = {sizeof (TOGGLEKEYS), 0};
FILTERKEYS StartupFilterKeys = {sizeof (FILTERKEYS), 0};


void AllowAccessibilityShortcutKeys (cbool bAllowKeys)
{
	static cbool initialized = false;

	if (!initialized)
	{	// Save the current sticky/toggle/filter key settings so they can be restored them later
		SystemParametersInfo (SPI_GETSTICKYKEYS, sizeof (STICKYKEYS), &StartupStickyKeys, 0);
		SystemParametersInfo (SPI_GETTOGGLEKEYS, sizeof (TOGGLEKEYS), &StartupToggleKeys, 0);
		SystemParametersInfo (SPI_GETFILTERKEYS, sizeof (FILTERKEYS), &StartupFilterKeys, 0);
		Con_DPrintf ("Accessibility key startup settings saved\n");
		initialized = true;
	}

	if (bAllowKeys)
	{
		// Restore StickyKeys/etc to original state
		// (note that this function is called "allow", not "enable"; if they were previously
		// disabled it will put them back that way too, it doesn't force them to be enabled.)
		SystemParametersInfo (SPI_SETSTICKYKEYS, sizeof (STICKYKEYS), &StartupStickyKeys, 0);
		SystemParametersInfo (SPI_SETTOGGLEKEYS, sizeof (TOGGLEKEYS), &StartupToggleKeys, 0);
		SystemParametersInfo (SPI_SETFILTERKEYS, sizeof (FILTERKEYS), &StartupFilterKeys, 0);

		Con_DPrintf ("Accessibility keys enabled\n");
	}
	else
	{
		// Disable StickyKeys/etc shortcuts but if the accessibility feature is on,
		// then leave the settings alone as its probably being usefully used
		STICKYKEYS skOff = StartupStickyKeys;
		TOGGLEKEYS tkOff = StartupToggleKeys;
		FILTERKEYS fkOff = StartupFilterKeys;

		if ((skOff.dwFlags & SKF_STICKYKEYSON) == 0)
		{
			// Disable the hotkey and the confirmation
			skOff.dwFlags &= ~SKF_HOTKEYACTIVE;
			skOff.dwFlags &= ~SKF_CONFIRMHOTKEY;

			SystemParametersInfo (SPI_SETSTICKYKEYS, sizeof (STICKYKEYS), &skOff, 0);
		}

		if ((tkOff.dwFlags & TKF_TOGGLEKEYSON) == 0)
		{
			// Disable the hotkey and the confirmation
			tkOff.dwFlags &= ~TKF_HOTKEYACTIVE;
			tkOff.dwFlags &= ~TKF_CONFIRMHOTKEY;

			SystemParametersInfo (SPI_SETTOGGLEKEYS, sizeof (TOGGLEKEYS), &tkOff, 0);
		}

		if ((fkOff.dwFlags & FKF_FILTERKEYSON) == 0)
		{
			// Disable the hotkey and the confirmation
			fkOff.dwFlags &= ~FKF_HOTKEYACTIVE;
			fkOff.dwFlags &= ~FKF_CONFIRMHOTKEY;

			SystemParametersInfo (SPI_SETFILTERKEYS, sizeof (FILTERKEYS), &fkOff, 0);
		}

		Con_DPrintf ("Accessibility keys disabled\n");
	}
}



LRESULT CALLBACK LLWinKeyHook(int Code, WPARAM wParam, LPARAM lParam)
{
	PKBDLLHOOKSTRUCT p;
	p = (PKBDLLHOOKSTRUCT) lParam;

    if (1 || vid.ActiveApp)
	{
		switch(p->vkCode)
		{
		case VK_LWIN:	// Left Windows Key
		case VK_RWIN:	// Right Windows key
		case VK_APPS: 	// Context Menu key

			return 1; // Ignore these keys
		}
	} else Con_Printf ("Not active app\n");

	return CallNextHookEx(NULL, Code, wParam, lParam);
}



void AllowWindowsShortcutKeys (cbool bAllowKeys)
{
	static cbool WinKeyHook_isActive = false;
	static HHOOK WinKeyHook;

	if (!bAllowKeys)
	{
		// Disable if not already disabled
		if (!WinKeyHook_isActive)
		{
			if (!(WinKeyHook = SetWindowsHookEx(13, LLWinKeyHook, GetModuleHandle(NULL), 0))) // GetModuleHandle(NULL) gets hinstance
			{
				Con_Printf("Failed to install winkey hook.\n");
				Con_Printf("Microsoft Windows NT 4.0, 2000 or XP is required.\n");
				return;
			}

			WinKeyHook_isActive = true;
			Con_DPrintf ("Windows and context menu key disabled\n");
		}
	}

	if (bAllowKeys)
	{	// Keys allowed .. stop hook
		if (WinKeyHook_isActive)
		{
			UnhookWindowsHookEx(WinKeyHook);
			WinKeyHook_isActive = false;
			Con_DPrintf ("Windows and context menu key enabled\n");
		}
	}
}
#endif // _WIN32

void Input_Local_Keyboard_Disable_Sticky_Keys (cbool bDoDisable)
{
#ifdef _WIN32
	if (bDoDisable)
	{
		AllowAccessibilityShortcutKeys (false);
	}
	else
	{
		AllowAccessibilityShortcutKeys (true);
	}
#endif // _WIN32
}

void Input_Local_Keyboard_Disable_Windows_Key (cbool bDoDisable)
{
#ifdef _WIN32
	if (bDoDisable)
	{
		AllowWindowsShortcutKeys (false);
	}
	else
	{
		AllowWindowsShortcutKeys (true);
	}
#endif // _WIN32
}


/*
=======
MapKey

Map from windows to quake keynums
=======
*/
int Input_Local_MapKey (int windowskey)
{
	static byte scantokey[128] =
	{
		0 ,	27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', K_BACKSPACE,
		9, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 13 ,
		K_CTRL, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
		K_SHIFT,'\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', K_SHIFT,'*',K_ALT,' ', 0 ,
		K_F1, K_F2, K_F3, K_F4, K_F5,K_F6, K_F7, K_F8, K_F9, K_F10, K_PAUSE, 0,
		K_HOME, K_UPARROW,K_PGUP,'-',K_LEFTARROW,'5',K_RIGHTARROW,'+',K_END,
		K_DOWNARROW,K_PGDN,K_INSERT,K_DEL,0,0, 0, K_F11, K_F12,0 , 0 , 0 , 0 , 0 , 0 , 0,
	};
	int key = (windowskey >> 16) & 255;

	if (key > 127)
		return 0;

	key = scantokey[key];
/*
	switch (key)
	{
		case K_KP_STAR:		return '*';
		case K_KP_MINUS:	return '-';
		case K_KP_5:		return '5';
		case K_KP_PLUS:		return '+';
	}
*/
	return key;
}


void Input_Local_Joy_AdvancedUpdate_f (lparse_t *unused)
{
}

cbool Input_Local_Joystick_Startup (void)
{

	return false;
}



void Input_Local_Joystick_Commands (void)
{

}

cbool Input_Local_Joystick_Read (void)
{
    return false; // I guess?
}



void Input_Local_Joystick_Shutdown (void)
{
	// We don't have to do anything here

}


void Input_Local_Init (void)
{
//    Not needed?
//    if (SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL) == -1)
//        Con_Warning ("Warning: SDL_EnableKeyRepeat() failed.\n");
}

void Input_Local_Shutdown (void)
{

}


// Baker: On Windows these might not only be key events.
void Input_Local_SendKeyEvents (void)
{
//    MSG        msg;
//
//	while (PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
//	{
//	// we always update if there are any event, even if we're paused
//		scr_skipupdate = 0;
//
//		if (!GetMessage (&msg, NULL, 0, 0))
//			System_Quit ();
//
//      	TranslateMessage (&msg);
//      	DispatchMessage (&msg);
//	}
}

// Baker: Stops drag flag on Mac (when activation is received by a mouseclick on title bar and user drags it.
//  On Windows do this too.
void Input_Local_Deactivate (void)
{


}

