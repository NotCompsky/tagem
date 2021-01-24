// 
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
// 

function $$$ask_user_to_input_tags(fn){
	function a(){
		fn($$$select3__get_dict($$$tagselect_popup));
	}
	$$$tagselect_popup_btn.addEventListener("click", a, {once:true});
	$$$unhide_node($$$tagselect_popup_container);
}

function $$$tag_stuff_then(alias, ids, btn_node, fn){
	// alias is either file 'f', dir 'd', or device 'D'
	const tagselect_node = btn_node.parentNode.getElementsByTagName("input")[0];
	const tags = tagselect_node.dataset.x;
	if(ids==="")
		return;
	if(!$$$logged_in())
		return $$$alert_requires_login();
	$$$ajax_POST_w_text_response(
		alias + "/t/" + ids + "/" + tags,
		function(){
			$$$select3__wipe_values(tagselect_node);
			fn(ids, tags);
		}
	);
}
function $$$after_tagged_selected_stuff(alias, ids, tags, col){
	// col is the index of the tag column
	for(let node of $$$document_getElementById(alias).getElementsByClassName('tr')){
		if(!ids.includes(node.dataset.id))
			continue;
		let _s = "";
		for(tag of tags)
			_s += $$$link_to_named_fn_w_param_id_w_human_name("$$$view_tag",tag.id,tag.text);
		node.getElementsByClassName('td')[col].innerHTML += _s;
	}
}

function $$$display_t_tbl(data){
	$$$unhide('tagselect-self-p');
	let s = "";
	for (const [id, name, thumb, size] of data){
		s += "<div class='tr' data-id=\"" + id + "\">";
			s += "<div class='td thumbnail' onclick='$$$view_tag(\"" + id + "\",0)'>";
				s += "<img src='" + thumb + "' onerror='$$$set_src_to_tag_svg(this)'/>";
			s += "</div>";
			s += "<div class='td'>";
				s += $$$escape_html_text(name);
			s += "</div>";
			s += "<div class='td dir-size'>" + size + "</div>";
		s += "</div>";
	}
	$$$get_tbl_body('t').innerHTML = s;
}

function $$$populate_t_id2name_table(arr){
	if (arr.length===0){
		$$$get_tbl_body('t').innerHTML = "";
		return;
	}
	$$$ajax_data_w_JSON_response(
		"GET",
		"a/t/id/0/"+arr.join(","),
		null,
		$$$display_t_tbl
	);
}

function $$$unlink_this_tag_from_this(node,alias,relation){
	// alias is either 'f' for file, or 'd' for directory, or 'D' for device
	if(!$$$logged_in())
		return $$$alert_requires_login();
	if(relation===undefined)
		// alias is 'f' or 'd' or 'D', and the relation can only be to a tag
		// Whereas a tag can have a relation only to a parent or child tag
		relation='t';
	$$$ajax_POST_w_text_response(
		"/"+alias+"/"+relation+"-/"+$$$tbl_alias_to_id_value(alias)+"/"+node.parentNode.dataset.id,
		function(){
			node.parentNode.classList.add("hidden");
		}
	);
}
function $$$unlink_tag_from_this_tag(node,relation){
	$$$unlink_this_tag_from_this(node,'t',relation);
}
function $$$unlink_this_parent_tag_from_this_tag(node){
	$$$unlink_tag_from_this_tag(node,'p');
}
function $$$unlink_this_child_tag_from_this_tag(node){
	$$$unlink_tag_from_this_tag(node,'c');
}
function $$$unlink_this_sibling_tag_from_this_tag(node){
	$$$unlink_tag_from_this_tag(node,'s');
}
function $$$display_tag(id, name, thumb, fn_name, alias){
	return "<div class='tag' data-id=\"" + id + "\">"
			+ ((thumb===null)?"":"<img src='" + ((thumb===null)?$$$BLANK_IMG_SRC:thumb) + "' class='icon'/>")
			+ "<a onclick='$$$view_tag(\"" + id + "\")'>" + $$$escape_html_text(name) + "</a>"
			+ "<a class=\"del\" onclick=\"" + fn_name + "(this,'" + alias + "')\">[-]</a>"
		+ "</div>";
}
function $$$display_tags_onto_node_from_url(url,node,fn_name,alias){
	if(url==="id/0/"){
		node.innerHTML = "";
		return;
	}
	$$$ajax_data_w_JSON_response(
		"GET",
		"a/t/"+url,
		null,
		function(data){
			let s = "";
			for (const [id, name, thumb, size] of data){
				s += $$$display_tag(id, name, thumb, fn_name, alias);
			}
			node.innerHTML = s;
		}
	);
}
function $$$display_tags_from_url(url,node,fn_name,alias){
	$$$display_tags_onto_node_from_url(url,node,fn_name,alias);
}
function $$$display_tags_onto_node(tag_ids, node, fn_name, alias){
	if(tag_ids.length===0){
		node.innerHTML = "";
		return;
	}
	$$$display_tags_onto_node_from_url("id/0/"+tag_ids.join(","),node,fn_name,alias);
}
function $$$display_tags(tag_ids, node_id, fn_name, alias){
	$$$display_tags_onto_node(tag_ids,$$$document_getElementById(node_id),fn_name,alias);
}
function $$$display_tags_add(tags, node_id, fn_name, alias){
	// TODO: Have server return tag dictionary in response to tagging a file
	const arr = tags.map(x => $$$display_tag(x.id, x.text, null, null, fn_name, alias));
	$$$document_getElementById(node_id).innerHTML += arr.join("");
}
function $$$display_parent_tags(_tag_id){
	$$$display_tags_from_url("p/0/"+_tag_id,$$$parent_tags_ls, "$$$unlink_this_parent_tag_from_this_tag");
}
function $$$display_child_tags(_tag_id){
	$$$display_tags_from_url("c/0/"+_tag_id,$$$child_tags_ls,  "$$$unlink_this_child_tag_from_this_tag");
}
function $$$display_sibling_tags(_tag_id){
	$$$display_tags_from_url("s/0/"+_tag_id,$$$sibling_tags_ls,"$$$unlink_this_sibling_tag_from_this_tag");
}

// Functions used in HTML
function $$$add_related_tags(rel1,rel2,reversed,relfn){
	const tagselect = $$$document_getElementById('tagselect-self-'+rel1);
	$$$ajax_POST_w_text_response(
		"/t/"+rel2+"/" + (
			reversed
			? ($$$select3__get_csv(tagselect) + "/" + $$$get_tag_ids())
			: ($$$get_tag_ids() + "/" + $$$select3__get_csv(tagselect))
		),
		function(){
			const ls = $$$split_on_commas__guaranteed_nonempty($$$get_tag_ids());
			if($$$get_tag_ids!==$$$get_this_tag_id)
				// if not currently displaying the tag page
				return;
			relfn(ls);
			$$$select3__wipe_values(tagselect);
		}
	);
}
function $$$add_child_tags(){
	$$$add_related_tags('c','p',1,$$$display_child_tags);
}
function $$$add_sibling_tags(){
	$$$add_related_tags('s','s',0,$$$display_sibling_tags);
}
function $$$add_parent_tags(){
	$$$add_related_tags('p','p',0,$$$display_parent_tags);
}

function $$$update_tag_thumb(){
	const url = prompt("New Thumbnail URL", "");
	if(url==="" || url===null)
		return;
	$$$ajax_POST_w_text_response(
		"/t/thumb/" + $$$tag_id + "/" + url,
		function(){
			$$$set_profile_thumb(url);
		}
	);
}

function $$$get_this_tag_id(){
	return $$$tag_id;
}
function $$$get_selected_tag_ids(){
	// Identical to $$$get_selected_file_ids, save for a different table ID, and not checking for IDs of 0
	let file_ids = "";
	for(let node of $$$get_tbl_body('t').getElementsByClassName('selected1')){
		file_ids += "," + node.dataset.id
	}
	return file_ids.substr(1);
}

function $$$view_tag(_tag_id,page){
	$$$hide_all_except(['parents-container','siblings-container','children-container','f','d','tagselect-files-container','tagselect-files-btn','tagselect-self-p-container','tagselect-self-p-btn','tagselect-self-s-container','tagselect-self-s-btn','tagselect-self-c-container','tagselect-self-c-btn','merge-files-btn','backup-files-btn','view-as-playlist-btn','meta','t1','t2','descr']);
	$$$document_getElementById_profile_img.onclick = $$$update_tag_thumb;
	
	$$$file_tagger_fn = $$$after_tagged_selected_files;
	$$$get_file_ids = $$$get_selected_file_ids;
	$$$get_tag_ids = $$$get_this_tag_id;
	
	if (_tag_id !== undefined){
		// It is undefined if we are just unhiding the tag view
		$$$tag_id = _tag_id;
		$$$populate_f_table('t', $$$tag_id, null, (page===undefined)?0:page);
		$$$ajax_GET_w_JSON_response(
			"a/d/t/0/"+$$$tag_id,
			$$$display_d_tbl
		);
	} else if ($$$tag_id === undefined)
		return;
	
	$$$ajax_GET_w_JSON_response(
		"a/t/info/"+$$$tag_id,
		function(data){
			const [id,name,thumb,size,t1,t2,descr] = data[0];
			$$$set_profile_name(name);
			$$$set_profile_thumb(thumb);
			$$$set_window_location_hash('t0/'+$$$tag_id);
			$$$set_t_origin(t1);
			$$$set_t_ended(t2);
			$$$set_description(descr);
		}
	);
	
	$$$display_parent_tags($$$tag_id);
	$$$display_sibling_tags($$$tag_id);
	$$$display_child_tags($$$tag_id);
}
function $$$setup_page_for_t_tbl(){
	$$$hide_all_except(['t','f-action-btns','tagselect-self-p-container','tagselect-self-p-btn']);
	$$$unset_window_location_hash();
	$$$get_tag_ids = $$$get_selected_tag_ids;
}

function $$$view_tags(ids){
	$$$ajax_GET_w_JSON_response(
		"a/t/id/0/"+ids.join(","),
		$$$display_t_tbl
	);
}


function $$$add_tags_dialog(){
	$$$hide_all_except(['tagselect-self-p-container','add-t-dialog']);
}
