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
	const tags = $('#tagselect-era').select2('data');
	$$$ajax_POST_w_text_response(
		"/e/add/"+$$$file_id+"/"+$$$era_start+"-"+$$$era_end+"/"+tags.map(x => x.id).join(","),
		function(){
			$$$document_getElementById('eras-info-tbody').innerHTML += $$$create_era_info_row([0,$$$era_start,$$$era_end,tags.map(x => "<a onclick='$$$view_tag(\'" + x.id + "\')'>" + x.text + "</a>")]);
			$$$hide_tagselect_era();
		}
	);
}

function $$$hide_tagselect_era(){
	$$$era_start = $$$era_end = null;
	$$$hide('tagselect-era-container');
}

function $$$setup_page_for_e_tbl(){
	$$$hide_all_except(['f','view-as-playlist-btn']);
	$$$set_profile_name("Eras");
}

function $$$ytdl_era(node){
	const dir_inp = $$$document_getElementById("dirselect");
	if(dir_inp.value===""){
		$$$alert("No directory selected");
		return;
	}
	$$$ajax_POST_data_w_text_response("/e/dl/"+dir_inp.value+"/"+node.dataset.id,function(){
		dir_inp.value = "";
		$$$alert("Saved to '" + $$$file_id + "@" + node.dataset.start + "-" + node.dataset.end + ".mkv'");
	});
}

function $$$create_era_info_row(era){
	const [id,start,end,era_tag_ids] = era;
	let _s = "";
	_s += "<tr class='tr' data-id='" + id + "' data-start='" + start + "' data-end='" + end + "'>";
		_s += "<td class='td'>" + id + "</td>";
		_s += "<td class='td'><a onclick=\"$$$view_file('"+$$$file_id+"@"+start+"-"+end+"')\">" + $$$t2human(start) + "</a></td>";
		_s += "<td class='td'><a onclick=\"$$$view_file('"+$$$file_id+"@"+end+"')\">" + $$$t2human(end) + "</a></td>";
		_s += "<td class='td'>" + era_tag_ids + "</td>";
		_s += "<td class='td'><a onclick=\"$$$ytdl_era(this)\">youtube-dl</a></td>";
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

function $$$get_file_id_and_selected_era_start_and_ends_csv(){
	const fid = $$$get_file_id(false);
	let csv = "";
	for(let node of $$$get_tbl_body('eras-info').getElementsByClassName('selected1')){
		csv += "," + fid + "@" + node.dataset.start + "-" + node.dataset.end;
	}
	return (csv==="") ? fid : csv.substr(1);
}
