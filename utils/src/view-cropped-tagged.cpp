#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp> // for cv::imshow
#include <stdio.h> // for printf

#include "sql_utils.hpp" // for mysu::*

char STMT[strlen("CALL descendant_tags_id_rooted_from(\"tmp_tagids\", \"") + 1024 + strlen("\")") + 1];

void view_img(){
    std::string tag = SQL_RES->getString(1);
    std::string fp = SQL_RES->getString(2);
    const double x = SQL_RES->getDouble(3);
    const double y = SQL_RES->getDouble(4);
    const double w = SQL_RES->getDouble(5);
    const double h = SQL_RES->getDouble(6);
    
    cv::Mat orig_img = cv::imread(fp.c_str());
    
    const double W = orig_img.cols - 0.5d;
    const double H = orig_img.rows - 0.5d;
    
    const int newX = x*W;
    const int newY = y*H;
    const int newW = (x + w > 1.0d) ? W - newX : w*W;
    const int newH = (y + h > 1.0d) ? H - newY : h*H;
    
    printf("%lf %lf  %lf %lf\n", x, y, w, h);
    printf("%d %d\n", orig_img.cols, orig_img.rows);
    printf("%s at %d,%d,%d,%d\tfrom %dx%d %s\n", tag.c_str(), newX, newY, newW, newH, orig_img.cols, orig_img.rows, fp.c_str());
    
    cv::Rect rect(newX, newY, newW, newH);
    cv::Mat img = orig_img(rect);
    
    cv::imshow("Cropped Section", img); // Window name is constant so that it is reused (rather than spawning a new window for each image)
    cv::waitKey(0);
}

int main(int argc, char** argv) {
    /*
    USAGE
      Non-rooted:
        ./make_image_db TAG1 TAG2 ... TAGN
      Rooted:
        ./make_image_db -r TAG1 TAG2 ... TAGN
      
    OPTIONS
        -r
            Descendant tags count as their heirarchical root tag
        -D [TAG]
            Ignore all instances of the following descendant tag
        -f [TAG]
            Only include files tagged with the following
        -F [TAG]
            Ignore files tagged with the following
    */
    int arg_n = 0;
    
    bool root_tags = false;
    std::vector<const char*> not_subtags;
    not_subtags.reserve(10);
    
    while (true){
        const char* arg = argv[++arg_n];
        if (arg[0] != '-')
            break;
        
        switch(arg[1]){
            case 'r': root_tags = true; break;
            case 'D': not_subtags.push_back(argv[++arg_n]); break;
            default: break;
        }
    }
    --arg_n; // For consistency.
    
    mysu::init(argv[++arg_n], "mytag");
    
    int i = 0;
    constexpr const char* a = "CALL descendant_tags_id_rooted_from(\"tmp_tagids\", \"";
    constexpr const char* b = "\")";
    
    memcpy(STMT + i,  a,  strlen(a));
    i += strlen(a);
    
    while(++arg_n < argc){
        const char* arg = argv[arg_n];
        STMT[i++] = '\'';
        memcpy(STMT + i,  arg,  strlen(arg));
        i += strlen(arg);
        STMT[i++] = '\'';
        STMT[i++] = ',';
    }
    --i; // Overwrite trailing comma
    
    memcpy(STMT + i,  b,  strlen(b));
    i += strlen(b);
    
    STMT[i] = 0;
    
    SQL_STMT->execute(STMT);
    
    
    if (not_subtags.size() != 0){
        i = 0;
        
        memcpy(STMT + i,  a,  strlen(a));
        i += strlen(a);
        
        STMT[strlen("CALL descendant_tags_id_rooted_from(\"tmp_")-1] = 'D'; // Replace '_' with 'D'
        
        for (auto j = 0;  j < not_subtags.size();  ++j){
            const char* arg = not_subtags[j];
            STMT[i++] = '\'';
            memcpy(STMT + i,  arg,  strlen(arg));
            i += strlen(arg);
            STMT[i++] = '\'';
            STMT[i++] = ',';
        }
        --i; // Overwrite trailing comma
        
        memcpy(STMT + i,  b,  strlen(b));
        i += strlen(b);
        
        STMT[i] = 0;
        
        SQL_STMT->execute(STMT);
        
        SQL_STMT->execute("DELETE FROM tmp_tagids WHERE node in (SELECT node FROM tmpDtagids)");
    }
    
    
    if (root_tags)
        SQL_RES = SQL_STMT->executeQuery("SELECT t.name, C.fp, C.x, C.y, C.w, C.h FROM tag t JOIN(SELECT name as fp,root,x,y,w,h FROM file JOIN(SELECT file_id,root,x,y,w,h FROM instance JOIN(SELECT instance_id,root FROM instance2tag JOIN tmp_tagids tt ON tt.node=tag_id)A ON A.instance_id = id) B ON B.file_id=id)C ON C.root = id GROUP BY root, t.name, C.fp, C.x, C.y, C.w, C.h");
    else
        // NOTE: This query will include duplicates, if an instance is tagged with multiple distinct tags in tmp_tagids (i.e. multiple tags inheriting from the tags we asked for)
        SQL_RES = SQL_STMT->executeQuery("SELECT t.name, C.fp, C.x, C.y, C.w, C.h FROM tag t JOIN(SELECT name as fp,tag_id,x,y,w,h FROM file JOIN(SELECT file_id,tag_id,x,y,w,h FROM instance JOIN(SELECT instance_id,tag_id FROM instance2tag WHERE tag_id IN(SELECT node FROM tmp_tagids))A ON A.instance_id = id) B ON B.file_id=id)C ON C.tag_id = id");
    
    while (SQL_RES->next())
        view_img();
    
    return 0;
}
