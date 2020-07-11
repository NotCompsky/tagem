function $$$set_profile_name_from_this_dir(){
	$$$set_profile_name($$$dir_name);
}
function $$$display_related_dirs(dir_id,rel,arr){
	$$$document_getElementById(rel).innerHTML = arr.filter(x => x[0]!==dir_id).map(([id,name]) => "<a onclick=\"$$$view_dir('" + id + "',0)\">" + $$$escape_html_text(name) + "</a>").join("<br/>");
}

function $$$populate_d_id2name_table(arr){
	const tbl = $$$document_getElementById('d').getElementsByClassName('tbody')[0];
	if (arr.length===0){
		tbl.innerHTML = "";
		return;
	}
	
	$$$ajax_data_w_JSON_response(
		"GET",
		"/a/d/id/"+arr.join(","),
		null,
		function(data){
			let s = "";
			for (const [id, name, device, size] of data){
				s += "<div class='tr'>";
					s += "<div class='td'>";
						s += "<a onclick='$$$view_dir(" + id + ",0)'>" + $$$escape_html_text(name) + "</a>";
					s += "</div>";
					s += "<div class='td'>";
						s += "<a onclick='$$$view_device(" + device + ")'>Device</a>";
					s += "</div>";
					s += "<div class='td dir-size'>" + size + "</div>";
				s += "</div>";
			}
			tbl.innerHTML = s;
		}
	);
}


function $$$view_dir(_dir_id_or_path, is_not_in_db, page){
	if(_dir_id_or_path==="")
		_dir_id_or_path=$$$dir_id;
	if(page===undefined)
		page = 0;
	let ls = ['f','f-action-btns','tags-container','parents-container','children-container','tagselect-files-container','tagselect-files-btn'];
	if(!is_not_in_db){
		ls.push('merge-files-btn');
		ls.push('backup-files-btn');
		ls.push('view-as-playlist-btn');
	}
	$$$hide_all_except(ls);
	
	$$$file_tagger_fn = $$$after_tagged_selected_files;
	$$$get_file_ids = $$$get_selected_file_ids;

	if (_dir_id_or_path !== undefined){
		if(is_not_in_db){
			const dir = Object.entries(d).filter(x => (x[1][0]===_dir_id_or_path))[0];
			if(dir===undefined){
				$$$alert("Directory does not exist in the database");
				$$$dir_id = "0";
			}else{
				$$$dir_id = dir[0];
			}
			$$$populate_f_table(':', null, _dir_id_or_path,page);
		}else{
			$$$dir_id = _dir_id_or_path;
			$$$ajax_GET_w_JSON_response("/a/d/i/"+$$$dir_id, function(data){
				$$$dir_name = data[0];
				$$$display_related_dirs($$$dir_id,"parents",data[1]);
				$$$display_related_dirs($$$dir_id,"children",data[2]);
				$$$display_tags(data[3], "#tags", "$$$unlink_this_tag_from_this", 'd');
				$$$set_profile_name_from_this_dir();
			});
			$$$populate_f_table('d', $$$dir_id, null, page);
		}
	}
	
	if(is_not_in_db){
		$$$set_profile_name(_dir_id_or_path);
	}else if(_dir_id_or_path===undefined){
		$$$set_profile_name_from_this_dir();
	}
}
function $$$view_dirs(ls){
	$$$hide_all_except(['d']);
	$$$populate_d_id2name_table(ls);
	$$$unset_window_location_hash();
	$$$set_profile_name("All Directories");
}
