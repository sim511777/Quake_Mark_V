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


void context_t::SetTextureState (DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	// filter state
	if (this->State.TextureStates[Stage][(int) Type] == Value) return;

	this->FlushGeometry ();

	// set the state and cache it back
	this->Device->SetTextureStageState (Stage, Type, Value);
	this->State.TextureStates[Stage][(int) Type] = Value;
}


void context_t::SetSamplerState (DWORD Stage, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	// filter state
	if (this->State.SamplerStates[Stage][(int) Type] == Value) return;

	this->FlushGeometry ();

	// set the state and cache it back
	this->Device->SetSamplerState (Stage, Type, Value);
	this->State.SamplerStates[Stage][(int) Type] = Value;
}


// combine functions
#define GLD3D_TEXTURE_ENV_MODE 0x2200
#define GLD3D_SUBTRACT 0x84E7
#define GLD3D_COMBINE 0x8570
#define GLD3D_COMBINE_RGB 0x8571
#define GLD3D_COMBINE_ALPHA 0x8572
#define GLD3D_RGB_SCALE 0x8573
#define GLD3D_ADD_SIGNED 0x8574
#define GLD3D_INTERPOLATE 0x8575
#define GLD3D_CONSTANT 0x8576
#define GLD3D_PRIMARY_COLOR 0x8577
#define GLD3D_PREVIOUS 0x8578
#define GLD3D_SOURCE0_RGB 0x8580
#define GLD3D_SOURCE1_RGB 0x8581
#define GLD3D_SOURCE2_RGB 0x8582
#define GLD3D_SOURCE0_ALPHA 0x8588
#define GLD3D_SOURCE1_ALPHA 0x8589
#define GLD3D_SOURCE2_ALPHA 0x858A
#define GLD3D_OPERAND0_RGB 0x8590
#define GLD3D_OPERAND1_RGB 0x8591
#define GLD3D_OPERAND2_RGB 0x8592
#define GLD3D_OPERAND0_ALPHA 0x8598
#define GLD3D_OPERAND1_ALPHA 0x8599
#define GLD3D_OPERAND2_ALPHA 0x859A
#define GLD3D_ALPHA_SCALE 0x0D1C


void context_t::InitTexEnv (void)
{
	for (int i = 0; i < D3D_MAX_TMUS; i++)
	{
		// texture object 0 is initially bound
		this->TMU[i].boundtexture = &this->Textures[0];
		this->TMU[i].enabled = FALSE;
		this->TMU[i].TexEnv.TEXTURE_ENV_MODE = GL_MODULATE;
	}
}


void context_t::TexEnv (GLenum target, GLenum pname, GLint param)
{
	tmustate_t *tmu = &this->TMU[this->State.CurrentTMU];

	if (target != GL_TEXTURE_ENV) System_Error ("glTexEnvf: unimplemented target");

	switch (pname)
	{
		case GLD3D_TEXTURE_ENV_MODE: tmu->TexEnv.TEXTURE_ENV_MODE = param; break;
		case GLD3D_COMBINE_RGB: tmu->TexEnv.COMBINE_RGB = param; break;
		case GLD3D_COMBINE_ALPHA: tmu->TexEnv.COMBINE_ALPHA = param; break;
		case GLD3D_SRC0_RGB: tmu->TexEnv.SRC0_RGB = param; break;
		case GLD3D_SRC1_RGB: tmu->TexEnv.SRC1_RGB = param; break;
		case GLD3D_SRC2_RGB: tmu->TexEnv.SRC2_RGB = param; break;
		case GLD3D_SRC0_ALPHA: tmu->TexEnv.SRC0_ALPHA = param; break;
		case GLD3D_SRC1_ALPHA: tmu->TexEnv.SRC1_ALPHA = param; break;
		case GLD3D_SRC2_ALPHA: tmu->TexEnv.SRC2_ALPHA = param; break;
		case GLD3D_OPERAND0_RGB: tmu->TexEnv.OPERAND0_RGB = param; break;
		case GLD3D_OPERAND1_RGB: tmu->TexEnv.OPERAND1_RGB = param; break;
		case GLD3D_OPERAND2_RGB: tmu->TexEnv.OPERAND2_RGB = param; break;
		case GLD3D_OPERAND0_ALPHA: tmu->TexEnv.OPERAND0_ALPHA = param; break;
		case GLD3D_OPERAND1_ALPHA: tmu->TexEnv.OPERAND1_ALPHA = param; break;
		case GLD3D_OPERAND2_ALPHA: tmu->TexEnv.OPERAND2_ALPHA = param; break;
		case GLD3D_RGB_SCALE: tmu->TexEnv.RGB_SCALE = param; break;
		case GLD3D_ALPHA_SCALE: tmu->TexEnv.ALPHA_SCALE = param; break;
	}

	// mark for state update
	tmu->texenvdirty = TRUE;
}


D3DTEXTUREOP TextureOpGLToD3D (GLenum op, float scale)
{
	switch (op)
	{
	case GL_REPLACE:
		return D3DTOP_SELECTARG1;

	case GL_MODULATE:
		if (scale > 2)
			return D3DTOP_MODULATE4X;
		else if (scale > 1)
			return D3DTOP_MODULATE2X;
		else return D3DTOP_MODULATE;

	case GL_ADD:
		return D3DTOP_ADD;

	case GLD3D_ADD_SIGNED:
		if (scale > 1)
			return D3DTOP_ADDSIGNED2X;
		else D3DTOP_ADDSIGNED;

	case GLD3D_INTERPOLATE:
		return D3DTOP_LERP;

	case GLD3D_SUBTRACT:
		return D3DTOP_SUBTRACT;

	case GLD3D_DOT3_RGB:
	case GLD3D_DOT3_RGBA:
		return D3DTOP_DOTPRODUCT3;
	}

	return D3DTOP_MODULATE;
}


DWORD TextureArgGLToD3D (GLenum arg, GLenum operand)
{
	DWORD modifiers = 0;

	switch (operand)
	{
	case GL_SRC_COLOR:
		modifiers = 0;
		break;

	case GL_ONE_MINUS_SRC_COLOR:
		modifiers = D3DTA_COMPLEMENT;
		break;

	case GL_SRC_ALPHA:
		modifiers = D3DTA_ALPHAREPLICATE;
		break;

	case GL_ONE_MINUS_SRC_ALPHA:
		modifiers = D3DTA_COMPLEMENT | D3DTA_ALPHAREPLICATE;
		break;
	}

	switch (arg)
	{
		// don't support texture_env_crossbar
	case GL_TEXTURE:
		return modifiers | D3DTA_TEXTURE;

	case GLD3D_CONSTANT:
		return modifiers | D3DTA_CONSTANT;

	case GLD3D_PRIMARY_COLOR:
		return modifiers | D3DTA_DIFFUSE;

	case GLD3D_PREVIOUS:
		return modifiers | D3DTA_CURRENT;
	}

	return 0;
}


void context_t::CheckDirtyTextureStates (int stage)
{
	tmustate_t *tmu = &this->TMU[stage];

	if (tmu->boundtexture)
	{
		if (tmu->texparamdirty)
		{
			// setup texture states - these ones are specific to the texture and come from glTexParameter
			this->SetSamplerState (stage, D3DSAMP_ADDRESSU, tmu->boundtexture->TexParms.AddressU);
			this->SetSamplerState (stage, D3DSAMP_ADDRESSV, tmu->boundtexture->TexParms.AddressV);
			this->SetSamplerState (stage, D3DSAMP_MAXANISOTROPY, tmu->boundtexture->TexParms.Anisotropy);

			// minfilter and magfilter need to switch to anisotropic
			if (tmu->boundtexture->TexParms.Anisotropy > 1)
			{
				this->SetSamplerState (stage, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
				this->SetSamplerState (stage, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
			}
			else
			{
				this->SetSamplerState (stage, D3DSAMP_MAGFILTER, tmu->boundtexture->TexParms.MagFilter);
				this->SetSamplerState (stage, D3DSAMP_MINFILTER, tmu->boundtexture->TexParms.MinFilter);
			}

			this->SetSamplerState (stage, D3DSAMP_MIPFILTER, tmu->boundtexture->TexParms.MipFilter);

			tmu->texparamdirty = FALSE;
		}
	}

	if (tmu->texenvdirty)
	{
		// these ones are specific to the TMU and come from glTexEnv
		switch (tmu->TexEnv.TEXTURE_ENV_MODE)
		{
		case GLD3D_COMBINE:
			this->SetTextureState (stage, D3DTSS_COLOROP, TextureOpGLToD3D (tmu->TexEnv.COMBINE_RGB, tmu->TexEnv.RGB_SCALE));
			this->SetTextureState (stage, D3DTSS_COLORARG1, TextureArgGLToD3D (tmu->TexEnv.SRC0_RGB, tmu->TexEnv.OPERAND0_RGB));
			this->SetTextureState (stage, D3DTSS_COLORARG2, TextureArgGLToD3D (tmu->TexEnv.SRC1_RGB, tmu->TexEnv.OPERAND1_RGB));

			this->SetTextureState (stage, D3DTSS_ALPHAOP, TextureOpGLToD3D (tmu->TexEnv.COMBINE_ALPHA, tmu->TexEnv.ALPHA_SCALE));
			this->SetTextureState (stage, D3DTSS_ALPHAARG1, TextureArgGLToD3D (tmu->TexEnv.SRC0_ALPHA, tmu->TexEnv.OPERAND0_ALPHA));
			this->SetTextureState (stage, D3DTSS_ALPHAARG2, TextureArgGLToD3D (tmu->TexEnv.SRC1_ALPHA, tmu->TexEnv.OPERAND1_ALPHA));
			break;

		case GL_ADD:
			this->SetTextureState (stage, D3DTSS_COLOROP, D3DTOP_ADD);
			this->SetTextureState (stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			this->SetTextureState (stage, D3DTSS_COLORARG2, (stage == 0 ? D3DTA_DIFFUSE : D3DTA_CURRENT));

			this->SetTextureState (stage, D3DTSS_ALPHAOP, D3DTOP_ADD);
			this->SetTextureState (stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			this->SetTextureState (stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			break;

		case GL_MODULATE:
			this->SetTextureState (stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
			this->SetTextureState (stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			this->SetTextureState (stage, D3DTSS_COLORARG2, (stage == 0 ? D3DTA_DIFFUSE : D3DTA_CURRENT));

			this->SetTextureState (stage, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			this->SetTextureState (stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			this->SetTextureState (stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			break;

		case GL_REPLACE:
			this->SetTextureState (stage, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			this->SetTextureState (stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			this->SetTextureState (stage, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

			this->SetTextureState (stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			this->SetTextureState (stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			this->SetTextureState (stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			break;

		case GL_DECAL:
			this->SetTextureState (stage, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
			this->SetTextureState (stage, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			this->SetTextureState (stage, D3DTSS_COLORARG2, (stage == 0 ? D3DTA_DIFFUSE : D3DTA_CURRENT));

			this->SetTextureState (stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			this->SetTextureState (stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			this->SetTextureState (stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			break;

		case GL_BLEND:
			this->SetTextureState (stage, D3DTSS_COLOROP, D3DTOP_MODULATE);
			this->SetTextureState (stage, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_COMPLEMENT);
			this->SetTextureState (stage, D3DTSS_COLORARG2, (this->State.CurrentTMU == 0 ? D3DTA_DIFFUSE : D3DTA_CURRENT));

			this->SetTextureState (stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			this->SetTextureState (stage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			this->SetTextureState (stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			break;
		}

		tmu->texenvdirty = FALSE;
	}
}


void context_t::GetTextureStates (void)
{
	this->FlushGeometry ();

	for (int i = 0; i < D3D_MAX_TMUS; i++)
	{
		this->Device->GetTextureStageState (i, D3DTSS_COLOROP, &this->State.TextureStates[i][D3DTSS_COLOROP]);
		this->Device->GetTextureStageState (i, D3DTSS_COLORARG1, &this->State.TextureStates[i][D3DTSS_COLORARG1]);
		this->Device->GetTextureStageState (i, D3DTSS_COLORARG2, &this->State.TextureStates[i][D3DTSS_COLORARG2]);
		this->Device->GetTextureStageState (i, D3DTSS_ALPHAOP, &this->State.TextureStates[i][D3DTSS_ALPHAOP]);
		this->Device->GetTextureStageState (i, D3DTSS_ALPHAARG1, &this->State.TextureStates[i][D3DTSS_ALPHAARG1]);
		this->Device->GetTextureStageState (i, D3DTSS_ALPHAARG2, &this->State.TextureStates[i][D3DTSS_ALPHAARG2]);
		this->Device->GetTextureStageState (i, D3DTSS_BUMPENVMAT00, &this->State.TextureStates[i][D3DTSS_BUMPENVMAT00]);
		this->Device->GetTextureStageState (i, D3DTSS_BUMPENVMAT01, &this->State.TextureStates[i][D3DTSS_BUMPENVMAT01]);
		this->Device->GetTextureStageState (i, D3DTSS_BUMPENVMAT10, &this->State.TextureStates[i][D3DTSS_BUMPENVMAT10]);
		this->Device->GetTextureStageState (i, D3DTSS_BUMPENVMAT11, &this->State.TextureStates[i][D3DTSS_BUMPENVMAT11]);
		this->Device->GetTextureStageState (i, D3DTSS_TEXCOORDINDEX, &this->State.TextureStates[i][D3DTSS_TEXCOORDINDEX]);
		this->Device->GetTextureStageState (i, D3DTSS_BUMPENVLSCALE, &this->State.TextureStates[i][D3DTSS_BUMPENVLSCALE]);
		this->Device->GetTextureStageState (i, D3DTSS_BUMPENVLOFFSET, &this->State.TextureStates[i][D3DTSS_BUMPENVLOFFSET]);
		this->Device->GetTextureStageState (i, D3DTSS_TEXTURETRANSFORMFLAGS, &this->State.TextureStates[i][D3DTSS_TEXTURETRANSFORMFLAGS]);
		this->Device->GetTextureStageState (i, D3DTSS_COLORARG0, &this->State.TextureStates[i][D3DTSS_COLORARG0]);
		this->Device->GetTextureStageState (i, D3DTSS_ALPHAARG0, &this->State.TextureStates[i][D3DTSS_ALPHAARG0]);
		this->Device->GetTextureStageState (i, D3DTSS_RESULTARG, &this->State.TextureStates[i][D3DTSS_RESULTARG]);
		this->Device->GetTextureStageState (i, D3DTSS_CONSTANT, &this->State.TextureStates[i][D3DTSS_CONSTANT]);

		this->Device->GetSamplerState (i, D3DSAMP_ADDRESSU, &this->State.SamplerStates[i][D3DSAMP_ADDRESSU]);
		this->Device->GetSamplerState (i, D3DSAMP_ADDRESSV, &this->State.SamplerStates[i][D3DSAMP_ADDRESSV]);
		this->Device->GetSamplerState (i, D3DSAMP_ADDRESSW, &this->State.SamplerStates[i][D3DSAMP_ADDRESSW]);
		this->Device->GetSamplerState (i, D3DSAMP_BORDERCOLOR, &this->State.SamplerStates[i][D3DSAMP_BORDERCOLOR]);
		this->Device->GetSamplerState (i, D3DSAMP_MAGFILTER, &this->State.SamplerStates[i][D3DSAMP_MAGFILTER]);
		this->Device->GetSamplerState (i, D3DSAMP_MINFILTER, &this->State.SamplerStates[i][D3DSAMP_MINFILTER]);
		this->Device->GetSamplerState (i, D3DSAMP_MIPFILTER, &this->State.SamplerStates[i][D3DSAMP_MIPFILTER]);
		this->Device->GetSamplerState (i, D3DSAMP_MIPMAPLODBIAS, &this->State.SamplerStates[i][D3DSAMP_MIPMAPLODBIAS]);
		this->Device->GetSamplerState (i, D3DSAMP_MAXMIPLEVEL, &this->State.SamplerStates[i][D3DSAMP_MAXMIPLEVEL]);
		this->Device->GetSamplerState (i, D3DSAMP_MAXANISOTROPY, &this->State.SamplerStates[i][D3DSAMP_MAXANISOTROPY]);
		this->Device->GetSamplerState (i, D3DSAMP_SRGBTEXTURE, &this->State.SamplerStates[i][D3DSAMP_SRGBTEXTURE]);
		this->Device->GetSamplerState (i, D3DSAMP_ELEMENTINDEX, &this->State.SamplerStates[i][D3DSAMP_ELEMENTINDEX]);
		this->Device->GetSamplerState (i, D3DSAMP_DMAPOFFSET, &this->State.SamplerStates[i][D3DSAMP_DMAPOFFSET]);
	}
}


void context_t::SetTextureStates (void)
{
	this->FlushGeometry ();

	for (int i = 0; i < D3D_MAX_TMUS; i++)
	{
		this->Device->SetTextureStageState (i, D3DTSS_COLOROP, this->State.TextureStates[i][D3DTSS_COLOROP]);
		this->Device->SetTextureStageState (i, D3DTSS_COLORARG1, this->State.TextureStates[i][D3DTSS_COLORARG1]);
		this->Device->SetTextureStageState (i, D3DTSS_COLORARG2, this->State.TextureStates[i][D3DTSS_COLORARG2]);
		this->Device->SetTextureStageState (i, D3DTSS_ALPHAOP, this->State.TextureStates[i][D3DTSS_ALPHAOP]);
		this->Device->SetTextureStageState (i, D3DTSS_ALPHAARG1, this->State.TextureStates[i][D3DTSS_ALPHAARG1]);
		this->Device->SetTextureStageState (i, D3DTSS_ALPHAARG2, this->State.TextureStates[i][D3DTSS_ALPHAARG2]);
		this->Device->SetTextureStageState (i, D3DTSS_BUMPENVMAT00, this->State.TextureStates[i][D3DTSS_BUMPENVMAT00]);
		this->Device->SetTextureStageState (i, D3DTSS_BUMPENVMAT01, this->State.TextureStates[i][D3DTSS_BUMPENVMAT01]);
		this->Device->SetTextureStageState (i, D3DTSS_BUMPENVMAT10, this->State.TextureStates[i][D3DTSS_BUMPENVMAT10]);
		this->Device->SetTextureStageState (i, D3DTSS_BUMPENVMAT11, this->State.TextureStates[i][D3DTSS_BUMPENVMAT11]);
		this->Device->SetTextureStageState (i, D3DTSS_TEXCOORDINDEX, this->State.TextureStates[i][D3DTSS_TEXCOORDINDEX]);
		this->Device->SetTextureStageState (i, D3DTSS_BUMPENVLSCALE, this->State.TextureStates[i][D3DTSS_BUMPENVLSCALE]);
		this->Device->SetTextureStageState (i, D3DTSS_BUMPENVLOFFSET, this->State.TextureStates[i][D3DTSS_BUMPENVLOFFSET]);
		this->Device->SetTextureStageState (i, D3DTSS_TEXTURETRANSFORMFLAGS, this->State.TextureStates[i][D3DTSS_TEXTURETRANSFORMFLAGS]);
		this->Device->SetTextureStageState (i, D3DTSS_COLORARG0, this->State.TextureStates[i][D3DTSS_COLORARG0]);
		this->Device->SetTextureStageState (i, D3DTSS_ALPHAARG0, this->State.TextureStates[i][D3DTSS_ALPHAARG0]);
		this->Device->SetTextureStageState (i, D3DTSS_RESULTARG, this->State.TextureStates[i][D3DTSS_RESULTARG]);
		this->Device->SetTextureStageState (i, D3DTSS_CONSTANT, this->State.TextureStates[i][D3DTSS_CONSTANT]);

		this->Device->SetSamplerState (i, D3DSAMP_ADDRESSU, this->State.SamplerStates[i][D3DSAMP_ADDRESSU]);
		this->Device->SetSamplerState (i, D3DSAMP_ADDRESSV, this->State.SamplerStates[i][D3DSAMP_ADDRESSV]);
		this->Device->SetSamplerState (i, D3DSAMP_ADDRESSW, this->State.SamplerStates[i][D3DSAMP_ADDRESSW]);
		this->Device->SetSamplerState (i, D3DSAMP_BORDERCOLOR, this->State.SamplerStates[i][D3DSAMP_BORDERCOLOR]);
		this->Device->SetSamplerState (i, D3DSAMP_MAGFILTER, this->State.SamplerStates[i][D3DSAMP_MAGFILTER]);
		this->Device->SetSamplerState (i, D3DSAMP_MINFILTER, this->State.SamplerStates[i][D3DSAMP_MINFILTER]);
		this->Device->SetSamplerState (i, D3DSAMP_MIPFILTER, this->State.SamplerStates[i][D3DSAMP_MIPFILTER]);
		this->Device->SetSamplerState (i, D3DSAMP_MIPMAPLODBIAS, this->State.SamplerStates[i][D3DSAMP_MIPMAPLODBIAS]);
		this->Device->SetSamplerState (i, D3DSAMP_MAXMIPLEVEL, this->State.SamplerStates[i][D3DSAMP_MAXMIPLEVEL]);
		this->Device->SetSamplerState (i, D3DSAMP_MAXANISOTROPY, this->State.SamplerStates[i][D3DSAMP_MAXANISOTROPY]);
		this->Device->SetSamplerState (i, D3DSAMP_SRGBTEXTURE, this->State.SamplerStates[i][D3DSAMP_SRGBTEXTURE]);
		this->Device->SetSamplerState (i, D3DSAMP_ELEMENTINDEX, this->State.SamplerStates[i][D3DSAMP_ELEMENTINDEX]);
		this->Device->SetSamplerState (i, D3DSAMP_DMAPOFFSET, this->State.SamplerStates[i][D3DSAMP_DMAPOFFSET]);
	}
}


void context_t::SetupTexturesForDrawing (void)
{
	for (int i = 0; i < D3D_MAX_TMUS; i++)
	{
		// end of TMUs
		if (!this->TMU[i].enabled || !this->TMU[i].boundtexture)
		{
			// explicitly disable alpha and color ops in this and subsequent stages
			this->SetTextureState (i, D3DTSS_COLOROP, D3DTOP_DISABLE);
			this->SetTextureState (i, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

			// done
			break;
		}

		// check for dirty states (this is needed as OpenGL sets state per-texture as well as per-stage)
		this->CheckDirtyTextureStates (i);

		// check the texture for dirty rects and update them if so
		this->TMU[i].boundtexture->CheckDirty (this);

		// bind the texture (now with added filtering!)
		this->SetTexture (i, this->TMU[i].boundtexture->TexImage);
	}
}


#endif
