#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp> // for cv::imshow
#include <stdio.h> // for fwrite

#include <compsky/asciify/asciify.hpp>
#include <compsky/mysql/mysql.hpp>
#include <compsky/mysql/query.hpp>

#include <sys/stat.h> // for mkdir


namespace _f {
	constexpr static const compsky::asciify::flag::concat::Start start;
	constexpr static const compsky::asciify::flag::concat::End end;
}

char BUF[4096];

char SAVE_FILES_TO[4096] = {0};
char* SAVE_FILES_TO_END = nullptr;


void view_img(const char* tag,  const char* fp,  const double x,  const double y,  const double w,  const double h,  int i){
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
    /*
    USAGE
      Non-rooted:
        ./view-instances TAG1 TAG2 ... TAGN
      Rooted:
        ./view-instances -r TAG1 TAG2 ... TAGN
      
    OPTIONS
        -r
            Descendant tags count as their heirarchical root tag
        -D [TAG]
            Ignore the following descendant tag of one of the specified tags
            NOTE: Does not ignore instances tagged with this tag, but ensures that, if this subtag is the only subtag of a specified tag, the instance is not added.
            E.g. if we have three instances, one tagged "Oak", the other tagged "Pine", and another tagged both "Tree" and "Pine", with "Pine" and "Oak" inheriting from "Tree", and we run the command `./view-instances -D Oak Tree`, it would display the first and third instances only.
        -w [DIRECTORY]
            Write cropped images to directory
    */
	
	constexpr static const size_t mysql_auth_sz = 512;
	char mysql_auth[mysql_auth_sz];
	
	MYSQL* mysql_obj;
	MYSQL_RES* mysql_res;
	MYSQL_ROW mysql_row;
    
    bool root_tags = false;
    std::vector<const char*> not_subtags;
    
    while (true){
		++argv;
		--argc;
		if(argc == 0)
			break;
        const char* arg = *argv;
        if (arg[0] != '-'  ||  arg[2] != 0)
            break;
        
		switch(arg[1]){
			case 'r':
				root_tags = true;
				break;
			case 'D':
				not_subtags.push_back(*(++argv));
				--argc;
				break;
			case 'w': {
				const size_t len = strlen(*(++argv));
				memcpy(SAVE_FILES_TO, *argv, len);
				SAVE_FILES_TO_END = SAVE_FILES_TO + len;
				--argc;
				break;
			}
			default: break;
        }
    }
    
    compsky::mysql::init(mysql_obj, mysql_auth, mysql_auth_sz, getenv("TAGEM_MYSQL_CFG"));
    
    
    {
    
	if(argc == 0){
		compsky::mysql::exec_buffer(mysql_obj, "CALL descendant_tags_id_rooted_from(\"tmp_tagids\", \"'foobar'\")");
		compsky::mysql::exec_buffer(mysql_obj, "INSERT IGNORE INTO tmp_tagids (node) SELECT id FROM tag");
	} else {
		compsky::mysql::exec(mysql_obj, BUF, "CALL descendant_tags_id_rooted_from(\"tmp_tagids\", \"'", _f::start, "','", 3, argv, argc, _f::end, "'\")");
	}
	
    if (not_subtags.size() != 0){
        compsky::mysql::exec(
			mysql_obj,
			BUF,
			"CALL descendant_tags_id_rooted_from(\"tmp", 'D', "tagids\", \"'",
				_f::start, "','", 3, not_subtags.data(), not_subtags.size(), _f::end,
			")"
		);
        
        compsky::mysql::exec_buffer(mysql_obj, "DELETE FROM tmp_tagids WHERE node in (SELECT node FROM tmpDtagids)");
    }
    
    }
    
	compsky::mysql::query_buffer(
		mysql_obj,
		mysql_res,
		(root_tags)
			? "SELECT t.name, C.fp, C.x, C.y, C.w, C.h FROM tag t JOIN(SELECT name as fp,root,x,y,w,h FROM file JOIN(SELECT file_id,root,x,y,w,h FROM instance JOIN(SELECT instance_id,root FROM instance2tag JOIN tmp_tagids tt ON tt.node=tag_id)A ON A.instance_id = id) B ON B.file_id=id)C ON C.root = id GROUP BY root, t.name, C.fp, C.x, C.y, C.w, C.h"
			: "SELECT t.name, C.fp, C.x, C.y, C.w, C.h FROM tag t JOIN(SELECT name as fp,tag_id,x,y,w,h FROM file JOIN(SELECT file_id,tag_id,x,y,w,h FROM instance JOIN(SELECT instance_id,tag_id FROM instance2tag WHERE tag_id IN(SELECT node FROM tmp_tagids))A ON A.instance_id=id) B ON B.file_id=id)C ON C.tag_id=id"
	);
    
    char* name;
    char* fp;
    
    constexpr static const compsky::asciify::flag::guarantee::BetweenZeroAndOneInclusive f;
    double x, y, w, h;
	int i = 0;
    while(compsky::mysql::assign_next_row(mysql_res, &mysql_row, &name, &fp, f, &x, f, &y, f, &w, f, &h))
        view_img(name, fp, x, y, w, h, ++i);
    
    compsky::mysql::wipe_auth(mysql_auth, mysql_auth_sz);
    
    return 0;
}
