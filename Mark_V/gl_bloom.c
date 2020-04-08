#ifdef GLQUAKE // GLQUAKE specific

/*
Copyright (C) 1997-2001 Id Software, Inc.

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
// gl_bloom.c: 2D lighting post process effect

//#include "r_local.h"
#include "quakedef.h"
/* 
============================================================================== 
 
                        LIGHT BLOOMS
 
============================================================================== 
*/ 

#define TEXPREF_WARPIMAGE 0 // Baker: evil override for the moment

static float Diamond8x[8][8] = 
{ 
        0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f, 
        0.0f, 0.0f, 0.2f, 0.3f, 0.3f, 0.2f, 0.0f, 0.0f, 
        0.0f, 0.2f, 0.4f, 0.6f, 0.6f, 0.4f, 0.2f, 0.0f, 
        0.1f, 0.3f, 0.6f, 0.9f, 0.9f, 0.6f, 0.3f, 0.1f, 
        0.1f, 0.3f, 0.6f, 0.9f, 0.9f, 0.6f, 0.3f, 0.1f, 
        0.0f, 0.2f, 0.4f, 0.6f, 0.6f, 0.4f, 0.2f, 0.0f, 
        0.0f, 0.0f, 0.2f, 0.3f, 0.3f, 0.2f, 0.0f, 0.0f, 
        0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 0.0f 
};

static float Diamond6x[6][6] = 
{ 
        0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f, 
        0.0f, 0.3f, 0.5, 0.5, 0.3f, 0.0f, 
        0.1f, 0.5, 0.9f, 0.9f, 0.5, 0.1f, 
        0.1f, 0.5, 0.9f, 0.9f, 0.5, 0.1f, 
        0.0f, 0.3f, 0.5, 0.5, 0.3f, 0.0f, 
        0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.0f 
};

static float Diamond4x[4][4] = 
{  
        0.3f, 0.4f, 0.4f, 0.3f, 
        0.4f, 0.9f, 0.9f, 0.4f, 
        0.4f, 0.9f, 0.9f, 0.4f, 
        0.3f, 0.4f, 0.4f, 0.3f 
};

static int g_bloom_size;

//cvar_t	r_bloom					= {"r_bloom", "0", true};
//cvar_t	gl_bloom_alpha			= {"gl_bloom_alpha", "0.3", true};
//cvar_t	gl_bloom_diamond_size	= {"gl_bloom_diamond_size", "8", true};
//cvar_t	gl_bloom_intensity		= {"gl_bloom_intensity", "1", true};
//cvar_t	gl_bloom_darken			= {"gl_bloom_darken", "4", true};
//cvar_t	gl_bloom_sample_size		= {"gl_bloom_sample_size", "320", true};
//cvar_t	gl_bloom_fast_sample		= {"gl_bloom_fast_sample", "0", true};
//
static gltexture_t *glt_bloomscreentexture;
static gltexture_t *glt_bloomeffecttexture;
static gltexture_t *glt_bloombackuptexture;
static gltexture_t *glt_bloomdownsamplingtexture;

static int g_screendownsamplingtexture_size;
static int g_screen_texture_width_pow2, g_screen_texture_height_pow2;
//static int  r_screenbackuptexture_size;
static int g_screenbackuptexture_width, g_screenbackuptexture_height; 

//current refdef size:
static int	curView_x;
static int  curView_y;
static int  curView_width;
static int  curView_height;

//texture coordinates of screen data inside screentexture
static float screenText_tcw;
static float screenText_tch;

static int g_sample_width;
static int g_sample_height;

//texture coordinates of adjusted textures
static float g_sampleText_tcw;
static float g_sampleText_tch;

//this macro is in sample size workspace coordinates
#define sGL_Bloom_SamplePass(xpos, ypos)                            \
    eglBegin(GL_QUADS);                                             \
    eglTexCoord2f	(0, g_sampleText_tch);							\
    eglVertex2f		(xpos, ypos);									\
    eglTexCoord2f	(0, 0);											\
    eglVertex2f		(xpos, ypos + g_sample_height);					\
    eglTexCoord2f	(g_sampleText_tcw, 0);							\
    eglVertex2f		(xpos + g_sample_width, ypos + g_sample_height);\
    eglTexCoord2f	(g_sampleText_tcw, g_sampleText_tch);			\
    eglVertex2f		(xpos + g_sample_width, ypos);					\
    eglEnd();

#define sGL_Bloom_Quad(x, y, width, height, textwidth, textheight)  \
    eglBegin		(GL_QUADS);                                     \
    eglTexCoord2f	(0, textheight);								\
    eglVertex2f		(x, y);											\
    eglTexCoord2f	(0, 0);											\
    eglVertex2f		(x, y + height);								\
    eglTexCoord2f	(textwidth, 0);                                 \
    eglVertex2f		(x+width, y + height);                          \
    eglTexCoord2f	(textwidth, textheight);                        \
    eglVertex2f		(x + width, y);                                 \
    eglEnd			();


#pragma message ("clheight is wrong, it's refdef.height or what not")
/*
=================
sGL_Bloom_InitBackUpTexture
=================
*/
void sGL_Bloom_InitBackUpTexture (int width, int height)
{
	
 
	g_screenbackuptexture_width  = width;
    g_screenbackuptexture_height = height;

	//glt_bloombackuptexture = GL_LoadTexture  ("bloomtex", width, height, data, TEX_LUMA, 4);
	{
		unsigned *dummy_data = calloc (g_screenbackuptexture_width * g_screenbackuptexture_height, sizeof(unsigned) /*RGBA_4*/);
		glt_bloombackuptexture = TexMgr_LoadImage (
			NULL,							// Model owner
			-1,								// BSP texture number
			"glt_bloombackuptexture",		// Description
			g_screenbackuptexture_width,	// Width
			g_screenbackuptexture_height,	// Height
			SRC_RGBA,						// Source format src_format_e
			dummy_data,						// Data
			"",								// Source filename (qpath)
			(src_offset_t)dummy_data,		// Offset into file or offset into memory offset_t or uintptr_t

			// Flags ... Recalculate, alpha, nearest, persistent, don't picmip, blended means don't txgamma us
			// But TEXPREF_WARPIMAGE may end up giving a size we don't want during recalc?
			TEXPREF_WARPIMAGE | TEXPREF_ALPHA | TEXPREF_NEAREST | TEXPREF_PERSIST | TEXPREF_NOPICMIP | TEXPREF_BLENDED
		);

		free (dummy_data);
	}
}

/*
=================
sGL_Bloom_InitEffectTexture
=================
*/
void sGL_Bloom_InitEffectTexture (void)
{
    if (gl_bloom_sample_size.value < 32)
        Cvar_SetValueQuick (&gl_bloom_sample_size, 32);

    //make sure bloom size is a power of 2
    g_bloom_size = Image_Power_Of_Two_Size (gl_bloom_sample_size.value);

    // make sure bloom size doesn't have stupid values
    if (g_bloom_size > g_screen_texture_width_pow2 || g_bloom_size > g_screen_texture_height_pow2)
        g_bloom_size = c_min (g_screen_texture_width_pow2, g_screen_texture_height_pow2);

	// make the cvar reflect the accepted value
    if (g_bloom_size != gl_bloom_sample_size.value)
        Cvar_SetValueQuick (&gl_bloom_sample_size, g_bloom_size);

	{ 
		unsigned *dummy_data = calloc (g_bloom_size * g_bloom_size, sizeof(unsigned) /*RGBA_4*/);
		// Baker: Make sure txgamma doesn't affect this.  Make sure is like warp texture and not reloaded.  But like warp texture, must size adjust.
		
		glt_bloomeffecttexture = TexMgr_LoadImage (
			NULL,							// Model owner
			-1,								// BSP texture number
			"glt_bloomeffecttexture",		// Description
			g_bloom_size,					// Width
			g_bloom_size,					// Height
			SRC_RGBA,						// Source format src_format_e
			dummy_data,						// Data
			"",								// Source filename (qpath)
			(src_offset_t)dummy_data,		// Offset into file or offset into memory offset_t or uintptr_t

			// Flags ... Recalculate, alpha, nearest, persistent, don't picmip, blended means don't txgamma us
			TEXPREF_WARPIMAGE | TEXPREF_ALPHA | TEXPREF_NEAREST | TEXPREF_PERSIST | TEXPREF_NOPICMIP | TEXPREF_BLENDED
		);
			
		free (dummy_data);
		//ORIGINAL: glt_bloomeffecttexture = GL_LoadTexture  ("***glt_bloomeffecttexture***", g_bloom_size, g_bloom_size, data, TEX_LUMA, 4);
		// See also: glt_bloomeffecttexture = LoadATex ( /*rgba store*/ &pt_persist_tex[0], 0, "gfx/particles/particlefont", "qmb:particlefont", true /*force 256 */);    
	}
}

/*
=================
sGL_Bloom_InitTextures
=================
*/
static void sGL_Bloom_Init_InitTextures (void)
{

	if (vid.direct3d == 8)
		return;  // No CopyTexImage

    // find closer power of 2 to screen size 
    g_screen_texture_width_pow2   = Image_Power_Of_Two_Size  (clwidth);
	g_screen_texture_height_pow2  = Image_Power_Of_Two_Size (clheight);

    //disable blooms if we can't handle a texture of that size
	
	if (g_screen_texture_width_pow2 > renderer.gl_max_texture_size || g_screen_texture_height_pow2 > renderer.gl_max_texture_size)
		System_Error ("Required power of 2 size too large.  Fix me.  Or disable the feature or something.");
	
    //init the screen texture
	{
		size_t bytes_size = g_screen_texture_width_pow2 * g_screen_texture_height_pow2 * sizeof(unsigned) /* RGBA_4*/;
		unsigned *data = malloc(bytes_size);
		memset (data, 255, bytes_size);	// White fill
    
	//  glt_bloomscreentexture = R_LoadPic ("***r_bloomscreentexture***", &data, g_screen_texture_width_pow2, g_screen_texture_height_pow2, IT_NOMIPMAP|IT_NOCOMPRESS|IT_NOPICMIP|IT_NOALPHA, 3);
	//	r_bloomscreentexture = GL_LoadTexture  ("***r_screenbackuptexture***", g_screen_texture_width_pow2, g_screen_texture_height_pow2, data, TEX_LUMA, 4);
		glt_bloomscreentexture = TexMgr_LoadImage (
			NULL,							// Model owner
			-1,								// BSP texture number
			"glt_bloomscreentexture",		// Description
			g_screen_texture_width_pow2,	// Width
			g_screen_texture_height_pow2,	// Height
			SRC_RGBA,						// Source format src_format_e
			data,							// Data
			"",								// Source filename (qpath)
			(src_offset_t)data,				// Offset into file or offset into memory offset_t or uintptr_t

			// Flags ... Recalculate, alpha, nearest, persistent, don't picmip, blended means don't txgamma us
			TEXPREF_WARPIMAGE | TEXPREF_ALPHA | TEXPREF_NEAREST | TEXPREF_PERSIST | TEXPREF_NOPICMIP | TEXPREF_BLENDED
		);

		free (data);
	}

    // validate bloom size and init the bloom effect texture
    sGL_Bloom_InitEffectTexture ();

    // if screensize is more than 2x the bloom effect texture, set up for stepped downsampling
    glt_bloomdownsamplingtexture = NULL;
    g_screendownsamplingtexture_size = 0;
    if (clwidth > (g_bloom_size * 2) && gl_bloom_fast_sample.value == 0) // Baker: What about height?  This is biased towards width always > height
    {
		int width_height_size = g_screendownsamplingtexture_size = (int)(g_bloom_size * 2);
		unsigned *dummy_data = calloc (g_screendownsamplingtexture_size * g_screendownsamplingtexture_size, RGBA_4);
		glt_bloomdownsamplingtexture = TexMgr_LoadImage (
			NULL,							// Model owner
			-1,								// BSP texture number
			"glt_bloomdownsamplingtexture",	// Description
			width_height_size,				// Width
			width_height_size,				// Height
			SRC_RGBA,						// Source format src_format_e
			dummy_data,						// Data
			"",								// Source filename (qpath)
			(src_offset_t)dummy_data,		// Offset into file or offset into memory offset_t or uintptr_t

			// Flags ... Recalculate, alpha, nearest, persistent, don't picmip, blended means don't txgamma us
			TEXPREF_WARPIMAGE | TEXPREF_ALPHA | TEXPREF_NEAREST | TEXPREF_PERSIST | TEXPREF_NOPICMIP | TEXPREF_BLENDED
		);

		free (dummy_data);
		// ORIGINAL: glt_bloomdownsamplingtexture = R_LoadPic ("***r_bloomdownsamplingtexture***", &data, g_screendownsamplingtexture_size, g_screendownsamplingtexture_size, IT_NOMIPMAP|IT_NOCOMPRESS|IT_NOPICMIP|IT_NOALPHA, 3);
    }

    // Init the screen backup texture
    if (g_screendownsamplingtexture_size)
        sGL_Bloom_InitBackUpTexture (g_screendownsamplingtexture_size, g_screendownsamplingtexture_size);
    else
        sGL_Bloom_InitBackUpTexture (g_bloom_size, g_bloom_size);
}

/*
=================
R_InitBloomTextures
=================
*/
void GL_Bloom_Init (void)
{
return; // Baker ... for now

    g_bloom_size = 0;

//	if (!gl_bloom.value)  Baker: BULLSHIT
//        return; 

    sGL_Bloom_Init_InitTextures ();
}


/*
=================
sGL_Bloom_DrawEffect
=================
*/
static void sGL_Bloom_DrawEffect (void)
{
	float bloom_alpha = CLAMP(0, gl_bloom_alpha.value, 1);
    //GL_Bind(0, glt_bloomeffecttexture);
	//glBindTexture(GL_TEXTURE_2D, glt_bloomeffecttexture);
	GL_Bind			(glt_bloomeffecttexture);
    eglEnable		(GL_BLEND);
//  eglBlendFunc	(GL_ONE, GL_ONE);
	eglBlendFunc	(GL_ONE, GL_ONE_MINUS_SRC_COLOR);

    eglColor4f		(bloom_alpha, bloom_alpha, bloom_alpha, 0.8);
    eglTexEnvf		(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    eglBegin		(GL_QUADS);                         

    eglTexCoord2f	(0, g_sampleText_tch);  
    eglVertex2f		(curView_x, curView_y);      
	
    eglTexCoord2f	(0, 0);              
    eglVertex2f		(curView_x, curView_y + curView_height);  

    eglTexCoord2f	(g_sampleText_tcw, 0);
    eglVertex2f		(curView_x + curView_width, curView_y + curView_height);  

    eglTexCoord2f	(g_sampleText_tcw, g_sampleText_tch);  
    eglVertex2f		(curView_x + curView_width, curView_y);

	eglEnd			();
    
    eglDisable(GL_BLEND);
}

/*
=================
sGL_Bloom_GeneratexDiamonds
=================
*/
static void sGL_BloomBlend_GeneratexDiamonds (void)
{
    int         i, j;
    static float intensity;
	extern cvar_t gl_overbright;

    //set up sample size workspace
    eglViewport				(0, 0, g_sample_width, g_sample_height);
    eglMatrixMode			(GL_PROJECTION);
    eglLoadIdentity			();
    eglOrtho				(0, g_sample_width, g_sample_height, 0, -10, 100);
    eglMatrixMode			(GL_MODELVIEW);
    eglLoadIdentity			();

    //copy small scene into glt_bloomeffecttexture
    //GL_Bind(0, glt_bloomeffecttexture);
	GL_Bind					(glt_bloomeffecttexture);
    eglCopyTexSubImage2D	(GL_TEXTURE_2D, 0, 0, 0, 0, 0, g_sample_width, g_sample_height);

    //start modifying the small scene corner
    eglColor4f				(1.0, 1.0, 1.0, 1.0);
    eglEnable				(GL_BLEND);

    //darkening passes
    if (gl_bloom_darken.value)
    {
        eglBlendFunc			(GL_DST_COLOR, GL_ZERO);
        eglTexEnvf				(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        
        for (i = 0; i < gl_bloom_darken.value; i++) {
            sGL_Bloom_SamplePass (0, 0);
        }
        eglCopyTexSubImage2D	(GL_TEXTURE_2D, 0, 0, 0, 0, 0, g_sample_width, g_sample_height);
    }

    //bluring passes
    //glBlendFunc(GL_ONE, GL_ONE);
    eglBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_COLOR);
    
    if (gl_bloom_diamond_size.value > 7 || gl_bloom_diamond_size.value <= 3)
    {
        if (gl_bloom_diamond_size.value != 8) 
			Cvar_SetValueQuick (&gl_bloom_diamond_size, 8);

        for (i = 0; i < gl_bloom_diamond_size.value; i++) 
		{
            for (j = 0; j < gl_bloom_diamond_size.value; j++) 
			{
				// Baker: Seriously?
				if (chase_active.value || gl_overbright.value || cl.viewent_gun.model->name == NULL)
					intensity = gl_bloom_intensity.value * 0.1 * Diamond8x[i][j];
				else
					intensity = gl_bloom_intensity.value * 0.3 * Diamond8x[i][j];

                if (intensity < 0.01f) continue;
                eglColor4f (intensity, intensity, intensity, 1.0);
                sGL_Bloom_SamplePass (i - 4, j - 4);
            }
        }
    } 
	else 
	if (gl_bloom_diamond_size.value > 5) 
	{
        
        if (gl_bloom_diamond_size.value != 6)
			Cvar_SetValueQuick (&gl_bloom_diamond_size, 6);

        for (i = 0; i < gl_bloom_diamond_size.value; i++) 
		{
            for(j = 0; j < gl_bloom_diamond_size.value; j++) 
			{
				if (chase_active.value || gl_overbright.value || cl.viewent_gun.model->name == NULL)
					intensity = gl_bloom_intensity.value * 0.1 * Diamond6x[i][j];
				else
					intensity = gl_bloom_intensity.value * 0.5 * Diamond6x[i][j];
                
				if (intensity < 0.01) 
					continue;

                eglColor4f (intensity, intensity, intensity, 1.0);
                sGL_Bloom_SamplePass (i-3, j-3);
            }
        }
    } 
	else 
	if (gl_bloom_diamond_size.value > 3) 
	{
		if (gl_bloom_diamond_size.value != 4) 
			Cvar_SetValueQuick (&gl_bloom_diamond_size, 4);

        for (i = 0; i < gl_bloom_diamond_size.value; i++) {
            for (j = 0; j < gl_bloom_diamond_size.value; j++) {
				
				if (chase_active.value || gl_overbright.value || cl.viewent_gun.model->name == NULL)
					intensity = gl_bloom_intensity.value * 0.1 * Diamond4x[i][j];
				else
					intensity = gl_bloom_intensity.value * 0.8 * Diamond4x[i][j];

                if (intensity < 0.01) 
					continue;
                eglColor4f		(intensity, intensity, intensity, 1.0);
                sGL_Bloom_SamplePass (i - 2, j - 2);
            }
        }
    }
    
    eglCopyTexSubImage2D		(GL_TEXTURE_2D, 0, 0, 0, 0, 0, g_sample_width, g_sample_height);

    //restore full screen workspace
    eglViewport					(0, 0, clwidth, clheight);
    eglMatrixMode				(GL_PROJECTION);
    eglLoadIdentity				();
    eglOrtho					(0, clwidth, clheight, 0, -10, 100); // Baker: Uh, your fucking kidding me right?
    eglMatrixMode				(GL_MODELVIEW);
    eglLoadIdentity				();
}                                           

/*
=================
sGL_Bloom_DownsampleView
=================
*/
void sGL_BloomBlend_DownsampleView (void)
{
    eglDisable (GL_BLEND);
    eglColor4f (1.0, 1.0, 1.0, 1.0);

    // stepped downsample
    if (g_screendownsamplingtexture_size)
    {
        int midsample_width = g_screendownsamplingtexture_size * g_sampleText_tcw;
        int midsample_height = g_screendownsamplingtexture_size * g_sampleText_tch;
        
        //copy the screen and draw resized
//      GL_Bind(0, r_bloomscreentexture);
		GL_Bind					(glt_bloomscreentexture);
        eglCopyTexSubImage2D	(GL_TEXTURE_2D, 0, 0, 0, curView_x, clheight - (curView_y + curView_height), curView_width, curView_height);
        sGL_Bloom_Quad			(0, clheight-midsample_height, midsample_width, midsample_height, screenText_tcw, screenText_tch );
        
        //now copy into Downsampling (mid-sized) texture
        //GL_Bind(0, r_bloomdownsamplingtexture);
		GL_Bind					(glt_bloomdownsamplingtexture);
        eglCopyTexSubImage2D	(GL_TEXTURE_2D, 0, 0, 0, 0, 0, midsample_width, midsample_height);

        //now draw again in bloom size
        eglColor4f				(0.5, 0.5, 0.5, 1.0);
        sGL_Bloom_Quad			(0, clheight-g_sample_height, g_sample_width, g_sample_height, g_sampleText_tcw, g_sampleText_tch);
        
        //now blend the big screen texture into the bloom generation space (hoping it adds some blur)
        eglEnable				(GL_BLEND);
        eglBlendFunc			(GL_ONE, GL_ONE);
        eglColor4f				(0.5, 0.5, 0.5, 1.0);
        //GL_Bind(0, r_bloomscreentexture);
		GL_Bind					(glt_bloomscreentexture);
        sGL_Bloom_Quad			(0, clheight-g_sample_height, g_sample_width, g_sample_height, screenText_tcw, screenText_tch);
        eglColor4f				(1.0, 1.0, 1.0, 1.0);
        eglDisable				(GL_BLEND);

    } else {    //downsample simple

        //GL_Bind(0, r_bloomscreentexture);
		GL_Bind					(glt_bloomscreentexture);
        eglCopyTexSubImage2D	(GL_TEXTURE_2D, 0 /*level*/, 0 /* xoffset within texture */, 0 /*y offset within texture*/, curView_x, clheight - (curView_y + curView_height), curView_width, curView_height);
        sGL_Bloom_Quad			(0, clheight - g_sample_height, g_sample_width, g_sample_height, screenText_tcw, screenText_tch);
    }
}

/*
=================
GL_BloomBlend
=================
*/
void GL_BloomBlend (void)
{
return; // For now ...
    if (!gl_bloom.value || vid.direct3d == 8) // DX8 - doesn't have CopyTexSubImage
        return;

    if (!g_bloom_size)
		System_Error ("Bloom_InitTextures never called, g_bloom_size is zero");

    if (g_screen_texture_width_pow2 < g_bloom_size || g_screen_texture_height_pow2 < g_bloom_size)
        return; // Baker: Isn't this an error?

    // set up full screen workspace
    eglViewport			(0, 0, clwidth, clheight);
    eglMatrixMode		(GL_PROJECTION);
    eglLoadIdentity		();
    eglOrtho			(0, clwidth, clheight, 0, -10, 100); // left right bottom top near far
    eglMatrixMode		(GL_MODELVIEW);
    eglLoadIdentity		();

	eglDisable			(GL_DEPTH_TEST);
    eglDisable			(GL_CULL_FACE);
    eglDisable			(GL_BLEND);
    eglEnable			(GL_TEXTURE_2D);
    eglColor4f			(1, 1, 1, 1);

    //set up current sizes
//  curView_x = fd->x;
//  curView_y = fd->y;
	curView_x = 0;
	curView_y = 0;
    curView_width = clwidth;
    curView_height = clheight;
    screenText_tcw = (float)clwidth / g_screen_texture_width_pow2;
    screenText_tch = (float)clheight / g_screen_texture_height_pow2;

    if (clheight > clwidth) {
        g_sampleText_tcw = (float)clwidth / (float)clheight;
		g_sampleText_tch = 1.0;
    } 
	else {
        g_sampleText_tcw = 1.0;
        g_sampleText_tch = (float)clheight / (float)clwidth;
    }

    g_sample_width  = g_bloom_size * g_sampleText_tcw;
    g_sample_height = g_bloom_size * g_sampleText_tch;
    
    // copy the screen space we'll use to work into the backup texture
	GL_Bind						(glt_bloombackuptexture);
    eglCopyTexSubImage2D		(GL_TEXTURE_2D, 0, 0, 0, 0, 0, g_screenbackuptexture_width, g_screenbackuptexture_height);  
	
    // create the bloom image
	sGL_BloomBlend_DownsampleView ();
    sGL_BloomBlend_GeneratexDiamonds ();

    //restore the screen-backup to the screen
    eglDisable					(GL_BLEND);
	GL_Bind						(glt_bloombackuptexture);
    eglColor4f					(1, 1, 1, 1);
    sGL_Bloom_Quad				(0, clheight - g_screenbackuptexture_height, g_screenbackuptexture_width, g_screenbackuptexture_height, 1.0, 1.0);
	sGL_Bloom_DrawEffect		();
	eglColor3f					(1, 1, 1);   
    eglDisable					(GL_BLEND); 
    eglEnable					(GL_TEXTURE_2D);   
	eglEnable					(GL_DEPTH_TEST);
    eglBlendFunc				(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);   
	eglViewport					(clx, cly, clwidth, clheight);
}


#endif // GLQUAKE specific