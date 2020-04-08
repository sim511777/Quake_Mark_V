/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others
Copyright (C) 2009-2014 Baker and others

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/


#include "menu_local.h"

// State, enter, draw, input
// These are pretty much informational only.  M_Draw, M_Key and such are still the gateway.
// Although we do need to absorb the cursor.
// Also have the gateway set the cursor?






/*
================
M_DrawCharacter

Draws one solid graphics character
================
*/
void M_DrawCharacter (int cx, int line, int num)
{
	Draw_Character (cx, line, num);
}

void M_Print (int cx, int cy, const char *str)
{
	while (*str)
	{
		M_DrawCharacter (cx, cy, (*str)+128);
		str++;
		cx += 8;
	}
}

void M_PrintWhite (int cx, int cy, const char *str)
{
	while (*str)
	{
		M_DrawCharacter (cx, cy, *str);
		str++;
		cx += 8;
	}
}


void M_DrawTransPic (int x, int y, qpic_t *pic, int count, int _colsize, int _rowsize)
{
	if (count) {
		int width  = (_colsize == USE_IMAGE_SIZE_NEG1) ? pic->width	: _colsize * count;
		int height = (_rowsize == USE_IMAGE_SIZE_NEG1) ? pic->height : _rowsize * count;
		Hotspots_Add (x, y, width, height, count, hotspottype_button);
	}
	Draw_TransPic (x, y, pic);
}

void M_DrawPic (int x, int y, qpic_t *pic)
{
	Draw_Pic (x, y, pic);
}

#define MENU_CANVAS_WIDTH_320 320
void M_DrawPicCentered (int y, qpic_t *pic)
{
	Draw_Pic ((MENU_CANVAS_WIDTH_320 - pic->width) / 2, y, pic);
}

#ifdef GLQUAKE_COLORMAP_TEXTURES
	void M_DrawTransPicTranslate (int x, int y, qpic_t *pic, int top, int bottom) //johnfitz -- more parameters
	{
		Draw_TransPicTranslate (x, y, pic, top, bottom); //johnfitz -- simplified becuase centering is handled elsewhere
	}
#else // The WinQuake way ...
	byte identityTable[PALETTE_COLORS_256];
	byte translationTable[PALETTE_COLORS_256];

	void M_BuildTranslationTable(int top, int bottom)
	{
		int		j;
		byte	*dest, *source;

		for (j = 0; j < 256; j++)
			identityTable[j] = j;
		dest = translationTable;
		source = identityTable;
		memcpy (dest, source, 256);

		if (top < 128)	// the artists made some backwards ranges.  sigh.
			memcpy (dest + TOP_RANGE, source + top, 16);
		else
			for (j=0 ; j<16 ; j++)
				dest[TOP_RANGE+j] = source[top+15-j];

		if (bottom < 128)
			memcpy (dest + BOTTOM_RANGE, source + bottom, 16);
		else
			for (j=0 ; j<16 ; j++)
				dest[BOTTOM_RANGE+j] = source[bottom+15-j];
	}


	void M_DrawTransPicTranslate (int x, int y, qpic_t *pic)
	{
		Draw_TransPicTranslate (x, y, pic, translationTable);
	}
#endif // GLQUAKE_COLORMAP_TEXTURES

void M_DrawTextBox (int x, int y, int width, int lines, int hotspot_emit_border)
{
	qpic_t	*p;
	int		cx, cy;
	int		n;

	if (hotspot_emit_border) {
		// Sheesh this is stupid.
		// Here's the deal, apparently 8 gets added :(  So if we want 6, we add 2 instead of 8
		// Example 6
		int amt = 8 - hotspot_emit_border;
		//int amtsz = hotspot_emit_border;
		Hotspots_Add (x + amt, y + amt, width * 8 + hotspot_emit_border * 2 , 8 + hotspot_emit_border * 2, 1, hotspottype_button);
		//M_DrawTextBox (basex, lanConfig_cursor_table[1]-8, 2, 1, 6 /* border */);
	}

	// draw left side
	cx = x;
	cy = y;
	p = Draw_CachePic ("gfx/box_tl.lmp");
	M_DrawTransPic (cx, cy, p, NO_HOTSPOTS_0, USE_IMAGE_SIZE_NEG1, USE_IMAGE_SIZE_NEG1);
	p = Draw_CachePic ("gfx/box_ml.lmp");
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		M_DrawTransPic (cx, cy, p, NO_HOTSPOTS_0, USE_IMAGE_SIZE_NEG1, USE_IMAGE_SIZE_NEG1);
	}
	p = Draw_CachePic ("gfx/box_bl.lmp");
	M_DrawTransPic (cx, cy+8, p, NO_HOTSPOTS_0, USE_IMAGE_SIZE_NEG1, USE_IMAGE_SIZE_NEG1);

	// draw middle
	cx += 8;
	while (width > 0)
	{
		cy = y;
		p = Draw_CachePic ("gfx/box_tm.lmp");
		M_DrawTransPic (cx, cy, p, NO_HOTSPOTS_0, USE_IMAGE_SIZE_NEG1, USE_IMAGE_SIZE_NEG1);
		p = Draw_CachePic ("gfx/box_mm.lmp");
		for (n = 0; n < lines; n++)
		{
			cy += 8;
			if (n == 1)
				p = Draw_CachePic ("gfx/box_mm2.lmp");
			M_DrawTransPic (cx, cy, p, NO_HOTSPOTS_0, USE_IMAGE_SIZE_NEG1, USE_IMAGE_SIZE_NEG1);
		}
		p = Draw_CachePic ("gfx/box_bm.lmp");
		M_DrawTransPic (cx, cy+8, p, NO_HOTSPOTS_0, USE_IMAGE_SIZE_NEG1, USE_IMAGE_SIZE_NEG1);
		width -= 2;
		cx += 16;
	}

	// draw right side
	cy = y;
	p = Draw_CachePic ("gfx/box_tr.lmp");
	M_DrawTransPic (cx, cy, p, NO_HOTSPOTS_0, USE_IMAGE_SIZE_NEG1, USE_IMAGE_SIZE_NEG1);
	p = Draw_CachePic ("gfx/box_mr.lmp");
	for (n = 0; n < lines; n++)
	{
		cy += 8;
		M_DrawTransPic (cx, cy, p, NO_HOTSPOTS_0, USE_IMAGE_SIZE_NEG1, USE_IMAGE_SIZE_NEG1);
	}
	p = Draw_CachePic ("gfx/box_br.lmp");
	M_DrawTransPic (cx, cy+8, p, NO_HOTSPOTS_0, USE_IMAGE_SIZE_NEG1, USE_IMAGE_SIZE_NEG1);
}



//=============================================================================
/* Menu Subsystem */

#define COUNT_UNSET_NEG1 -1
void M_Init (void)
{
	Cmd_AddCommands (M_Init);
	Hotspots_Init (); // Doesn't do that much

	{ int n; for (n = menu_state_None_0 + 1 /*let's skip none shall we?*/; n < menux_count; n ++) {
		// Fire the Init function
		menux_t *this_menu = &menux[n];
		
		this_menu->cursor_solid_count = COUNT_UNINIT_NEG2; // Set this
		this_menu->InitOnce_Function (/*self */ this_menu); 

		//if (this_menu->cursor_solid_count == COUNT_UNINIT_NEG2) { // Debug
		//	alert ("%s didn't set a solid count.  n = %d", this_menu->name, n);
		//}

	}} // end for

}



void M_Draw (void)
{
	if (sMenu.menu_state == menu_state_None_0 || key_dest != key_menu)
		return;

	if (!sMenu.recursiveDraw)
	{
		if (console1.visible_pct)
		{
			// Baker: We need this because console background can draw
			// At stupid times without it, at least for software Quake
			if (key_dest == key_console || key_dest == key_message)
				Draw_ConsoleBackground ();
			S_ExtraUpdate ();
		}

		Draw_FadeScreen (); //johnfitz -- fade even if console fills screen

#ifdef WINQUAKE_RENDERER_SUPPORT
// Baker: I suspect this isn't necessary, but haven't verified.
// because I think something above might set both or some other condition.
		winquake_scr_copyeverything = 1;
		winquake_scr_fullupdate = 0;
#endif // WINQUAKE_RENDERER_SUPPORT
	}
	else
	{
		sMenu.recursiveDraw = false;
	}

	Draw_SetCanvas (CANVAS_MENU); //johnfitz
	
	if (!hotspot_menu_group.focus && menux[sMenu.menu_state].hover && !isin2(menux[sMenu.menu_state].hover->hotspottype, hotspottype_screen, hotspottype_inert) ) {
		crect_t *r = &menux[sMenu.menu_state].hover->rect;
		if (menux[sMenu.menu_state].hover->hotspottype == hotspottype_button_line) {
			// Outline highlight
			Draw_Fill (PRECT_SEND(r),				QUAKE_RED_251, 1);
			Draw_Fill (PRECT_SEND_INSET(r, 1),		QUAKE_BLACK_0, 1);
		}
		else Draw_Fill (PRECT_SEND(r), QUAKE_TAN_125, 0.10);
	}

	Hotspots_Begin (sMenu.menu_state, 0);

	{ // Fire the draw function
		menux_t *this_menu = &menux[sMenu.menu_state];
		this_menu->Draw_Function (); 
	}

	if (sMenu.entersound) {
		MENU_ENTER_LOUD_SOUND ();
		sMenu.entersound = false;
	}

	S_ExtraUpdate ();
}


void M_Keydown (key_scancode_e key, int hotspot)
{
	{ // Fire the key function
		menux_t *this_menu = &menux[sMenu.menu_state];

		if (hotspot < 0)
			hotspot = hotspot; // When does this happen?  Demos/Levels?

		if (!sMenu.keys_bind_grab && isin2 (key, K_MOUSEWHEELUP, K_MOUSEWHEELDOWN)) {
			return; // We don't do mousewheel events for menu except for keygrab.  A couple of menu items fake mousewheel.
		}

		if (!sMenu.keys_bind_grab && hotspot != NO_HOTSPOT_HIT_NEG1 && key == K_MOUSE1) {
			// If mouse1 was selected, set the cursor
			#pragma message ("Isn't the hotspot sometimes item data?")
			if (hotspot >=0 /*as opposed to a negative hotspot*/)
				this_menu->cursor = hotspot;

			key = K_ENTER;
		}

		this_menu->Key_Function (key, hotspot); 
	}


}




