/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others
Copyright (C) 2007-2008 Kristian Duske
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
// r_misc.c

#include "quakedef.h"



/*
====================
GL_Fullbrights_f -- johnfitz
====================
*/
void GL_Fullbrights_f (cvar_t *var)
{
	TexMgr_ReloadNobrightImages ();
}

/*
====================
R_SetClearColor_f -- johnfitz
====================
*/
void R_SetClearColor_f (cvar_t *var)
{
	byte	*rgb;
	int		s;

	s = (int)r_clearcolor.value & 0xFF;
	rgb = (byte*)(vid.d_8to24table + s);
	eglClearColor (rgb[0]/255.0,rgb[1]/255.0,rgb[2]/255.0,0);
}

/*
====================
R_Novis_f -- johnfitz
====================
*/
void R_VisChanged (cvar_t *var)
{
	extern int vis_changed;
	vis_changed = 1;
}


/*
===============
R_Envmap_f

Grab six views for environment mapping tests
===============
*/
void R_Envmap_f (lparse_t *unused)
{
	unsigned	buffer[256*256];

	if (vid.direct3d)
	{
		// Direct3D wrapper doesn't seem to support size on the fly at this time.
		Con_Printf ("Not supported for Direct3D at this time\n");
		return;
	}

	if (cls.state != ca_connected)
	{
		Con_Printf ("No map running\n");
		return;
	}

	envmap = true;

	clx = cly = r_refdef.vrect.x = r_refdef.vrect.y = 0;
	clwidth = clheight = r_refdef.vrect.width = r_refdef.vrect.height = 256;

	r_refdef.viewangles[0] = 0;
	r_refdef.viewangles[1] = 0;
	r_refdef.viewangles[2] = 0;

	R_RenderView ();
	eglFinish (); // Force update before grabbing pixels.
	eglReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	
	Image_Save_PNG_QPath ("env_0.png", buffer, 256, 256);

	r_refdef.viewangles[1] = 90;
	R_RenderView ();
	eglFinish ();
	eglReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	
	Image_Save_PNG_QPath ("env_1.png", buffer, 256, 256);

	r_refdef.viewangles[1] = 180;
	R_RenderView ();
	eglFinish ();
	eglReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	Image_Save_PNG_QPath ("env_2.png", buffer, 256, 256);

	r_refdef.viewangles[1] = 270;
	R_RenderView ();
	eglFinish ();
	eglReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	
	Image_Save_PNG_QPath ("env_3.png", buffer, 256, 256);

	r_refdef.viewangles[0] = -90;
	r_refdef.viewangles[1] = 0;
	R_RenderView ();
	eglFinish ();
	eglReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	Image_Save_PNG_QPath ("env_4.png", buffer, 256, 256);

	r_refdef.viewangles[0] = 90;
	r_refdef.viewangles[1] = 0;
	eglFinish ();
	R_RenderView ();
	eglReadPixels (0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	Image_Save_PNG_QPath ("env_5.png", buffer, 256, 256);

	envmap = false;

	Recent_File_Set_QPath ("env_0.png");
	Con_Printf ("Envmaps env files created\n");
	vid.recalc_refdef = true; // Recalc the refdef next frame
}

/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////
/////////////////////////////////////////////////

/*
===============
R_Init
===============
*/

void R_Init_Local (void)
{	
	Cmd_AddCommands (R_Init_Local);

	R_Init_FlashBlend_Bubble ();

	Entity_Inspector_Init ();
	TexturePointer_Init ();


	Fog_Init (); //johnfitz
}

/*
===============
R_NewGame -- johnfitz -- handle a game switch
===============
*/
void R_NewGame (void)
{
	int i;

	//clear playertexture pointers (the textures themselves were freed by texmgr_newgame)
	for (i = 0; i < MAX_COLORMAP_SKINS_1024; i ++)
		playertextures[i] = NULL;
}

Point3D R_SurfCenter (msurface_t *surf)
{
	Point3D center;
	float *verts = surf->polys->verts[0];

	vec3_t mins = { 99999,  99999,  99999};
	vec3_t maxs = {-99999, -99999, -99999};
	
	// Draw polygon while collecting information for the center.
	int i; for (i = 0 ; i < surf->polys->numverts ; i++, verts+= VERTEXSIZE)
		VectorExtendLimits (verts, mins, maxs);
	VectorAverage (mins, maxs, center.vec3);

	return center;
}

cbool GL_Mirrors_Is_TextureName_Mirror (const char *txname)
{
	cbool Is_Texture_Prefix (const char *texturename, const char *prefixstring);

	if (vid.direct3d)
		return false; // Not supported right now.
	
	if (Is_Texture_Prefix (txname, gl_texprefix_mirror.string) && String_Does_Match_Caseless (gamedir_shortname(), GAMENAME)) {
		return true; // id1 gamedir
	}
	
	if (Is_Texture_Prefix (txname, "mirror_")) {
		return true;
	}

	return false;
}

/*
===============
R_NewMap_Local
===============
*/

void R_NewMap_Local (void)
{
	Stains_WipeStains_NewMap ();
//	TexturePointer_ClearCaches_NewMap (); // Nov 16: is this a reminder on an outstanding refinement we should do in future?  If so, will be done in TexturePointer_Reset
	GL_BuildLightmaps_Upload_All_NewMap (); // Verts become available.

	Fog_NewMap (); //johnfitz -- global fog in worldspawn

	TexturePointer_Reset ();

		// Baker addition:

	if (level.mirror)  { 
		int i, j;
		//byte		*vis;
		 
		mnode_t		*node;
		msurface_t	*surf;
		mleaf_t		*mirror_leaf;

		//Con_SafePrintf ("Num nodes is %d\n", cl.worldmodel->numnodes);
		//Con_Printf ("Num surfaces is %d\n", cl.worldmodel->numnodes); // ?

		//Con_Printf ("Num leafs %d\n", cl.worldmodel->numleafs);
		// Scan for mirrors
		for (i = 0, node = cl.worldmodel->nodes ; i < cl.worldmodel->numnodes ; i++, node++)
			for (j = 0, surf = &cl.worldmodel->surfaces[node->firstsurface] ; (unsigned int)j < node->numsurfaces ; j++, surf++)
				if (Flag_Check (surf->flags, SURF_DRAWMIRROR)) {
					
					Point3D center = R_SurfCenter (surf); //+ surf->plane->normal;
					VectorAdd (center.vec3, surf->plane->normal, center.vec3); // works
					mirror_leaf = Mod_PointInLeaf (center.vec3, cl.worldmodel);
					if (mirror_leaf->contents != CONTENTS_SOLID) {
						// Add matching leafs to the surface.  Isn't there just one?  We can't easily repeat the vis.  Just finish this dammit.
						byte *vis = Mod_LeafPVS (mirror_leaf, cl.worldmodel);					
						
						int hunk_add_mark = Hunk_LowMark ();
						int hunk_add_size = 0;
						mleaf_t	*leaf;
						int j2;//, k2;
						size_t siz=sizeof( *(surf->xtraleafs));
						surf->xtraleafs = Hunk_AllocName (sizeof(surf->xtraleafs[0]) * MAX_MAP_LEAFS, va("mr%d", j)); // We'll chop this.
						
						//Con_SafePrintf ("Mirror Surface %x (%d) appears in vis node %d viewleaf is %x\n", surf, j, i, mirror_leaf);
						for (j2 = 0, leaf = &cl.worldmodel->leafs[1]; j2 < cl.worldmodel->numleafs; j2++, leaf++)
						{
							// We could add the surface specific leafs here .. like if it fails the viskey, 
							if (vis[j2 >> 3] & (1 << (j2 & 7))) { // >> 3 = divide by 8
								// Mirror sees you.
								surf->xtraleafs[surf->xtraleafs_count] = leaf;
								surf->xtraleafs_count ++;
							}
						}
						
						hunk_add_size = surf->xtraleafs_count * siz;
						// Too lazy to chop it.
						// If we try to chop it ourselves, we trash a sentinel.
						// Easy enough to fix really, memdup and put that on the hunk.
						// Next time.
						//Hunk_FreeToLowMark ( hunk_add_mark + hunk_add_size);
					}
				}
	}


	load_subdivide_size = gl_subdivide_size.value; //johnfitz -- is this the right place to set this?
}



/*
===============
R_TranslateNewModelSkinColormap

Looks through our table and returns an existing model/skin/pants/shirt combination already uploaded.
If does not exist.  Uploads it.

Since this re-uses colormapped skins (2 red players = same texture) this will upload fewer skins
than the GLQuake way, but will colormap everything.

r_nocolormap_list provides a means to exclude colormapping trivial things like gibs.  Although it
supports 1024 combinations, it will rarely use more than 10 to 30 in practice.  All skins are
deleted on a new map.
===============
*/

// This is for an entity WITH an existing skin texture
// So we know it is an alias model (or at least was until now!)
cbool R_SkinTextureChanged (entity_t *cur_ent)
{
	gltexture_t *skintexture	= cur_ent->coloredskin;
	int entnum = cur_ent - cl_entities;

	if (skintexture->owner != cur_ent->model)
	{
#if 0
		Con_Printf ("ent %i Model changed\n", entnum);
#endif
		return true;	// Model changed
	}

	do
	{
		int playerslot				= cur_ent->colormap - 1;
		int shirt_color				= (cl.scores[playerslot].colors & 0xf0) >> 4;
		int pants_color				= cl.scores[playerslot].colors & 15;

		if (skintexture->pants != pants_color)
		{
#if 0
			Con_Printf ("ent %i: Pants changed\n", entnum);		// Pants changed
#endif
			return true;
		}

		if (skintexture->shirt != shirt_color)
		{
#if 0
			Con_Printf ("ent %i: Shirt changed\n", entnum);		// Shirt changed
#endif
			return true;
		}

		if (skintexture->skinnum != cur_ent->skinnum)
		{
#if 0
			Con_Printf ("ent %i: Player skin changed\n", entnum);		// Skin changed
#endif
			return true; // Skin changed
		}

		// NOTE: Baker --> invalid skin situation can persistently trigger "skin changed"
		return false;

	} while (0);

}


gltexture_t *R_TranslateNewModelSkinColormap (entity_t *cur_ent)
{
	int entity_number = cur_ent - cl_entities; // If player, this will be 1-16
	int shirt_color, pants_color, skinnum, matchingslot;
	aliashdr_t	*paliashdr;

	do	// REJECTIONS PHASE
	{
		// No model or it isn't an alias model
		if (!cur_ent->model || cur_ent->model->type != mod_alias)
			return NULL;

		// No color map or it is invalid
		if (cur_ent->colormap <= 0 || cur_ent->colormap > cl.maxclients)
			return NULL;

		// Certain models just aren't worth trying to colormap
		if (cur_ent->model->modelflags & MOD_NOCOLORMAP)
			return NULL;

		//TODO: move these tests to the place where skinnum gets received from the server
		paliashdr = (aliashdr_t *)Mod_Extradata (cur_ent->model);
		skinnum = cur_ent->skinnum;

		if (skinnum < 0 || skinnum >= paliashdr->numskins)
		{
			// Baker: Note I do not believe this ever happens!!!
			Con_DPrintf("(%d): Invalid player skin #%d\n", entity_number, skinnum);
			skinnum = 0;
		}

	} while (0);

	do // SEE IF WE HAVE SKIN + MODEL + COLOR ALREADY PHASE
	{
		int playerslot = cur_ent->colormap - 1;

		shirt_color = (cl.scores[playerslot].colors & 0xf0) /16; //>> 4; // Divide by 16
		pants_color = cl.scores[playerslot].colors & 15;

		// Con_Printf ("Seeking shirt %d, pants %d, skin %d, model %x\n", shirt_color, pants_color, skinnum, cur_ent->model);

		for (matchingslot = 0; matchingslot < MAX_COLORMAP_SKINS_1024; matchingslot ++)
		{
			gltexture_t *curtex = playertextures[matchingslot];

			if (playertextures[matchingslot] == NULL)
				break; // Not found, but use this slot

			//Con_Printf ("Slot %d: Shirt %d, pants %d, skin %d, model %x %x ... ", matchingslot, curtex->shirt, curtex->pants, curtex->skinnum, curtex->owner, curtex);

			if (curtex->shirt != shirt_color) {
				//Con_Printf ("Slot %d: Failed on shirt\n", matchingslot);
				continue;
			}

			if (curtex->pants != pants_color) {
				//Con_Printf ("Slot %d: Failed on pants\n", matchingslot);
				continue;
			}

			if (curtex->skinnum != skinnum) {
				//Con_Printf ("Slot %d: Failed on skin\n", matchingslot);
				continue;
			}

			if (curtex->owner != cur_ent->model) {
				//Con_Printf ("Slot %d: Failed on model\n", matchingslot);
				continue;
			}

			//Con_Printf ("Match = %d\n");

			// Found an existing translation for this
			return curtex;

		}

		if (matchingslot == MAX_COLORMAP_SKINS_1024)
		{
			Host_Error ("Color Slots Full");
			return NULL;
		}

		// If we are here matchingslot is our new texture slot

	} while (0);

//	matchingslot = matchingslot;
//	Con_SafePrintf ("Skin max: %d\n", matchingslot);
	do // UPLOAD THE NEW SKIN + MODEL PHASE (MAYBE COLOR)
	{
		aliashdr_t	*paliashdr = (aliashdr_t *)Mod_Extradata (cur_ent->model);
		byte		*pixels = (byte *)paliashdr + paliashdr->texels[skinnum]; // This is not a persistent place!
		char		description_name[MAX_QPATH_64];
#if 1
		c_snprintf3(description_name, "player_color_shirt_%d_pants_%d_skin_%d", /*entity_number,*/ shirt_color, pants_color, skinnum); // Entity number just hurts compression, right?
#else
		c_snprintf(name, "player_%d", entity_number);
#endif

//		Con_Printf ("New upload\n");

		//upload new image
		playertextures[matchingslot] = TexMgr_LoadImage (cur_ent->model, -1 /*not bsp texture*/, description_name, paliashdr->skinwidth, paliashdr->skinheight,
		SRC_INDEXED, pixels, paliashdr->gltextures[skinnum][0]->source_file, paliashdr->gltextures[skinnum][0]->source_offset, TEXPREF_PAD /*| TEXPREF_OVERWRITE*/);
		// Kill the overwrite?
		if (playertextures[matchingslot])
		{
			playertextures[matchingslot]->skinnum = skinnum;
			TexMgr_ReloadImage (playertextures[matchingslot], shirt_color, pants_color);
		}

	} while (0);

	return playertextures[matchingslot];
}


