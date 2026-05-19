#pragma once

#include <raylib.h>
#include <unordered_map>
#include <array>
#include <span>
#include <functional>


class Canvas
{
public:
		Vector2 origin{ 0,0 };

		Canvas() = default;
		explicit Canvas(Vector2 origin)
				: origin(origin)
		{

		}

		// Helper methods for converting a point to a local/screen point
		Vector2 ToScreen(Vector2 p) const { return Vector2{ p.x + origin.x, p.y + origin.y }; }
		Vector2 ToLocal(Vector2 p) const { return Vector2{ p.x - origin.x, p.y - origin.y }; }

		Vector2 MouseLocal() const { return ToLocal(GetMousePosition()); }


		// Drawing wrappers (add more as needed):
		void FillColor(Color color) const;
		void DrawCircle(Vector2 center, float radius, Color color) const;
		void DrawCircleLines(Vector2 center, float radius, Color color) const;
		void DrawRect(Rectangle rect, Color color) const;
		void DrawRectLines(Rectangle rect, int thick, Color color) const;
		void DrawTextLocal(const char* txt, Vector2 localPos, int fontSize, Color color) const;
};


class ScopedClip
{
		static inline bool s_isActive = false;
public:
		ScopedClip(const Rectangle& rect);
		~ScopedClip()
		{
				EndScissorMode();
				s_isActive = false;
		}

		// Disallow copying/moving so we don't run EndScissorMode consecutively - ScopedClip should always be temporary!
		ScopedClip(const ScopedClip& other) = delete;
		ScopedClip& operator=(const ScopedClip&) = delete;
		ScopedClip(ScopedClip&& other) = delete;
		ScopedClip& operator=(ScopedClip&&) = delete;
};


class Panel
{

// Helper structs
private:
		struct PanelContext
		{
				Canvas canvas;          // origin-aware drawing & coords
				Rectangle rect;         // panel rect
				bool hovered;           // maybe useful
				Vector2 mouseLocal;     // cached convenience
				std::array<bool, 3> mouseButtons{}; // left/middle/right button states

				void refresh(const Panel& panel);
		};

		PanelContext m_Context{};
public:
		struct UpdateContext
		{
				float dt;            // delta time for smooth movement
				bool hovered;        // is mouse over this panel?
				Vector2 mouseLocal;  // local mouse position
				std::array<bool, 3> mouseButtons; // left/middle/right button states
		};

		/// @brief Logic function called every frame. 
		/// @param ctx Contains deltaTime, mouse position, and hover state.
		using UpdateFunc = std::function<void(UpdateContext&)>;

		/// @brief Drawing function called every frame. 
		/// @param canvas A helper for drawing with the panel's origin as (0,0).
		using DrawFunc = std::function<void(Canvas&)>;

private:
		friend class PanelManager; // so PanelManager can call private Draw/Update
		DrawFunc OnDraw;
		UpdateFunc OnUpdate;

		void Draw();
		void Update();
public:
		Rectangle Rect;

		Panel(Rectangle rect,
				DrawFunc OnDraw,
				UpdateFunc OnUpdate = {})
				: Rect(rect), OnDraw(OnDraw), OnUpdate(OnUpdate)
		{
		}

		Panel(Panel&& other) noexcept = default;

		bool Contains(Vector2 screenPt) const
		{
				return CheckCollisionPointRec(screenPt, Rect);
		}

		Vector2 MouseLocal() const
		{
				return { GetMouseX() - Rect.x, GetMouseY() - Rect.y };
		}
};

class PanelManager
{
private:
		std::unordered_map<std::string, Panel> m_Panels;

public:

		PanelManager() = default;

		Panel& AddPanel(const std::string& id,
				Rectangle rect,
				Panel::DrawFunc OnDraw,
				Panel::UpdateFunc OnUpdate = {});

		Panel& AddPanel(Rectangle rect,
				Panel::DrawFunc OnDraw,
				Panel::UpdateFunc OnUpdate = {});

		Panel& AddPanel(const std::string& id, Panel&& panel);
		bool RemovePanel(const std::string& id) { return m_Panels.erase(id) > 0; }

		void DrawAll();
		void UpdateAll();

		auto& GetPanels() & { return m_Panels; }
		const auto& GetPanels() const& { return m_Panels; } // for const PanelManager
		const Panel& getPanel(const std::string& id) const { return m_Panels.at(id); }

		bool HasPanel(const std::string& id) const { return m_Panels.contains(id); }

		// forbid calling on rvalues - references will be invalid anyways
		decltype(m_Panels)& GetPanels() && = delete;
		const decltype(m_Panels)& GetPanels() const&& = delete;
};