// 
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
// 
function $$$is_playlist_running(){
	return ($$$playlist_file_ids!==undefined)
}

const $$$playlist_listeners = new Array(5);
const $$$playlist_listeners_types = ['img','video','audio','iframe','object'];
const $$$playlist_listeners_eventnames = ['load','ended','ended','load','load'];
const $$$playlist_listeners_fns = [$$$playlist_listener_delayed, $$$playlist_listener, $$$playlist_listener, $$$playlist_listener_delayed, $$$playlist_listener_delayed];
var $$$playlist_file_ids;
function $$$playlist_listener(){
	if((!$$$is_playlist_running())||$$$playlist_repeat_mode===0)
		return;
	$$$yt_player.jump_to_t = undefined;
	if($$$playlist_repeat_mode!==2)
		// Rotate the playlist
		$$$playlist_file_ids.push($$$playlist_file_ids.shift());
	$$$view_file($$$playlist_file_ids[0]);
}
async function $$$playlist_listener_delayed(){
	await $$$sleep($$$local_storage_get('sleep_for_inanimate_media'));
	$$$playlist_listener();
}
async function $$$on_media_error(){
	// TODO: Iterate through backups (either remote or "existing" local devices), and only then move to next file in playlist
	if($$$is_playlist_running()){
		await $$$sleep($$$local_storage_get('sleep_on_media_err'));
		$$$playlist_listener();
	}
}

function $$$unhide_playlist_repeatness_node(){
	if(!$$$is_playlist_running())
		$$$playlist_repeat_mode = 0;
	for(let i of [0,1,2])
		$$$set_node_visibility($$$document_getElementsByClassName('repeatness')[i],(i===$$$playlist_repeat_mode));
}

function $$$view_files_as_playlist(file_ids_csv){
	if(file_ids_csv===undefined)
		file_ids_csv = $$$get_selected_file_ids(true);
	if(file_ids_csv===""){
		$$$playlist_file_ids = undefined;
		return;
	}
	$$$playlist_file_ids = $$$split_on_commas__guaranteed_nonempty(file_ids_csv);
	$$$playlist_repeat_mode = ($$$playlist_file_ids.length===1)?2:1;
	$$$document_getElementById('autoplay').checked = true;
	for(var i=0; i<5; ++i)
		$$$playlist_listeners[i] = $$$document_getElementById('view-'+$$$playlist_listeners_types[i]).addEventListener($$$playlist_listeners_eventnames[i], $$$playlist_listeners_fns[i]);
	$$$view_file($$$playlist_file_ids[0]);
}

function $$$play_current_files_as_playlist_if_not_already(){
	if($$$is_playlist_running())
		return;
	$$$view_files_as_playlist($$$get_file_ids(true));
}

var $$$playlist_repeat_mode = 0;
// 0 for "none", 1 for "all", 2 for "one"

function $$$playlist_repeat_none(){
	$$$playlist_repeat_mode = 0;
	$$$unhide_playlist_repeatness_node();
	$$$play_current_files_as_playlist_if_not_already(); // Because it is still a playlist! The other items in the playlist - most likely eras in this specific case - must be preserved in case the user decides to change the playlist to a different repeatness.
}
function $$$playlist_repeat_one(){
	$$$playlist_repeat_mode = 2;
	$$$unhide_playlist_repeatness_node();
	$$$play_current_files_as_playlist_if_not_already();
}
function $$$playlist_repeat_all(){
	$$$playlist_repeat_mode = 1;
	$$$unhide_playlist_repeatness_node();
	$$$play_current_files_as_playlist_if_not_already();
}
