#include "PlayMode.hpp"


#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <fstream>
#include "data_path.hpp"
#include "Sound.hpp"

#include <dirent.h>


Load< std::map<std::string, Sound::Sample>> sound_samples(LoadTagDefault, []() -> std::map<std::string, Sound::Sample> const* {
		auto map_p = new std::map<std::string, Sound::Sample>();
		std::string base_dir = data_path("musics");

		struct dirent *entry;
		DIR *dp;

		dp = opendir(&base_dir[0]);
		if (dp == nullptr) {
			std::cout<<"Cannot open "<<base_dir<<"\n";
			throw std::runtime_error("Cannot open dir");
		}
		std::cout<<base_dir<<std::endl;
		while ((entry = readdir(dp))) {
#if defined(_WIN32)
	std::string path_string = base_dir + "\\" + std::string(entry->d_name);
#else
	std::string path_string = base_dir + "/" + std::string(entry->d_name);
#endif
			size_t start = 0;
			size_t end = path_string.find(".opus");

			if(end != std::string::npos) {
				std::cout << path_string.substr(start, end) << std::endl;
				map_p->emplace(path_string.substr(start, end), Sound::Sample(path_string));
			}
		}
		return map_p;
	});

Load< Sound::Sample > load_typing_effect(LoadTagDefault, []() -> Sound::Sample const* {
		return new Sound::Sample(data_path("musics/typing.opus"));
	});

PlayMode::PlayMode() {
	FT_Library library;
	FT_Error error_lib = FT_Init_FreeType(&library);
	if (error_lib) {
		std::cout << "Init library failed." << std::endl;
	}

	std::string desc_font_path = data_path("ReallyFree-ALwl7.ttf");
	FT_Error error_1 = FT_New_Face(library, &(desc_font_path[0]), 0, &desc_face);

	if (error_1) {
		std::cout<<"Error code: "<<error_1<<std::endl;
		throw std::runtime_error("Cannot open font file");
	}

	FT_Error error_2 = FT_New_Face(library, &(desc_font_path[0]), 0, &option_face);
	if (error_2) {
		std::cout<<"Error code: "<<error_2<<std::endl;
		throw std::runtime_error("Cannot open font file");
	}

	if (!desc_face || !option_face) {
		throw std::runtime_error("Wrong font!");
	}

//	drawFont_p = new Sentence(desc_face, 720 / LINE_CNT);
//	drawFont_p->SetText("abcdefghijklmnopqrstuvwxyz\nabcdefghijklmnopqrstuvwxyz\n3\n4\n5\n6\n7\n8\n9\n10", 5000, glm::u8vec4(0x0, 0x0, 0xff, 0xff), glm::vec2(-1.0f, 0.9f));
//	drawFont_p->Draw(glm::vec2(1280, 720));

	scene_sen = new Sentence(desc_face, 720 / LINE_CNT); // hard code height of each line
	for(int i=0; i<5; i++) {
		option_sens.emplace_back(new Sentence(option_face, 720 / LINE_CNT));
	}

	load_text_scenes();
	curr_choice = 0;
	curr_scene = 1;
}

PlayMode::~PlayMode() {

}

bool PlayMode::handle_event(SDL_Event const& evt, glm::uvec2 const& window_size) {
	// Reference https://github.com/Dmcdominic/15-466-f20-game4/blob/menu-mode/MenuMode.cpp
	if (!text_scenes[curr_scene].choice_descriptions.empty() &&
			text_scenes[curr_scene].description.size() == text_scenes[curr_scene].visible_desc.size()) {
		// only response key board after showing all scene description
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
				text_scenes[curr_scene].visible_desc.clear();//reset
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
		auto cur_len = (uint32_t)text_scenes[curr_scene].visible_desc.size();
//		std::cout<<"cur_len="<<cur_len<<std::endl;
		if (!typing_sample && cur_len == 0) {
			typing_sample = Sound::loop(*load_typing_effect, 1.0f);
		}

		if (typing_sample && cur_len == (uint32_t)text_scenes[curr_scene].description.size()) {
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
}

void PlayMode::draw(glm::uvec2 const &window_size) {
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
		float scene_y_anchor = 1.0f - 2.0f / (float)LINE_CNT;
		scene_sen->SetText(&(text_scenes[curr_scene].visible_desc[0]), 4000,
		                   scene_desc_color,glm::vec2(-1.0f, scene_y_anchor));

		for (auto s : option_sens) {
			s->ClearText();
		}

		int lines_of_desc = (int)std::count(text_scenes[curr_scene].description.begin(),
								 text_scenes[curr_scene].description.end(), '\n') + 1;
		float option_y_anchor = 1.0f - (2.0f / (float)LINE_CNT) * (float)(lines_of_desc + 1);
		for (uint32_t i = 0; i < text_scenes[curr_scene].choice_descriptions.size() && i < option_sens.size(); i++) {
			glm::u8vec4 color = (int)i == curr_choice ? option_select_color : option_unselect_color;

			option_sens[i]->SetText(&(text_scenes[curr_scene].choice_descriptions[i][0]),
			                        3000, color, glm::vec2(-1.0f, option_y_anchor));
			option_y_anchor -= (2.0f / (float)LINE_CNT) / 2.0f;
		}

		scene_sen->Draw(window_size);

		if(text_scenes[curr_scene].description.size() == text_scenes[curr_scene].visible_desc.size()) {
			// only draw until all scene desc appear
			for(auto s: option_sens) {
				s->Draw(window_size);
			}
		}

//		drawFont_p->Draw(window_size);
	}
}


void PlayMode::load_text_scenes() {
	// From https://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c
	std::string base_dir = data_path("texts");

	struct dirent *entry;
	DIR *dp;

	dp = opendir(&base_dir[0]);
	if (dp == nullptr) {
		std::cout<<"Cannot open "<<base_dir<<"\n";
		throw std::runtime_error("Cannot open dir");
	}
	std::cout<<base_dir<<std::endl;
	while ((entry = readdir(dp))) {
#if defined(_WIN32)
		std::string txt_path = base_dir + "\\" + std::string(entry->d_name);
#else
		std::string txt_path = base_dir + "/" + std::string(entry->d_name);
#endif
		std::cout << txt_path << std::endl;
		std::ifstream f(txt_path);
		std::string line;
		if (!f.is_open()) {
			std::cout << "Unable to open file " << txt_path << std::endl;
			continue;
		}
		if (!std::getline(f, line)) {
			std::cout << txt_path << " is an empty file! Skipped." << std::endl;
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
	closedir(dp);

	std::map<int, TextScene>::iterator it;
	for (it = text_scenes.begin(); it != text_scenes.end(); it++) {
		//		std::cout << "Current Scene: " << it->first << std::endl << "Description: " << it->second.description << std::endl;
		for (uint32_t i = 0; i < it->second.next_scene.size(); i++) {
			//			std::cout << "Choice: " << it->second.next_scene[i] << std::endl;
			//			std::cout << "Description: " << it->second.choice_descriptions[i] << std::endl;
		}
	}
}