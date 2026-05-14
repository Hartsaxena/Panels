#include <raylib.h>
#include <vector>
#include <stdexcept>

#include "Panel.hpp"

void Canvas::FillColor(Color color) const
{
		ClearBackground(color);
}

void Canvas::DrawCircle(Vector2 center, float radius, Color color) const
{
		Vector2 c = ToScreen(center);
		DrawCircleV(c, radius, color);
}

void Canvas::DrawCircleLines(Vector2 center, float radius, Color color) const
{
		Vector2 c = ToScreen(center);
		::DrawCircleLines(c.x, c.y, radius, color);
}

void Canvas::DrawRect(Rectangle rect, Color color) const
{
		Rectangle rr = { rect.x + origin.x, rect.y + origin.y, rect.width, rect.height };
		DrawRectangleRec(rr, color);
}

void Canvas::DrawRectLines(Rectangle rect, int thick, Color color) const
{
		Rectangle rr = { rect.x + origin.x, rect.y + origin.y, rect.width, rect.height };
		DrawRectangleLinesEx(rr, (float) thick, color);
}

void Canvas::DrawTextLocal(const char* txt, Vector2 localPos, int fontSize, Color color) const
{
		Vector2 sp = ToScreen(localPos);
		DrawText(txt, (int) sp.x, (int) sp.y, fontSize, color);
}


ScopedClip::ScopedClip(const Rectangle& rect)
{
		if (s_isActive) {
				throw std::runtime_error("Nested ScopedClip instances are not allowed!");
		}
		s_isActive = true;

		int x = static_cast<int>(rect.x);
		int y = static_cast<int>(rect.y);
		int width = static_cast<int>(rect.width);
		int height = static_cast<int>(rect.height);

		BeginScissorMode(x, y, width, height);
}

void Panel::Draw()
{
		ScopedClip clip(Rect);

		if (OnDraw) {
				OnDraw(m_Context.canvas);
		}
}

void Panel::Update()
{
		m_Context.refresh(*this);
		UpdateContext ctx {
				GetFrameTime(), // raylib's delta time
				m_Context.mouseLocal,
				m_Context.hovered
		};

		if (OnUpdate) {
				OnUpdate(ctx);
		}
}

void Panel::PanelContext::refresh(const Panel& panel)
{
		rect = panel.Rect;
		canvas.origin = { panel.Rect.x, panel.Rect.y };
		hovered = panel.Contains(GetMousePosition());
		mouseLocal = panel.MouseLocal();
}

Panel& PanelManager::AddPanel(Rectangle rect,
		Panel::DrawFunc OnDraw,
		Panel::UpdateFunc OnUpdate)
{
		this->m_Panels.emplace_back(rect, OnDraw, OnUpdate);
		return this->m_Panels.back();
}

Panel& PanelManager::AddPanel(Panel&& panel)
{
		this->m_Panels.push_back(std::move(panel));
		return this->m_Panels.back();
}

void PanelManager::DrawAll()
{
		for (auto& panel : m_Panels) {
				panel.Draw();
		}
}

void PanelManager::UpdateAll()
{
		for (auto& panel : m_Panels) {
				panel.Update();
		}
}