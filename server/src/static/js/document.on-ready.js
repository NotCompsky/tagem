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
	// Initialise the variables from globals.js - which may be required in later steps within this function
	$$$document_getElementById_username = $$$document_getElementById("username");
	$$$document_getElementById_view = $$$document_getElementById("view");
	$$$document_getElementById_values = $$$document_getElementById("values");
	$$$dirselect = $$$document_getElementById("dirselect");
	$$$document_getElementById_f = $$$document_getElementById("f");
	$$$document_getElementById_t = $$$document_getElementById("t");
	$$$document_getElementById_d = $$$document_getElementById("d");
	$$$document_getElementById_cmnts = $$$document_getElementById("cmnts");
	$$$document_getElementById_autoplay = $$$document_getElementById("autoplay");
	$$$document_getElementById_descr = $$$document_getElementById("descr");
	$$$document_getElementById_tasks = $$$document_getElementById("tasks");
	$$$document_getElementById_qry = $$$document_getElementById("qry");
	
	$$$parent_tags_ls = $$$document_getElementById("parents");
	$$$child_tags_ls = $$$document_getElementById("children");
	$$$sibling_tags_ls = $$$document_getElementById("siblings");
	
	$$$document_getElementById_audio_only = $$$document_getElementById("audio-only");
	$$$document_getElementById_add_f_backup_ytdl = $$$document_getElementById("add-f-backup-ytdl");
	$$$document_getElementById_profile_name = $$$document_getElementById("profile-name");
	$$$document_getElementById_view_img = $$$document_getElementById("view-img");
	$$$document_getElementById_profile_img = $$$document_getElementById("profile-img");
	$$$document_getElementById_post_user = $$$document_getElementById("post-user");
	$$$document_getElementById_add_f_backup_toggle = $$$document_getElementById("add-f-backup-toggle");
	$$$text_edit = $$$document_getElementById("text-edit");
	$$$view_btns = $$$document_getElementById("view-btns");
	$$$document_getElementById_eras_info_tbody = $$$document_getElementById("eras-info-tbody");
	$$$recent_pages = $$$document_getElementById("recent-pages");
	
	$$$tagselect_popup_container = $$$document_getElementById("tagselect-era-container");
	$$$tagselect_popup_btn = $$$document_getElementById("tagselect-era-btn");
	
	$$$jquery_dirselect = $('#dirselect');
	
	$$$css_root = document.querySelector(':root').style;
	$$$document_body = $$$document_getElementsByTagName("body")[0];
	
	$.ajaxPrefilter($$$ajax_prefilter);
	
	$$$hide_all_except([]);
	
	const uname = $$$get_cookie("username");
	$$$document_getElementById_username.innerText = (uname === undefined)?"GUEST":uname;
	
	$$$refetch_all_jsons();
	$$$init_selects__ajax('d');
	$$$init_selects__ajax('t');
	$$$init_selects('$$$users_dict');
	$$$document_getElementById("select-user-for-edit").addEventListener("change", $$$load_user_for_edit);
	
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
	
	const thm = $$$local_storage_get("css-theme") || "dark";
	$$$set_css_theme(thm);
	$$$document_getElementById("css-theme").value = thm;
	for(let opt of document.getElementById("my-settings").getElementsByClassName("css-colour-picker")){
		$$$set_css_colour(opt.dataset.for,$$$local_storage_get("css_colour_"+opt.dataset.for));
	}
	
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
	
	$$$document_getElementById_view_img.addEventListener('mousedown', $$$mousedown_on_image);
	
	$$$get_descr_obj().addEventListener("mousedown", $$$mousedown_on_descr);
	$$$get_descr_obj().addEventListener("mouseleave", $$$mouseleave_descr);
}
