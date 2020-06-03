//#define PLATFORM_PSP
#ifdef PLATFORM_PSP

#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>

#include <pspge.h>
#include <pspgu.h>
#include <pspgum.h>

#include <pspiofilemgr.h>

//#include <psputils.h>
#include <vram.h>
#include <png.h>

#include <malloc.h>
#include <cstdarg>

#include "platform.h"
#include "constants.h"

#define PSP_PNGSIGSIZE   (8)

namespace platform
{
	namespace video
	{
		enum psp_texture_place 
		{
			PSP_PLACE_RAM,
			PSP_PLACE_VRAM,
			PSP_PLACE_UNKNOWN
		};

		struct psp_texture 
		{
			int    place;               // RAM or VRAM
			bool   swizzled;            // swizzled or not
			bool   has_alpha;
			int    pixel_format;        // Pixel Storage Format
			size_t width, height;       // Texture width and height (non base 2)
			size_t pow2_w, pow2_h;      // Texture width and height (base 2)
			size_t stride, data_size;
			void * data;
		};

		//	Grabbed from: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
		unsigned int next_pow2(unsigned int v)
		{
			v--; v |= v >> 1; v |= v >> 2; v |= v >> 4;	v |= v >> 8; v |= v >> 16;
			return v + 1;
		}

		void psp_flush_texture(psp_texture * texture)
		{
			sceKernelDcacheWritebackRange(texture->data, texture->data_size);
		}

		psp_texture * psp_create_texture(int w, int h, int place)
		{
			psp_texture * texture = (psp_texture *)malloc(sizeof(psp_texture));
			if (texture)
			{
				texture->width  = w;
				texture->height = h;
				texture->pow2_w = next_pow2(w);
				texture->pow2_h = next_pow2(h);

				texture->pixel_format = GU_PSM_8888;
				texture->swizzled = GU_FALSE;
				texture->has_alpha = GU_TRUE;

				texture->stride = texture->pow2_w * 4;
				texture->data_size = texture->stride * texture->pow2_h;

				//	If there's not enough space in the VRAM, then allocate it in the RAM
				if (place == PSP_PLACE_RAM || texture->data_size > vlargestblock()) 
				{
					texture->data = memalign(16, texture->data_size);
					texture->place = PSP_PLACE_RAM;
				} 
				else if (place == PSP_PLACE_VRAM)
				{
					texture->data = valloc(texture->data_size);
					texture->place = PSP_PLACE_VRAM;
				}
				memset(texture->data, 0, texture->data_size);
				psp_flush_texture(texture);
				return texture;
			}
			return NULL;
		}

		void psp_free_texture(psp_texture * texture)
		{
			if (texture->place == PSP_PLACE_RAM) 
			{
				free(texture->data);
			} 
			else if (texture->place == PSP_PLACE_VRAM)
			{
				vfree(texture->data);
			}
			free(texture);
		}

		void psp_read_png_file_fn(png_structp png_ptr, png_bytep data, png_size_t length)
		{
			SceUID fd = *(SceUID*) png_get_io_ptr(png_ptr);
			sceIoRead(fd, data, length);
		}

		psp_texture * psp_load_png_generic(void* io_ptr, png_rw_ptr read_data_fn, int place)
		{
			psp_texture * texture = NULL;
			png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
			if(png_ptr != NULL)
			{
				png_infop info_ptr = png_create_info_struct(png_ptr);
				if(info_ptr != NULL)
				{
					png_bytep* row_ptrs = NULL;
					if(!setjmp(png_jmpbuf(png_ptr)))
					{
						png_set_read_fn(png_ptr, (png_voidp)io_ptr, read_data_fn);
						png_set_sig_bytes(png_ptr, PSP_PNGSIGSIZE);
						png_read_info(png_ptr, info_ptr);

						unsigned int width, height;
						int bit_depth, color_type;

						png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
						&color_type, NULL, NULL, NULL);

						if (color_type == PNG_COLOR_TYPE_PALETTE && bit_depth <= 8) png_set_expand(png_ptr);
						if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand(png_ptr);
						if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_expand(png_ptr);
						if (bit_depth == 16) png_set_strip_16(png_ptr);

						//PSP: 16 or 32 bpp (not 0xRRGGBB)
						if (bit_depth == 8 && color_type == PNG_COLOR_TYPE_RGB)
						{
							png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
						}
						if (color_type == PNG_COLOR_TYPE_PALETTE) 
						{
							png_set_palette_to_rgb(png_ptr);
							png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
						}
						if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
						{
							png_set_expand_gray_1_2_4_to_8(png_ptr);
						}
						if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) 
						{
							png_set_tRNS_to_alpha(png_ptr);
						}

						if(bit_depth < 8) png_set_packing(png_ptr);
						png_read_update_info(png_ptr, info_ptr);

						texture = psp_create_texture(width, height, place);
						texture->has_alpha = true;

						row_ptrs = (png_bytep*)malloc(sizeof(png_bytep) * height);
						for (unsigned int i = 0; i < height; ++i) 
						{
							row_ptrs[i] = (png_bytep)(texture->data) + (i * texture->stride);
						}
						png_read_image(png_ptr, row_ptrs);
						png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)0);
						free(row_ptrs);

						psp_flush_texture(texture);
						return texture;
					}
					else
					{
						png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)0);
						if(row_ptrs != NULL) free(row_ptrs);
					}
				}
				else
				{
					png_destroy_read_struct(&png_ptr, (png_infopp)0, (png_infopp)0);
				}
			}
			return texture;
		}

		psp_texture* psp_load_png_file(const char* filename, int place)
		{
			psp_texture * texture = NULL;
			SceUID fd = sceIoOpen(filename, PSP_O_RDONLY, 0777);
			if (fd >= 0)
			{
				png_byte pngsig[PSP_PNGSIGSIZE];
				if (sceIoRead(fd, pngsig, PSP_PNGSIGSIZE) == PSP_PNGSIGSIZE && png_sig_cmp(pngsig, 0, PSP_PNGSIGSIZE) == 0)
				{
					texture = psp_load_png_generic((void*) &fd, psp_read_png_file_fn, place);
				}
				sceIoClose(fd);
			}
			return texture;
		}

		////////////////////////////////////////////////////////////////////////

		psp_texture * Convert(Surface * surface)
		{
			return surface ? reinterpret_cast<psp_texture*>(surface->m_hw_surface) : NULL;
		}

		int Surface::GetW() const
		{
			return Convert(const_cast<Surface*>(this))->width;
		}

		int Surface::GetH() const
		{
			return Convert(const_cast<Surface*>(this))->height;
		}

		Color * Surface::GetPixels() const
		{
			return reinterpret_cast<Color*>(Convert(const_cast<Surface*>(this))->data);
		}

		Surface * LoadSurface(const std::string & file)
		{
			if(psp_texture * texture = psp_load_png_file(file.c_str(), PSP_PLACE_RAM))
			{
				Surface * surface = new Surface();
				surface->m_hw_surface = texture;
				return surface;
			}
			return NULL;
		}

		Texture * CreateTexture(Surface * surface)
		{
			return NULL;
		}

		Texture * CreateTexture(int w, int h)
		{
			return NULL;
		}

		void Free(Surface * surface)
		{
			if (surface)
			{
				//
			}
		}

		void Free(Texture * texture)
		{
			if (texture)
			{
				//
			}
		}

		void SetRenderTarget(Texture * texture)
		{
			//
		}

		void SetTextureBlending(Texture * texture, Blending blending)
		{
			switch (blending.underlying())
			{
			case Blending::Normal:
				//
				break;

			case Blending::Multiply:
				//
				break;
			}
		}

		void SetTextureAlpha(Texture * texture, Alpha alpha)
		{
			//
		}

		void RenderClear(Color color)
		{
			//
		}

		void RenderCopy(Texture * texture, const Rect * srcRect, const Rect * dstRect, bool flipHorizontal)
		{
			//
		}

		void RenderPresent()
		{
			//
		}
	}

	namespace audio
	{
		void Resume()
		{
			//
		}

		void Pause()
		{
			//
		}
	}

	namespace input
	{
		u16 GetInputMask()
		{
			u16 mask = 0;
			
			//

			return mask;
		}

		bool IsQuitRequested()
		{
			//

			return false;
		}

		bool IsLayoutSwitchRequested()
		{
			//

			return false;
		}

		bool IsLayoutFlipRequested()
		{
			//

			return false;
		}
	}

	namespace other
	{
		float GetBatteryLevel()
		{
			return 1.0f;
		}
	}

	void Init(AudioSampleProducer * audioSampleProducer)
	{
		//
	}

	void Free()
	{
		//
	}

	void Dbg(const char * fmt, ...)
	{
		SceUID fd;
		if((fd = sceIoOpen("debug.txt", PSP_O_WRONLY | PSP_O_APPEND | PSP_O_CREAT, 0777)))
		{
			char buffer[256];
			va_list args;
			va_start(args, fmt);
			vsnprintf(buffer, 256, fmt, args);
			sceIoWrite(fd, buffer, strlen(buffer));
			va_end(args);
			sceIoClose(fd);
		}
	}
}

#endif
