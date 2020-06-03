#ifdef PLATFORM_SDL2

#include <SDL.h>
#include <SDL_image.h>

#include "platform.h"
#include "constants.h"

namespace platform
{
	static int m_windowScale = 1;
	static int m_renderScale = m_windowScale;

	static SDL_Window * m_window = NULL;
	static SDL_Renderer * m_renderer = NULL;

	static SDL_Rect m_src;
	static SDL_Rect m_dst;

	namespace video
	{
		SDL_Texture * Convert(Texture * texture)
		{
			return texture ? reinterpret_cast<SDL_Texture*>(texture->m_hw_texture) : NULL;
		}

		SDL_PixelFormatEnum Convert(PixelFormat format)
		{
			switch (format.underlying())
			{
			case PixelFormat::RGB888:   return SDL_PixelFormatEnum::SDL_PIXELFORMAT_RGB888;
			case PixelFormat::ARGB8888: return SDL_PixelFormatEnum::SDL_PIXELFORMAT_ARGB8888;
			case PixelFormat::RGB565:   return SDL_PixelFormatEnum::SDL_PIXELFORMAT_RGB565;
			case PixelFormat::ARGB4444: return SDL_PixelFormatEnum::SDL_PIXELFORMAT_ARGB4444;
			}
			return SDL_PixelFormatEnum::SDL_PIXELFORMAT_UNKNOWN;
		}

		Texture * LoadTexture(const u08* data, size_t size, PixelFormat format)
		{
			SDL_PixelFormatEnum pixel_format = Convert(format);

			SDL_Surface* original = IMG_LoadPNG_RW(SDL_RWFromConstMem(data, size));
			SDL_Surface* processed = SDL_ConvertSurfaceFormat(original, pixel_format, 0);
			SDL_FreeSurface(original);

			Texture* texture = new Texture();
			texture->m_hw_texture = SDL_CreateTextureFromSurface(m_renderer, processed);
			SDL_FreeSurface(processed);
			
		#if 1
			int w, h;
			Uint32 test_pixel_format;
			SDL_QueryTexture(Convert(texture), &test_pixel_format, NULL, &w, &h);
			SDL_PixelFormat* test_texture_format = SDL_AllocFormat(test_pixel_format);
			SDL_FreeFormat(test_texture_format);
		#endif

			return texture;
		}

		Texture * CreateTexture(int w, int h, PixelFormat format)
		{
			SDL_PixelFormatEnum pixel_format = Convert(format);

			Texture* texture = new Texture();
			texture->m_hw_texture = SDL_CreateTexture(m_renderer, pixel_format, SDL_TEXTUREACCESS_TARGET, w, h);
			return texture;
		}

		void Free(Texture * texture)
		{
			if (texture)
			{
				SDL_DestroyTexture(Convert(texture));
				delete texture;
			}
		}

		void SetRenderTarget(Texture * texture)
		{
			m_renderScale = texture ? 1 : m_windowScale;
			SDL_SetRenderTarget(m_renderer, Convert(texture));
		}

		void SetTextureBlending(Texture * texture, Blending blending)
		{
			switch (blending.underlying())
			{
			case Blending::Normal:
				SDL_SetTextureBlendMode(Convert(texture), SDL_BlendMode::SDL_BLENDMODE_NONE);
				break;

			case Blending::Multiply:
				SDL_SetTextureBlendMode(Convert(texture), SDL_BlendMode::SDL_BLENDMODE_MOD);
				break;
			}
		}

		void SetTextureAlpha(Texture * texture, Alpha alpha)
		{
			SDL_SetTextureAlphaMod(Convert(texture), Uint8(alpha));
		}

		void RenderClear(Color color)
		{
			Uint8 a(color >> 24), r(color >> 16), g(color >> 8), b(color);
			SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
			SDL_RenderClear(m_renderer);
		}

		void RenderCopy(Texture * texture, const Rect * srcRect, const Rect * dstRect, bool flipHorizontal)
		{
			SDL_Rect* src = NULL;
			SDL_Rect* dst = NULL;

			if (srcRect)
			{
				src = &m_src;
				src->x = srcRect->m_x;
				src->y = srcRect->m_y;
				src->w = srcRect->m_w;
				src->h = srcRect->m_h;
			}

			if (dstRect)
			{
				dst = &m_dst;
				dst->x = dstRect->m_x * m_renderScale;
				dst->y = dstRect->m_y * m_renderScale;
				dst->w = dstRect->m_w * m_renderScale;
				dst->h = dstRect->m_h * m_renderScale;
			}

			SDL_RenderCopyEx(m_renderer, Convert(texture), src, dst, 0, NULL, flipHorizontal ? SDL_RendererFlip::SDL_FLIP_HORIZONTAL : SDL_RendererFlip::SDL_FLIP_NONE);
		}

		void RenderRect(Color color, const Rect* rect)
		{
			Uint8 a(color >> 24), r(color >> 16), g(color >> 8), b(color);
			SDL_SetRenderDrawColor(m_renderer, r, g, b, a);

			SDL_Rect* dst = &m_dst;
			dst->x = rect->m_x * m_renderScale;
			dst->y = rect->m_y * m_renderScale;
			dst->w = rect->m_w * m_renderScale;
			dst->h = rect->m_h * m_renderScale;
			SDL_RenderDrawRect(m_renderer, dst);
		}

		void RenderPresent()
		{
			SDL_RenderPresent(m_renderer);

			//static Uint32 oldFrameTick = 0;
			//Uint32 newFrameTick = SDL_GetTicks();
			//Dbg("%d\t", newFrameTick - oldFrameTick);
			//oldFrameTick = newFrameTick;
		}
	}

	namespace audio
	{
		void AdioCallback(void* userData, Uint8* stream, int len)
		{
			AudioSampleProducer * producer = reinterpret_cast<AudioSampleProducer *>(userData);
			while (len > 0)
			{
				float sample = 0; int count = 0;
				producer->ProduceAudioSample(sample, count);

				for (; count > 0 && len > 0; --count, --len, ++stream)
				{
					*(stream) = (Uint8)(0x7F * 0.5f * sample);
				}
			}
		}

		void Resume()
		{
			SDL_PauseAudio(0);
		}

		void Pause()
		{
			SDL_PauseAudio(1);
		}
	}

	namespace input
	{	
		uint16_t GetInputMask()
		{
			uint16_t mask = 0;
			const Uint8* state = SDL_GetKeyboardState(NULL);

			if (state[SDL_SCANCODE_1]) mask |= MASK_GAME_A;
			if (state[SDL_SCANCODE_2]) mask |= MASK_GAME_B;
			if (state[SDL_SCANCODE_3]) mask |= MASK_TIME;
			if (state[SDL_SCANCODE_4]) mask |= MASK_ALARM;
			if (state[SDL_SCANCODE_5]) mask |= MASK_ACL;
			if (state[SDL_SCANCODE_0]) mask |= MASK_CHEAT;

			if (state[SDL_SCANCODE_Q]) mask |= MASK_LEFT_UP;
			if (state[SDL_SCANCODE_A]) mask |= MASK_LEFT_DOWN;
			if (state[SDL_SCANCODE_P]) mask |= MASK_RIGHT_UP;
			if (state[SDL_SCANCODE_L]) mask |= MASK_RIGHT_DOWN;
			
			if (state[SDL_SCANCODE_UP   ]) mask |= MASK_UP;
			if (state[SDL_SCANCODE_DOWN ]) mask |= MASK_DOWN;
			if (state[SDL_SCANCODE_LEFT ]) mask |= MASK_LEFT;
			if (state[SDL_SCANCODE_RIGHT]) mask |= MASK_RIGHT;
			
			if (state[SDL_SCANCODE_SPACE]) mask |= MASK_ACTION;
			return mask;
		}

		bool IsQuitRequested()
		{
			SDL_Event event;
			SDL_PollEvent(&event);

			int newScale = m_windowScale;
			if (event.key.keysym.sym == SDLK_F1) newScale = 1;
			if (event.key.keysym.sym == SDLK_F2) newScale = 2;
			if (event.key.keysym.sym == SDLK_F3) newScale = 3;
			if (newScale != m_windowScale)
			{
				m_windowScale = newScale;
				SDL_SetWindowSize(m_window, SCREEN_W * m_windowScale, SCREEN_H * m_windowScale);
				SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
			}

			if (event.type == SDL_QUIT) return true;
			if (event.key.keysym.sym == SDLK_ESCAPE) return true;

			return false;
		}

		bool IsLayoutSwitchRequested()
		{
			static bool isAlreadyActive = false;
			const Uint8* state = SDL_GetKeyboardState(NULL);

			if (state[SDL_SCANCODE_RETURN])
			{
				if (!isAlreadyActive)
				{
					return (isAlreadyActive = true);
				}
			}
			else
			{
				isAlreadyActive = false;
			}
			return false;
		}

		bool IsLayoutFlipRequested()
		{
			static bool isAlreadyActive = false;
			const Uint8* state = SDL_GetKeyboardState(NULL);

			if (state[SDL_SCANCODE_BACKSPACE])
			{
				if (!isAlreadyActive)
				{
					return (isAlreadyActive = true);
				}
			}
			else
			{
				isAlreadyActive = false;
			}
			return false;
		}
	}

	namespace other
	{
		float GetBatteryLevel()
		{
			return 1.0f;
		}

		u32 GetTime()
		{
			return SDL_GetTicks();
		}
	}

	void Init(AudioSampleProducer * audioSampleProducer)
	{
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
		{
			Dbg("SDL_Init error: %s\n", SDL_GetError());
		}

		if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG)
		{
			Dbg("IMG_Init error: %s\n", SDL_GetError());
			SDL_Quit();
		}

		
		m_window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_W * m_windowScale, SCREEN_H * m_windowScale, SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);
		m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);

		SDL_AudioSpec spec;
		spec.freq = k_audioFrequencyHz;
		spec.format = AUDIO_S8;
		spec.channels = 1;
		spec.samples  = k_audioSamplesNumb;
		spec.callback = audio::AdioCallback;
		spec.userdata = audioSampleProducer;

		if (SDL_OpenAudio(&spec, NULL) < 0)
		{
			Dbg("SDL_OpenAudio error: %s\n", SDL_GetError());
			SDL_Quit();
		}
	}

	static Uint32 m_profileBeginTick;

	void Free()
	{
		SDL_CloseAudio();
		SDL_DestroyRenderer(m_renderer);
		SDL_DestroyWindow(m_window);
		SDL_Quit();
	}

	void Dbg(const char * fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}

	void ProfileBegin()
	{
		m_profileBeginTick = SDL_GetTicks();
	}

	void ProfileEnd()
	{
		Uint32 elapsedTicks = SDL_GetTicks() - m_profileBeginTick;
		Dbg("%d\t", elapsedTicks);
	}
}

#endif