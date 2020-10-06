#include "Mode.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include "Sentence.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>
#include <map>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &window_size) override;

	float total_elapsed = 0.0f;
	FT_Library library;
	FT_Face desc_face;
	FT_Face option_face;

	Sentence* scene_sen = nullptr;
	std::vector<Sentence*> option_sens;

	//music coming from the tip of the leg (as a demonstration):

	struct TextScene {
		int id;
		std::string description;
		std::vector<std::string> choice_descriptions;
		std::vector<int> next_scene;
		std::map<int, std::string> sounds;
		std::map<int, bool> played;
		float elapsed; // used for text animation
		std::string visible_desc;
	};
	std::map<int, TextScene> text_scenes;
	int curr_scene;
	int curr_choice;
	void load_text_scenes();
	bool have_cd = false;
	bool pc_unlock = false;
	const std::string pc_password = "EBGDAE";
	const std::string pc_original = "EBG__E";
	const std::string door_password = "15666";
	const std::string door_original = "-----";

	// secs interval to pop out a char
	float pop_char_interval = 0.03f;

	// sound effect of typing
	std::shared_ptr<Sound::PlayingSample> typing_sample;

	std::shared_ptr<Sound::PlayingSample> bgm_loop;

	// max line count
	const int LINE_CNT = 10;

	// color used for game
	glm::u8vec4 option_select_color = glm::u8vec4(0xBD, 0xFC, 0xC9, 0xff);
	glm::u8vec4 option_unselect_color = glm::u8vec4(0xc0, 0xc0, 0xc0, 0x55);
	glm::u8vec4 scene_desc_color = glm::u8vec4(0xff, 0xff, 0xff, 0xff);
};
