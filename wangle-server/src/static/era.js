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

function $$$view_eras(ls){
	$$$hide_all_except(['f','view-as-playlist-btn']);
	
	if (ls === undefined)
		$$$unset_window_location_hash();
	else{
		if (ls.length === 0){
			$$$get_tbl_body("e").innerHTML = "";
		}else{
			$$$populate_f_table('e/id',ls.join(","),null,0);
		}
	}
	
	$$$set_profile_name("Eras");
}

var $$$era_qry_url;
var $$$era_qry_url_paramsythings;
function $$$populate_e_table(path,params,page_n){
	$$$file_qry_page_n=parseInt(page_n);
	if(path!==null){
		$$$era_qry_url=path;
		$$$era_qry_url_paramsythings=params;
	}
	$$$ajax_GET_w_JSON_response(
		"/a/e/"+$$$era_qry_url+'/'+$$$file_qry_page_n+(($$$era_qry_url_paramsythings===null)?"":("/"+$$$era_qry_url_paramsythings)),
		function(datas){
			let s = "";
			const [files,data,tags] = datas;
			for (const [f_id,f_title,f_thumb,t_start,t_end,tag_ids] of data){
				s += "<div class='tr' data-id='" + id + "@" + t_start + "-" + t_end + "'>";
					s += '<div class="td"><img class="thumb" onclick="$$$view_file(this.parentNode.parentNode.dataset.id)" src="' + f_thumb + '"></img></div>';
					//"s += "<td><a href='/d#" + ls[1] + "'>" + ls[2] + "</a></td>"; // Dir  ID and name
					s += "<div class='td fname'>" + f_title + "</div>"; // File ID and name
					s += "<div class='td'>" + tag_ids + "</div>"; // 4th column i.e. col[3]
					s += "<div class='td' data-n=" + t_start + ">" + $$$t2human(t_start) + "</div>";
					s += "<div class='td' data-n=" + t_end   + ">" + $$$t2human(t_end)   + "</div>";
				s += "</div>";
			}
			$$$set_node_visibility($$$document_getElementById('f').getElementsByClassName('next-page')[0], ($$$file_qry_page_n!==0));
			$$$set_node_visibility($$$document_getElementById('f').getElementsByClassName('next-page')[1], (data.length===$$$MAX_RESULTS_PER_PAGE));
			$$$get_tbl_body("f").innerHTML = s;
			$$$column_id2name('x', "#f .tbody", '$$$view_db', 3);
			$$$column_id2name(tags,"#f .tbody", '$$$view_tag', 4);
			
			$$$apply_thumbnail_width();
		}
	);
}
