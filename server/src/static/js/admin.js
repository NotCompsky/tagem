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

var $$$users_tags_dict;

function $$$toggle_admin_dashboard(){
	if(!$$$logged_in())
		return;
	$$$hide_all_except(['admin-dashboard']);
	$$$ajax_GET_w_JSON_response("/a/users.json",function(data){
		[$$$users_dict, $$$users_tags_dict] = data;
	});
}


function $$$recursively_record_filesystem_dir_dialog(){
	$$$toggle('record-fs-dir');
}

function $$$recursively_record_filesystem_dir(){
	const s = $$$prompt("!!!MACRO!!!PATH_OF_DIRECTORY_EXAMPLE") !!!MACRO!!!JS__REPLACE_PATH_SEP;
	if((s==="")||(s===null))
		return;
	const max_depth = $$$get_int_with_prompt("Maximum depth - 0 being 'no limit'");
	if (max_depth === null)
		return;
	let tag_ids = $$$select3__get_csv($$$document_getElementById("tagselect-record-fs-dir"));
	if (tag_ids === "")
		tag_ids = "0";
	$$$ajax_POST_data_w_text_response(
		"/record-local-dir/"+max_depth+"/"+tag_ids,
		s,
		function(){
			$$$hide('record-fs-dir');
		}
	);
}

function $$$generate_thumbs(){
	$$$ajax_POST_w_text_response_generic_success("/gen-thumbs/");
}

function $$$update_video_metadatas(){
	$$$ajax_POST_w_text_response_generic_success("/update-vid-metas/");
}

function $$$guess_mimetypes(){
	$$$ajax_POST_w_text_response_generic_success("/guess-mimes/");
}

function $$$get_id_of_user_currently_being_edited(){
	return $$$select3__get_csv($$$document_getElementById('select-user-for-edit'));
}

function $$$user_tag_bl_row(id,name){
	return "<div class='tr' data-id='"+id+"'><div class='td username'>"+name+"</div><div class='td' onclick='$$$rm_user_bl_tag(this.parentNode)'>Remove</div></div>";
}

function $$$load_user_for_edit(user_id_name_tpls){
	const [name,vals,bl] = $$$users_dict[$$$get_id_of_user_currently_being_edited()];
	$$$document_getElementById("edit-user-permissions-username").innerText = name;
	let s = "";
	const ls = $$$get_tbl_body("edit-user-permissions-tbl").getElementsByClassName("tr");
	const vals_ls = vals.split(",");
	for(let i=0, n=vals_ls.length;  i<n;  ++i){
		ls[i].getElementsByClassName('td')[1].textContent = (vals_ls[i]==="1")?"TRUE":"FALSE";
	}
	
	s = "";
	for(let id of $$$split_on_commas_but_make_empty_if_empty(bl)){
		s += $$$user_tag_bl_row(id,$$$users_tags_dict[id][0]);
	}
	$$$get_tbl_body("edit-user-tag-bl-tbl").innerHTML = s;
}

function $$$edit_permission_value(e){
	const par = e.parentNode.parentNode;
	const field_name = par.childNodes[0].textContent;
	const b = $$$confirm("Allow "+$$$users_dict[$$$get_id_of_user_currently_being_edited()][0]+" access to "+field_name+"?");
	$$$ajax_POST_data_w_text_response_generic_success("/user/permission/"+$$$get_id_of_user_currently_being_edited()+"/"+(b?"1":"0"), field_name);
	par.childNodes[1].textContent = b?"TRUE":"FALSE";
}

function $$$toggle_edit_user_thing(id){
	if($$$is_visible(id)){
		$$$hide(id);
		return;
	}
	$$$unhide(id);
}

function $$$rm_user_bl_tag(e){
	$$$ajax_POST_w_text_response_generic_success("/user/bl/t-/"+$$$get_id_of_user_currently_being_edited()+"/"+e.dataset.id);
	e.remove();
}

function $$$add_new_user_tag_blacklist_rule(){
	const node = $$$document_getElementById('tagselect-useredit');
	const tags = $$$select3__get_csv(node);
	if(tags==="")
		return;
	$$$ajax_POST_w_text_response("/user/bl/t+/"+$$$get_id_of_user_currently_being_edited()+"/"+tags, function(){
		let s = "";
		for(let x of $$$select3__get_dict(node))
			s += $$$user_tag_bl_row(x.id,x.text);
		$$$get_tbl_body("edit-user-tag-bl-tbl").innerHTML += s;
		$$$select3__wipe_values(node);
	});
}

function $$$add_user(){
	const name = $$$prompt("User name");
	if((name===null)||(name===""))
		return;
	$$$ajax_POST_data_w_text_response_generic_success("/user/+",name);
}
