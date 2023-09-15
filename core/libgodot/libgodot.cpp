/**************************************************************************/
/*  libgodot.cpp                                                          */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifdef LIBRARY_ENABLED
#include "libgodot.h"
#include "core/extension/gdextension_manager.h"

#include "main/main.h"
#include "platform/windows/os_windows.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/main/scene_tree.h"
#include "scene/main/window.h"
#include <iostream>

#pragma optimize("", off)

static void run(OS_Windows *os) {
	auto *main_loop = os->get_main_loop();
	if (!main_loop) {
		return;
	}
}
static void get_viewport_size(int &outWidth, int &outHeight) {
	auto *en = Engine::get_singleton();
	SceneTree *tree = SceneTree::get_singleton();
	Window *root = tree->get_root();
	//root->print_tree();
	auto *vp = root->get_viewport();
	auto tex = vp->get_texture();
	auto img = tex->get_image();
	outWidth = img->get_width();
	outHeight = img->get_height();
}
static void get_viewport_data(uint8_t *outData) {
	auto *en = Engine::get_singleton();
	SceneTree *tree = SceneTree::get_singleton();
	Window *root = tree->get_root();
	//root->print_tree();
	auto *vp = root->get_viewport();
	auto tex = vp->get_texture();
	auto img = tex->get_image();
	auto data = img->get_data();
	auto format = img->get_format();
	auto w = img->get_width();
	auto h = img->get_height();

	memcpy(outData, data.ptr(), w * h * 3);
}

static libgd::ImageLoadFunc g_imageLoadFunc = nullptr;
void libgd::set_image_load_function(const ImageLoadFunc &fc) {
	g_imageLoadFunc = fc;
}

static Image::Format to_godot_format(libgd::ImageFormat format) {
	static_assert(static_cast<uint32_t>(libgd::ImageFormat::Max) == static_cast<uint32_t>(Image::Format::FORMAT_MAX));
	return static_cast<Image::Format>(format);
}

static Image *load_image(const char *fileName) {
	if (!g_imageLoadFunc)
		return nullptr;
	libgd::ImageFormat format;
	uint32_t w, h;
	size_t dataSize;
	auto *data = g_imageLoadFunc(fileName, format, w, h, dataSize);
	if (!data)
		return nullptr;
	PackedByteArray imgData;
	imgData.resize(dataSize);
	memcpy(imgData.ptrw(), data, dataSize);
	Image *img = memnew(Image);
	img->initialize_data(w, h, true, to_godot_format(format), imgData);
	return img;
}

#include <scene/3d/light_3d.h>
#include <scene/resources/image_texture.h>

static void add_point_light_source(float *pos, float *color, float intensity) {
	auto *en = Engine::get_singleton();
	SceneTree *tree = SceneTree::get_singleton();
	Window *root = tree->get_root();

	auto *light = memnew(OmniLight3D);
	light->set_shadow(false);
	light->set_color(Color(color[0], color[1], color[2]));
	// light->set_bake_mode(Light3D::BakeMode::BAKE_STATIC);
	//light->set_temperature(0);
	light->set_position(Vector3(pos[0], pos[1], pos[2]));
	light->set_param(Light3D::PARAM_ENERGY, intensity);

	root->add_child(light);
}

static void add_actor(float *vertexData, float *normalData, float *uvData, uint32_t vertexCount, uint32_t *indexData, uint32_t indexCount, const char *texture) {
	auto *en = Engine::get_singleton();
	SceneTree *tree = SceneTree::get_singleton();
	Window *root = tree->get_root();
	//std::cout << "Tree: " << std::endl;
	//root->print_tree();

	static_assert(sizeof(Vector3) == sizeof(float) * 3);
	float scale = 10.f;
	PackedVector3Array vertices;
	vertices.resize(vertexCount);
	memcpy(vertices.ptrw(), vertexData, vertexCount * sizeof(Vector3));

	PackedVector3Array normals;
	normals.resize(vertexCount);
	memcpy(normals.ptrw(), normalData, vertexCount * sizeof(Vector3));

	static_assert(sizeof(Vector2) == sizeof(float) * 2);
	PackedVector2Array uvs;
	uvs.resize(vertexCount);
	memcpy(uvs.ptrw(), uvData, vertexCount * sizeof(Vector2));

	PackedInt32Array indices;
	indices.resize(indexCount);
	for (auto i = decltype(indexCount){ 0u }; i < indexCount; ++i)
		indices.set(i, indexData[i]);

	Array arrays;
	arrays.resize(ArrayMesh::ARRAY_MAX);
	arrays[ArrayMesh::ARRAY_VERTEX] = std::move(vertices);
	arrays[ArrayMesh::ARRAY_NORMAL] = std::move(normals);
	arrays[ArrayMesh::ARRAY_TEX_UV] = std::move(uvs);
	arrays[ArrayMesh::ARRAY_INDEX] = std::move(indices);

	auto *mesh = memnew(ArrayMesh);
	mesh->add_surface_from_arrays(Mesh::PrimitiveType::PRIMITIVE_TRIANGLES, arrays);

	auto *mat = memnew(StandardMaterial3D);

	Image *img = load_image(texture);
	if (img) {
		auto tex = ImageTexture::create_from_image(img);
		mat->set_texture(StandardMaterial3D::TEXTURE_ALBEDO, tex);
	}

	mesh->surface_set_material(0, mat);

	auto *meshInstance = memnew(MeshInstance3D);
	meshInstance->set_mesh(mesh);

	root->add_child(meshInstance);
	std::cout << "ADDED NODE!" << std::endl;
}

class EngineWrapper {
public:
	enum class StepResult : uint8_t {
		None = 0,
		ShouldClose = 1
	};
	static std::unique_ptr<EngineWrapper> Create(int argc, char **argv_utf8);
	EngineWrapper();
	~EngineWrapper();
	StepResult Step();

private:
	void Finalize();
	bool Start(int argc, char **argv_utf8);

	std::unique_ptr<OS> m_os;
};

std::unique_ptr<EngineWrapper> EngineWrapper::Create(int argc, char **argv_utf8) {
	auto wrapper = std::make_unique<EngineWrapper>();
	if (!wrapper->Start(argc, argv_utf8))
		return nullptr;
	return wrapper;
}

EngineWrapper::EngineWrapper() :
		m_os{ std::make_unique<OS_Windows>(nullptr) } {}

EngineWrapper::~EngineWrapper() { Finalize(); }

bool EngineWrapper::Start(int argc, char **argv_utf8) {
	setlocale(LC_CTYPE, "");

	// TEST_MAIN_PARAM_OVERRIDE(argc, argv_utf8)

	Error err = Main::setup(argv_utf8[0], argc - 1, &argv_utf8[1]);

	if (err != OK) {
		if (err == ERR_HELP) { // Returned by --help and --version, so success.
			return false;
		}
		return false;
	}

	if (!Main::start()) {
		return false;
		//run(&os);
		//os.run();
	}

	auto *main_loop = m_os->get_main_loop();
	main_loop->initialize();
	return true;
	//os.get_exit_code();

	/*
		for node in nodes:
			var scene : Node = load(node).instance()
			add_child(scene)
			yield(get_tree(), "idle_frame")
			var imageTexture = ImageTexture.new()
			var image = Image.new()
			image.copy_from(scene.get_node("Viewport").get_texture().get_data())
			imageTexture.create_from_image(image)
			add_item(scene.get_node("Viewport/Node").title, imageTexture)
			remove_child(scene)
			scene.queue_free()
			*/
}
void EngineWrapper::Finalize() {
	if (m_os) {
		auto *main_loop = m_os->get_main_loop();
		if (main_loop)
			main_loop->finalize();
	}
	Main::cleanup();
}
EngineWrapper::StepResult EngineWrapper::Step() {
	auto *main_loop = m_os->get_main_loop();
	DisplayServer::get_singleton()->process_events(); // get rid of pending events
	return Main::iteration() ? StepResult::ShouldClose : StepResult::None;
}

static std::unique_ptr<EngineWrapper> g_engineWrapper = nullptr;
static bool start_engine(int argc, char **argv_utf8) {
	g_engineWrapper = EngineWrapper::Create(argc, argv_utf8);
	return g_engineWrapper != nullptr;
}
static void stop_engine() {
	g_engineWrapper = nullptr;
}
static bool step_engine() {
	return static_cast<bool>(g_engineWrapper->Step());
}

#ifdef __cplusplus
extern "C" {
#endif

void *godotsharp_game_main_init;
GDExtensionInitializationFunction initialization_function;
void (*scene_load_function)(void *);

void *libgodot_sharp_main_init() {
	return godotsharp_game_main_init;
}

LIBGODOT_API void libgodot_mono_bind(void *sharp_main_init, void (*scene_function_bind)(void *)) {
	godotsharp_game_main_init = sharp_main_init;
	scene_load_function = scene_function_bind;
}

LIBGODOT_API void libgodot_gdextension_bind(GDExtensionInitializationFunction initialization_bind, void (*scene_function_bind)(void *)) {
	initialization_function = initialization_bind;
	scene_load_function = scene_function_bind;
}

void libgodot_scene_load(void *scene) {
	if (scene_load_function != nullptr) {
		scene_load_function(scene);
	}
}

int libgodot_is_scene_loadable() {
	return scene_load_function != nullptr;
}

void libgodot_init_resource() {
	if (initialization_function != nullptr) {
		Ref<GDExtension> libgodot;
		libgodot.instantiate();
		Error err = libgodot->initialize_extension_function(initialization_function, "LibGodot");
		if (err != OK) {
			ERR_PRINT("LibGodot Had an error initialize_extension_function'");
		} else {
			print_verbose("LibGodot initialization");
			libgodot->set_path("res://LibGodotGDExtension");
			GDExtensionManager::get_singleton()->load_extension("res://LibGodotGDExtension");
		}
	}
}

__declspec(dllexport) bool godot_start(int argc, char **argv_utf8) {
	return start_engine(argc, argv_utf8);
}
__declspec(dllexport) void godot_stop() {
	return stop_engine();
}
__declspec(dllexport) bool godot_step() {
	return step_engine();
}
__declspec(dllexport) void godot_get_viewport_size(int &outWidth, int &outHeight) {
	get_viewport_size(outWidth, outHeight);
}
__declspec(dllexport) void godot_get_viewport_data(uint8_t *outData) {
	get_viewport_data(outData);
}
__declspec(dllexport) void godot_add_actor(float *vertexData, float *normalData, float *uvData, uint32_t vertexCount, uint32_t *indexData, uint32_t indexCount, const char *texture) {
	add_actor(vertexData, normalData, uvData, vertexCount, indexData, indexCount, texture);
}
__declspec(dllexport) void godot_add_omni_light(float *pos, float *color, float intensity) {
	add_point_light_source(pos, color, intensity);
}

#ifdef __cplusplus
}
#endif
#endif
