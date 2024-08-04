#include "app.hpp"
#include "assets.hpp"
#include "game.hpp"

const float BUTTON_SZ = 320.0f;

namespace gui {
	void displayFPSCounter(unsigned int fps)
	{
		State* state = State::get();
		nk_context* ctx = state->getNkContext();
		ctx->style.text.color = nk_rgb(255, 255, 255);
		if(nk_begin(ctx, "fps", nk_rect(0, 0, 64, 20), NK_WINDOW_NO_SCROLLBAR)) {
            nk_layout_row_static(ctx, 20, 64, 1);
			char str[16];
			snprintf(str, 15, "FPS: %u", fps);
			nk_label(ctx, str, NK_TEXT_ALIGN_LEFT);
		}
		nk_end(ctx);
	}

	std::string displayPauseMenu()
	{
		std::string action = "";
		State* state = State::get();
		nk_context* ctx = state->getNkContext();
		int w, h;
		glfwGetWindowSize(state->getWindow(), &w, &h);
		nk_style* s = &ctx->style;
		s->text.color = nk_rgb(255, 255, 255);
		nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(32, 32, 32, 192)));
		nk_style_push_style_item(ctx, &s->button.normal, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
		nk_style_push_style_item(ctx, &s->button.hover, nk_style_item_color(nk_rgba(64, 64, 64, 255)));
		s->button.border = 0.0f;
		if(nk_begin(ctx, "pause", nk_rect(0, 0, w, h), NK_WINDOW_NO_SCROLLBAR)) {
			nk_layout_row_dynamic(ctx, h / 6, 1);	
			nk_spacing(ctx, 1);
			
			nk_layout_row_dynamic(ctx, 64.0f, 1);	
			FONTS->pushFont("armata_large");
			nk_label(ctx, "PAUSED", NK_TEXT_CENTERED);
			FONTS->popFont(); 

			nk_layout_row_begin(ctx, NK_STATIC, 16.0f, 1);
			nk_spacing(ctx, 1);
			nk_layout_row_end(ctx);

			float padding = float(w / 2) - BUTTON_SZ / 2.0f;
			nk_layout_row_begin(ctx, NK_STATIC, 64.0f, 3);
			FONTS->pushFont("armata_medium");
			nk_layout_row_push(ctx, padding);
			nk_spacing(ctx, 1);
			nk_layout_row_push(ctx, BUTTON_SZ);
			if(nk_button_label(ctx, "Resume"))
				action = "unpause";
			nk_layout_row_end(ctx);

			nk_layout_row_begin(ctx, NK_STATIC, 64.0f, 2);
			nk_layout_row_push(ctx, padding);
			nk_spacing(ctx, 1);
			nk_layout_row_push(ctx, BUTTON_SZ);
			if(nk_button_label(ctx, "Quit to main menu"))
				action = "quit";
			FONTS->popFont();
			nk_layout_row_end(ctx);
		}
		nk_end(ctx);
		nk_style_pop_style_item(ctx);
		nk_style_pop_style_item(ctx);
		nk_style_pop_style_item(ctx);
		return action;
	}

	void displayDeathScreen()
	{
		State* state = State::get();
		nk_context* ctx = state->getNkContext();
		int w, h;
		glfwGetWindowSize(state->getWindow(), &w, &h);
		nk_style* s = &ctx->style;
		s->text.color = nk_rgb(255, 255, 255);
		nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(255, 0, 0, 128)));
		nk_style_push_style_item(ctx, &s->button.normal, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
		nk_style_push_style_item(ctx, &s->button.hover, nk_style_item_color(nk_rgba(64, 64, 64, 255)));
		s->button.border = 0.0f;
		if(nk_begin(ctx, "deathscreen", nk_rect(0, 0, w, h), NK_WINDOW_NO_SCROLLBAR)) {
			nk_layout_row_dynamic(ctx, h / 6, 1);	
			nk_spacing(ctx, 1);
			
			nk_layout_row_dynamic(ctx, 64.0f, 1);	
			FONTS->pushFont("armata_large");
			nk_label(ctx, "YOU CRASHED", NK_TEXT_CENTERED);
			FONTS->popFont(); 

			nk_layout_row_begin(ctx, NK_STATIC, 16.0f, 1);
			nk_spacing(ctx, 1);
			nk_layout_row_end(ctx);


			FONTS->pushFont("armata_medium");	
			nk_layout_row_dynamic(ctx, 64.0f, 1);	
			nk_label(ctx, "press ESC to return to main menu", NK_TEXT_CENTERED);
			FONTS->popFont();
		}
		nk_end(ctx);
		nk_style_pop_style_item(ctx);
		nk_style_pop_style_item(ctx);
		nk_style_pop_style_item(ctx);
	}
}
