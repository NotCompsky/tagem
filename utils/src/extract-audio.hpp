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


inline
int extract_audio(char* const output_filepath_basename,  const char* const input_filepath,  const bool _extract_audio,  const bool print_output_file_path){
	int err = ERR_SUCCESS;
	AVFormatContext* input_fmt_ctx = avformat_alloc_context();
	AVFormatContext* output_fmt_ctx;
	AVPacket pkt;
	const char* file_ext;
	AVOutputFormat* outfmt;
	AVStream* input_audio;
	AVStream* output_audio;
	
	if (!input_fmt_ctx){
		return ERR_CANNOT_ALLOC_INPUT_CONTEXT;
	}
	
	if (avformat_open_input(&input_fmt_ctx, input_filepath, NULL, NULL)){
		err = ERR_CANNOT_OPEN_FILE;
		goto cleanup1;
	}
	
	if (avformat_find_stream_info(input_fmt_ctx, NULL)){
		err = ERR_CANNOT_FIND_STREAM_INFO;
		goto cleanup2;
	}
	
	///AVCodec* output_audio_codec;
	input_audio;
	for (int i = 0;  i < input_fmt_ctx->nb_streams;  i++){
		input_audio = input_fmt_ctx->streams[i];

		// if output context has audio codec support and current input stream is audio
		if (input_audio == NULL  ||  input_audio->codecpar->codec_type != AVMEDIA_TYPE_AUDIO)
			continue;
		
		break;
	}
	if (input_audio == NULL){
		err = ERR_CANNOT_FIND_AUDIO;
		goto cleanup2;
	}
	
	file_ext = get_ext(input_audio->codecpar->codec_id);
	if (file_ext == nullptr){
		fprintf(stderr,  "Not implemented for codec of file: %s\n", input_filepath);
		goto cleanup2;
	}
	memcpy(output_filepath_basename + strlen(output_filepath_basename),  file_ext,  strlen(file_ext) + 1);
	if (print_output_file_path){
		printf("%s\n", output_filepath_basename);
	}
	if (is_file(output_filepath_basename)  or  not _extract_audio)
		goto cleanup2;
	
	/*if (file_ext == nullptr){
		const char* const _file_ext = 
		std::filesystem::copy(input_filepath, output_filepath_basename_basename);
		return ERR_SUCCESS;
	}*/
	
	outfmt = av_guess_format(NULL, output_filepath_basename, NULL);
	if (outfmt == NULL){
		err = ERR_CANNOT_GUESS_FORMAT;
		goto cleanup2;
	}
	
	avformat_alloc_output_context2(&output_fmt_ctx, outfmt, NULL, output_filepath_basename);
	if (!output_fmt_ctx){
		err = ERR_CANNOT_ALLOC_OUTPUT_CONTEXT;
		goto cleanup2;
	}
	
	output_audio = avformat_new_stream(output_fmt_ctx, avcodec_find_decoder(input_audio->codecpar->codec_id)); ///output_audio_codec);
	if (NULL == output_audio){
		err = ERR_CANNOT_ALLOCATE_AUDIO_OUTPUT_STREAM;
		goto cleanup3;
	}
	
	if (avcodec_parameters_copy(output_audio->codecpar, input_audio->codecpar)){
		err = ERR_CANNOT_COPY_CODEC_PARAMS;
		goto cleanup4;
	}
	
	output_audio->duration = input_audio->duration;
	output_audio->time_base.num = input_audio->time_base.num;
	output_audio->time_base.den = input_audio->time_base.den;
	
	if (avio_open(&output_fmt_ctx->pb, output_filepath_basename, AVIO_FLAG_WRITE)){
		err = ERR_CANNOT_OPEN_OUTPUT_FILE;
		goto cleanup4;
	}
	
	if (avformat_write_header(output_fmt_ctx, NULL)){
		err = ERR_CANNOT_WRITE_HEADER;
		goto cleanup5;
	}
	
	while(av_read_frame(input_fmt_ctx, &pkt) == 0){
		if (pkt.stream_index != input_audio->index)
			goto free_packet;
		pkt.stream_index = output_audio->index;
		pkt.pts = av_rescale_q(pkt.pts, input_audio->time_base, output_audio->time_base);
		pkt.dts = av_rescale_q(pkt.dts, input_audio->time_base, output_audio->time_base);
		if (av_interleaved_write_frame(output_fmt_ctx, &pkt) != 0){
			err = ERR_INTERLEAVED_WRITE_FAILED;
			av_packet_unref(&pkt);
			goto cleanup5;
		}
		free_packet:
		av_packet_unref(&pkt);
	}
	
	av_write_trailer(output_fmt_ctx);
	
	cleanup5:
	avio_closep(&output_fmt_ctx->pb);
	
	cleanup4: // A context's streams are freed in same function as the context itself
	cleanup3:
	avformat_free_context(output_fmt_ctx);
	
	cleanup2:
	avformat_close_input(&input_fmt_ctx);
	
	cleanup1:
	avformat_free_context(input_fmt_ctx);
	
	return 0;
}
