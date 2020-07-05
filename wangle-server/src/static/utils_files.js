var $$$file_qry_url;
var $$$file_qry_url_paramsythings;
var $$$file_qry_post_data;
var $$$file_qry_page_n;

function $$$next_page(tbl_id,direction){
	switch(tbl_id){
		case 'f':
			$$$populate_f_table(null,null,null,$$$file_qry_page_n+direction);
			break;
	}
}

function $$$populate_f_table(path,params,post_data,page_n){
	$$$file_qry_page_n=parseInt(page_n);
	if((params!==null)&&(post_data!==null))
		return $$$alert("ERROR: At most one of (params, post_data) can be non-null");
	if(path!==null){
		$$$file_qry_url=path;
		$$$file_qry_url_paramsythings=params;
		$$$file_qry_post_data=post_data;
	}
	$$$set_window_location_hash($$$file_qry_url+page_n+"/"+(($$$file_qry_post_data===null)?$$$file_qry_url_paramsythings:$$$file_qry_post_data));
	$$$ajax_data_w_JSON_response(
		($$$file_qry_post_data===null)?"GET":"POST",
		"/a/f/"+$$$file_qry_url+'/'+$$$file_qry_page_n+(($$$file_qry_url_paramsythings===null)?"":("/"+$$$file_qry_url_paramsythings)),
		$$$file_qry_post_data,
		function(datas){
			$$$get_file_ids = $$$get_selected_file_ids;
			let s = "";
			$$$file2post = {};
			const [a,data] = datas;
			if(a==="0"){
				if($$$dir_id==="0"){
					$$$alert("ERROR: Directory ID has not been set.");
				}
			}else{
				$$$dir_id=a;
			}
			for (const [thumb, id, name, sz, ext_db_n_post_ids, tag_ids] of data){
				s += "<div class='tr' data-id='" + id + "'>";
					s += '<div class="td"><img class="thumb" src="' + thumb + '"></img></div>';
					//"s += "<td><a href='/d#" + ls[1] + "'>" + ls[2] + "</a></td>"; // Dir  ID and name
					s += "<div class='td fname'><a onclick='$$$view_file(this.parentNode.parentNode.dataset.id)'>" + name + "</a></div>"; // File ID and name
					s += "<div class='td'>" + ext_db_n_post_ids + "</div>"; // 3rd column i.e. col[2]
					s += "<div class='td'>" + tag_ids + "</div>"; // 4th column i.e. col[3]
					s += "<div class='td' data-n=" + sz + ">" + $$$bytes2human(parseInt(sz)) + "</div>"; // 5th column i.e. col[4]
					
					// Populate file2post dictionary
					$$$file2post[id] = ext_db_n_post_ids.split(":"); // database_ids, post_ids
					
				s += "</div>";
			}
			$$$set_node_visibility($$$document_getElementById('f').getElementsByClassName('next-page')[0], ($$$file_qry_page_n!==0));
			$$$set_node_visibility($$$document_getElementById('f').getElementsByClassName('next-page')[1], (data.length===$$$MAX_RESULTS_PER_PAGE));
			document.querySelector("#f .tbody").innerHTML = s;
			$$$column_id2name('x', "#f .tbody", '$$$view_db', 2);
			$$$column_id2name('t', "#f .tbody", '$$$view_tag', 3);
		}
	);
}

function $$$add_files_to_db(nodes){
	if($$$dir_id===0){
		$$$alert("Cannot add files to DB unless their directory ID is set");
		return;
	}
	if(nodes.length===0)
		return;
	$$$ajax_POST_data_w_JSON_response(
		"/f/record/"+$$$dir_id,
		nodes.map(x => '"'+x.getElementsByClassName('fname')[0].innerText.replace('"','\"')+'"').join(','),
		function(data){
			for(let node of nodes){
				node.dataset.id = data[node.getElementsByClassName('fname')[0].innerText];
			}
			$$$alert("Files newly registered.\nPlease now repeat your command.");
		}
	);
}

function $$$merge_files(){
	const master_file_ids = $$$get_selected_file_ids().split(",");
	const dupl_file_ids   = $$$get_selected2_file_ids().split(",");
	if(master_file_ids.length !== 1){
		$$$alert("There must be exactly one master file (use left mouse button to select them)");
		return;
	}
	if(dupl_file_ids.length === 0){
		$$$alert("No duplicate files selected (use middle mouse button to select them)");
		return;
	}
	const master_f_id = master_file_ids[0];
	if(dupl_file_ids.includes(master_f_id)){
		$$$alert("File cannot be selected as both master and duplicate");
		return;
	}
	$$$ajax_POST_w_text_response(
		"/f/merge/"+master_f_id+"/"+dupl_file_ids.join(","),
		function(){
			$$$deselect_rows('#f .tbody .tr', 1);
			$("#f .tbody .tr.selected2").remove();
		}
	);
}

function $$$get_file_id(){
	return $$$file_id;
}
function $$$get_selected2_file_ids(){
	return $("#f .tbody .tr.selected2").map((i, el) => el.dataset.id).get().join(",");
}
function $$$get_selected_file_ids(){
	let file_ids = "";
	let files_wo_ids = [];
	for(let node of $$$document_getElementById('f').getElementsByClassName('tbody')[0].getElementsByClassName('selected1')){
		if(node.dataset.id==="0")
			files_wo_ids.push(node);
		else 
			file_ids += "," + node.dataset.id
	}
	if(files_wo_ids.length!==0){
		$$$add_files_to_db(files_wo_ids);
		// Seems I can't simply wait for a promise and take the value; I'd have to transform every function relying on it
		return "";
	}
	return file_ids.substr(1);
}

function $$$tag_files_then(file_ids, selector, fn){
	const tagselect = $(selector);
	const tag_ids = tagselect.val();
	if(file_ids==="")
		return;
	if(!$$$logged_in())
		return $$$alert_requires_login();
	$$$ajax_POST_w_text_response(
		"/f/t/" + file_ids + "/" + tag_ids.join(","),
		function(){
			tagselect.val("").change(); // Deselect all
			fn(file_ids, tag_ids);
		}
	);
}
function $$$after_tagged_this_file(file_ids, tag_ids){
	$$$display_tags_add(tag_ids, '#tags')
}
function $$$after_tagged_selected_files(file_ids, tag_ids){
	for(let node of $$$document_getElementById('f').getElementsByClassName('tr')){
		if(!file_ids.includes(node.dataset.id))
			continue;
		if(node.dataset.id==="0"){
			$$$debug__err_alert("after_tagged_selected_files ($$$after_tagged_selected_files) attempting to tag file of ID 0");
			continue;
		}
		let _s = "";
		for(id of tag_ids)
			_s += $$$link_to_named_fn_w_param_id_w_human_name("$$$view_tag",id,$$$t[id]);
		node.getElementsByClassName('td')[3].innerHTML += _s;
	}
}


function $$$hide_all_views_except(except){
	for(let type of ['img','video','audio','iframe','object','yt-player']){
		if(type === except){
			$$$unhide('view-'+type);
			const node = $$$document_getElementById('view-'+type).getElementsByTagName('source')[0];
			if(node !== undefined)
				node.removeAttribute('src');
			if(type === 'yt-player')
				$$$yt_player.pauseVideo(); // Not using stopVideo, as that might leave the player in the ENDED state, which might be problematic for playlist cycling.
			continue;
		}
		$$$hide('view-'+type);
	}
	if(except !== null)
		$$$unhide('view-'+except);
}
function $$$set_file_view_src(type, src, _mimetype){
	$$$hide_all_views_except(type);
	const x = $$$document_getElementById('view-'+type);
	x.src = src;
}
function $$$play(type, src, _mimetype){
	$$$hide_all_views_except(type);
	const player = $$$document_getElementById('view-'+type).getElementsByTagName('source')[0];
	player.type = _mimetype;
	player.src = src;
	player.parentNode.load();
	player.parentNode.play();
}

function $$$YTPlayer_onStateChange(e){
	if ((e.data != YT.PlayerState.ENDED) || ($$$playlist_file_ids===undefined))
		return;
	$$$playlist_listener();
}
function $$$view_yt_video(idstr){
	$$$yt_player.loadVideoById(idstr);
	$$$hide_all_views_except('yt-player');
}

function $$$set_embed_html(_dir_id, _mimetype, _file_name){
	const [_dir_name, _device_id] = d[(_dir_id === undefined) ? $$$dir_id : _dir_id];
	if((_dir_id!==undefined)&&(_dir_name[0]!=="/")){
		$$$alert("BUG: Cannot play remote backup files");
		return;
	}
	if(_device_id === $$$YOUTUBE_DEVICE_ID)
		return $$$view_yt_video(_file_name);
	const embed_pre = $$$D[_device_id][2];
	if (embed_pre === ""){
		const _src_end = (_dir_id === undefined) ? "" : "/" + _dir_id;
		const src = (_dir_name.startsWith("http")) ? (_dir_name + _file_name) : ("/S/f/" + $$$file_id + _src_end);
		
		const _mimetype_str = $$$mt[(_mimetype===undefined)?$$$mimetype:_mimetype];
		switch(_mimetype_str.substring(0, _mimetype_str.indexOf('/'))){
			case 'image':
				$$$set_file_view_src('img', src, null);
				break;
			case 'audio':
				$$$play('audio', src, _mimetype_str);
				break;
			case 'video':
				$$$play('video', src, _mimetype_str);
				break;
			case '':
				// If no slash in the mimetype (probably "!!NONE!!")
				$$$set_file_view_src('iframe', src, null);
				break;
			default:
				$$$set_file_view_src('object', src, _mimetype_str);
		}
		
		if($$$playlist_file_ids === undefined)
			for(var i=0; i<5; ++i)
				$$$document_getElementById('view-'+$$$playlist_listeners_types[i]).removeEventListener($$$playlist_listeners_eventnames[i], $$$playlist_listeners[i]);
	} else {
		$$$set_file_view_src('iframe', embed_pre+_file_name+$$$D[_device_id][3], null);
	}
}

function $$$view_this_files_dir(){
	$$$view_dir($$$dir_id,0);
}
function $$$display_this_file(_dir_id, _mimetype){
	$$$set_embed_html(_dir_id, _mimetype, $$$file_name);
}
function $$$undisplay_this_file(){
	$$$hide_all_views_except(null);
}
function $$$autoplay(){
	return $$$document_getElementById('autoplay').checked;
}
function $$$display_external_db(id, name, post_id){
	return "<a class='db-link' onclick='$$$view_post(" + id + ",\"" + post_id + "\")'>View post on " + name + "</a>"; // post_id is enclosed in quotes because Javascript uses doubles for integers and rounds big integers
}
function $$$display_external_dbs(db_and_post_ids){
	let s = "";
	for (var i = 0;  i < db_and_post_ids.length;  ++i){
		s += $$$display_external_db(db_and_post_ids[i][0], $$$x[db_and_post_ids[i][0]], db_and_post_ids[i][1]) + "<br/>";
	}
	$$$document_getElementById("db-links").innerHTML = s;
}

function $$$view_files_by_value(var_name){
	$$$populate_f_table('$',var_name,null,0);
	$$$hide_all_except(['f','tagselect-files-container','tagselect-files-btn','merge-files-btn','backup-files-btn','view-as-playlist-btn']);
	$$$set_profile_name('Files assigned ' + var_name);
}
function $$$display_file2_var(name, value){
	return "<div class='value'><div class='value-name'><a onclick='$$$view_files_by_value(\"" + name + "\")'>" + name + "</a></div>" + value + "</div>";
}
function $$$assign_value_to_file(){
	const select = $('#file2-select');
	const var_indx = select.val();
	if(var_indx.length === 0)
		return;
	const _file_ids = $$$get_file_ids();
	if(_file_ids==="")
		return;
	const input = $$$document_getElementById('file2-value');
	const value = input.value;
	if(value==="")
		return;
	$$$ajax_POST_w_text_response(
		"/f/f2/"+_file_ids+"/"+value+"/"+$$$f2[var_indx],
		function(){
			select.val("").change(); // Deselect all
			input.value = "";
			if($$$is_visible('values'))
				$$$document_getElementById('values').innerHTML += $$$display_file2_var($$$f2[var_indx], value);
		}
	);
}

function $$$view_file(_file_id){
	if(_file_id==0){
		$$$alert("Cannot view file of ID 0");
		return;
	}
	$$$hide_all_except(['file2-container','values-container','tags-container','file-info','tagselect-files-container','tagselect-files-btn']);
	$$$hide('add-f-backup');
	
	$$$file_tagger_fn = $$$after_tagged_this_file;
	$$$get_file_ids = $$$get_file_id;
	
	if (_file_id !== undefined){
		$$$file_id = _file_id;
		$$$ajax_GET_w_JSON_response(
			"/a/f/i/"+$$$file_id,
			function(data){
				const [thumb, _dir_id, name, sz, ext_db_n_post_ids, tag_ids, mime, backups, file2_values_csv] = data;
				$$$set_profile_thumb(thumb);
				$$$dir_id = _dir_id;
				$$$file_name = name;
				const db_and_post_ids = ext_db_n_post_ids;
				if (db_and_post_ids !== "")
					$$$display_external_dbs(db_and_post_ids.split(",").map(x => x.split(":")));
				
				let _vals = "";
				const file2_values = file2_values_csv.split(",");
				for(var i=0, len=file2_values.length;  i<len;  ++i){
					if(file2_values[i] !== "0")
						_vals += $$$display_file2_var(f2[i], file2_values[i]);
				}
				$$$document_getElementById('values').innerHTML = _vals;
				
				if(tag_ids === "")
					$$$file_tags = [];
				else{
					$$$file_tags = tag_ids.split(",");
					$$$display_tags($$$file_tags, "#tags");
				}
				
				$$$mimetype = mime;
				
				$$$document_getElementById('dir_name').onclick = $$$view_this_files_dir;
				$$$document_getElementById('dir_name').innerText = $$$d[$$$dir_id][0];
				
				$$$document_getElementById('file_name').innerText = name;
				$$$document_getElementById('file_name').href = $$$d[$$$dir_id][0] + name;
				
				$$$set_profile_name($$$file_name);
				
				let _s = "";
				if ($$$autoplay()){
					$$$display_this_file();
				} else {
					$$$undisplay_this_file();
					if(backups!==""){
						for(const _dir_id_to_mimetype of backups.split(",")){
							const [_dir_id, _mimetype] = _dir_id_to_mimetype.split(":");
							// dir_id of backup file
							_s += '<a class="view-btn" onclick="$$$display_this_file(' + _dir_id + ',' + _mimetype + ')">' + $$$d[_dir_id][0] + '</a>';
						}
					}
				}
				$$$document_getElementById("view-btns-backups").innerHTML = _s;
			}
		);
	} else {
		$$$set_profile_name($$$file_name);
	}
	
	$$$set_window_location_hash('f' + $$$file_id);
}

function $$$view_files(ls){
	$$$hide_all_except(['f','tagselect-files-container','tagselect-files-btn','merge-files-btn','backup-files-btn','view-as-playlist-btn']);
	
	$$$file_tagger_fn = $$$after_tagged_selected_files;
	$$$get_file_ids = $$$get_selected_file_ids;
	
	if (ls !== undefined){
		if (ls.length !== 0){
			$$$populate_f_table('id',ls.join(","),null,0);
		}
	}
	
	$$$unset_window_location_hash();
	
	$$$set_profile_name("Files");
}

function $$$toggle_file_add_backup_dialog(){
	$$$toggle('dirselect-container');
	$$$toggle('add-f-backup');
}
function $$$backup_files(){
	const file_ids = $$$get_file_ids(); // CSV string
	if(file_ids.length === ""){
		$$$alert("No files selected");
		return;
	}
	const _dir_id = $$$document_getElementById("dirselect").value;
	let url = $$$document_getElementById("add-f-backup-url").value;
	const is_ytdl = $$$document_getElementById("add-f-backup-ytdl").checked;
	if(_dir_id===""){
		$$$alert("No directory selected");
		return;
	}
	if(url !== ""){
		if((!url.startsWith('http')) && (!url.startsWith('/'))){
			$$$alert("Non-empty URL does not start with http or /");
			return;
		}
	}
	if(is_ytdl)
		url = "ytdl/" + url;
	$$$ajax_POST_w_text_response(
		"/f/backup/" + file_ids + "/" + _dir_id + "/" + url,
		function(){
			$$$hide('dirselect-container');
			$$$hide('add-f-backup');
		}
	);
}

const $$$playlist_listeners = new Array(5);
const $$$playlist_listeners_types = ['img','video','audio','iframe','object'];
const $$$playlist_listeners_eventnames = ['load','ended','ended','load','load'];
const $$$playlist_listeners_fns = [$$$playlist_listener_delayed, $$$playlist_listener, $$$playlist_listener, $$$playlist_listener_delayed, $$$playlist_listener_delayed];
var $$$playlist_file_ids;
function $$$playlist_listener(){
	if($$$playlist_file_ids===undefined)
		return;
	$$$playlist_file_ids.push($$$playlist_file_ids.shift());
	$$$view_file($$$playlist_file_ids[0]);
}
async function $$$playlist_listener_delayed(){
	await $$$sleep(2000);
	$$$playlist_listener();
}
function $$$view_files_as_playlist(){
	const file_ids_csv = $$$get_file_ids();
	if(file_ids_csv===""){
		$$$playlist_file_ids = undefined;
		return;
	}
	$$$playlist_file_ids = file_ids_csv.split(",");
	$$$document_getElementById('autoplay').checked = true;
	for(var i=0; i<5; ++i)
		$$$playlist_listeners[i] = $$$document_getElementById('view-'+$$$playlist_listeners_types[i]).addEventListener($$$playlist_listeners_eventnames[i], $$$playlist_listeners_fns[i]);
	$$$view_file($$$playlist_file_ids[0]);
}
