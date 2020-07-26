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
