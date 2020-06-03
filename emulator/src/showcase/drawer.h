#pragma once

#include "game.h"

class Drawer
{
public:
	Drawer(Game* game);

	void Init(int layoutId, bool isFlipped);
	void Draw();
	void Free();

	int  GetLayoutId() const;
	bool IsFlipped() const;

private:
	void DrawLayer(Layout::LayerType layerType, Layout::AssetType assetType, bool doFlip);
	void DrawDisplay(bool doFlip);
	void DrawSegments(float opacity, float offset, bool doFlip);

private:
	Game * m_game;
	platform::video::Texture * m_display;

	int  m_layoutId;
	bool m_isFlipped;
};