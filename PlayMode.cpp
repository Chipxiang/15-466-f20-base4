#include "PlayMode.hpp"


#include <iostream>
constexpr const char * const kFontPath { "/System/Library/Fonts/SFCompactRounded.ttf" };

PlayMode::PlayMode()
{

	FT_Error error = FT_Init_FreeType(&library);
	if (error) {
		std::cout << "Init library failed." << std::endl;
	}

	error = FT_New_Face(library, kFontPath, 0, &test_face);

	if (error == FT_Err_Unknown_File_Format) {
		throw std::runtime_error("The font file could be opened and read, but it appears that its font format is unsupported!");
	} else if (error) {
		throw std::runtime_error("The font file could not be opened or read, or that it is broken!");
	}

	if (!test_face) {
		throw std::runtime_error("Wrong font!");
	}
//	overlay.AddText("test", "abcdefghijklmnopqrstuvwxyz", {0.0f, 0.0f}, kFontPath, 6000, glm::u8vec4(0x0, 0x0, 0xff, 0xff));
	drawFont_p = new Sentence(library, kFontPath);
	drawFont_p->SetText("XXX", 5000, glm::u8vec4(0x0, 0x0, 0xff, 0xff), glm::vec2(0.0f, 0.0f));
//	std::string text_, glm::u8vec4 &color_, FT_Library& library_,
//	const char *font_path, FT_F26Dot6 char_size, glm::vec2& anchor_
}

PlayMode::~PlayMode() 
{
	
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) 
{
	return false;
}

void PlayMode::update(float elapsed) 
{
//	if(total_elapsed - int(total_elapsed) >= 0.5) {
//		overlay.AddText("test", "YEAH", {-1.0f, 0.0f}, test_face, 10000, glm::u8vec4(0xff, 0x0, 0x0, 0xff));
//	} else {
//		overlay.AddText("test", "abcdefghijklmnopqrstuvwxyz", {-1.0f, 0.0f}, test_face, 6000, glm::u8vec4(0x0, 0x0, 0xff, 0xff));
//	}
//	total_elapsed += elapsed;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) 
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		drawFont_p->Draw(drawable_size);
	}
}
