/*************************************************************************/
/*  lightmap_denoiser.cpp                                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "lightmap_denoiser.h"

#include "denoise_wrapper.h"

LightmapDenoiser *LightmapDenoiserOIDN::create_oidn_denoiser() {
	return memnew(LightmapDenoiserOIDN);
}

void LightmapDenoiserOIDN::make_default_denoiser() {
	create_function = create_oidn_denoiser;
}

Ref<Image> LightmapDenoiserOIDN::denoise_image(const Ref<Image> &p_image) {
	Ref<Image> img = p_image->duplicate();

	img->convert(Image::FORMAT_RGBF);

	int rand = Math::rand();

	img->save_exr(vformat("pre_denoise_%d.exr", rand), false);

	PoolByteArray data = img->get_data();
	{
		PoolByteArray::Write w = data.write();
		if (!oidn_denoise(device, (float *)w.ptr(), img->get_width(), img->get_height())) {
			return p_image;
		}

		float *ptr = (float *)w.ptr();
		for (int i = 0; i < 100; i++) {
			print_line(vformat("%d %d -> %s %s %s", i % img->get_width(), i / img->get_width(), ptr[i * 3 + 0], ptr[i * 3 + 1], ptr[i * 3 + 2]));
		}
	}

	img->create(img->get_width(), img->get_height(), false, img->get_format(), data);
	img->save_exr(vformat("post_denoise_%d.exr", rand), false);
	return img;
}

LightmapDenoiserOIDN::LightmapDenoiserOIDN() {
	device = oidn_denoiser_init();
}

LightmapDenoiserOIDN::~LightmapDenoiserOIDN() {
	oidn_denoiser_finish(device);
}
