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
			$$$document_getElementById('eras-info-tbody').innerHTML += $$$create_era_info_row([$$$era_start,$$$era_end,_tag_ids]);
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
	const [start,end,era_tag_ids] = era;
	let _s = "";
	_s += "<tr class='tr'>";
		_s += "<td class='td'><a onclick=\"$$$view_file('"+$$$file_id+"@"+start+"-"+end+"')\">" + $$$t2human(start) + "</a></td>";
		_s += "<td class='td'><a onclick=\"$$$view_file('"+$$$file_id+"@"+end+"')\">" + $$$t2human(end) + "</a></td>";
		_s += "<td class='td'>" + era_tag_ids + "</td>";
	_s += "</tr>";
}
