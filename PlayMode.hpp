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
	virtual void draw(glm::uvec2 const &drawable_size) override;

	float total_elapsed = 0.0f;
	FT_Library library;
	FT_Face test_face;
	FT_Face test_face2;

	Sentence* drawFont_p;
	Sentence* scene_sen = nullptr;
	std::vector<Sentence*> option_sens;

	//music coming from the tip of the leg (as a demonstration):

	//camera:
	Scene::Camera *camera = nullptr;

	struct TextScene {
		int id;
		std::string description;
		std::vector<std::string> choice_descriptions;
		std::vector<int> next_scene;

		float elapsed; // used for text animation
		std::string visible_desc;
	};
	std::map<int, TextScene> text_scenes;
	int curr_scene;
	int curr_choice;
	void load_text_scenes();

	// secs interval to pop out a char
	float pop_char_interval = 0.1f;

	std::shared_ptr<Sound::PlayingSample> typing_sample;
};
