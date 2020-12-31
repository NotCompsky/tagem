// NOTE: Use lvh.me - it is an alias for localhost, and therefore browsers allow CORS from it
// For some reason, browsers decided that your own machine (localhost) is actually *less* trustworthy than every other machine on the planet (every other server), so requests made from localhost are only allowed to send, and not receive, information. lvh.me works around that genius restriction.


function $$$upload_img_to_yandex(){
	$$$ajax_GET_w_JSON_response("i/reverse/"+$$$get_file_id(), console.log); //$$$display_f_tbl);
}
