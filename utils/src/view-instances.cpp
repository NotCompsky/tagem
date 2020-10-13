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
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp> // for cv::imshow
#include <stdio.h> // for fwrite

#include <compsky/asciify/asciify.hpp>
#include <compsky/mysql/mysql.hpp>
#include <compsky/mysql/query.hpp>

#include <sys/stat.h> // for mkdir


namespace _f {
	using namespace compsky::asciify::flag;
	constexpr static concat::Start start;
	constexpr static concat::End end;
	constexpr static Zip<2> zip2;
	constexpr static guarantee::BetweenZeroAndOneInclusive between_0_1_incl;
}

namespace _err {
	enum {
		none,
		generic,
		bad_option,
		not_implemented
	};
}

char BUF[4096];

char SAVE_FILES_TO[4096] = {0};
char* SAVE_FILES_TO_END = nullptr;


void view_img(const char* tag,  const char* root_tag_name,  const char* const fp,  const double x,  const double y,  const double w,  const double h,  int i){
    cv::Mat orig_img = cv::imread(fp);
    
    if (orig_img.cols == 0)
        // File does not exist
        return;
    
    const double W = orig_img.cols - 0.5;
    const double H = orig_img.rows - 0.5;
    
    const int newX = x*W;
    const int newY = y*H;
    const int newW = (x + w > 1.0) ? W - newX : w*W;
    const int newH = (y + h > 1.0) ? H - newY : h*H;
    
    /* Debugging */
    
    int u2fz = 2;
    
    cv::Rect rect(newX, newY, newW, newH);
    cv::Mat img = orig_img(rect);
    
    if (img.cols == 0  ||  img.rows == 0){
        // Bad crop
		char* itr = BUF;
        compsky::asciify::asciify(itr, "Bad crop:\t", tag, '\t', x, u2fz, ',', y, u2fz, '\t', w, u2fz, 'x', h, u2fz, '\t', "@\t", newX, ',', newY, ' ', newW, 'x', newH, "\tfrom\t", orig_img.cols, 'x', orig_img.rows, '\t', fp, '\n');
        fwrite(BUF,  1,  (uintptr_t)itr - (uintptr_t)BUF,  stderr);
        return;
    }
    
    if(SAVE_FILES_TO_END){
		char* itr = SAVE_FILES_TO_END;
		*(itr++) = '/';
		while(*tag != 0)
			*(itr++) = *(tag++);
		if (root_tag_name != nullptr){
			*(itr++) = '/';
			while(*root_tag_name != 0)
				*(itr++) = *(root_tag_name++);
		}
		*itr = 0;
		mkdir(SAVE_FILES_TO, 0777); // TODO: Check return value
		*(itr++) = '/';
		while(i != 0){
			*(itr++) = (i % 10) + '0';
			i /= 10;
		}
		*(itr++) = '.';
		*(itr++) = 'p';
		*(itr++) = 'n';
		*(itr++) = 'g';
		*itr = 0;
		cv::imwrite(SAVE_FILES_TO, img);
		*(itr++) = '\n';
		*itr = 0;
		fwrite(SAVE_FILES_TO,  (uintptr_t)itr - (uintptr_t)SAVE_FILES_TO,  1,  stderr);
	} else {
		cv::imshow("Cropped Section", img); // Window name is constant so that it is reused (rather than spawning a new window for each image)
		cv::waitKey(0);
	}
}

int main(int argc,  const char** argv) {
	constexpr static const char* help_txt = R"=====(
	USAGE
		./tagem-instances [OPTIONS] -- TAG1 TAG2 ... TAGN
		
	OPTIONS
		-d
			Depth of descendant tags for boxes. NOT the maximum depth.
			For instance, if the depth is 1, and the tags are 'Animal' and 'Vehicle', only boxes tagged 'Cat', 'Dog', 'Car' and 'Truck' will be used.
			TODO: Descendant tags deeper than that count as their relevant ancestor tag. For instance, in the above example, a box tagged only 'Tabby Cat' will count as being tagged 'Cat'.
			A depth of 0 - the default - only uses the root tag(s) themselves.
		-D [TAG]
			Ignore the following descendant tag of one of the specified tags
			NOTE: Does not ignore boxes tagged with this tag, but ensures that, if this subtag is the only subtag of a specified box, the box is not added.
			E.g. if we have three tags, one tagged "Oak", the other tagged "Pine", and another tagged both "Tree" and "Pine", with "Pine" and "Oak" inheriting from "Tree", and we run the command `./view-instances -D Oak Tree`, it would display the first and third boxes only.
		-s [TAG]
			Split on a tag.
			For instance, if we split on 'Animal' and 'Image Type', we would end up with the directory structure
				[Root directory]
				|-- Cat
					|-- Photograph
					|-- Sketch
				|- Dog
					|-- Photograph
					|-- Sketch
				|- Fox
					|-- Photograph
					|-- Sketch
				|- Mouse
					|-- Photograph
					|-- Sketch
			Can be specified multiple times.
		-w [DIRECTORY]
			Write cropped images to directory
    )=====";
	
	constexpr static const size_t mysql_auth_sz = 512;
	char mysql_auth[mysql_auth_sz];
	
	MYSQL* mysql_obj;
	MYSQL_RES* mysql_res;
	MYSQL_ROW mysql_row;
    
	unsigned depth = 0;
    std::vector<const char*> not_subtags;
	std::vector<const char*> split_on_tags;
	std::vector<const char*> root_tags;
    
    while (true){
		++argv;
		--argc;
		if(argc == 0)
			break;
        const char* arg = *argv;
        if (arg[0] == '-'  &&  arg[1] == '-'  &&  arg[2] == 0)
            break;
        
		switch(arg[1]){
			case 'd':
				depth = atoi(*(++argv));
				break;
			case 'D':
				not_subtags.push_back(*(++argv));
				--argc;
				break;
			case 's':
				split_on_tags.push_back(*(++argv));
				--argc;
				break;
			case 'w': {
				const size_t len = strlen(*(++argv));
				memcpy(SAVE_FILES_TO, *argv, len);
				SAVE_FILES_TO_END = SAVE_FILES_TO + len;
				--argc;
				break;
			}
			default:
				fprintf(stderr, "%s", help_txt);
				return 1;
        }
    }
	while(*argv != nullptr){
		root_tags.push_back(*argv);
		++argv;
	}
    
    compsky::mysql::init(mysql_obj, mysql_auth, mysql_auth_sz, getenv("TAGEM_MYSQL_CFG"));
    
	compsky::mysql::query(
		mysql_obj,
		mysql_res,
		BUF,
		"SELECT t.name, p.name, CONCAT(d.full_path, f.name), b.x, b.y, b.w, b.h "
		"FROM tag p "
		"JOIN tag2parent_tree t2pt ON t2pt.parent=p.id "
		"JOIN tag t ON t.id=t2pt.id "
		"JOIN box2tag b2t ON b2t.tag=t.id "
		"JOIN box b ON b.id=b2t.box "
		"JOIN file f ON f.id=b.file "
		"JOIN dir d ON d.id=f.dir "
		"WHERE p.name IN(\"", _f::zip2, root_tags.size(), "\",\"", root_tags, "\")"
		  "AND p.name NOT IN(\"", _f::zip2, not_subtags.size(), "\",\"", not_subtags, "\")"
		  "AND t2pt.depth=", depth, " "
		"GROUP BY t.name, b.id"
	);
    
    char* name;
	char* root_tag_name;
    char* fp;
    double x, y, w, h;
	int i = 0;
	while(compsky::mysql::assign_next_row(mysql_res, &mysql_row, &name, &root_tag_name, &fp, _f::between_0_1_incl,  &x, _f::between_0_1_incl, &y, _f::between_0_1_incl, &w, _f::between_0_1_incl, &h)){
        view_img(name, root_tag_name, fp, x, y, w, h, ++i);
	}
    
    compsky::mysql::wipe_auth(mysql_auth, mysql_auth_sz);
    
    return 0;
}
