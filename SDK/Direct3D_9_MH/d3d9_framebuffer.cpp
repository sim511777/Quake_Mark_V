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


void context_t::BlendFunc (D3DRENDERSTATETYPE rs, GLenum factor)
{
	switch (factor)
	{
	case GL_ZERO: this->SetRenderState (rs, D3DBLEND_ZERO); break;
	case GL_ONE: this->SetRenderState (rs, D3DBLEND_ONE); break;
	case GL_SRC_COLOR: this->SetRenderState (rs, D3DBLEND_SRCCOLOR); break;
	case GL_ONE_MINUS_SRC_COLOR: this->SetRenderState (rs, D3DBLEND_INVSRCCOLOR); break;
	case GL_DST_COLOR: this->SetRenderState (rs, D3DBLEND_DESTCOLOR); break;
	case GL_ONE_MINUS_DST_COLOR: this->SetRenderState (rs, D3DBLEND_INVDESTCOLOR); break;
	case GL_SRC_ALPHA: this->SetRenderState (rs, D3DBLEND_SRCALPHA); break;
	case GL_ONE_MINUS_SRC_ALPHA: this->SetRenderState (rs, D3DBLEND_INVSRCALPHA); break;
	case GL_DST_ALPHA: this->SetRenderState (rs, D3DBLEND_DESTALPHA); break;
	case GL_ONE_MINUS_DST_ALPHA: this->SetRenderState (rs, D3DBLEND_INVDESTALPHA); break;
	case GL_SRC_ALPHA_SATURATE: this->SetRenderState (rs, D3DBLEND_SRCALPHASAT); break;
	default: return;
	}
}


void context_t::Clear (DWORD ClearFlags)
{
	if (ClearFlags)
	{
		this->FlushGeometry ();

		// D3D clear is clipped to the viewport; because FitzQuake has (probably) left us with a smaller viewport at the
		// end of the previous frame, we must set it back to the full rendertarget dimensions
		this->ResetViewport ();

		// back to normal operation
		this->Device->Clear (0, NULL, ClearFlags, this->State.Clear.Color, this->State.Clear.Depth, this->State.Clear.Stencil);
	}
}


void context_t::StencilOp (D3DRENDERSTATETYPE d3dState, GLenum GLstate)
{
	switch (GLstate)
	{
	case GL_KEEP: this->SetRenderState (d3dState, D3DSTENCILOP_KEEP); break;
	case GL_ZERO: this->SetRenderState (d3dState, D3DSTENCILOP_ZERO); break;
	case GL_REPLACE: this->SetRenderState (d3dState, D3DSTENCILOP_REPLACE); break;
	case GL_INCR: this->SetRenderState (d3dState, D3DSTENCILOP_INCRSAT); break;
	case GL_INCR_WRAP: this->SetRenderState (d3dState, D3DSTENCILOP_INCR); break;
	case GL_DECR: this->SetRenderState (d3dState, D3DSTENCILOP_DECRSAT); break;
	case GL_DECR_WRAP: this->SetRenderState (d3dState, D3DSTENCILOP_DECR); break;
	case GL_INVERT: this->SetRenderState (d3dState, D3DSTENCILOP_INVERT); break;
	}
}


void context_t::ReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, unsigned char *dstdata)
{
	IDirect3DSurface9 *bbsurf;
	IDirect3DSurface9 *locksurf;
	D3DLOCKED_RECT lockrect;
	D3DSURFACE_DESC desc;

	int srcbytes = 4; // no 24-bit sources
	int dstbytes = D3D_BytesForFormat (format);

	if (SUCCEEDED (this->Device->GetRenderTarget (0, &bbsurf)))
	{
		// because we don't have a lockable backbuffer we instead copy it off to an image surface
		// this will also handle translation between different backbuffer formats
		if (SUCCEEDED (bbsurf->GetDesc (&desc)))
		{
			if (SUCCEEDED (this->Device->CreateOffscreenPlainSurface (desc.Width, desc.Height, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &locksurf, NULL)))
			{
				if (SUCCEEDED (QD3DXLoadSurfaceFromSurface (locksurf, NULL, NULL, bbsurf, NULL, NULL, D3DX_FILTER_NONE, 0)))
				{
					// now we have a surface we can lock
					if (SUCCEEDED (locksurf->LockRect (&lockrect, NULL, D3DLOCK_READONLY)))
					{
						// invert the copied image
						for (int row = (y + height - 1); row >= y; row--)
						{
							// to do: use D3D_CopyTexels
							// because we're using a D3DPOOL_SCRATCH texture, we don't need to adjust for non-multiple-of-4 sizes
							unsigned char *srcdata = &((unsigned char *) lockrect.pBits)[row * width * srcbytes];

							for (int col = x; col < (x + width); col++, srcdata += srcbytes, dstdata += dstbytes)
							{
								// srcdata is always 4-byte even if dstdata is 3; reverse the sense of RGBA/BGRA because D3D is BGRA natively
								// (so is OpenGL but it pretends not to be)
								switch (format)
								{
								case GL_RGB:
									dstdata[2] = srcdata[0];
									dstdata[1] = srcdata[1];
									dstdata[0] = srcdata[2];
									break;

								case GL_BGR_EXT:
									dstdata[2] = srcdata[2];
									dstdata[1] = srcdata[1];
									dstdata[0] = srcdata[0];
									break;

								case GL_RGBA:
									dstdata[3] = srcdata[3];
									dstdata[2] = srcdata[0];
									dstdata[1] = srcdata[1];
									dstdata[0] = srcdata[2];
									break;

								case GL_BGRA_EXT:
									dstdata[3] = srcdata[3];
									dstdata[2] = srcdata[2];
									dstdata[1] = srcdata[1];
									dstdata[0] = srcdata[0];
									break;
								}
							}
						}

						locksurf->UnlockRect ();
					}
				}
			}

			SAFE_RELEASE (locksurf);
		}

		SAFE_RELEASE (bbsurf);
	}
}


void context_t::ScreenShot (const char *filename, D3DXIMAGE_FILEFORMAT format)
{
	IDirect3DSurface9 *bbsurf;

	if (SUCCEEDED (this->Device->GetRenderTarget (0, &bbsurf)))
	{
		QD3DXSaveSurfaceToFile (filename, format, bbsurf, NULL, NULL);
		SAFE_RELEASE (bbsurf);
	}
}


#endif

