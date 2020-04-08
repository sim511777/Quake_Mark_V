/*
Copyright (C) 2013 Baker

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
// input.c -- input


#include "quakedef.h"
#include "input.h"		// Courtesy
#include "menu_local.h"	// Privledged access

#ifdef INPUT_RELATIVE
	int input_accum_x, input_accum_y;
	/// #define Input_Local_Mouse_Cursor_SetPos // Baker: wicked evil ... but not just yet
#endif // INPUT_RELATIVE

// How do we convert this to Quake?
keyvalue_t key_scancodes_table [108] = {
	{ "BACKSPACE",      K_BACKSPACE         },
	{ "TAB",            K_TAB               },
	{ "ENTER",          K_ENTER             },
	{ "ESCAPE",         K_ESCAPE            },
	{ "SPACE",          K_SPACE             },
	{ "SEMICOLON",      K_SEMICOLON         },
	{ "TILDE",          K_GRAVE             },
	{ "LCTRL",          K_LCTRL             },
	{ "RCTRL",          K_RCTRL             },
	{ "LALT",           K_LALT              },
	{ "RALT",           K_RALT              },
	{ "LSHIFT",         K_LSHIFT            },
	{ "RSHIFT",         K_RSHIFT            },
	{ "LWIN",           K_LWIN              },
	{ "RWIN",           K_RWIN              },
	{ "MENU",           K_MENU              },
	{ "CAPSLOCK",       K_CAPSLOCK          },
	{ "NUMLOCK",        K_NUMLOCK           }, // Is same as keypad numlock or is different key?
	{ "SCROLLLOCK",     K_SCROLLLOCK        },
	{ "PAUSE",          K_PAUSE             },
	{ "PRINTSCREEN",    K_PRINTSCREEN       },
	{ "INS",            K_INSERT            },
	{ "DEL",            K_DELETE            },
	{ "LEFTARROW",      K_LEFTARROW         },
	{ "RIGHTARROW",     K_RIGHTARROW        },
	{ "UPARROW",        K_UPARROW           },
	{ "DOWNARROW",      K_DOWNARROW         },
	{ "PGUP",           K_PAGEUP            },
	{ "PGDN",           K_PAGEDOWN          },
	{ "HOME",           K_HOME              },
	{ "END",            K_END               },
	{ "F1",             K_F1                },
	{ "F2",             K_F2                },
	{ "F3",             K_F3                },
	{ "F4",             K_F4                },
	{ "F5",             K_F5                },
	{ "F6",             K_F6                },
	{ "F7",             K_F7                },
	{ "F8",             K_F8                },
	{ "F9",             K_F9                },
	{ "F10",            K_F10               },
	{ "F11",            K_F11               },
	{ "F12",            K_F12               },

	{ "KP_0",           K_NUMPAD_0          },	// Emission
	{ "KP_1",           K_NUMPAD_1          },
	{ "KP_2",           K_NUMPAD_2          },
	{ "KP_3",           K_NUMPAD_3          },
	{ "KP_4",           K_NUMPAD_4          },
	{ "KP_5",           K_NUMPAD_5          },
	{ "KP_6",           K_NUMPAD_6          },
	{ "KP_7",           K_NUMPAD_7          },
	{ "KP_8",           K_NUMPAD_8          },
	{ "KP_9",           K_NUMPAD_9          },
	{ "KP_MULTIPLY",    K_NUMPAD_MULTIPLY   },
	{ "KP_PLUS",        K_NUMPAD_PLUS       },
	{ "KP_SEPARATOR",   K_NUMPAD_SEPARATOR  },
	{ "KP_MINUS",       K_NUMPAD_MINUS      },
	{ "KP_PERIOD",      K_NUMPAD_PERIOD     },
	{ "KP_DIVIDE",      K_NUMPAD_DIVIDE     },
#if 0 // Disallow, I think - Build 1010
	{ "KP_ENTER",		K_NUMPAD_ENTER		},	// Emission!!!!
#endif
	{ "MOUSE1",         K_MOUSE1            },
	{ "MOUSE2",         K_MOUSE2            },
	{ "MOUSE3",         K_MOUSE3            },
	{ "MOUSE4",         K_MOUSE4            },
	{ "MOUSE5",         K_MOUSE5            },

	{ "MWHEELUP",       K_MOUSEWHEELUP      },
	{ "MWHEELDOWN",     K_MOUSEWHEELDOWN    },
	{ "JOY1",           K_JOY1              },
	{ "JOY2",           K_JOY2              },
	{ "JOY3",           K_JOY3              },
	{ "JOY4",           K_JOY4              },
	{ "AUX1",           K_AUX1              },
	{ "AUX2",           K_AUX2              },
	{ "AUX3",           K_AUX3              },
	{ "AUX4",           K_AUX4              },
	{ "AUX5",           K_AUX5              },
	{ "AUX6",           K_AUX6              },
	{ "AUX7",           K_AUX7              },
	{ "AUX8",           K_AUX8              },
	{ "AUX9",           K_AUX9              },
	{ "AUX10",          K_AUX10             },
	{ "AUX11",          K_AUX11             },
	{ "AUX12",          K_AUX12             },
	{ "AUX13",          K_AUX13             },
	{ "AUX14",          K_AUX14             },
	{ "AUX15",          K_AUX15             },
	{ "AUX16",          K_AUX16             },
	{ "AUX17",          K_AUX17             },
	{ "AUX18",          K_AUX18             },
	{ "AUX19",          K_AUX19             },
	{ "AUX20",          K_AUX20             },
	{ "AUX21",          K_AUX21             },
	{ "AUX22",          K_AUX22             },
	{ "AUX23",          K_AUX23             },
	{ "AUX24",          K_AUX24             },
	{ "AUX25",          K_AUX25             },
	{ "AUX26",          K_AUX26             },
	{ "AUX27",          K_AUX27             },
	{ "AUX28",          K_AUX28             },
	{ "AUX29",          K_AUX29             },
	{ "AUX30",          K_AUX30             },
	{ "AUX31",          K_AUX31             },
	{ "AUX32",          K_AUX32             },
NULL, 0}; // Null term


void Input_Force_CenterView_f (lparse_t *unnused) { cl.viewangles[PITCH] = 0; }


typedef enum { ENUM_FORCE_INT_GCC_ (input_state)
	input_none,
	input_have_keyboard,
	input_have_mouse_keyboard,
	input_have_windowskey,
} input_state_t;

typedef struct
{
	input_state_t	current_state;
	cbool			initialized, have_mouse, have_keyboard;
	cbool			disabled_windows_key;

// Internals
	mrect_t			mouse_clip_screen_rect;
	int				mouse_accum_x, mouse_accum_y;
	int				mouse_old_button_state;
} inp_info_t;


#define MRECT_PRINT(_x) _x.left, _x.top, _x.right, _x.bottom, _x.center_x, _x.center_y
enum { GET_IT = 1, LOSE_IT = 2 };


keyvalue_t input_state_text [] =
{
	KEYVALUE (input_none),
	KEYVALUE (input_have_keyboard),
	KEYVALUE (input_have_mouse_keyboard),
NULL, 0 };  // NULL termination

static inp_info_t inps;


void Input_Info_f (void)
{
	Con_PrintLinef ("IN Info ...");
	Con_PrintLinef ("%-25s :  %s", "current_state", KeyValue_GetKeyString (input_state_text, inps.current_state) );
	Con_PrintLinef ("%-25s :  %d", "initialized", inps.initialized);
	Con_PrintLinef ("%-25s :  %d", "have_mouse", inps.have_mouse);
	Con_PrintLinef ("%-25s :  %d", "have_keyboard", inps.have_keyboard);
	Con_PrintLinef ("%-25s :  %d", "disabled_windows_key", inps.disabled_windows_key);
	Con_PrintLinef ("%-25s :  (%d, %d)-(%d, %d) center: %d, %d", "mouse_clip_screen_rect:", MRECT_PRINT(inps.mouse_clip_screen_rect) );
	Con_PrintLinef ("%-25s :  %d", "mouse_accum_x", inps.mouse_accum_x);
	Con_PrintLinef ("%-25s :  %d", "mouse_accum_y", inps.mouse_accum_y);
	Con_PrintLinef ("%-25s :  %d", "mouse_old_button_state", inps.mouse_old_button_state);
}

#pragma message ("OS X mouse input has to be purely event oriented, we can't just nab the screen at any given time")
#ifdef PLATFORM_OSX
void Input_Think (void) { }
#else
void Input_Think (void)
{
	input_state_t	newstate = (inps.initialized && vid.ActiveApp && !vid.Minimized && !vid.Hidden) ? input_have_keyboard : input_none;
	cbool		windowed_mouse_grab = !cl.paused && !console1.forcedup && ( key_dest == key_game  || key_dest == key_message || (key_dest == key_menu && sMenu.keys_bind_grab));
	cbool		mouse_grab = (vid.screen.type == MODESTATE_FULLSCREEN || windowed_mouse_grab) && !(key_dest == key_menu);

	cbool		disable_windows_key = input_have_keyboard && vid.screen.type == MODESTATE_FULLSCREEN;
//	cbool		can_mouse_track = inps.initialized && !vid.Minimized && !vid.Hidden) && dont have mouse

	if (disable_windows_key != inps.disabled_windows_key)
	{
		switch (disable_windows_key)
		{
		case true:
			if (vid.system_enhanced_keys) Shell_Input_KeyBoard_Capture (true /*capture*/, false /*act on stickey*/, vid.screen.type == MODESTATE_FULLSCREEN /*act on windows key*/);
			break;
		case false:
			if (vid.system_enhanced_keys) Shell_Input_KeyBoard_Capture (false /*capture*/, false /*act on stickey*/, vid.screen.type == MODESTATE_FULLSCREEN /*act on windows key*/);
			break;
		}

		inps.disabled_windows_key = disable_windows_key;
	}

	// newstate upgrades from should have "keyboard" to should have "mouse"
	// If the key_dest is game or we are binding keys in the menu
	if (newstate == input_have_keyboard && mouse_grab && in_nomouse.value == 0 && vid.nomouse == 0)
		newstate = input_have_mouse_keyboard;

#if 0
	Con_PrintLinef ("current_state: %s (init %d active %d mini %d)", Keypair_String (input_state_text, inps.current_state),
		inps.initialized, vid.ActiveApp, vid.Minimized);
#endif

	if (newstate != inps.current_state)
	{ // New state.
		char	mouse_action	= ( newstate == input_have_mouse_keyboard && inps.have_mouse == false) ? GET_IT :  (( newstate != input_have_mouse_keyboard && inps.have_mouse == true) ? LOSE_IT : 0);
		char	keyboard_action = ( newstate != input_none && inps.have_keyboard == false) ? GET_IT :  (( newstate == input_none && inps.have_keyboard == true) ? LOSE_IT : 0);

#if 0
		Con_PrintLinef ("State change");
#endif

		switch (keyboard_action)
		{
		case GET_IT:
			// Sticky keys
			if (vid.system_enhanced_keys) Shell_Input_KeyBoard_Capture (true /*capture*/, true /*act on stickey*/, vid.screen.type == MODESTATE_FULLSCREEN /*act on windows key*/);

			inps.have_keyboard = true;
			break;

		case LOSE_IT:
			// Note we still need our key ups when entering the console
			// Sticky keys, Window key reenabled

			if (vid.system_enhanced_keys) Shell_Input_KeyBoard_Capture (false, true /*act on stickey*/, vid.screen.type == MODESTATE_FULLSCREEN);
			// Key ups

			inps.have_keyboard = false;
			break;
		}

		switch (mouse_action)
		{
		case GET_IT:

			// Load window screen coords to mouse_clip_screen_rect
			// And clip the mouse cursor to that area
			Input_Local_Update_Mouse_Clip_Region_Think (&inps.mouse_clip_screen_rect);

			// Hide the mouse cursor and attach it
			Input_Local_Capture_Mouse (true);

			// Center the mouse on-screen
			Input_Local_Mouse_Cursor_SetPos (inps.mouse_clip_screen_rect.center_x, inps.mouse_clip_screen_rect.center_y);

			// Clear movement accumulation
			inps.mouse_accum_x = inps.mouse_accum_y = 0;

			inps.have_mouse = true;
			break;

		case LOSE_IT:
			// Baker: We have to release the mouse buttons because we can no longer receive
			// mouse up events.
			Key_Release_Mouse_Buttons ();

			// Release it somewhere out of the way
			Input_Local_Mouse_Cursor_SetPos (inps.mouse_clip_screen_rect.right - 80, inps.mouse_clip_screen_rect.top + 80);

			// Release the mouse and show the cursor.  Also unclips mouse.
			Input_Local_Capture_Mouse (false);

			// Clear movement accumulation and buttons
			inps.mouse_accum_x = inps.mouse_accum_y = inps.mouse_old_button_state = 0;

			inps.have_mouse = false;
			break;
		}
		inps.current_state = newstate;
	}

	if (inps.have_mouse && Input_Local_Update_Mouse_Clip_Region_Think (&inps.mouse_clip_screen_rect) == true)
	{
		// Re-center the mouse cursor and clear mouse accumulation
		Input_Local_Mouse_Cursor_SetPos (inps.mouse_clip_screen_rect.center_x, inps.mouse_clip_screen_rect.center_y);
		inps.mouse_accum_x = inps.mouse_accum_y = 0;
	}

	// End of function
}

// A specific mouse action.
// TODO focus and hotspots.  Wave.  Be plentiful!
// Sigh.
// How does multitouch fit into this?

// Wow this requires tons of information
void QGetPitchYaw (reply float *pitch_delta_out, reply float *yaw_delta_out, 
	int mouse_x, int mouse_y, required const int *viewport, const glmatrix *m_projection3d, 
	const glmatrix *m_modelview, const vec3_t qcamera_angles, int screenheight, int winy, float znear, float zfar)
{
	position3d		pfar, pnear;

	// This is if we want center of pixel.
	// m.view has the camera rolled into it, so I think it's ok.
	//	gg.inps.raw_x = gg.client_canvas.width / 2 + 0.5;
	//	gg.inps.raw_y = gg.client_canvas.height / 2 + 0.5;

	// Make sure height isn't stale?
	Mat4_UnProject_Smart (mouse_x, mouse_y,  /* near */ 0, m_modelview, m_projection3d, viewport, clheight, &pnear.x, &pnear.y, &pnear.z);
	Mat4_UnProject_Smart (mouse_x, mouse_y,  /* far */  1, m_modelview, m_projection3d, viewport, clheight, &pfar.x,  &pfar.y,  &pfar.z );

	{
		vec3_t	slope;
		
		slope[0] /*pitch*/ = pfar.x - pnear.x; // far - near
		slope[1] /*roll*/  = pfar.y - pnear.y; // far - near
		slope[2] /*yaw*/   = pfar.z - pnear.z; // far - near

		{
			float raylength = VectorLength(slope); // normalize the length to distance where sum of all = 1.
			vec3_t	slope_normalized;

			// I disagree with this because it pitch/roll/raw should be degrees, right?  But we are a true vector here.
			slope_normalized[0]	= slope[0] / raylength;  // Pitch
			slope_normalized[1]	= slope[1] / raylength;  // Roll
			slope_normalized[2]	= slope[2] / raylength;  // Yaw
			
			//slope_normalized[2] = 0; // For Quake this is roll.  We don't want it.
			{
#if 1
				vec3_t	degrees;  VectorToAngles (slope_normalized, degrees);; //

#else
				vec3_t	degrees;  //QVector_To_Angles_Rollless (slope_normalized, degrees);; // We need to get the roll out of the pitch
				c_swapf (&slope_normalized[1], &slope_normalized[2]);
				QVector_To_Angles_Rollless (slope_normalized, degrees); // <--- I don't trust
				//son of a bitch!  IT IS A SPHERE!  We are in a sphere.  The yaw and pitch do change
				//We want the literal yaw and pitch on the screen
#endif
//				degrees[0] = angledelta_maybe_wrap (degrees[0] - qcamera_angles[PITCH /*0*/], NULL);  // Pitch is fine.
//				degrees[2] = angledelta_maybe_wrap (degrees[2] - qcamera_angles[YAW   /*2*/], NULL);  // OpenGL yaw and Quake yaw and different index.
				// It's giving me 315 to 40

				NOT_MISSING_ASSIGN (pitch_delta_out, angledelta_maybe_wrap(-degrees[0], NULL));
				NOT_MISSING_ASSIGN (yaw_delta_out,   angledelta_maybe_wrap(degrees[1], NULL));
			}
		}
	}
}


//typedef struct {
//	void			*something_here;		// Translation from game portal to 
//	float			game_projection[16];	// Projection for the view.
//	int				game_viewport[4];
//	float			r_foxy, r_fovy;
//	vec3_t			game_org, game_angles;	// We eventually want double click
//
//	//
//	void			*focused;
//	int				focus_num;
//	ticktime_t		last_up;				// For comparing.  We cannot assume game is 0 for multitouch.
//	cbool			is_down;
//	int				downx, downy;
//} focusor_t;


// We kind of need the buttons?
// Howz this going to work with 16 presses
void LoadupI (int newx, int newy, reply int *storex, reply int *storey)
{
	NOT_MISSING_ASSIGN (storex, newx);
	NOT_MISSING_ASSIGN (storey, newy);

}

void LoadupF (float newx, float newy, reply float *storex, reply float *storey)
{
	NOT_MISSING_ASSIGN (storex, newx);
	NOT_MISSING_ASSIGN (storey, newy);

}



void Key_Game_ButtonPressMove (mousebuttonbits_e mousebuttons, int rawx, int rawy)
{
	float newpitch, newyaw; QGetPitchYaw (&newpitch, &newyaw, rawx, rawy, focus0.game_viewport, &focus0.game_projection, &focus0.game_modelview, focus0.game_angles, clwidth, 0 /* winy */, focus0.znear, focus0.zfar); // pitch 0, yaw 2 for 77

	if (Flag_Check_Bool (mousebuttons, mousebuttonbits_mouse1_bit_1) && focus0.is_down) {
		float oldpitch, oldyaw; QGetPitchYaw (&oldpitch, &oldyaw, focus0.downx, focus0.downy, focus0.game_viewport, &focus0.game_projection, &focus0.game_modelview, focus0.game_angles, clwidth, 0 /* winy */, focus0.znear, focus0.zfar); // pitch 0, yaw 2 for 77
		
		if (focus0.downx != rawx || focus0.downy != rawy)  {
			float delta_pitch	= angledelta_maybe_wrap (newpitch - oldpitch, NULL);	// I think this takes the shortest path?
			float delta_yaw	    = angledelta_maybe_wrap (newyaw - oldyaw, NULL);

			// STUPIDITY: Quake pitch is 90 (full down) to -90 (full up)
			cl.viewangles[PITCH] = angle_maybe_wrap (cl.viewangles[PITCH] - delta_pitch, NULL);
			cl.viewangles[YAW]   = angle_maybe_wrap (cl.viewangles[YAW] - delta_yaw, NULL);

			// Baker: OOF!
			if (cl.viewangles[PITCH] > 180)
				cl.viewangles[PITCH] = cl.viewangles[PITCH] - 360; // 358?  ---> -2   320? ---> -40

			CL_BoundViewPitch (cl.viewangles);

			View_StopPitchDrift (); // Stop drift?  Did this work?			
			LoadupI (rawx, rawy, &focus0.downx, &focus0.downy);
		}
	}
	if (Flag_Check_Bool (mousebuttons, mousebuttonbits_mouse2_bit_2) && focus0.is_attack_click) {
		// Update it.
		if (newpitch > 180)
			newpitch = newpitch - 360; // 358?  ---> -2   320? ---> -40

		VectorSet (focus0.attack_angles, newpitch, newyaw, 0);	CL_BoundViewPitch (focus0.attack_angles); // No roll
	}

}

focusor_t focus0;

// Up off hotspot is not a click
// Then again if the hotspot is continuous, we want the down.
void IN_AttackDown (lparse_t* line);
static void Key_Game_ButtonPressDown (key_scancode_e scancode, int rawx, int rawy)
{
	float newpitch, newyaw; QGetPitchYaw (&newpitch, &newyaw, rawx, rawy, focus0.game_viewport, &focus0.game_projection, &focus0.game_modelview, focus0.game_angles, clwidth, 0 /* winy */, focus0.znear, focus0.zfar); // pitch 0, yaw 2 for 77

	switch (scancode) {
	default:
	case_break K_MOUSE1:
		// Move
		if (focus0.is_down) 
			break; // If somehow this happens get out!

		LoadupI (rawx, rawy, &focus0.downx, &focus0.downy);
		focus0.is_down	= true;

	case_break K_MOUSE2:

		if (focus0.is_attack_click) break; // If somehow this happens get out!

		if (newpitch > 180)
			newpitch = newpitch - 360; // 358?  ---> -2   320? ---> -40
		
		VectorSet (focus0.attack_angles, newpitch, newyaw, 0);	CL_BoundViewPitch (focus0.attack_angles); // No roll

		IN_AttackDown (NULL);
		focus0.is_attack_firing  = true;						// Begin firing?
		focus0.is_attack_click	= true;							// Stays true until release
	}

}


// This by definition must be with mousedown?
void IN_AttackUp (lparse_t *line);
static void Key_Game_ButtonPressUp (key_scancode_e scancode, int rawx, int rawy)
{
	float newpitch, newyaw; QGetPitchYaw (&newpitch, &newyaw, rawx, rawy, focus0.game_viewport, &focus0.game_projection, &focus0.game_modelview, focus0.game_angles, clwidth, 0 /* winy */, focus0.znear, focus0.zfar); // pitch 0, yaw 2 for 77
	switch (scancode) {
	case_break K_MOUSE1:
		if (!focus0.is_down)			break; // Get out if not applicable
		View_StopPitchDrift (); // Stop drift?  Did this work?
		focus0.is_down		 = false;
			
	case_break K_MOUSE2:
		
		if (!focus0.is_attack_click)	break; // Get out if not applicable

		if (newpitch > 180)
			newpitch = newpitch - 360; // 358?  ---> -2   320? ---> -40

		VectorSet (focus0.attack_angles, newpitch, newyaw, 0);	CL_BoundViewPitch (focus0.attack_angles); // No roll

		IN_AttackUp (NULL);

		focus0.is_attack_click = false;
	}

}

focus_part_e Focus_Part (hotspot_menu_item_t *hs, int left, int top)
{
	switch (hs->hotspottype) {
	default: System_Error ("Focus_Part Unknown hotspot type");
	case_break hotspottype_vscroll:
		if (RECT_HIT_Y(hs->r_thumb, top))		return focus_part_thumb;
		if (RECT_HIT_Y(hs->r_button1, top))		return focus_part_button1;
		if (RECT_HIT_Y(hs->r_button2, top))		return focus_part_button2;
		if (RECT_HIT_Y(hs->r_track1, top))		return focus_part_track1;
		if (RECT_HIT_Y(hs->r_track2, top))		return focus_part_track2;
		Con_PrintLinef ("Bad focus on vscroll, unknown part");	
	case_break hotspottype_hscroll:
		if (RECT_HIT_X(hs->r_thumb, left))		return focus_part_thumb;
		if (RECT_HIT_X(hs->r_button1, left))	return focus_part_button1;
		if (RECT_HIT_X(hs->r_button2, left))	return focus_part_button2;
		if (RECT_HIT_X(hs->r_track1, left))		return focus_part_track1;
		if (RECT_HIT_X(hs->r_track2, left))		return focus_part_track2;
		Con_PrintLinef ("Bad focus on vscroll, unknown part");	
	}
	return 0;
}

static void Key_Mouse_Button (key_scancode_e scancode, mouseaction_e mouseaction, cbool havemouse, int rawx, int rawy)
{
	hotspot_menu_group_t *f = &hotspot_menu_group;

	// TODO:  Release non-mode?
	if (havemouse) {
		// Game mode
		if (mouseaction == mouseaction_down)	Key_Event_Ex (NO_WINDOW_NULL, scancode, true,  ASCII_0, UNICODE_0, CORE_SHIFTBITS_UNREAD_NEG1);
		if (mouseaction == mouseaction_up)		Key_Event_Ex (NO_WINDOW_NULL, scancode, false, ASCII_0, UNICODE_0, CORE_SHIFTBITS_UNREAD_NEG1);
	}
	else
	{
#ifdef WINQUAKE_RENDERER_SUPPORT
		rawx /= vid.stretch_x; 
		rawy /= vid.stretch_y;

#endif
		//extern cbool scr_drawdialog; // Sigh

		// Not game mode
		if (key_dest == key_game) {
			// We will receive a mousedown or mouseup.  To do mousemove.
			#pragma message ("Mouse cancel")
			#pragma message ("Mouse move")

			switch (mouseaction) {
			case mouseaction_down:	Key_Game_ButtonPressDown (scancode, rawx, rawy); break;
			case mouseaction_move:	Key_Game_ButtonPressMove (scancode, rawx, rawy); break;
			case mouseaction_up:	Key_Game_ButtonPressUp   (scancode, rawx, rawy); break;
			}
			goto dont_have_mouse_done;
		}
		else if (key_dest == key_menu /*&& scr_drawdialog == false*/) {
			// I can't help but think we are missing clx and cly or something?
			// Doesn't matter because of viewport?  But is that true for winquake?

			//VID_Set_Window_Title ("%g %g %g --> %d", x, y, z, hit ? hit->idx : -1)
//#pragma message ("Behavior if grab key?  Well it shouldn't reach here because grab mouse is false.  That is a weird situation.")

			hotspot_menu_item_t *hit = menux[sMenu.menu_state].hover = 
				Menu_Hotspot_Refresh_For_Mouse (rawx, rawy, &menux[sMenu.menu_state].hoverx, &menux[sMenu.menu_state].hovery);
			int left = menux[sMenu.menu_state].hoverx, top  = menux[sMenu.menu_state].hovery;

			// We do this here to degrade it to a mousemove
			if (mouseaction == mouseaction_up && f->focus) {
				// No need to degrade it to a mousemove.  Above should hover it ok.
				f->focus = NULL; // Wipe it
				goto dont_have_mouse_done;
			}


			switch (mouseaction) 
			{
			default: break;
			case_break mouseaction_move:

				//
				// MOUSEMOVE - Mostly scrollbars
				//

				// CHECK FOR FOCUS HERE!
				if (!f->focus)
					break; // We have no focus

				switch (f->focus->hotspottype) {
				default:							break; // If not a vscroll we do nothing special.
				case_break hotspottype_vscroll: {
					int local_top;

					if (f->focus_part != focus_part_thumb)
						break; // Do not want!
					
					f->focus_move_y				= top;
					f->focus_move_thumb_offset	= f->focus_down_thumb_offset; // 7 or such
					f->focus_move_thumb_top		= top - f->focus_down_thumb_offset; // Thumb is higher than top.  This is screen Y.
					local_top					= f->focus_move_thumb_top - f->focus_down_track_top; // 0 is true top.
					local_top					= CLAMP (0, local_top, f->focus_down_track_usable - 1);
					f->focus_move_thumb_top		= f->focus_down_track_top + local_top;

					// Pencil in event
					f->focus_event				= focus_event_thumb_position;
					//f->focus_event_msgtime_ext	= 0;
					f->focus_event_thumb_top	= local_top;
					//VID_Set_Window_Title ("New thumb top relative %d", f->focus_event_thumb_top);
				
				} // End of vscroll
				case_break hotspottype_hscroll: {
					int local_top;

					if (f->focus_part != focus_part_thumb)
						break; // Do not want!
					
					f->focus_move_y				= left;
					f->focus_move_thumb_offset	= f->focus_down_thumb_offset; // 7 or such
					f->focus_move_thumb_top		= left - f->focus_down_thumb_offset; // Thumb is higher than top.  This is screen Y.
					local_top					= f->focus_move_thumb_top - f->focus_down_track_top; // 0 is true top.
					local_top					= CLAMP (0, local_top, f->focus_down_track_usable - 1);
					f->focus_move_thumb_top		= f->focus_down_track_top + local_top;

					// Pencil in event
					f->focus_event				= focus_event_thumb_position;
					//f->focus_event_msgtime_ext	= 0;
					f->focus_event_thumb_top	= local_top;
					//VID_Set_Window_Title ("New thumb top relative %d", f->focus_event_thumb_top);
				
				} // End of hscroll
				} // End of hotspot switch

			case_break mouseaction_up: {

#pragma message ("This should be also if we have no current focus, like scrollbar in progress")
				//
				// MOUSEUP - About everything
				//
				// CHECK FOR FOCUS HERE!
				hotspottype_e hotspottype = hit ? hit->hotspottype : hotspottype_none_0;

				if (scancode == K_MOUSE2) {
					M_Keydown (K_ESCAPE, NO_HOTSPOT_HIT_NEG1);
					goto dont_have_mouse_done; // K_MOUSE2 is always ESC.  No exceptions.
				}
				if (scancode >  K_MOUSE2) {
					goto dont_have_mouse_done; // Not sure what to do.
				}

				switch (hotspottype) {
				default:	break; // I guess
				case_break hotspottype_none_0:		// No hotspot was hit
				case_break hotspottype_inert:		// An inert (placeholder) was hit.  We do nothing.
				case_break hotspottype_toggle:		M_Keydown (scancode, hit->idx); // 2 or more choices.  We aren't supporting reverse.
				case_break hotspottype_slider:		
													//if (in_range (hit->slider_start, menux[sMenu.menu_state].hoverx, hit->slider_end)) {
													//	key_scancode_e emitk = 0;
													//	if      (menux[sMenu.menu_state].hoverx < hit->slider_midstart)	emitk = K_LEFTARROW;
													//	else if (menux[sMenu.menu_state].hoverx < hit->slider_midbeyond)	emitk = 0; // Ignore
													//	else														emitk = K_RIGHTARROW;
													//	if (emitk) 
													//		M_Keydown (emitk, hit->idx); // Slider.					Responds to left and right.
													//}
				
				case_break hotspottype_button:		//
													M_Keydown (scancode, hit->idx);	// Execute.					Example: Single Player: Reset Defaults
				case_break hotspottype_button_line:	//if (scancode == K_MOUSE1) M_Keydown (K_ENTER, hit->idx);	// Execute.					Example: Single Player: Reset Defaults
													M_Keydown (scancode, hit->idx);	// Execute.					Example: Single Player: Reset Defaults

				
				case_break hotspottype_listitem:	// if (scancode == K_MOUSE1) M_Keydown (K_ENTER, HS_LIST_LISTINDEX_ENCODE(hit->listindex) ); // A list item
													M_Keydown (scancode, HS_LIST_LISTINDEX_ENCODE(hit->listindex) ); // A list item
				case_break hotspottype_text:		// Do nothing.  Used.
				case_break hotspottype_textbutton:	//M_Keydown (K_ENTER, hit->idx); // Used for "your name" to go to name maker.
													M_Keydown (scancode, hit->idx); // Used for "your name" to go to name maker.
													
				case_break hotspottype_screen:		M_Keydown (scancode, hit->idx); // Help menu pages.
				case_break hotspottype_vscroll:		// What about other parts of the scrollbar?  Track2, button1, etc.
													break;
				case_break hotspottype_hscroll:		break;
				} // End of switch hotspottype

			} // End of mouseup

			case_break mouseaction_down:
				
#pragma message ("Not only clear hover, but clear is_down?  Do we need to erase the members?")
#pragma message ("Also keep in mind as we scroll number of hotspots can change!  Perhaps we paint inert ones when a scroll bar is involved?")
				//
				// MOUSEDOWN - Mostly scrollbars
				//

				if (scancode >  K_MOUSE2) {
					goto dont_have_mouse_done; // Not sure what to do.
				}

				if (!hit)
					goto dont_have_mouse_done; // Not sure what to do.

#ifdef _DEBUG
					if (f->focus == hit) { alert ("Already attached?"); break; }
					if (f->focus != NULL) { alert ("Already have focus?"); break; }
#endif // _DEBUG

				if (isin2(hit->hotspottype, hotspottype_vscroll, hotspottype_hscroll)) {
					f->focus					= hit;
					f->focus_menu_state			= sMenu.menu_state;
					f->focus_idx				= hit->idx;
					f->focus_part				= Focus_Part (hit, left, top);

					f->focus_event				= focus_event_none_0;	// Down does not have a message.
					f->focus_event_msgtime_ext	= 0;					// Down does not have a time.
					f->focus_event_thumb_top	= 0;					// For completion.

					// Event vars for when move happens.
					f->focus_move_y				= 0;
					f->focus_move_thumb_offset	= 0;
					f->focus_move_thumb_top		= 0;

					f->focus_down_possibles		= hit->possible_first_rows;

					// Information at the time of down.  Where mouse was.  Thumb top.  The offset by extension.
					switch (hit->hotspottype) {
					default: break; // If not a vscroll we do nothing special.
					case_break hotspottype_vscroll:
						f->focus_down_y				= top;
						f->focus_down_thumb_top		= hit->r_thumb.top;
						f->focus_down_thumb_offset	= RECT_HIT_Y_OFFSET(hit->r_thumb, top);
						f->focus_down_track_top		= hit->r_track.top;
						f->focus_down_track_usable	= hit->r_track.height - hit->r_thumb.height + 1;						
					
					case_break hotspottype_hscroll:
						f->focus_down_y				= left;
						f->focus_down_thumb_top		= hit->r_thumb.left;
						f->focus_down_thumb_offset	= RECT_HIT_X_OFFSET(hit->r_thumb, left);
						f->focus_down_track_top		= hit->r_track.left;
						f->focus_down_track_usable	= hit->r_track.width - hit->r_thumb.width + 1;						
						
					} // End of switch
					//VID_Set_Window_Title ("Focus gained down at %d, thumb %d offset %d", f->focus_down_y, f->focus_down_thumb_top, f->focus_down_thumb_offset);
				} // End of hotspot switch

			} // End of if hscroll/vscroll block
				
		} // End of key menu

dont_have_mouse_done:
		(0); // Dummy statement required in C after goto, right?
	} // End of don't have mouse


}




void Input_Mouse_Button_Event (int mstate, cbool is_mousemove, int rawx, int rawy)
{
	cbool is_attached = inps.have_mouse || (key_dest == key_menu && sMenu.keys_bind_grab);

	if (host_initialized) //inps.have_mouse || (key_dest == key_menu && sMenu.keys_bind_grab) )
	{  // perform button actions
		int i;
		for (i = 0 ; i < INPUT_NUM_MOUSE_BUTTONS ; i ++)
		{
			int button_bit = (1 << i);
			cbool button_pressed  =  (mstate & button_bit) && !(inps.mouse_old_button_state & button_bit);
			cbool button_released = !(mstate & button_bit) &&  (inps.mouse_old_button_state & button_bit);
			cbool nochange		  =  ((mstate & button_bit) == (inps.mouse_old_button_state & button_bit));
			int reference		  = inps.mouse_old_button_state;

			// Modal message is a clusterfuck, we can re-enter here recursively and also have statements not taking effect.
			// A potential for chaos and double fire is possible.
			// What should really happen is the modal dialog should be its own thing like key_menu
			// But for simplicity, maybe should be own menu item that knows where it came from?
			// That creates a separate problem that keyboard binds could fire making us leave that screen.
			// Still, the quit menu is quite effective.
			// However, the quit menu is non-blocking so we would need to feed it a function to run on a YES.

			// mousecancel?
			if (button_pressed)		Key_Mouse_Button (K_MOUSE1 + i, mouseaction_down, is_attached, rawx, rawy);
			if (is_mousemove)		Key_Mouse_Button (mstate,       mouseaction_move, is_attached, rawx, rawy); // Touches are independent, though.
			if (button_released)	Key_Mouse_Button (K_MOUSE1 + i, mouseaction_up,   is_attached, rawx, rawy);
		}
		inps.mouse_old_button_state = mstate;
	}
}

// This re-centers the mouse, so it means more than simple build-up of accumulation alone.
// S_ExtraUpdate calls this, which is called several places.
// The only other caller is Input_Mouse_Move (us!)
// In perfect work, something like DirectInput would always be used making this unnecessary.
void Input_Mouse_Accumulate (void)
{
	static int last_key_dest;
	int new_mouse_x, new_mouse_y;

	Input_Think ();

	if (inps.have_mouse)
	{
		cbool nuke_mouse_accum = false;

		// Special cases: fullscreen doesn't release mouse so doesn't clear accum
		// when entering/exiting the console.  I consider those input artifacts.  Also
		// we simply don't want accum from fullscreen if not key_dest == key_game.
		if (vid.screen.type == MODESTATE_FULLSCREEN)
		{
			if (cl.paused)
				nuke_mouse_accum = true;
			else
			{
				cbool in_game_or_message = (key_dest == key_game || key_dest == key_message);
				cbool was_in_game_or_message = (last_key_dest == key_game || last_key_dest == key_message);
				cbool entered_game_or_message = in_game_or_message && !was_in_game_or_message;
				if (entered_game_or_message || !in_game_or_message)
					nuke_mouse_accum = true;
			}
		}

#ifdef INPUT_RELATIVE
		inps.mouse_accum_x += input_accum_x; input_accum_x = 0;
		inps.mouse_accum_y += input_accum_y; input_accum_y = 0;
#else // ^^^ INPUT_RELATIVE
		Input_Local_Mouse_Cursor_GetPos (&new_mouse_x, &new_mouse_y, false); // GetCursorPos (&current_pos);

		inps.mouse_accum_x += new_mouse_x - inps.mouse_clip_screen_rect.center_x;
		inps.mouse_accum_y += new_mouse_y - inps.mouse_clip_screen_rect.center_y;
#endif // !INPUT_RELATIVE

		// Re-center the mouse cursor
		Input_Local_Mouse_Cursor_SetPos (inps.mouse_clip_screen_rect.center_x, inps.mouse_clip_screen_rect.center_y);

		if (nuke_mouse_accum)
			inps.mouse_accum_x = inps.mouse_accum_y = 0;
	}
	last_key_dest = key_dest;
}

void Input_Mouse_Move (usercmd_t *cmd)
{
	Input_Mouse_Accumulate ();

	if (inps.mouse_accum_x || inps.mouse_accum_y)
	{
		int	mouse_x = inps.mouse_accum_x *= sensitivity.value;
		int mouse_y = inps.mouse_accum_y *= sensitivity.value;
	// add mouse X/Y movement to cmd
		if ( (in_strafe.state & 1) || (lookstrafe.value && MOUSELOOK_ACTIVE ))
			cmd->sidemove += m_side.value * mouse_x;
		else cl.viewangles[YAW] -= m_yaw.value * mouse_x;

		if (MOUSELOOK_ACTIVE)
			View_StopPitchDrift ();

		if ( MOUSELOOK_ACTIVE && !(in_strafe.state & 1))
		{
			cl.viewangles[PITCH] += m_pitch.value * mouse_y;

			CL_BoundViewPitch (cl.viewangles);
		}
		else
		{
			if ((in_strafe.state & 1) && cl.noclip_anglehack)
				cmd->upmove -= m_forward.value * mouse_y;
			else cmd->forwardmove -= m_forward.value * mouse_y;
		}
		inps.mouse_accum_x = inps.mouse_accum_y = 0;
	}
}
#endif // !PLATFORM_OSX

void Input_Move (usercmd_t *cmd)
{
    Input_Mouse_Move (cmd);
    Input_Joystick_Move (cmd);
}


cbool joy_avail;

/*
===========
IN_JoyMove
===========
*/

//#ifdef _WIN32
//#include "winquake.h"
//#endif // _WIN32

void Input_Joystick_Move (usercmd_t *cmd)
{
}

void Input_Commands (void)
{
#if defined(PLATFORM_OSX) || defined(PLATFORM_IOS)
void Key_Console_Repeats (void);
	Key_Console_Repeats ();
#endif // PLATFORM_OSX
	Input_Local_Joystick_Commands ();

}

void Input_Joystick_Init (void)
{
	// joystick variables
 	// assume no joystick
	joy_avail = Input_Local_Joystick_Startup();

	if (joy_avail)
	{
		Cmd_AddCommands (Input_Joystick_Init);
	}
}

void Input_Init (void)
{
	focus0.phave_mouse = &inps.have_mouse;
	Cmd_AddCommands (Input_Init);

#pragma message ("Baker: Implement m_filter on Windows")

	// This doesn't work because the config.cfg will be read and just override it.
	// Now we do it earlier -- see video startup --- no we had to bail on that, now we use command line parm
	//if (COM_CheckParm ("-nomouse"))
	//	Cvar_SetValueQuick (&in_nomouse, 1);

	if (!COM_CheckParm ("-nojoy"))
		Input_Joystick_Init ();

	Input_Local_Init (); // Mac

	inps.initialized = true;
	Input_Think ();
	Con_PrintLinef ("Input initialized");
}

void Input_Shutdown (void)
{
	Input_Local_Shutdown (); // Mac

	inps.initialized = false;
	Input_Think (); // Will shut everything off
}

void Input_System_Enhanced_Keys_Changed (cvar_t *var)
{
	// Too late, remember this reads early in SND_Read_Early_Cvars
	if (host_post_initialized) {
		Con_PrintLinef ("System enhanced keys changed.  Requires engine restart to take effect.");
	}
}
