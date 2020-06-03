#pragma once

#include "commons.h"

namespace platform
{
	const int SCREEN_W = 480;
	const int SCREEN_H = 272;

	class AudioSampleProducer
	{
	public:
		virtual void ProduceAudioSample(float & sample, int & count) = 0;
	};

	void Init(AudioSampleProducer * audioSampleProducer);
	void Free();
	void Dbg(const char * fmt, ...);
	void ProfileBegin();
	void ProfileEnd();

	namespace video
	{
		typedef u32 Color;
		typedef u08 Alpha;

		const Alpha ALPHA_TRANSP = 0x00;
		const Alpha ALPHA_OPAQUE = 0xFF;

		simple_safe_enum(Blending, Normal, Multiply);
		simple_safe_enum(PixelFormat, ARGB8888, RGB888, ARGB4444, RGB565);

		struct Rect
		{
			int m_x, m_y, m_w, m_h; 
		};

		struct Texture
		{
			Texture() : m_hw_texture(NULL) {}

			void * m_hw_texture;
		};

		Texture* LoadTexture(const u08* data, size_t size, PixelFormat format);
		Texture* CreateTexture(int w, int h, PixelFormat format);
		void Free(Texture* texture);

		void SetRenderTarget(Texture* texture);
		void SetTextureBlending(Texture* texture, Blending blending);
		void SetTextureAlpha(Texture* texture, Alpha alpha);

		void RenderClear(Color color = 0);
		void RenderCopy(Texture* texture, const Rect* srcRect, const Rect* dstRect, bool flipHorizontal = false);
		void RenderRect(Color color, const Rect* rect);
		void RenderPresent();
	}

	namespace audio
	{
		void Resume();
		void Pause();
	}

	namespace input
	{
		enum
		{
			MASK_UP         = 1 << 0,
			MASK_DOWN       = 1 << 1,
			MASK_LEFT       = 1 << 2,
			MASK_RIGHT      = 1 << 3,
			MASK_LEFT_UP    = 1 << 4,
			MASK_LEFT_DOWN  = 1 << 5,
			MASK_RIGHT_UP   = 1 << 6,
			MASK_RIGHT_DOWN = 1 << 7,
			MASK_ACTION     = 1 << 8,
			MASK_GAME_A     = 1 << 9,
			MASK_GAME_B     = 1 << 10,
			MASK_TIME       = 1 << 11,
			MASK_ALARM      = 1 << 12,
			MASK_ACL        = 1 << 13,
			MASK_CHEAT      = 1 << 14,
			MASK_TEST       = 1 << 15
		};

		u16 GetInputMask();

		bool IsQuitRequested();
		bool IsLayoutSwitchRequested();
		bool IsLayoutFlipRequested();
	}

	namespace other
	{
		float GetBatteryLevel();
		u32   GetTime();
	}
}