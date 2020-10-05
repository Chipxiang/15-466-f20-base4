#include "Mode.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include "Sentence.hpp"

#include <glm/glm.hpp>

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

	Sentence* drawFont_p;

};
