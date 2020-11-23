// Copyright 2020 Adam Gray
// This file is part of the tagem program.
// The tagem program is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published by the
// Free Software Foundation version 3 of the License.
// The tagem program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// This copyright notice should be included in any copy or substantial copy of the tagem source code.
// The absense of this copyright notices on some other files in this project does not indicate that those files do not also fall under this license, unless they have a different license written at the top of the file.

function $$$on_document_ready(){
	$$$hide_all_except([]);
	
	const uname = $$$get_cookie("username");
	$$$document_getElementById('username').innerText = (uname === undefined)?"GUEST":uname;
	
	$$$refetch_all_jsons();
	$$$init_selects__ajax('d');
	$$$init_selects__ajax('t');
	$$$init_selects('$$$users_dict');
	$($$$document.body).on("change","#select-user-for-edit",$$$load_user_for_edit);
	
	if(!!!MACRO!!!USE_REMOTE_YT_JS__STR){
	// From google's documentation: This code loads the IFrame Player API code asynchronously.
	let node = document.createElement('script');
	node.src = "https://www.youtube.com/iframe_api";
	let firstScriptTag = $$$document_getElementsByTagName('script')[0];
	firstScriptTag.parentNode.insertBefore(node, firstScriptTag);
	}
	
	let css = $$$local_storage_get("css");
	if(css===null)
		css = $$$stylesheet_opts[0];
	$$$switch_stylesheet(css);
	
	$$$use_regex = $$$get_and_set_default_user_setting_tofrom_storage__bool("use_regex",false);
	$$$prioritise_local_autoplay = $$$get_and_set_default_user_setting_tofrom_storage__bool("prioritise_local_autoplay",false);
	$$$get_and_set_default_user_setting_tofrom_storage__tbl_f_hide_col('thumbnail',false);
	$$$get_and_set_default_user_setting_tofrom_storage__tbl_f_hide_col('fname',false);
	$$$get_and_set_default_user_setting_tofrom_storage__tbl_f_hide_col('ftitle',false);
	$$$get_and_set_default_user_setting_tofrom_storage__tbl_f_hide_col('tags',false);
	$$$get_and_set_default_user_setting_tofrom_storage__tbl_f_hide_col('dateadded',true);
	$$$get_and_set_default_user_setting_tofrom_storage__tbl_f_hide_col('date_origin',false);
	$$$get_and_set_default_user_setting_tofrom_storage__tbl_f_hide_col('file_size',false);
	$$$get_and_set_default_user_setting_tofrom_storage__tbl_f_hide_col('duration',false);
	$$$get_and_set_default_user_setting_tofrom_storage__tbl_f_hide_col('w',true);
	$$$get_and_set_default_user_setting_tofrom_storage__tbl_f_hide_col('h',true);
	$$$get_and_set_default_user_setting_tofrom_storage__tbl_f_hide_col('views',true);
	$$$get_and_set_default_user_setting_tofrom_storage__tbl_f_hide_col('likes',true);
	$$$get_and_set_default_user_setting_tofrom_storage__tbl_f_hide_col('dislikes',true);
	$$$get_and_set_default_user_setting_tofrom_storage__tbl_f_hide_col('fps',true);
	
	$$$set_css_colour("fg",$$$local_storage_get("css_colour_fg"));
	$$$set_css_colour("bg",$$$local_storage_get("css_colour_bg"));
	$$$set_css_colour("bg2",$$$local_storage_get("css_colour_bg2"));
	
	for(let [key,val] of [['sleep_on_inanimate_media',2],['sleep_after_media_err',2]]){
		const val2 = $$$local_storage_get(key);
		$$$set_user_setting_as(key,((val2===null)?val:val2));
	}
	
	let w = $$$local_storage_get("w");
	if(w===null)
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
	
	for(node of $$$document_getElementsByClassName('css-colour-picker'))
		node.addEventListener("change",$$$on_css_colour_picker_change);
	
	$$$document_getElementById('view-img').addEventListener('mousedown', $$$mousedown_on_image);
	
	$$$get_descr_obj().addEventListener("mousedown", $$$mousedown_on_descr);
	$$$get_descr_obj().addEventListener("mouseleave", $$$mouseleave_descr);
}
