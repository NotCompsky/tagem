/*
Copyright 2020 Adam Gray
This file is part of the tagem program.
The tagem program is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by the
Free Software Foundation version 3 of the License.
The tagem program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
This copyright notice should be included in any copy or substantial copy of the tagem source code.
The absense of this copyright notices on some other files in this project does not indicate that those files do not also fall under this license, unless they have a different license written at the top of the file.
*/
#pragma once

#include "libffmpegthumbnailer/filmstripfilter.h"
#include "libffmpegthumbnailer/videothumbnailer.h"


void generate_thumbnail(const char* const input_path,  const char* const output_path){
	using namespace ffmpegthumbnailer;
	
	constexpr bool workaround_issues = true;
	constexpr bool maintain_aspect_ratio = true;
	constexpr int quality = 8;
	constexpr bool smart_frame_selection = true;
	constexpr int thumbnail_size = 256;
	
	static FilmStripFilter filmstrip_filter;
	
	constexpr ThumbnailerImageType imageType = ThumbnailerImageType::Png;
	
	static VideoThumbnailer video_thumbnailer(0, workaround_issues, maintain_aspect_ratio, quality, smart_frame_selection);
	video_thumbnailer.setThumbnailSize(thumbnail_size);
	video_thumbnailer.addFilter(&filmstrip_filter);
	
	video_thumbnailer.generateThumbnail(input_path, imageType, output_path);
}
