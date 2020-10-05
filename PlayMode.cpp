#include "PlayMode.hpp"


#include <iostream>
//constexpr const char * const kFontPath { "/System/Library/Fonts/SFCompactRounded.ttf" };


#include <random>
#include <hb.h>
#include <hb-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "DrawLines.hpp"

#include <fstream>
#include <filesystem>
#include "data_path.hpp"


//Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
//	return new Sound::Sample(data_path("dusty-floor.opus"));
//});

PlayMode::PlayMode() {
	FT_Library library;
	FT_Error error = FT_Init_FreeType(&library);
	if (error) {
		std::cout << "Init library failed." << std::endl;
	}
	std::string k_font_path_str = data_path("ReallyFree-ALwl7.ttf");
	const char* kFontPath = k_font_path_str.c_str();
	error = FT_New_Face(library, kFontPath, 0, &test_face);
	FT_New_Face(library, kFontPath, 0, &test_face2);

	if (error == FT_Err_Unknown_File_Format) {
		throw std::runtime_error("The font file could be opened and read, but it appears that its font format is unsupported!");
	} else if (error) {
		throw std::runtime_error("The font file could not be opened or read, or that it is broken!");
	}

	if (!test_face) {
		throw std::runtime_error("Wrong font!");
	}

	drawFont_p = new Sentence(test_face);
	drawFont_p->SetText("XXX", 5000, glm::u8vec4(0x0, 0x0, 0xff, 0xff), glm::vec2(0.0f, 0.0f));
	scene_sen = new Sentence(test_face);

	for(int i=0; i<5; i++) {
		option_sens.emplace_back(new Sentence(test_face2));
	}

	load_text_scenes();
	curr_choice = 0;
	curr_scene = 1;
}

PlayMode::~PlayMode() {
	
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	/*if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		}
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			);
			return true;
		}
	}*/
	// Reference https://github.com/Dmcdominic/15-466-f20-game4/blob/menu-mode/MenuMode.cpp
	if (!text_scenes[curr_scene].choice_descriptions.empty()) {
		if (evt.type == SDL_KEYDOWN) {
			if (evt.key.keysym.sym == SDLK_DOWN) {
				curr_choice += 1;
				curr_choice %= text_scenes[curr_scene].choice_descriptions.size();
				return true;
			}
			else if (evt.key.keysym.sym == SDLK_UP) {
				curr_choice -= 1;
				if (curr_choice < 0)
					curr_choice += (int)text_scenes[curr_scene].choice_descriptions.size();
				return true;
			}
			else if (evt.key.keysym.sym == SDLK_SPACE) {
				curr_scene = text_scenes[curr_scene].next_scene[curr_choice];
				curr_choice = 0;
			}
		}
	}
	
	return false;
}

void PlayMode::update(float elapsed) {
//	if(total_elapsed - int(total_elapsed) >= 0.5) {
//		overlay.AddText("test", "YEAH", {-1.0f, 0.0f}, test_face, 10000, glm::u8vec4(0xff, 0x0, 0x0, 0xff));
//	} else {
//		overlay.AddText("test", "abcdefghijklmnopqrstuvwxyz", {-1.0f, 0.0f}, test_face, 6000, glm::u8vec4(0x0, 0x0, 0xff, 0xff));
//	}
//	total_elapsed += elapsed;
//	glm::u8vec4 color = glm::u8vec4(0x00, 0x00, 0x00, 0x00);

}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.


	/*scene.draw(*camera);*/
	// Reference: https://github.com/Dmcdominic/15-466-f20-game4/blob/menu-mode/MenuMode.cpp
	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);

		scene_sen->ClearText();
		scene_sen->SetText(&(text_scenes[curr_scene].description[0]), 5000,
		                   glm::u8vec4(0x0, 0x0, 0xff, 0xff), glm::vec2(0.0f, 0.0f));

		for (auto s: option_sens) {
			s->ClearText();
		}

		float y_offset = -0.2f;
		for (int i = 0; i < text_scenes[curr_scene].choice_descriptions.size() && i < option_sens.size(); i++) {
			bool is_selected = i == curr_choice;
			glm::u8vec4 color = (is_selected ? glm::u8vec4(0x00, 0xff, 0xff, 0xff) :
			                     glm::u8vec4(0xff, 0x00, 0x00, 0xff));

			std::cout<<text_scenes[curr_scene].choice_descriptions[i]<<std::endl;
			option_sens[i]->SetText(&(text_scenes[curr_scene].choice_descriptions[i][0]),
			                        4000, color, glm::vec2(0.0f, y_offset));
			y_offset -= 0.1f;
		}

		scene_sen->Draw(drawable_size);

		for(auto s: option_sens) {
			s->Draw(drawable_size);
		}
	}
}



void PlayMode::load_text_scenes() {
	// From https://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
	std::string path = data_path("texts");
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		std::cout << entry.path() << std::endl;
		std::ifstream f (entry.path());
		std::string line;
		if (!f.is_open()) {
			std::cout << "Unable to open file " << entry.path() << std::endl;
			continue;
		}
		if (!std::getline(f, line)) {
			std::cout << entry.path() << " is an empty file! Skipped." << std::endl;
			continue;
		}
		int id = std::stoi(line);
		std::string description = "";

		while (std::getline(f, line)) {
			if (line.rfind("##", 0) == 0)
				break;
			description.append(line.append("\n"));
		}
		description = description.substr(0, description.size() - 1);
		TextScene ts = { id, description };
		while (line.rfind("##", 0) == 0) {
			ts.next_scene.push_back(std::stoi(line.substr(2)));
			std::string choice_des = "";
			while (std::getline(f, line)) {
				if (line.rfind("##", 0) == 0)
					break;
				choice_des.append(line.append("\n"));
			}
			choice_des = choice_des.substr(0, choice_des.size() - 1);

			ts.choice_descriptions.push_back(choice_des);
		}
		text_scenes[id] = ts;
		f.close();
	}
	std::map<int, TextScene>::iterator it;
	for (it = text_scenes.begin(); it != text_scenes.end(); it++) {
//		std::cout << "Current Scene: " << it->first << std::endl << "Description: " << it->second.description << std::endl;
		for (int i = 0; i < it->second.next_scene.size(); i++) {
//			std::cout << "Choice: " << it->second.next_scene[i] << std::endl;
//			std::cout << "Description: " << it->second.choice_descriptions[i] << std::endl;
		}
	}
}

//void PlayMode::clear_text() {
//	if(scene_sen) {
//		scene_sen-;
//	}
//
//	for(Sentence* s_p: option_sens) {
//		delete s_p;
//	}
//	option_sens.clear();
//}
