#include <raylib.h>
#include <vector>
#include <string>
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
				m_Context.hovered,
				m_Context.mouseLocal,
				m_Context.mouseButtons
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
		mouseButtons[0] = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
		mouseButtons[1] = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
		mouseButtons[2] = IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE);
}

Panel& PanelManager::AddPanel(const std::string& id,
		Rectangle rect,
		Panel::DrawFunc OnDraw,
		Panel::UpdateFunc OnUpdate
		)
{
		auto [it, inserted] = this->m_Panels.emplace(id, Panel(rect, OnDraw, OnUpdate));
		return it->second;
}

Panel& PanelManager::AddPanel(Rectangle rect,
		Panel::DrawFunc OnDraw,
		Panel::UpdateFunc OnUpdate
)
{
		// Auto-generate a unique ID for this panel.
		static int autoId = 0;
		std::string id = "auto_panel_" + std::to_string(autoId++);
		while (HasPanel(id)) {
				autoId = (autoId + 1) % 1000000; // wrap around to avoid overflow, though this is unlikely to happen
				std::string id = "auto_panel_" + std::to_string(autoId++);
		}
		return AddPanel(id, rect, OnDraw, OnUpdate);
}

Panel& PanelManager::AddPanel(const std::string& id, Panel&& panel)
{
		auto [it, inserted] = this->m_Panels.emplace(id, std::move(panel));
		return it->second;
}

bool PanelManager::RemovePanel(const std::string& id)
{
		if (m_Panels.contains(id)) {
				m_PanelsToRemove.push_back(id);
				return true;
		}
		return false;
}

void PanelManager::DrawAll()
{
		for (auto& [id, panel] : m_Panels) {
				panel.Draw();
		}
}

void PanelManager::UpdateAll()
{
		for (auto& [id, panel] : m_Panels) {
				panel.Update();
		}

		for (std::string& id : m_PanelsToRemove) {
				m_Panels.erase(id);
		}
}