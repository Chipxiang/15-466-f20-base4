#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "Texture2DProgram.hpp"

#include <random>
#include <hb.h>
#include <hb-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H



GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("hexapod.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("hexapod.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = hexapod_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("dusty-floor.opus"));
});

PlayMode::PlayMode() : scene(*hexapod_scene) {



	//get pointers to leg for convenience:
	for (auto &transform : scene.transforms) {
		if (transform.name == "Hip.FL") hip = &transform;
		else if (transform.name == "UpperLeg.FL") upper_leg = &transform;
		else if (transform.name == "LowerLeg.FL") lower_leg = &transform;
	}
	if (hip == nullptr) throw std::runtime_error("Hip not found.");
	if (upper_leg == nullptr) throw std::runtime_error("Upper leg not found.");
	if (lower_leg == nullptr) throw std::runtime_error("Lower leg not found.");

	hip_base_rotation = hip->rotation;
	upper_leg_base_rotation = upper_leg->rotation;
	lower_leg_base_rotation = lower_leg->rotation;

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	//start music loop playing:
	// (note: position will be over-ridden in update())
//	leg_tip_loop = Sound::loop_3D(*dusty_floor_sample, 1.0f, get_leg_tip_position(), 10.0f);
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
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
	}

	return false;
}

void PlayMode::update(float elapsed) {

//	//slowly rotates through [0,1):
//	wobble += elapsed / 10.0f;
//	wobble -= std::floor(wobble);
//
//	hip->rotation = hip_base_rotation * glm::angleAxis(
//		glm::radians(5.0f * std::sin(wobble * 2.0f * float(M_PI))),
//		glm::vec3(0.0f, 1.0f, 0.0f)
//	);
//	upper_leg->rotation = upper_leg_base_rotation * glm::angleAxis(
//		glm::radians(7.0f * std::sin(wobble * 2.0f * 2.0f * float(M_PI))),
//		glm::vec3(0.0f, 0.0f, 1.0f)
//	);
//	lower_leg->rotation = lower_leg_base_rotation * glm::angleAxis(
//		glm::radians(10.0f * std::sin(wobble * 3.0f * 2.0f * float(M_PI))),
//		glm::vec3(0.0f, 0.0f, 1.0f)
//	);
//
//	//move sound to follow leg tip position:
////	leg_tip_loop->set_position(get_leg_tip_position(), 1.0f / 60.0f);
//
//	//move camera:
//	{
//
//		//combine inputs into a move:
//		constexpr float PlayerSpeed = 30.0f;
//		glm::vec2 move = glm::vec2(0.0f);
//		if (left.pressed && !right.pressed) move.x =-1.0f;
//		if (!left.pressed && right.pressed) move.x = 1.0f;
//		if (down.pressed && !up.pressed) move.y =-1.0f;
//		if (!down.pressed && up.pressed) move.y = 1.0f;
//
//		//make it so that moving diagonally doesn't go faster:
//		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;
//
//		glm::mat4x3 frame = camera->transform->make_local_to_parent();
//		glm::vec3 right = frame[0];
//		//glm::vec3 up = frame[1];
//		glm::vec3 forward = -frame[2];
//
//		camera->transform->position += move.x * right + move.y * forward;
//	}
//
//	{ //update listener to camera position:
//		glm::mat4x3 frame = camera->transform->make_local_to_parent();
//		glm::vec3 right = frame[0];
//		glm::vec3 at = frame[3];
//		Sound::listener.set_position_right(at, right, 1.0f / 60.0f);
//	}
//
//	//reset button press counters:
//	left.downs = 0;
//	right.downs = 0;
//	up.downs = 0;
//	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
//	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
//	// TODO: consider using the Light(s) in the scene to do this
//	glUseProgram(lit_color_texture_program->program);
//	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
//	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
//	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
//	glUseProgram(0);
//
//	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
//	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//	glEnable(GL_DEPTH_TEST);
//	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

//	scene.draw(*camera);

//	{ //use DrawLines to overlay some text:
//		glDisable(GL_DEPTH_TEST);
//		float aspect = float(drawable_size.x) / float(drawable_size.y);
//		DrawLines lines(glm::mat4(
//			1.0f / aspect, 0.0f, 0.0f, 0.0f,
//			0.0f, 1.0f, 0.0f, 0.0f,
//			0.0f, 0.0f, 1.0f, 0.0f,
//			0.0f, 0.0f, 0.0f, 1.0f
//		));
//
//		constexpr float H = 0.09f;
//		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
//			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
//			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
//			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
//		float ofs = 2.0f / drawable_size.y;
//		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
//			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
//			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
//			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
//	}
//	GL_ERRORS();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.4f, 0.9f, 0.9f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	FT_Library library;
	FT_Face* face = new FT_Face;
	FT_Error error = FT_Init_FreeType(&library);
	if (error) { std::cout << "Init library failed." << std::endl; }
	hb_buffer_t* buf = hb_buffer_create();
	hb_buffer_add_utf8(buf, "Hello", -1, 0, -1);
	hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
	hb_buffer_set_script(buf, HB_SCRIPT_LATIN);
	hb_buffer_set_language(buf, hb_language_from_string("en", -1));
//	error = FT_New_Face(library, "c:/windows/fonts/brushsci.ttf", 0, face);
	error = FT_New_Face(library, "/System/Library/Fonts/SFCompactRounded.ttf", 0, face);
	if (error == FT_Err_Unknown_File_Format)
	{
		std::cout << "Unknown font format." << std::endl;
	}
	else if (error)
	{
		std::cout << "Can't open font file, error code: "<< error << std::endl;
	}
//	assert(face != nullptr);
	FT_Set_Char_Size(*face, 0, 1000, 0, 0);
	hb_font_t* font = hb_ft_font_create(*face, NULL);
	hb_shape(font, buf, NULL, 0);

	unsigned int glyph_count;
	hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buf, &glyph_count);
	hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);
	double cursor_x = 0.0, cursor_y = 0.0;

	glUseProgram(texture2d_program->program);

	for (uint32_t i = 0; i < glyph_count; ++i) {
		auto glyphid = glyph_info[i].codepoint;
		auto x_offset = glyph_pos[i].x_offset / 64.0;
		auto y_offset = glyph_pos[i].y_offset / 64.0;
		auto x_advance = glyph_pos[i].x_advance / 64.0;
		auto y_advance = glyph_pos[i].y_advance / 64.0;
		//   draw_glyph(glyphid, cursor_x + x_offset, cursor_y + y_offset);


		FT_Load_Glyph(*face,
		              glyphid, // the glyph_index in the font file
		              FT_LOAD_DEFAULT);
		FT_GlyphSlot slot = (*face)->glyph;
		FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);
		FT_Bitmap ftBitmap = slot->bitmap; //this contains the actual data

//		int twidth = pow(2, ceil(log(ftBitmap.width)/log(2)));
//		int theight = pow(2, ceil(log(ftBitmap.rows)/log(2)));

//		auto tdata = new unsigned char[twidth * theight] ();
//
//		for(int iy = 0; iy < ftBitmap.rows; ++iy) {
//			memcpy(tdata + iy * twidth, ftBitmap.buffer + iy * ftBitmap.width, ftBitmap.width);
//		}
//
//		for(int x=0; x < theight; x ++) {
//			for(int y=0; y < twidth; y ++) {
//				std::cout<<(int)*(tdata + x * twidth + y)<<" ";
//			}
//			std::cout<<std::endl;
//		}
//		std::cout<<std::endl;

		GLuint texture_id = 0;

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D, texture_id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ftBitmap.width, ftBitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, ftBitmap.buffer);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_ONE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_ONE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ONE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);

		glBindTexture(GL_TEXTURE_2D, 0);

		auto GetOpenGLPos = [](float pos, int drawable_size) {
			return (2 * pos) / drawable_size - 1;
		};

		float start_x = GetOpenGLPos(cursor_x + x_offset, 1280);
		float start_y = GetOpenGLPos(cursor_y + y_offset, 720);
		float end_x = start_x + ftBitmap.width * 2.0f / 1280;
		float end_y = start_y + ftBitmap.rows * 2.0f / 720;

		glm::u8vec4 color(0xff, 0xff, 0xff, 0x78);
		Texture2DProgram::Vertex vertexes[] {
				{{start_x, start_y}, color, {0, 1}},
				{{end_x, start_y}, color, {1, 1}},
				{{start_x, end_y}, color, {0, 0}},
				{{end_x, end_y}, color, {1, 0}}
		};

		GLuint vertex_buffer, vertex_array;
		GLuint index_buffer;
		unsigned int index_buffer_content[] {0, 1, 2, 1, 2, 3};

		glGenBuffers(1, &vertex_buffer);
		vertex_array = texture2d_program->GetVao(vertex_buffer);

		glGenBuffers(1, &index_buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), index_buffer_content, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Texture2DProgram::Vertex), static_cast<const void*>(vertexes), GL_STATIC_DRAW);

		glBindVertexArray(vertex_array);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_id);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, static_cast<const void*>(0));


		cursor_x += x_advance;
		cursor_y += y_advance;
	}
	hb_buffer_destroy(buf);
	hb_font_destroy(font);
}

glm::vec3 PlayMode::get_leg_tip_position() {
	//the vertex position here was read from the model in blender:
	return lower_leg->make_local_to_world() * glm::vec4(-1.26137f, -11.861f, 0.0f, 1.0f);
}
