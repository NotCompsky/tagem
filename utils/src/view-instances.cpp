#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp> // for cv::imshow
#include <stdio.h> // for printf

#include <compsky/asciify/init.hpp>
#include <compsky/asciify/asciify.hpp> // for asciify
#include <compsky/mysql/mysql.hpp> // for compsky::mysql::*, BUF, BUF_INDX
#include <compsky/mysql/query.hpp> // for ROW, RES, COL, ERR

#include <sys/stat.h> // for mkdir


MYSQL_RES* RES;
MYSQL_ROW ROW;

namespace compsky {
	namespace asciify {
		char* BUF;
		char* ITR;
	}
}

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
        compsky::asciify::reset_index();
        compsky::asciify::asciify("Bad crop:\t", tag, '\t', x, u2fz, ',', y, u2fz, '\t', w, u2fz, 'x', h, u2fz, '\t', "@\t", newX, ',', newY, ' ', newW, 'x', newH, "\tfrom\t", orig_img.cols, 'x', orig_img.rows, '\t', fp, '\n');
        fwrite(compsky::asciify::BUF, 1, compsky::asciify::get_index(), stderr);
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
        -f [TAG]
            Only include files tagged with the following
        -F [TAG]
            Ignore files tagged with the following
    */

	if(compsky::asciify::alloc(1024))
		return 4;
    
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
            case 'r': root_tags = true; break;
            case 'D': not_subtags.push_back(*(++argv)); --argc; break;
			case 's': { const size_t len = strlen(*(++argv));  memcpy(SAVE_FILES_TO, *argv, len);  SAVE_FILES_TO_END = SAVE_FILES_TO + len; --argc; }  break;
            default: break;
        }
    }
    
    compsky::mysql::init(getenv("TAGEM_MYSQL_CFG"));
    
    
    {
    
    constexpr static const compsky::asciify::flag::concat::Start f_start;
    constexpr static const compsky::asciify::flag::concat::End f_end;
	if(argc == 0){
		compsky::mysql::exec_buffer("CALL descendant_tags_id_rooted_from(\"tmp_tagids\", \"'foobar'\")");
		compsky::mysql::exec_buffer("INSERT IGNORE INTO tmp_tagids (node) SELECT id FROM tag");
	} else {
		compsky::asciify::asciify("CALL descendant_tags_id_rooted_from(\"tmp_tagids\", \"'", f_start, "','", 3, argv, argc, f_end, "'\")");
		compsky::mysql::exec_buffer(compsky::asciify::BUF, compsky::asciify::get_index());
	}
	
    if (not_subtags.size() != 0){
        compsky::asciify::BUF[strlen("CALL descendant_tags_id_rooted_from(\"tmp_")-1] = 'D'; // Replace '_' with 'D', i.e. "tmp_tagids" -> "tmpDtagids"
        
        compsky::asciify::asciify(/* a already included in BUF */ f_start, "','", 3, not_subtags.data(), not_subtags.size(), f_end, ')');
        
        compsky::mysql::exec_buffer(compsky::asciify::BUF, compsky::asciify::get_index());
        
        compsky::mysql::exec_buffer("DELETE FROM tmp_tagids WHERE node in (SELECT node FROM tmpDtagids)");
    }
    
    }
    
    if (root_tags)
        compsky::mysql::query_buffer(&RES, "SELECT t.name, C.fp, C.x, C.y, C.w, C.h FROM tag t JOIN(SELECT name as fp,root,x,y,w,h FROM file JOIN(SELECT file_id,root,x,y,w,h FROM instance JOIN(SELECT instance_id,root FROM instance2tag JOIN tmp_tagids tt ON tt.node=tag_id)A ON A.instance_id = id) B ON B.file_id=id)C ON C.root = id GROUP BY root, t.name, C.fp, C.x, C.y, C.w, C.h");
    else
        // NOTE: This query will include duplicates, if an instance is tagged with multiple distinct tags in tmp_tagids (i.e. multiple tags inheriting from the tags we asked for)
        compsky::mysql::query_buffer(&RES, "SELECT t.name, C.fp, C.x, C.y, C.w, C.h FROM tag t JOIN(SELECT name as fp,tag_id,x,y,w,h FROM file JOIN(SELECT file_id,tag_id,x,y,w,h FROM instance JOIN(SELECT instance_id,tag_id FROM instance2tag WHERE tag_id IN(SELECT node FROM tmp_tagids))A ON A.instance_id=id) B ON B.file_id=id)C ON C.tag_id=id");
    
    char* name;
    char* fp;
    
    constexpr static const compsky::asciify::flag::guarantee::BetweenZeroAndOneInclusive f;
    double x, y, w, h;
	int i = 0;
    while(compsky::mysql::assign_next_row(RES, &ROW, &name, &fp, f, &x, f, &y, f, &w, f, &h))
        view_img(name, fp, x, y, w, h, ++i);
    
    compsky::mysql::exit_mysql();
    
    return 0;
}
