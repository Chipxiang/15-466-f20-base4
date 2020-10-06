#include "PlayMode.hpp"


#include <iostream>
//constexpr const char * const kFontPath { "/System/Library/Fonts/SFCompactRounded.ttf" };


#include <random>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <fstream>
#include <filesystem>
#include "data_path.hpp"
#include "Sound.hpp"


//Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
//	return new Sound::Sample(data_path("dusty-floor.opus"));
//});

// From https://github.com/wrystal/CrazyDriver
Load< std::map<std::string, Sound::Sample>> sound_samples(LoadTagDefault, []() -> std::map<std::string, Sound::Sample> const* {
	auto map_p = new std::map<std::string, Sound::Sample>();
	std::string path = data_path("musics");
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		std::string path_string = entry.path().filename().string();
		size_t start = 0;
		size_t end = path_string.find(".opus");
		std::cout << path_string.substr(start, end) << std::endl;
		map_p->emplace(path_string.substr(start, end), Sound::Sample(entry.path().string()));
	}
	return map_p;
	});

Load< Sound::Sample > load_typing_effect(LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample(data_path("musics/typing.opus"));
	});

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
	}
	else if (error) {
		throw std::runtime_error("The font file could not be opened or read, or that it is broken!");
	}

	if (!test_face) {
		throw std::runtime_error("Wrong font!");
	}

	drawFont_p = new Sentence(test_face);
	drawFont_p->SetText("XXX", 5000, glm::u8vec4(0x0, 0x0, 0xff, 0xff), glm::vec2(0.0f, 0.0f));
	scene_sen = new Sentence(test_face);

	for (int i = 0; i < 5; i++) {
		option_sens.emplace_back(new Sentence(test_face2));
	}

	load_text_scenes();
	curr_choice = 0;
	curr_scene = 1;
}

PlayMode::~PlayMode() {

}

bool PlayMode::handle_event(SDL_Event const& evt, glm::uvec2 const& window_size) {
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
				text_scenes[curr_scene].elapsed = 0.0f;//reset
				std::map<int, bool>::iterator it;
				for (it = text_scenes[curr_scene].played.begin(); it != text_scenes[curr_scene].played.end(); it++) {
					it->second = false;
				}
				curr_scene = text_scenes[curr_scene].next_scene[curr_choice];
				curr_choice = 0;
			}
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	{
		// update scene description word
		// check if needs to play sound
		int cur_len = (int)text_scenes[curr_scene].visible_desc.size();
		if (!typing_sample && cur_len == 0) {
			typing_sample = Sound::loop(*load_typing_effect, 1.0f);
		}

		if (typing_sample && cur_len == text_scenes[curr_scene].description.size()) {
			typing_sample->stop();
			typing_sample = nullptr;
		}

		text_scenes[curr_scene].elapsed += elapsed;
		int char_size = (int)(text_scenes[curr_scene].elapsed / pop_char_interval);
		text_scenes[curr_scene].visible_desc = text_scenes[curr_scene].description.substr(0, char_size);
		if (text_scenes[curr_scene].sounds.find((int)text_scenes[curr_scene].visible_desc.size()) != text_scenes[curr_scene].sounds.end()) {
			if (!text_scenes[curr_scene].played[(int)text_scenes[curr_scene].visible_desc.size()]) {
				Sound::play((*sound_samples).at(text_scenes[curr_scene].sounds[(int)text_scenes[curr_scene].visible_desc.size()]));
				text_scenes[curr_scene].played[(int)text_scenes[curr_scene].visible_desc.size()] = true;
			}
		}
	}

	//	if(total_elapsed - int(total_elapsed) >= 0.5) {
	//		overlay.AddText("test", "YEAH", {-1.0f, 0.0f}, test_face, 10000, glm::u8vec4(0xff, 0x0, 0x0, 0xff));
	//	} else {
	//		overlay.AddText("test", "abcdefghijklmnopqrstuvwxyz", {-1.0f, 0.0f}, test_face, 6000, glm::u8vec4(0x0, 0x0, 0xff, 0xff));
	//	}
	//	total_elapsed += elapsed;
	//	glm::u8vec4 color = glm::u8vec4(0x00, 0x00, 0x00, 0x00);

}

void PlayMode::draw(glm::uvec2 const& drawable_size) {
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
		scene_sen->SetText(&(text_scenes[curr_scene].visible_desc[0]), 4000,
			glm::u8vec4(0x0, 0x0, 0xff, 0xff), glm::vec2(-1.0f, 0.9f));

		for (auto s : option_sens) {
			s->ClearText();
		}

		float y_offset = 0.8f;
		for (int i = 0; i < text_scenes[curr_scene].choice_descriptions.size() && i < option_sens.size(); i++) {
			bool is_selected = i == curr_choice;
			glm::u8vec4 color = (is_selected ? glm::u8vec4(0x00, 0xff, 0xff, 0xff) :
				glm::u8vec4(0xff, 0x00, 0x00, 0xff));

			option_sens[i]->SetText(&(text_scenes[curr_scene].choice_descriptions[i][0]),
				3000, color, glm::vec2(-1.0f, y_offset));
			y_offset -= 0.1f;
		}

		scene_sen->Draw(drawable_size);

		for (auto s : option_sens) {
			s->Draw(drawable_size);
		}
	}
}


void PlayMode::load_text_scenes() {
	// From https://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
	std::string path = data_path("texts");
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		std::cout << entry.path() << std::endl;
		std::ifstream f(entry.path());
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
				if (line.rfind("&&", 0) == 0)
					break;
				choice_des.append(line.append("\n"));
			}
			choice_des = choice_des.substr(0, choice_des.size() - 1);
			ts.choice_descriptions.push_back(choice_des);
		}
		while (line.rfind("&&", 0) == 0) {
			int start_pos = std::stoi(line.substr(2));
			if (!std::getline(f, line)) {
				std::cout << "Music not specified!" << std::endl;
				continue;
			}
			ts.sounds[start_pos] = line;
			ts.played[start_pos] = false;
		}
		text_scenes[id] = ts;
		text_scenes[id].elapsed = 0.0f; // init timer
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