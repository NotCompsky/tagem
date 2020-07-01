function $$$populate_t_id2name_table(selector, arr){
	let s = "";
	for (const [id, tpl] of Object.entries($$$t)){
		if ((arr !== null)  &&  (!arr.includes(id)))
			continue;
		s +=
				"<div class='tr'><div class='td'>"
			+ $$$display_tag(id, tpl)
			+ "</div></div>"
		;
	}
	document.querySelector(selector).innerHTML = s;
}
function $$$set_profile_name_from_this_tag(){
	$$$set_profile_name($$$t[$$$tag_id][0]);
}
function $$$set_profile_thumb_from_this_tag(){
	$$$set_profile_thumb($$$t[$$$tag_id][1]);
}
function $$$set_profile_cover_from_this_tag(){
	$$$set_profile_cover($$$t[$$$tag_id][2]);
}

function $$$display_tag(id, tpl){
	return "<div class='user'><img src='" + tpl[1] + "' class='icon'/><div class='username'><a onclick='$$$view_tag(" + id + ")'>" + tpl[0] + "</a></div></div>";
}
function $$$display_tags(tag_ids, selector){
	const arr = tag_ids.map(x => $$$display_tag(x, t[x]));
	document.querySelector(selector).innerHTML = arr.join("<br/>");
}
function $$$display_tags_add(tag_ids, selector){
	const arr = tag_ids.map(x => $$$display_tag(x, t[x]));
	document.querySelector(selector).innerHTML += arr.join("<br/>");
}
function $$$display_parent_tags(_tag_id){
	$$$display_tags($$$t2p.filter(x => (x[0] == _tag_id)).map(x => x[1]), '#parents');
}
function $$$display_child_tags(_tag_id){
	$$$display_tags($$$t2p.filter(x => (x[1] == _tag_id)).map(x => x[0]), '#children');
}

function $$$add_child_tags_then(_tag_id, selector, fn){
	const tagselect = $(selector);
	$$$ajax_POST_w_text_response(
		"/t/c/" + _tag_id + "/" + tagselect.val().join(","),
		function(){
			tagselect.val("").change(); // Deselect all
		}
	);
	$$$add_to_json_then('t2p', $$$zipsplitarr(tagselect.val(), [_tag_id]), '/a/t2p.json', function(){
		fn();
	});
}
function $$$add_parent_tags_then(_tag_id, selector, fn){
	const tagselect = $(selector);
	$$$ajax_POST_w_text_response(
		"/t/p/" + _tag_id + "/" + tagselect.val().join(","),
		function(){
			tagselect.val("").change(); // Deselect all
		}
	);
	$$$add_to_json_then('t2p', $$$zipsplitarr([_tag_id], tagselect.val()), '/a/t2p.json', function(){
		fn();
	});
}

function $$$update_tag_thumb(){
	const url = prompt("New Thumbnail URL", "");
	if(url==="" || url===null)
		return;
	$$$ajax_POST_w_text_response(
		"/t/thumb/" + $$$tag_id + "/" + url,
		function(){
			$$$t[$$$tag_id][1] = url;
			$$$set_profile_thumb_from_this_tag();
		}
	);
}

function $$$view_tag(_tag_id){
	$$$hide_all_except([$$$parents-container,$$$children-container,'f','tagselect-files-container','tagselect-files-btn','tagselect-self-p-container','tagselect-self-p-btn','tagselect-self-c-container','tagselect-self-c-btn',$$$merge-files-btn,$$$backup-files-btn,$$$view-as-playlist-btn]);
	$$$document_getElementById('profile-img').onclick = $$$update_tag_thumb;
	
	$$$file_tagger_fn = $$$after_tagged_selected_files;
	$$$get_file_ids = $$$get_selected_file_ids;
	
	if (_tag_id !== undefined){
		// It is undefined if we are just unhiding the tag view
		$$$tag_id = _tag_id;
		$$$populate_f_table('/a/f/t/' + $$$tag_id);
	}
	
	$$$set_profile_name_from_this_tag();
	$$$set_profile_thumb_from_this_tag();
	$$$set_profile_cover_from_this_tag();
	
	$$$display_parent_tags($$$tag_id);
	$$$display_child_tags($$$tag_id);
	
	window.location.hash = 't' + $$$tag_id;
}
function $$$view_tags(ls){
	$$$hide_all_except(['t','files-tagging']);
	if(ls !== undefined)
		$$$populate_t_id2name_table('#t .tbody', ls);
	window.location.hash = '';
}
