var $$$era_start = null;

function $$$set_era_vertex(){
	const t = ($$$active_media.currentTime || $$$active_media.getCurrentTime());
	if(t===undefined)
		return;
	const T = ($$$active_media.duration || $$$active_media.getDuration());
	if($$$era_start===null){
		$$$era_start = t/T;
	}else{
		const era_end = t/T;
		$$$alert("Era from " + $$$era_start + " to " + era_end);
		$$$era_start = null;
	}
}
