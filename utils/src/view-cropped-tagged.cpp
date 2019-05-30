#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp> // for cv::imshow
#include <stdio.h> // for printf

#include "utils.hpp" // for asciify
#include "mymysql.hpp" // for mymysql::*, BUF, BUF_INDX

namespace res1 {
    #include "mymysql_results.hpp" // for ROW, RES, COL, ERR
}


void view_img(const char* tag,  const char* fp,  const double x,  const double y,  const double w,  const double h){
    cv::Mat orig_img = cv::imread(fp);
    
    if (orig_img.cols == 0)
        // File does not exist
        return;
    
    const double W = orig_img.cols - 0.5d;
    const double H = orig_img.rows - 0.5d;
    
    const int newX = x*W;
    const int newY = y*H;
    const int newW = (x + w > 1.0d) ? W - newX : w*W;
    const int newH = (y + h > 1.0d) ? H - newY : h*H;
    
    /* Debugging */
    BUF_INDX = 0;
    int u2fz = 2;
    asciify(tag, '\t', x, u2fz, ',', y, u2fz, '\t', w, u2fz, 'x', h, u2fz, '\t', "@\t", newX, ',', newY, 'x', newW, ',', newH, "\tfrom\t", orig_img.cols, 'x', orig_img.rows, fp, '\n', '\0');
    printf("%s", BUF);
    
    cv::Rect rect(newX, newY, newW, newH);
    cv::Mat img = orig_img(rect);
    
    cv::imshow("Cropped Section", img); // Window name is constant so that it is reused (rather than spawning a new window for each image)
    cv::waitKey(0);
}

int main(const int argc, const char** argv) {
    /*
    USAGE
      Non-rooted:
        ./view-cropped-tagged [MYSQL_CFG_FILE] TAG1 TAG2 ... TAGN
      Rooted:
        ./view-cropped-tagged [MYSQL_CFG_FILE] -r TAG1 TAG2 ... TAGN
      
    OPTIONS
        -r
            Descendant tags count as their heirarchical root tag
        -D [TAG]
            Ignore the following descendant tag of one of the specified tags
            NOTE: Does not ignore instances tagged with this tag, but ensures that, if this subtag is the only subtag of a specified tag, the instance is not added.
            E.g. if we have three instances, one tagged "Oak", the other tagged "Pine", and another tagged both "Tree" and "Pine", with "Pine" and "Oak" inheriting from "Tree", and we run the command `./view-cropped-tagged -D Oak "${MYSQL_CFG_FILE}" Tree`, it would display the first and third instances only.
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
    
    mymysql::init(argv[++arg_n]);
    
    StartConcatWithApostrapheAndCommaFlag start_cwaacf;
    EndConcatWithApostrapheAndCommaFlag end_cwaacf;
    
    constexpr const char* a = "CALL descendant_tags_id_rooted_from(\"tmp_tagids\", \"";
    mymysql::exec(a, start_cwaacf, argv+2, argc-2, end_cwaacf, "\")");
    
    
    if (not_subtags.size() != 0){
        BUF_INDX = strlen(a);
        
        BUF[strlen("CALL descendant_tags_id_rooted_from(\"tmp_")-1] = 'D'; // Replace '_' with 'D', i.e. "tmp_tagids" -> "tmpDtagids"
        
        mymysql::exec(/* a already included in BUF */ start_cwaacf, not_subtags, not_subtags.size(), end_cwaacf, ')');
        
        mymysql::exec("DELETE FROM tmp_tagids WHERE node in (SELECT node FROM tmpDtagids)");
    }
    
    
    if (root_tags)
        res1::query("SELECT t.name, C.fp, C.x, C.y, C.w, C.h FROM tag t JOIN(SELECT name as fp,root,x,y,w,h FROM file JOIN(SELECT file_id,root,x,y,w,h FROM instance JOIN(SELECT instance_id,root FROM instance2tag JOIN tmp_tagids tt ON tt.node=tag_id)A ON A.instance_id = id) B ON B.file_id=id)C ON C.root = id GROUP BY root, t.name, C.fp, C.x, C.y, C.w, C.h");
    else
        // NOTE: This query will include duplicates, if an instance is tagged with multiple distinct tags in tmp_tagids (i.e. multiple tags inheriting from the tags we asked for)
        res1::query("SELECT t.name, C.fp, C.x, C.y, C.w, C.h FROM tag t JOIN(SELECT name as fp,tag_id,x,y,w,h FROM file JOIN(SELECT file_id,tag_id,x,y,w,h FROM instance JOIN(SELECT instance_id,tag_id FROM instance2tag WHERE tag_id IN(SELECT node FROM tmp_tagids))A ON A.instance_id = id) B ON B.file_id=id)C ON C.tag_id = id");
    
    char* name;
    char* fp;
    DoubleBetweenZeroAndOne zao_x(0.0), zao_y(0.0), zao_w(0.0), zao_h(0.0);
    while(res1::assign_next_result(&name, &fp, &zao_x, &zao_y, &zao_w, &zao_h))
        view_img(name, fp, zao_x.value, zao_y.value, zao_w.value, zao_h.value);
    
    return 0;
}
