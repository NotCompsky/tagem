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
// All code is copied from my "utils" repository's extract-audio.cpp
// TODO: Deduplicate - compile extract-audio
// Missing nice GCC features, such as designated initialisation, which would have allowed for easily mapping the AV_CODEC_ID_* enums to strings (rather than only being some unknown number at runtime)
//    #define MAP_ENUM(name) [ name ] = #name

#pragma once

extern "C" {
# include <libavformat/avformat.h>
# include <libavcodec/avcodec.h>
}
#include <cstring>
#include <sys/stat.h>


enum {
	ERR_SUCCESS,
	ERR_CANNOT_ALLOC_INPUT_CONTEXT,
	ERR_CANNOT_OPEN_FILE,
	ERR_CANNOT_FIND_STREAM_INFO,
	ERR_CANNOT_FIND_AUDIO,
	ERR_CANNOT_GUESS_FORMAT,
	ERR_CANNOT_ALLOC_OUTPUT_CONTEXT,
	ERR_CANNOT_ALLOCATE_AUDIO_OUTPUT_STREAM,
	ERR_CANNOT_COPY_CODEC_PARAMS,
	ERR_CANNOT_OPEN_OUTPUT_FILE,
	ERR_CANNOT_WRITE_HEADER,
	ERR_INTERLEAVED_WRITE_FAILED,
	ERR_NOT_IMPLEMENTED
};


inline
const char* get_ext(const int codec_id){
	const char* file_ext;
	switch(codec_id){
		case AV_CODEC_ID_OPUS:
			return ".opus";
		case AV_CODEC_ID_AAC:
			return ".m4a";
		case AV_CODEC_ID_VORBIS:
			return ".ogg";
		case AV_CODEC_ID_VP9:
			return ".webm";
		
		// Pure audio formats
		case AV_CODEC_ID_MP3:
			return ".mp3";
		
		default:
			return nullptr;
	}
}


inline
bool is_file(const char* const path){
	static struct stat buffer;
	return (stat(path, &buffer) == 0); 
}
