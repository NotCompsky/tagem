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

var $$$active_media = null;
var $$$file_title;


function $$$get_file_id(and_era){
	return (and_era) ? $$$get_file_id_and_selected_era_start_and_ends_csv() : $$$file_id;
}

function $$$after_tagged_this_file(ids, tags){
	$$$display_tags_add(tags, 'tags', 'f')
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
	if ((e.data == YT.PlayerState.ENDED) && (!$$$is_playlist_running()))
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
	if($$$is_playlist_running())
		$$$playlist_listener(); // Go to next file in playlist
	else
		$$$try_to_pause_yt_video();
}

function $$$remove_playlist_event_listener(x,e,fn){
	x.removeEventListener(e,fn);
}

function $$$set_embed_html(_dir_id, _mimetype, _file_name){
	const [_dir_name, _device_id] = $$$d[(_dir_id === "") ? $$$dir_id : _dir_id];
	$$$unhide_class('file-era');
	$$$unhide('tagselect-eras-container');
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
			case 'text':
				$$$ajax_GET_w_text_response(src,function(data){
					const x = $$$document_getElementById('text-edit');
					x.innerText = data;
					$$$unhide_node(x);
					$$$unhide('text-editor');
				});
				break;
			case '':
				// If no slash in the mimetype (probably "!!NONE!!")
				$$$set_file_view_src('iframe', src, null);
				break;
			default:
				$$$set_file_view_src('object', src, _mimetype_str);
		}
		
		if(!$$$is_playlist_running()){
			for(var i=0; i<5; ++i)
				$$$remove_playlist_event_listener($$$document_getElementById('view-'+$$$playlist_listeners_types[i]), $$$playlist_listeners_eventnames[i], $$$playlist_listeners[i]);
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

function $$$display_file2_var(name, value){
	return "<div class='value'><div class='value-name'><a onclick='$$$view_files_by_value(\"" + name + "\")'>" + name + "</a></div>" + value + "</div>";
	// NOTE: No need to $$$escape_html_text(name), as name is already restricted due to SQL table name restrictions
}
function $$$assign_value_to_file(){
	const select = $('#file2-select');
	const f2_id = select.val();
	if(f2_id === "")
		return;
	const _file_ids = $$$get_file_ids();
	if(_file_ids==="")
		return;
	const input = $$$document_querySelector('.file2-value:not(.hidden)');
	let value = input.value;
	if(value==="")
		return;
	if(input.type === "datetime-local")
		value = $$$dt2timestamp(value);
	$$$ajax_POST_w_text_response(
		"/f/f2/"+_file_ids+"/"+value+"/"+$$$f2[f2_id][0],
		function(){
			select.val("").change(); // Deselect all
			input.value = "";
			if($$$is_visible('values'))
				$$$document_getElementById('values').innerHTML += $$$display_file2_var($$$f2[f2_id][0], value);
		}
	);
}

function $$$view_file__hides(){
	// To allow deferred reshowing (once content is loaded) to wrongly displaying old content
	$$$hide_all_except(['file2-container','values-container','tags-container','file-info','tagselect-files-container','tagselect-files-btn','descr'],['file-meta']);
	if($$$active_media_is_video())
		$$$unhide('tagselect-eras-container');
	else
		$$$hide_class('file-era');
	if($$$is_playlist_running())
		$$$unhide('next-f-in-playlist');
	$$$unhide_playlist_repeatness_node();
}

function $$$next_video_when_it_reaches(){
	if(this.currentTime >= this.next_video_when_reach_t){
		if(!$$$is_playlist_running())
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
		$$$active_media.play();
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
		($$$is_playlist_running())
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
			"!!!MACRO!!!SERVER_ROOT_URL/a/f/i/"+$$$file_id,
			function(data){
				const [[thumb, _dir_id, name, title, sz, t_added_to_db, t_origin, duration, w, h, views, likes, dislikes, fps, ext_db_n_post_ids, tag_ids, mime, description, file2_values_csv], eras, backups, _d, t_dict] = data;
				$$$set_profile_thumb(thumb);
				$$$dir_id = _dir_id;
				$$$d = _d;
				$$$file_name = name;
				$$$file_title = (title==="")?$$$file_name:title;
				$$$display_external_dbs($$$split_on_commas_but_make_empty_if_empty(ext_db_n_post_ids).map(x => x.split(":")));
				
				let _vals = "";
				const file2_values = $$$split_on_commas_but_make_empty_if_empty(file2_values_csv);
				for(var i=0, len=file2_values.length;  i<len;  ++i){
					if(file2_values[i] !== "0")
						_vals += $$$display_file2_var($$$f2_as_array[i][1][0], file2_values[i]);
				}
				$$$document_getElementById('values').innerHTML = _vals;
				
				$$$file_tags = (tag_ids==="") ? [] : $$$split_on_commas_but_make_empty_if_empty(tag_ids);
				$$$display_tags($$$file_tags, "tags", "$$$unlink_this_tag_from_this", "f");
				
				$$$mimetype = mime;
				
				$$$set_profile_name($$$file_title);
				$$$set_t_added(t_added_to_db);
				$$$set_t_origin(t_origin);
				
				let _s = $$$create__view_dir_and_filename_w_filename_playable("",$$$mimetype,name);
				for(const [dir,fname,mime] of backups){
					_s += $$$create__view_dir_and_filename_w_filename_playable(dir,mime,fname);
				}
				if ($$$autoplay()){
					$$$display_this_file('',$$$mimetype);
					$$$skip_to_era(_t);
				}
				$$$document_getElementById("view-btns").innerHTML = _s;
				$$$view_file__hides();
				
				$$$document_getElementById('descr').textContent = description;
				
				$$$document_getElementById('eras-info-tbody').innerHTML = eras.map(x=>$$$create_era_info_row(x)).join("");
				$$$column_id2name(t_dict,"eras-info", '$$$view_tag', 3);
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





function $$$show_update_orig_src_dialog(){
	$$$unhide('orig-src-dialog');
}
function $$$hide_update_orig_src_dialog(){
	$$$hide('orig-src-dialog');
}

function $$$update_orig_src(){
	const inp = $$$document_getElementById("orig-src-input");
	const url = inp.value;
	$$$ajax_POST_data_w_text_response(
		"/f/orig/" + $$$file_id,
		url,
		$$$hide_update_orig_src_dialog
	);
}
