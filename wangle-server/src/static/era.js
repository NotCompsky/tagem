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
	$$$ajax_POST_w_text_response(
		"/e/add/"+$$$file_id+"/"+$$$era_start+"-"+$$$era_end+"/"+$('#tagselect-era').val().join(","),
		$$$hide_tagselect_era
	);
}

function $$$hide_tagselect_era(){
	$$$era_start = $$$era_end = null;
	$$$hide('tagselect-era-container');
}
