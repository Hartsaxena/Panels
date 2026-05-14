#include <raylib.h>
#include <iostream>

#include "Panel.hpp"

int main()
{
		SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
		SetTargetFPS(60);
		InitWindow(1280, 800, "Hello Raylib");

		Texture Mommy = LoadTexture("resources/mommy.png");
		if (Mommy.id == 0) {
				std::cout << "Where did Mommy Ahri go?\n";
		}

		PanelManager panels;
		panels.AddPanel({ 50,50, 300,200 },
				[](Canvas& c) {
						c.FillColor(DARKGRAY);
						c.DrawRectLines({ 0,0,300,200 }, 2, RAYWHITE);
						c.DrawTextLocal("Hello, Panel!", { 12,12 }, 20, RAYWHITE);
				},
				[](Panel::UpdateContext context) {
						// handle input using c.MouseLocal() if needed
				});

		panels.AddPanel({ 400,50, 300,200 },
				[](Canvas& c) {
						c.FillColor(DARKGRAY);
						c.DrawRectLines({ 0,0,300,200 }, 2, RAYWHITE);
						c.DrawTextLocal("Shut up, Panel.", { 12,12 }, 20, RAYWHITE);
				});

		// game loop
		while (!WindowShouldClose())		// run the loop untill the user presses ESCAPE or presses the Close button on the window
		{
				BeginDrawing();

				ClearBackground(BLACK);
				DrawTexture(Mommy, 0, 0, WHITE);
				DrawText("Hello Raylib~~", 1100, 400, 20, WHITE);
				panels.UpdateAll();
				panels.DrawAll();

				EndDrawing();
		}

		// cleanup
		// unload our texture so it can be cleaned up
		UnloadTexture(Mommy);

		// destroy the window and cleanup the OpenGL context
		CloseWindow();
		return 0;
}
