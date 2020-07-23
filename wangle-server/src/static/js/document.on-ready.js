function $$$on_document_ready(){
	$$$hide_all_except([]);
	
	const uname = $$$get_cookie("username");
	$$$document_getElementById('username').innerText = (uname === undefined)?"GUEST":uname;
	
	$$$refetch_all_jsons();
	$$$init_selects__ajax('d');
	$$$init_selects__ajax('t');
	
	// From google's documentation: This code loads the IFrame Player API code asynchronously.
	let node = document.createElement('script');
	node.src = "https://www.youtube.com/iframe_api";
	let firstScriptTag = $$$document_getElementsByTagName('script')[0];
	firstScriptTag.parentNode.insertBefore(node, firstScriptTag);
	
	let css = $$$get_cookie("css");
	if(css===undefined)
		css = $$$stylesheet_opts[0];
	$$$switch_stylesheet(css);
	
	$$$use_regex = $$$get_and_set_default_user_setting_tofrom_cookie__bool("use_regex",false);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('thumbnail',false);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('fname',false);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('ftitle',false);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('tags',false);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('dateadded',true);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('date_origin',false);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('file_size',false);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('duration',false);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('w',true);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('h',true);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('views',true);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('likes',true);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('dislikes',true);
	$$$get_and_set_default_user_setting_tofrom_cookie__tbl_f_hide_col('fps',true);
	
	for(let [key,val] of [['sleep_on_inanimate_media',2],['sleep_after_media_err',2]]){
		const val2 = $$$get_cookie(key);
		$$$set_user_setting_as(key,((val2===undefined)?val:val2));
	}
	
	let w = $$$get_cookie("w");
	if(w===undefined)
		w=256;
	$$$set_thumbnail_width(w);
	
	$$$make_tbl_selectable("f");
	$$$make_tbl_selectable("t");
	$$$make_tbl_selectable("eras-info");
	$$$init_qry();
	
	for(node of $$$document_getElementsByTagName('input')){
		node.addEventListener('focusin',  $$$del_key_intercepts);
		node.addEventListener('focusout', $$$add_key_intercepts);
	}
	$$$add_key_intercepts();
}
