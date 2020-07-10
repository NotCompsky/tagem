var $$$file_qry_url;
var $$$file_qry_url_paramsythings;
var $$$file_qry_post_data;
var $$$file_qry_page_n;

var $$$active_media = null;

var $$$file_title;

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
	$$$ajax_data_w_JSON_response(
		($$$file_qry_post_data===null)?"GET":"POST",
		"/a/f/"+$$$file_qry_url+'/'+$$$file_qry_page_n+(($$$file_qry_url_paramsythings===null)?"":("/"+$$$file_qry_url_paramsythings)),
		$$$file_qry_post_data,
		function(datas){
			$$$get_file_ids = $$$get_selected_file_ids;
			let s = "";
			$$$file2post = {};
			const [a,data,tags] = datas;
			if(a==="0"){
				if($$$dir_id==="0"){
					$$$alert("ERROR: Directory ID has not been set.");
				}
			}else{
				$$$dir_id=a;
			}
			for (let [thumb, id, name, title, sz, t_added_to_db, t_origin, duration, w, h, views, likes, dislikes, fps, ext_db_n_post_ids, tag_ids, era_start, era_end] of data){
				if(era_end!==0){
					// era_end should be >= era_start, so era_end===0 implies era_start===0 too
					duration = era_end - era_start;
					id += "@" + era_start + "-" + era_end;
				}
				s += "<div class='tr' data-id='" + id + "'>";
					s += '<div class="td"><img class="thumb" onclick="$$$view_file(this.parentNode.parentNode.dataset.id)" src="' + thumb + '"></img></div>';
					//"s += "<td><a href='/d#" + ls[1] + "'>" + ls[2] + "</a></td>"; // Dir  ID and name
					s += "<div class='td fname'>" + name + "</div>"; // File ID and name
					s += "<div class='td fname'>" + title + "</div>";
					s += "<div class='td'>" + ext_db_n_post_ids + "</div>"; // 3rd column i.e. col[2]
					s += "<div class='td'>" + tag_ids + "</div>"; // 4th column i.e. col[3]
					s += "<div class='td' data-n=" + sz + ">" + $$$bytes2human(parseInt(sz)) + "</div>"; // 5th column i.e. col[4]
					s += "<div class='td' data-n=" + t_added_to_db + ">" + $$$timestamp2dt(t_added_to_db) + "</div>";
					s += "<div class='td' data-n=" + t_origin + ">" + $$$timestamp2dt(t_origin) + "</div>";
					s += "<div class='td' data-n=" + duration + ">" + $$$t2human(duration) + "</div>";
					s += "<div class='td w' data-n=" + w + ">" + w + "</div>";
					s += "<div class='td h' data-n=" + h + ">" + h + "</div>";
					s += "<div class='td views' data-n=" + views + ">" + $$$n2human(views) + "</div>";
					s += "<div class='td likes' data-n=" + likes + ">" + $$$n2human(likes) + "</div>";
					s += "<div class='td dislikes' data-n=" + dislikes + ">" + $$$n2human(dislikes) + "</div>";
					s += "<div class='td fps' data-n=" + fps + ">" + fps + "</div>";
					
					// Populate file2post dictionary
					$$$file2post[id] = ext_db_n_post_ids.split(":"); // database_ids, post_ids
					
				s += "</div>";
			}
			$$$set_node_visibility($$$document_getElementById('f').getElementsByClassName('next-page')[0], ($$$file_qry_page_n!==0));
			$$$set_node_visibility($$$document_getElementById('f').getElementsByClassName('next-page')[1], (data.length===$$$MAX_RESULTS_PER_PAGE));
			$$$get_tbl_body("f").innerHTML = s;
			$$$column_id2name('x', "f", '$$$view_db', 3);
			$$$column_id2name(tags,"f", '$$$view_tag', 4);
			
			$$$apply_thumbnail_width();
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
	for(let node of $$$get_tbl_body('f').getElementsByClassName('selected1')){
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
	const tags = tagselect.select2('data');
	if(file_ids==="")
		return;
	if(!$$$logged_in())
		return $$$alert_requires_login();
	$$$ajax_POST_w_text_response(
		"/f/t/" + file_ids + "/" + tags.map(x => x.id).join(","),
		function(){
			tagselect.val("").change(); // Deselect all
			fn(file_ids, tags);
		}
	);
}
function $$$after_tagged_this_file(file_ids, tags){
	$$$display_tags_add(tags, '#tags')
}
function $$$after_tagged_selected_files(file_ids, tags){
	for(let node of $$$document_getElementById('f').getElementsByClassName('tr')){
		if(!file_ids.includes(node.dataset.id))
			continue;
		if(node.dataset.id==="0"){
			$$$debug__err_alert("after_tagged_selected_files ($$$after_tagged_selected_files) attempting to tag file of ID 0");
			continue;
		}
		let _s = "";
		for(tag of tags)
			_s += $$$link_to_named_fn_w_param_id_w_human_name("$$$view_tag",tag.id,tag.text);
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
			continue;
		}
		$$$hide('view-'+type);
		if (type === 'yt-player')
			$$$try_to_pause_yt_video(); // Not using stopVideo, as that might leave the player in the ENDED state, which might be problematic for playlist cycling.
	}
	if(except !== null)
		$$$unhide('view-'+except);
}
function $$$set_file_view_src(type, src, _mimetype){
	$$$hide_all_views_except(type);
	const x = $$$document_getElementById('view-'+type);
	$$$active_media = x;
	x.src = src;
}
function $$$play(type, src, _mimetype){
	$$$hide_all_views_except(type);
	$$$active_media = $$$document_getElementById('view-'+type);
	const player = $$$active_media.getElementsByTagName('source')[0];
	player.type = _mimetype;
	player.src = src;
	player.parentNode.load();
	player.parentNode.play();
}

var $$$yt_player_timeout;
function $$$YTPlayer_onStateChange(e){
	clearTimeout($$$yt_player_timeout);
	if (e.data == YT.PlayerState.PLAYING){
		if($$$yt_player.jump_to_t!==undefined){
			$$$yt_player.seekTo($$$yt_player.jump_to_t);
			$$$yt_player.jump_to_t = undefined;
		}
		const T = $$$yt_player.next_video_when_reach_t;
		const t = $$$yt_player.getCurrentTime();
		if ((T!==undefined) && (t < T)){
			const t_remaining = (T - t) / $$$yt_player.getPlaybackRate();
			$$$yt_player_timeout = setTimeout($$$next_video_from_yt_video,  t_remaining * 1000);
		}
		return;
	}
	if ((e.data == YT.PlayerState.ENDED) && ($$$playlist_file_ids===undefined))
		$$$playlist_listener();
}
function $$$view_yt_video__from_onReady(){
	$$$yt_player.loadVideoById($$$YTPlayer_onReady.my_idstr);
	$$$YTPlayer_onReady = $$$dummy;
}
function $$$view_yt_video(idstr){
	if($$$yt_player.loadVideoById!==undefined)
		$$$yt_player.loadVideoById(idstr);
	else{
		$$$YTPlayer_onReady.my_idstr = idstr;
		$$$YTPlayer_onReady_fn = $$$view_yt_video__from_onReady;
	}
	$$$hide_all_views_except('yt-player');
}
function $$$try_to_pause_yt_video(){
	if($$$yt_player.pauseVideo!==undefined)
		$$$yt_player.pauseVideo();
}
function $$$next_video_from_yt_video(){
	$$$yt_player.next_video_when_reach_t = undefined;
	if($$$playlist_file_ids === undefined)
		$$$try_to_pause_yt_video();
	else
		$$$playlist_listener(); // Go to next file in playlist
}

function $$$set_embed_html(_dir_id, _mimetype, _file_name){
	const [_dir_name, _device_id] = $$$d[(_dir_id === "") ? $$$dir_id : _dir_id];
	if(_device_id === $$$YOUTUBE_DEVICE_ID){
		$$$active_media = $$$yt_player;
		return $$$view_yt_video(_file_name);
	}
	$$$try_to_pause_yt_video();
	const embed_pre = $$$D[_device_id][2];
	if (embed_pre === ""){
		const _src_end = (_dir_id === "") ? "" : "/" + _dir_id;
		const src = (_dir_name.startsWith("http")) ? (_dir_name + _file_name) : ("/S/f/" + $$$file_id + _src_end);
		
		const _mimetype_str = $$$mt[_mimetype];
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
		
		if($$$playlist_file_ids === undefined){
			for(var i=0; i<5; ++i)
				$$$document_getElementById('view-'+$$$playlist_listeners_types[i]).removeEventListener($$$playlist_listeners_eventnames[i], $$$playlist_listeners[i]);
		}
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
	$$$active_media = null;
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

function $$$view_file__hides(){
	// To allow deferred reshowing (once content is loaded) to wrongly displaying old content
	$$$hide_all_except(['file2-container','values-container','tags-container','file-info','tagselect-files-container','tagselect-files-btn'],['file-meta']);
	if($$$playlist_file_ids!==undefined)
		$$$unhide('next-f-in-playlist');
}

function $$$next_video_when_it_reaches(){
	if(this.currentTime >= this.next_video_when_reach_t){
		if($$$playlist_file_ids === undefined)
			this.pause();
		else
			$$$playlist_listener(); // Go to next file in playlist
		this.removeEventListener("timeupdate", $$$next_video_when_it_reaches);
	}
}
function $$$next_video_when_reach_time(t){
	$$$active_media.next_video_when_reach_t = t;
	if($$$active_media===$$$yt_player)
		return;
	$$$active_media.addEventListener("timeupdate", $$$next_video_when_it_reaches);
}
function $$$skip_to_era(e){
	if(e===undefined)
		return;
	const [a,b] = e.split("-");
	if ($$$active_media.currentTime!==undefined)
		$$$active_media.currentTime = a;
	else{
		// If we have loaded a new video, seekTo(a) will seemingly fire before that video is finished loading, and is therefore ignored. Otherwise, there is no event fired, so jump_to_t will be ignored (until the user moves the video or something), so we must seekTo anyway.
		$$$active_media.jump_to_t = a;
		if($$$active_media.seekTo!==undefined)
			$$$active_media.seekTo(a);
	}
	$$$next_video_when_reach_time(b);
}
function $$$play_active_media(){
	if($$$active_media.play!==undefined)
		$$$active_media.play()
	else
		$$$active_media.playVideo();
}

function $$$active_media_is_video(){
	return ($$$active_media!==null)&&(($$$active_media===$$$yt_player)||($$$active_media.play!==undefined))
}

function $$$view_file(_file_id_and_t){
	const [_file_id, _t] = (_file_id_and_t===undefined)?[null,null]:_file_id_and_t.split("@");
	// WARNING: _t is only respected if autoplay is on // TODO: Fix this
	
	if(_file_id==0){
		$$$alert("Cannot view file of ID 0");
		return;
	}
	$$$hide('add-f-backup');
	
	$$$file_tagger_fn = $$$after_tagged_this_file;
	$$$get_file_ids = $$$get_file_id;
	
	$$$set_window_location_hash(
		($$$playlist_file_ids!==undefined)
		? ('F' + $$$playlist_file_ids.join(","))
		: ('f' + ((_file_id_and_t===undefined)?$$$file_id:_file_id_and_t))
		// BUG: This discards era information when viewing a single file and clicking on the File tab again.
		// TODO: Fix this
	);
	
	if (_file_id !== null){
		if((_file_id === $$$file_id)&&($$$active_media_is_video())){
			// We haven't left the current file - perhaps we are skipping to a different era
			$$$skip_to_era(_t);
			$$$view_file__hides();
			$$$play_active_media();
		}else{
		$$$undisplay_this_file();
		$$$file_id = _file_id;
		$$$ajax_GET_w_JSON_response(
			"/a/f/i/"+$$$file_id,
			function(data){
				const [[thumb, _dir_id, name, title, sz, t_added_to_db, t_origin, duration, w, h, views, likes, dislikes, fps, ext_db_n_post_ids, tag_ids, mime, file2_values_csv], eras, backups, _d, t_dict] = data;
				$$$set_profile_thumb(thumb);
				$$$dir_id = _dir_id;
				$$$d = _d;
				$$$file_name = name;
				$$$file_title = (title==="")?$$$file_name:title;
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
				
				$$$file_tags = (tag_ids==="") ? [] : tag_ids.split(",");
				$$$display_tags($$$file_tags, "#tags", "$$$unlink_this_tag_from_this_file");
				
				$$$mimetype = mime;
				
				$$$set_profile_name($$$file_title);
				$$$set_t_added(t_added_to_db);
				$$$set_t_origin(t_origin);
				
				let _s = $$$create__view_dir_and_filename_w_filename_playable("",$$$mimetype,name);
				if ($$$autoplay()){
					$$$display_this_file('',$$$mimetype);
					$$$skip_to_era(_t);
				} else {
					for(const [dir,fname,mime] of backups){
						_s += $$$create__view_dir_and_filename_w_filename_playable(dir,mime,fname);
					}
				}
				$$$document_getElementById("view-btns").innerHTML = _s;
				$$$view_file__hides();
				
				_s = "";
				for(const [start,end,era_tag_ids] of eras){
					_s += "<tr class='tr'>";
						_s += "<td class='td'><a onclick=\"$$$view_file('"+$$$file_id+"@"+start+"-"+end+"')\">" + $$$t2human(start) + "</a></td>";
						_s += "<td class='td'><a onclick=\"$$$view_file('"+$$$file_id+"@"+end+"')\">" + $$$t2human(start) + "</a></td>";
						_s += "<td class='td'>" + era_tag_ids + "</td>";
					_s += "</tr>";
				}
				$$$document_getElementById('eras-info-tbody').innerHTML = _s;
				$$$column_id2name(t_dict,"eras-info", '$$$view_tag', 2);
			}
		);
		}
	} else {
		$$$view_file__hides();
		$$$set_profile_name($$$file_title);
	}
}

function $$$create__view_dir_and_filename_w_filename_playable(id,mime,fname){
	return '<a onclick="$$$view_dir(\'' + id + '\',0)">' + $$$d[(id==="")?$$$dir_id:id][0] + '</a><a class="view-btn" onclick="$$$display_this_file(\'' + id + '\',\'' + mime + '\')">' + fname + '</a></br>';
}

function $$$view_files(ls){
	$$$hide_all_except(['f','tagselect-files-container','tagselect-files-btn','merge-files-btn','backup-files-btn','view-as-playlist-btn']);
	
	$$$file_tagger_fn = $$$after_tagged_selected_files;
	$$$get_file_ids = $$$get_selected_file_ids;
	
	if (ls === undefined)
		$$$unset_window_location_hash();
	else{
		if (ls.length === 0){
			$$$get_tbl_body("f").innerHTML = "";
		}else{
			$$$populate_f_table('id',ls.join(","),null,0);
		}
	}
	
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
function $$$view_files_as_playlist(file_ids_csv){
	if(file_ids_csv===undefined)
		file_ids_csv = $$$get_file_ids();
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
