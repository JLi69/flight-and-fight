#include "app.hpp"
#include "assets.hpp"
#include "game.hpp"
#include <fstream>

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

	void displayHUD(unsigned int score, float speed) 
	{
		State* state = State::get();
		int w, h;
		glfwGetWindowSize(state->getWindow(), &w, &h);

		nk_context* ctx = state->getNkContext();
		nk_style* s = &ctx->style;
		ctx->style.text.color = nk_rgb(255, 255, 0);
		nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
		if(nk_begin(ctx, "hud", nk_rect(w - 256, h - 80, 256, 96), NK_WINDOW_NO_SCROLLBAR)) {
			FONTS->pushFont("armata_medium");
            nk_layout_row_static(ctx, 32, 240, 1);
			char str[32];
			snprintf(str, 31, "SPEED: %.2f", speed);
			nk_label(ctx, str, NK_TEXT_ALIGN_RIGHT);
			snprintf(str, 31, "SCORE: %05u", score);
			nk_label(ctx, str, NK_TEXT_ALIGN_RIGHT);	
			FONTS->popFont();
		}
		nk_end(ctx);
		nk_style_pop_style_item(ctx);
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
		s->button.text_hover = nk_rgb(255, 255, 255);
		s->button.text_normal = nk_rgb(255, 255, 255);
		s->button.text_active = nk_rgb(255, 255, 255);
		nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(32, 32, 32, 192)));
		nk_style_push_style_item(ctx, &s->button.normal, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
		nk_style_push_style_item(ctx, &s->button.hover, nk_style_item_color(nk_rgba(16, 16, 16, 255)));
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

	void displayDeathScreen(unsigned int finalscore)
	{
		State* state = State::get();
		nk_context* ctx = state->getNkContext();
		int w, h;
		glfwGetWindowSize(state->getWindow(), &w, &h);
		nk_style* s = &ctx->style;
		s->text.color = nk_rgb(255, 255, 255);
		s->button.text_hover = nk_rgb(255, 255, 255);
		s->button.text_normal = nk_rgb(255, 255, 255);
		s->button.text_active = nk_rgb(255, 255, 255);
		nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(255, 0, 0, 128)));
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

			char str[32];
			snprintf(str, 31, "Final Score: %u", finalscore);

			FONTS->pushFont("armata_medium");	
			nk_layout_row_dynamic(ctx, 64.0f, 1);
			if(finalscore > 0)
				nk_label(ctx, str, NK_TEXT_CENTERED);
			nk_label(ctx, "press ESC to return to main menu", NK_TEXT_CENTERED);
			FONTS->popFont();
		}
		nk_end(ctx);
		nk_style_pop_style_item(ctx);
	}

	game::GameMode displayMainMenu()
	{
		game::GameMode selected = game::NONE_SELECTED;

		State* state = State::get();
		nk_context* ctx = state->getNkContext();
	
		int w, h;
		glfwGetWindowSize(state->getWindow(), &w, &h);
		nk_style* s = &ctx->style;	
		s->text.color = nk_rgb(80, 160, 235);
		s->button.border = 0.0f;
		s->button.text_hover = nk_rgb(255, 255, 255);
		s->button.text_normal = nk_rgb(255, 255, 255);
		s->button.text_active = nk_rgb(255, 255, 255);
		nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
		nk_style_push_style_item(ctx, &s->button.hover, nk_style_item_color(nk_rgba(80, 160, 255, 255)));
		nk_style_push_style_item(ctx, &s->button.normal, nk_style_item_color(nk_rgba(100, 180, 255, 255)));	
		nk_style_push_style_item(ctx, &s->button.active, nk_style_item_color(nk_rgba(80, 160, 255, 255)));
		if(nk_begin(ctx, "mainmenu", nk_rect(0, 0, w, h), NK_WINDOW_NO_SCROLLBAR)) {	
			nk_layout_row_begin(ctx, NK_STATIC, 32.0f, 1);
			nk_spacing(ctx, 1);
			nk_layout_row_end(ctx);

			float padding = 48.0f;	
			FONTS->pushFont("armata_large");
			nk_layout_row_begin(ctx, NK_STATIC, 64.0f, 2);
			nk_layout_row_push(ctx, padding);
			nk_spacing(ctx, 1);
			nk_layout_row_push(ctx, w - padding);
			nk_label(ctx, "Flight & Fight", NK_TEXT_LEFT);
			nk_layout_row_end(ctx);
			FONTS->popFont();

			nk_layout_row_dynamic(ctx, 16.0f, 1);
			nk_spacing(ctx, 1);

			FONTS->pushFont("armata_medium");
			nk_layout_row_begin(ctx, NK_STATIC, 64.0f, 2);
		
			nk_layout_row_push(ctx, padding);
			nk_spacing(ctx, 1);
			nk_layout_row_push(ctx, BUTTON_SZ);
			if(nk_button_label(ctx, "Fight Mode"))
				selected = game::FIGHT;

			nk_layout_row_push(ctx, padding);
			nk_spacing(ctx, 1);
			nk_layout_row_push(ctx, BUTTON_SZ);
			if(nk_button_label(ctx, "Casual Mode"))
				selected = game::CASUAL;

			nk_layout_row_push(ctx, padding);
			nk_spacing(ctx, 1);
			nk_layout_row_push(ctx, BUTTON_SZ);
			if(nk_button_label(ctx, "Credits"))
				selected = game::CREDITS;

			nk_layout_row_push(ctx, padding);
			nk_spacing(ctx, 1);
			nk_layout_row_push(ctx, BUTTON_SZ);
			if(nk_button_label(ctx, "High Scores"))
				selected = game::HIGH_SCORE_SCREEN;
			
			nk_layout_row_push(ctx, padding);
			nk_spacing(ctx, 1);
			nk_layout_row_push(ctx, BUTTON_SZ);
			if(nk_button_label(ctx, "Quit"))
				exit(0);
			
			nk_layout_row_end(ctx);
			
			FONTS->popFont();
		}
		nk_end(ctx);
		nk_style_pop_style_item(ctx);
		nk_style_pop_style_item(ctx);
		nk_style_pop_style_item(ctx);
		nk_style_pop_style_item(ctx);
		
		return selected;
	}

	bool displayCredits(const std::vector<std::string> &credits)
	{
		State* state = State::get();
		nk_context* ctx = state->getNkContext();
		bool close = false;
	
		int w, h;
		glfwGetWindowSize(state->getWindow(), &w, &h);
		nk_style* s = &ctx->style;	
		s->text.color = nk_rgb(255, 255, 255);
		s->button.border = 0.0f;
		s->button.text_hover = nk_rgb(255, 255, 255);
		s->button.text_normal = nk_rgb(255, 255, 255);
		s->button.text_active = nk_rgb(255, 255, 255);

		nk_style_push_style_item(ctx, &s->button.hover, nk_style_item_color(nk_rgba(64, 64, 64, 255)));
		nk_style_push_style_item(ctx, &s->button.normal, nk_style_item_color(nk_rgba(16, 16, 16, 255)));	
		nk_window_set_focus(ctx, "credits");
		if(nk_begin(ctx, "credits", nk_rect(32, 32, 700, 480), 
			NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
			NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
			FONTS->pushFont("armata_small");
			nk_layout_row_dynamic(ctx, 16.0f, 1);

			for(const auto &line : credits)
				nk_label(ctx, line.c_str(), NK_TEXT_LEFT);

			nk_layout_row_begin(ctx, NK_STATIC, 32.0f, 1);
			nk_layout_row_push(ctx, 120);
			if(nk_button_label(ctx, "Close"))
				close = true;
			nk_layout_row_end(ctx);
			FONTS->popFont();
		}
		nk_style_pop_style_item(ctx);
		nk_style_pop_style_item(ctx);
		nk_end(ctx);

		return close;
	}

	bool displayHighScores(const HighScoreTable &highscores)
	{
		bool close = false;
		State* state = State::get();
		nk_context* ctx = state->getNkContext();
		int w, h;
		glfwGetWindowSize(state->getWindow(), &w, &h);
		nk_style* s = &ctx->style;
		s->text.color = nk_rgb(80, 160, 235);
		s->button.text_hover = nk_rgb(255, 255, 255);
		s->button.text_normal = nk_rgb(255, 255, 255);
		s->button.text_active = nk_rgb(255, 255, 255);
		nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
		nk_style_push_style_item(ctx, &s->button.hover, nk_style_item_color(nk_rgba(80, 160, 255, 255)));
		nk_style_push_style_item(ctx, &s->button.normal, nk_style_item_color(nk_rgba(100, 180, 255, 255)));	
		nk_style_push_style_item(ctx, &s->button.active, nk_style_item_color(nk_rgba(80, 160, 255, 255)));
		s->button.border = 0.0f;
		if(nk_begin(ctx, "deathscreen", nk_rect(0, 0, w, h), NK_WINDOW_NO_SCROLLBAR)) {
			nk_layout_row_dynamic(ctx, h / 32, 1);	
			nk_spacing(ctx, 1);
			
			nk_layout_row_dynamic(ctx, 64.0f, 1);	
			FONTS->pushFont("armata_large");
			nk_label(ctx, "High Scores", NK_TEXT_CENTERED);
			FONTS->popFont(); 

			s->text.color = nk_rgb(60, 120, 235);
			nk_layout_row_dynamic(ctx, 32.0f, 1);
			FONTS->pushFont("armata_medium");
			nk_spacing(ctx, 1);
			char str[32];
			int index = 1;
			for(auto &score : highscores) {
				snprintf(str, 32, "%2d: %05u", index, score);
				nk_label(ctx, str, NK_TEXT_CENTERED);
				index++;
			}

			nk_layout_row_dynamic(ctx, 4.0f, 1);
			nk_spacing(ctx, 1);

			nk_layout_row_begin(ctx, NK_STATIC, 64.0f, 2);
			nk_layout_row_push(ctx, w / 2 - BUTTON_SZ / 2);
			nk_spacing(ctx, 1);
			nk_layout_row_push(ctx, BUTTON_SZ);
			if(nk_button_label(ctx, "Main Menu"))
				close = true;
			nk_layout_row_end(ctx);

			FONTS->popFont();
		}
		nk_end(ctx);
		nk_style_pop_style_item(ctx);	
		nk_style_pop_style_item(ctx);
		nk_style_pop_style_item(ctx);
		nk_style_pop_style_item(ctx);

		return close;
	}

	std::vector<std::string> readTextFile(const char *path)
	{
		std::ifstream file(path);

		if(!file.is_open()) {
			fprintf(stderr, "failed to open: %s\n", path);
			return {};
		}

		std::vector<std::string> lines;
		std::string line;
		while(std::getline(file, line))
			lines.push_back(line);

		file.close();

		return lines;
	}
}
