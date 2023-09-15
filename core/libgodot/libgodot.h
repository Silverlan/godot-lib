/**************************************************************************/
/*  libgodot.h                                                            */
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

#ifndef LIBGODOT_H
#define LIBGODOT_H

#if defined(LIBRARY_ENABLED)

#if defined(WINDOWS_ENABLED) | defined(UWP_ENABLED)
#define LIBGODOT_API __declspec(dllexport)
#elif defined(ANDROID_ENABLED)
#include <jni.h>
#define LIBGODOT_API JNIEXPORT
#else
#define LIBGODOT_API
#endif

#include "core/extension/gdextension_interface.h"

/*namespace libgd {

class LIBGODOT_API Mesh {
};

class LIBGODOT_API Model {
public:
	Model();
};

class LIBGODOT_API Entity {
};

class LIBGODOT_API

class LIBGODOT_API IManager {
public:
	virtual ~IManager(){};
	virtual bool LoadImageData() const;
};
}; //namespace libgd
*/

namespace libgd {
enum class ImageFormat {
	L8, //luminance
	LA8, //luminance-alpha
	R8,
	RG8,
	RGB8,
	RGBA8,
	RGBA4444,
	RGB565,
	RF, //float
	RGF,
	RGBF,
	RGBAF,
	RH, //half float
	RGH,
	RGBH,
	RGBAH,
	RGBE9995,
	DXT1, //s3tc bc1
	DXT3, //bc2
	DXT5, //bc3
	RGTC_R,
	RGTC_RG,
	BPTC_RGBA, //btpc bc7
	BPTC_RGBF, //float bc6h
	BPTC_RGBFU, //unsigned float bc6hu
	ETC, //etc1
	ETC2_R11, //etc2
	ETC2_R11S, //signed, NOT srgb.
	ETC2_RG11,
	ETC2_RG11S,
	ETC2_RGB8,
	ETC2_RGBA8,
	ETC2_RGB8A1,
	ETC2_RA_AS_RG, //used to make basis universal happy
	DXT5_RA_AS_RG, //used to make basis universal happy
	ASTC_4x4,
	ASTC_4x4_HDR,
	ASTC_8x8,
	ASTC_8x8_HDR,
	Max
};

using ImageLoadFunc = uint8_t *(*)(const char *, ImageFormat &, uint32_t &, uint32_t &, size_t &);
LIBGODOT_API void set_image_load_function(const ImageLoadFunc &fc);
}; //namespace libgd

#ifdef __cplusplus
extern "C" {
#endif

void libgodot_init_resource();

void libgodot_scene_load(void *scene);

int libgodot_is_scene_loadable();

void *libgodot_sharp_main_init();

LIBGODOT_API void libgodot_mono_bind(void *sharp_main_init, void (*scene_function_bind)(void *));

LIBGODOT_API void libgodot_gdextension_bind(GDExtensionInitializationFunction initialization_bind, void (*scene_function_bind)(void *));

LIBGODOT_API int godot_main(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif

#endif // LIBGODOT_H
