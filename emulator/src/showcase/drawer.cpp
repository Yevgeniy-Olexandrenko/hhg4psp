#include "commons.h"
#include "drawer.h"

#define DRAW_BACKGROUND 1
#define DRAW_DISPLAY    1
#define DRAW_FOREGROUND 1
#define DRAW_FRAME      1
#define DRAW_BOUNDARIES 0

static const Layout::LayerType k_refLayerType = Layout::LayerType::BACKGROUND;

Drawer::Drawer(Game* game)
	: m_game(game)
	, m_display(NULL)
	, m_layoutId(0)
	, m_isFlipped(false)
{
}

void Drawer::Init(int layoutId, bool isFlipped)
{
	m_layoutId = layoutId;	
	Layout& layout = m_game->GetLayouts()[m_layoutId];
	layout.LoadAssets();

	m_isFlipped = isFlipped && layout.IsFlippingSupported();

	m_display = platform::video::CreateTexture(
		layout.GetDisplay().m_w,
		layout.GetDisplay().m_h,
		layout.GetAssetPixelFormat(Layout::AssetType::OPAQUE));
	platform::video::SetTextureBlending(m_display, platform::video::Blending::Multiply);
}

void Drawer::Draw()
{
	platform::video::RenderClear(0xFFFF00FF);

#if DRAW_BACKGROUND
	DrawLayer(Layout::LayerType::BACKGROUND, Layout::AssetType::BACKGROUND, m_isFlipped);
#endif

#if DRAW_DISPLAY
	DrawDisplay(m_isFlipped);
#endif

#if DRAW_FOREGROUND
	DrawLayer(Layout::LayerType::FOREGROUND, Layout::AssetType::FOREGROUND, m_isFlipped);
#endif

#if DRAW_FRAME
	DrawLayer(Layout::LayerType::FRAME, Layout::AssetType::FRAME, false);
#endif

#if DRAW_BOUNDARIES
	Layout& layout = m_game->GetLayouts()[m_layoutId];
	platform::video::Rect refRect = layout.GetLayer(k_refLayerType).GetRect().GetDstRect();
	for (int i = 0; i < Layout::LayerType::count; ++i)
	{
		Layout::LayerType layerType = Layout::LayerType(i);
		platform::video::Rect dstRect = (layerType == Layout::LayerType::SEGMENT)
			? layout.GetDisplay().GetDstRect()
			: layout.GetLayer(layerType).GetRect().GetDstRect();
		if (m_isFlipped && layerType != Layout::LayerType::FRAME)
		{
			float delta = (refRect.m_w - dstRect.m_w) - 2 * (dstRect.m_x - refRect.m_x);
			dstRect.m_x += delta;
		}
		platform::video::RenderRect(0xFF00FFFF, &dstRect);
	}
#endif

	platform::video::RenderPresent();
}

void Drawer::Free()
{
	Layout& layout = m_game->GetLayouts()[m_layoutId];
	layout.FreeAssets();

	platform::video::Free(m_display);
	m_display = NULL;
}

int Drawer::GetLayoutId() const
{
	return m_layoutId;
}

bool Drawer::IsFlipped() const
{
	return m_isFlipped;
}

void Drawer::DrawLayer(Layout::LayerType layerType, Layout::AssetType assetType, bool doFlip)
{
	Layout& layout = m_game->GetLayouts()[m_layoutId];
	Layout::Layer layer = layout.GetLayer(layerType);
	platform::video::Texture* assets = layout.GetAssetTexture(assetType);

	platform::video::Rect refRect = layout.GetLayer(k_refLayerType).GetRect().GetDstRect();
	platform::video::Rect dstRect = layer.GetRect().GetDstRect();

	int delta = (refRect.m_w - dstRect.m_w) - 2 * (dstRect.m_x - refRect.m_x);

	for (int i = 0; i < layer.GetTilesCount(); i++)
	{
		const Layout::Tile& tile = layer.GetTile(i);

		platform::video::Rect tileSrcRect = tile.GetSrcRect();
		platform::video::Rect tileDstRect = tile.GetDstRect();

		if (doFlip)
		{
			tileDstRect.m_x = 2 * dstRect.m_x - tileDstRect.m_x + dstRect.m_w - tileDstRect.m_w + delta;
		}
		platform::video::RenderCopy(assets, &tileSrcRect, &tileDstRect, doFlip);
	}
}

void Drawer::DrawDisplay(bool doFlip)
{
	const Layout& layout = m_game->GetLayouts()[m_layoutId];
	u32 bgColor = m_game->GetPCB()->IsCheatEnabled() ? 0xFFFFFF8C : layout.GetDisplayBgColor();

	platform::video::SetRenderTarget(m_display);
	platform::video::RenderClear(bgColor);

	DrawSegments(0.15f, 0.005f, doFlip);
	DrawSegments(1.00f, 0.000f, doFlip);

	platform::video::SetRenderTarget(NULL);
	platform::video::Rect refRect = layout.GetLayer(k_refLayerType).GetRect().GetDstRect();
	platform::video::Rect dstRect = layout.GetDisplay().GetDstRect();

	if (doFlip)
	{
		int delta = (refRect.m_w - dstRect.m_w) - 2 * (dstRect.m_x - refRect.m_x);
		dstRect.m_x += delta;
	}
	platform::video::RenderCopy(m_display, NULL, &dstRect);
}

void Drawer::DrawSegments(float opacity, float offset, bool doFlip)
{
	const Layout& layout = m_game->GetLayouts()[m_layoutId];
	platform::video::Texture* assets = layout.GetAssetTexture(Layout::AssetType::SEGMENT);

	int pxOffset = (int)(offset * sqrtf(square<float>(layout.GetDisplay().m_w) + square<float>(layout.GetDisplay().m_h)));
	for (int h = 0; h < k_mcuLcdHCount; ++h)
	{
		for (int o = 0; o < k_mcuLcdOCount; ++o)
		{
			for (int s = 0; s < k_mcuLcdSCount; ++s)
			{
				float level = m_game->GetPCB()->GetLCD()->GetSegmentLevel(h, o, s);
				platform::video::SetTextureAlpha(assets, platform::video::Alpha(0xFF * level * opacity));

				int segmentId = s | h << 2 | o << 4;
				const Layout::Segment& segment = layout.GetSegment(segmentId);

				platform::video::Rect srcRect = segment.GetSrcRect();
				platform::video::Rect dstRect = segment.GetDstRect(doFlip);
				dstRect.m_x += pxOffset;
				dstRect.m_y += pxOffset;

				bool doSegmentFlip = doFlip && segment.IsFlipAllowed();
				platform::video::RenderCopy(assets, &srcRect, &dstRect, doSegmentFlip);
			}
		}
	}
	platform::video::SetTextureAlpha(assets, platform::video::ALPHA_OPAQUE);
}
