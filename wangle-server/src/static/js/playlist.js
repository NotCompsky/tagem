function $$$is_playlist_running(){
	return ($$$playlist_file_ids!==undefined)
}

const $$$playlist_listeners = new Array(5);
const $$$playlist_listeners_types = ['img','video','audio','iframe','object'];
const $$$playlist_listeners_eventnames = ['load','ended','ended','load','load'];
const $$$playlist_listeners_fns = [$$$playlist_listener_delayed, $$$playlist_listener, $$$playlist_listener, $$$playlist_listener_delayed, $$$playlist_listener_delayed];
var $$$playlist_file_ids;
function $$$playlist_listener(){
	if(!$$$is_playlist_running())
		return;
	$$$yt_player.jump_to_t = undefined;
	$$$playlist_file_ids.push($$$playlist_file_ids.shift());
	$$$view_file($$$playlist_file_ids[0]);
}
async function $$$playlist_listener_delayed(){
	await $$$sleep($$$get_cookie('sleep_for_inanimate_media'));
	$$$playlist_listener();
}
async function $$$on_media_error(){
	// TODO: Iterate through backups (either remote or "existing" local devices), and only then move to next file in playlist
	if($$$is_playlist_running()){
		await $$$sleep($$$get_cookie('sleep_on_media_err'));
		$$$playlist_listener();
	}
}
function $$$view_files_as_playlist(file_ids_csv){
	if(file_ids_csv===undefined)
		file_ids_csv = $$$get_selected_file_ids(true);
	if(file_ids_csv===""){
		$$$playlist_file_ids = undefined;
		return;
	}
	$$$playlist_file_ids = $$$split_on_commas__guaranteed_nonempty(file_ids_csv);
	$$$document_getElementById('autoplay').checked = true;
	for(var i=0; i<5; ++i)
		$$$playlist_listeners[i] = $$$document_getElementById('view-'+$$$playlist_listeners_types[i]).addEventListener($$$playlist_listeners_eventnames[i], $$$playlist_listeners_fns[i]);
	$$$view_file($$$playlist_file_ids[0]);
}
