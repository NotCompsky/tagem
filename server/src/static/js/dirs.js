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

function $$$set_profile_name_from_this_dir(){
	$$$set_profile_name($$$dir_name);
}
function $$$display_related_dirs(dir_id,rel,arr){
	$$$document_getElementById(rel).innerHTML = arr.filter(x => x[0]!==dir_id).map(([id,name]) => "<a onclick=\"$$$view_dir('" + id + "',0)\">" + $$$escape_html_text(name) + "</a>").join("<br/>");
}


function $$$view_dir(_dir_id_or_path, is_not_in_db, page){
	if(_dir_id_or_path===null)
		// From prompt
		return;
	if(_dir_id_or_path==="")
		_dir_id_or_path=$$$dir_id;
	if(page===undefined)
		page = 0;
	let ls = ['f','f-action-btns','tags-container','parents-container','children-container','tagselect-dirs-container','tagselect-files-container','tagselect-files-btn'];
	if(!is_not_in_db){
		ls.push('merge-files-btn');
		ls.push('backup-files-btn');
		ls.push('view-as-playlist-btn');
	}
	$$$hide_all_except(ls);
	
	$$$file_tagger_fn = $$$after_tagged_selected_files;
	$$$dir_tagger_fn = $$$after_tagged_this_dir;
	$$$select2_ids = $$$select2_ids_default;
	$$$get_file_ids = $$$get_selected_file_ids;
	$$$get_dir_ids = $$$get_this_dir_id;

	if (_dir_id_or_path !== undefined){
		if(is_not_in_db){
			$$$dir_id = "0"; // $$$dir_id will be set by the call to populate_f_table
			$$$populate_f_table(':', null, _dir_id_or_path,page);
			$$$set_window_location_hash(':'+page+'/'+_dir_id_or_path);
		}else{
			$$$dir_id = _dir_id_or_path;
			$$$ajax_GET_w_JSON_response("a/d/i/"+$$$dir_id, function(data){
				$$$dir_name = data[0];
				$$$display_related_dirs($$$dir_id,"parents",data[1]);
				$$$display_related_dirs($$$dir_id,"children",data[2]);
				$$$display_tags(data[3], "tags", "$$$unlink_this_tag_from_this", 'd');
				$$$set_profile_name_from_this_dir();
			});
			$$$populate_f_table('d', $$$dir_id, null, page);
			$$$set_window_location_hash('d'+page+'/'+$$$dir_id);
		}
	}else{
		// Returning to directory listing via "Dir" tab/button
		$$$set_window_location_hash('d'+page+'/'+$$$dir_id);
		// BUG: This switches :PAGE/directory/path/ to dPAGE/DIR_ID
	}
	
	if(is_not_in_db){
		$$$set_profile_name(_dir_id_or_path);
	}else if(_dir_id_or_path===undefined){
		$$$set_profile_name_from_this_dir();
	}
	$$$set_profile_thumb('d');
}
function $$$get_selected_dir_ids(){
	return $$$get_selected_ids('d');
}
function $$$get_this_dir_id(){
	return $$$dir_id;
}

function $$$after_tagged_this_dir(ids, tags){
	$$$display_tags_add(tags, 'tags', 'd')
}
function $$$after_tagged_selected_dirs(ids, tags){
	$$$after_tagged_selected_stuff('d',ids,tags,3);
}

function $$$display_d_tbl(data){
	let s = "";
	$$$get_dir_ids = $$$get_selected_dir_ids;
	for (const [id, name, device, tag_ids, size] of data[0]){
		s += "<div class='tr' data-id='" + id + "'>";
			s += "<div class='td'>";
				s += "<a onclick='$$$view_dir(this.parentNode.parentNode.dataset.id,0)'>" + $$$escape_html_text(name) + "</a>";
			s += "</div>";
			s += "<div class='td'>";
				s += "<a onclick='$$$view_device(" + device + ")'>Device</a>";
			s += "</div>";
			s += "<div class='td dir-size'>" + size + "</div>";
			s += "<div class='td'>" + tag_ids + "</div>";
		s += "</div>";
	}
	$$$get_tbl_body("d").innerHTML = s;
}

function $$$view_dirs(ids){
	$$$ajax_GET_w_JSON_response(
		"a/d/id/0/"+ids.join(","),
		$$$display_d_tbl
	);
}

function $$$setup_page_for_d_tbl(){
	$$$hide_all_except(['d']);
	$$$dir_tagger_fn = $$$after_tagged_selected_dirs;
	$$$select2_ids = $$$select2_ids_default;
	$$$get_dir_ids = $$$get_selected_dir_ids;
}

function $$$add_dirs_dialog(){
	$$$hide_all_except(['tagselect-files-container', 'add-d-dialog']);
}
