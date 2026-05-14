#pragma once

#include <raylib.h>
#include <vector>
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
				Canvas canvas;         // origin-aware drawing & coords
				Rectangle rect;        // panel rect
				bool hovered;          // maybe useful
				Vector2 mouseLocal;    // cached convenience

				void refresh(const Panel& panel);
		};

		PanelContext m_Context{};
public:
		struct UpdateContext
		{
				float dt;            // delta time for smooth movement
				Vector2 mouseLocal;  // local mouse position
				bool hovered;        // is mouse over this panel?
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
public:

		PanelManager() = default;

		Panel& AddPanel(Rectangle rect,
				Panel::DrawFunc OnDraw,
				Panel::UpdateFunc OnUpdate = {});

		Panel& AddPanel(Panel&& panel);

		void DrawAll();
		void UpdateAll();

		// NOTE: std::span instances are invalid the moment m_Panels is changed in any way
		const std::span<Panel> GetPanels()& { return m_Panels; }
		const std::span<const Panel> GetPanels() const& { return m_Panels; } // for const PanelManager

		// forbid calling on rvalues - std::span will be invalid anyways
		const std::span<Panel> GetPanels() && = delete;
		const std::span<const Panel> GetPanels() const&& = delete;

private:
		std::vector<Panel> m_Panels; // ownership
};