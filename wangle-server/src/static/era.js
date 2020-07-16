var $$$era_start = null;
var $$$era_end = null;

function $$$set_era_vertex(){
	const t = ($$$active_media.currentTime || $$$active_media.getCurrentTime());
	if(t===undefined)
		return;
	const T = ($$$active_media.duration || $$$active_media.getDuration());
	if($$$era_start===null){
		$$$era_start = t;
	}else{
		$$$era_end = t;
		$$$unhide("tagselect-era-container");
	}
}

function $$$tag_era(){
	if(($$$era_start===null)||($$$era_end===null))
		return $$$alert("Error: era_start==="+era_start+" and era_end==="+era_end);
	const _tag_ids = $('#tagselect-era').val().join(",");
	$$$ajax_POST_w_text_response(
		"/e/add/"+$$$file_id+"/"+$$$era_start+"-"+$$$era_end+"/"+_tag_ids,
		function(){
			$$$document_getElementById('eras-info-tbody').innerHTML += $$$create_era_info_row([0,$$$era_start,$$$era_end,_tag_ids]);
			$$$hide_tagselect_era();
		}
	);
}

function $$$hide_tagselect_era(){
	$$$era_start = $$$era_end = null;
	$$$hide('tagselect-era-container');
}

function $$$view_eras(ls){
	$$$hide_all_except(['f','view-as-playlist-btn']);
	
	if (ls === undefined)
		$$$unset_window_location_hash();
	else{
		if (ls.length === 0){
			$$$get_tbl_body("f").innerHTML = "";
		}else{
			$$$populate_f_table('e/id',ls.join(","),null,0);
		}
	}
	
	$$$set_profile_name("Eras");
}

function $$$create_era_info_row(era){
	const [id,start,end,era_tag_ids] = era;
	let _s = "";
	_s += "<tr class='tr' data-id='" + id + "'>";
		_s += "<td class='td'>" + id + "</td>";
		_s += "<td class='td'><a onclick=\"$$$view_file('"+$$$file_id+"@"+start+"-"+end+"')\">" + $$$t2human(start) + "</a></td>";
		_s += "<td class='td'><a onclick=\"$$$view_file('"+$$$file_id+"@"+end+"')\">" + $$$t2human(end) + "</a></td>";
		_s += "<td class='td'>" + era_tag_ids + "</td>";
	_s += "</tr>";
	return _s;
}

function $$$era_tagger_fn(ids,t_dict){
	for(x of $$$document_getElementById('eras-info-tbody')){
		if(!ids.includes(x.dataset.id))
			continue;
		x.getElementsByClassName('td')[3].innerHTML += "and some new tags";
	}
}

function $$$get_era_ids(){
	let ids = "";
	for(let node of $$$get_tbl_body('eras-info').getElementsByClassName('selected1')){
		if(node.dataset.id==="0"){
			$$$alert("Can only tag eras with non-zero IDs.\nSome eras have ID of zero, probably because they are newly added eras.");
			return "";
		}else 
			ids += "," + node.dataset.id
	}
	return ids.substr(1);
}
