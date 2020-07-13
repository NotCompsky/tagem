const $$$dummy = function(){};
var $$$YTPlayer_onReady_fn = $$$dummy;
function $$$YTPlayer_onReady(){
	$$$YTPlayer_onReady_fn();
}
function onYouTubeIframeAPIReady(){
	// NOTE: Must not be name-mangled - used by YouTube IFrame API
	$$$yt_player = new YT.Player('yt-player', {
		height: '390',
		width: '640',
		events:{
			'onError': $$$on_media_error,
			'onReady': $$$YTPlayer_onReady,
			'onStateChange':$$$YTPlayer_onStateChange
		}
	});
	$$$when_data_loaded();
}

function $$$load_page_from_a_hash_string(S){
	const s = decodeURI(S);
	let _x = s.substr(2);
	const c = s.substr(1,1);
	const _xs = /([^\/]+)\/(.*)$/.exec(_x);
	switch(c){
		case "f": $$$view_file(_x); break;
		case "F": $$$view_files_as_playlist(_x); break;
		case "t": $$$view_tag(_xs[2],_xs[1]);  break;
		case "x":
			$$$db_id = _xs[1];
			_x = _x2.substr(1);
			switch(_xs[2][0]){
				case "u": $$$view_user($$$db_id, _x); break;
			}
			break;
		case "$": $$$view_files_by_value(_x); break;
		case "d":
		case ":":
			$$$view_dir(_xs[2], (c===":"), _xs[1]);
			break;
		case '?':
			$$$view_qry(_x);
			break;
	}
}

function $$$when_data_loaded(){
	if($$$D === undefined || $$$P === undefined || $$$t2p === undefined || $$$x === undefined || $$$mt === undefined || $$$f2 === undefined || $$$yt_player === undefined)
		return;
	$$$init_tbls();
	$$$load_page_from_a_hash_string($$$window.location.hash);
}

function $$$refetch_all_jsons(){
	$$$refetch_json('D', '/a/D.json', $$$when_data_loaded);
	$$$refetch_json('P', '/a/P.json', $$$when_data_loaded);
	$$$refetch_json('t2p', '/a/t2p.json', $$$when_data_loaded);
	$$$refetch_json('x', '/a/x.json', $$$when_data_loaded);
	$$$refetch_json('mt', '/a/mt.json', $$$when_data_loaded);
	$$$refetch_json('f2', '/a/f2.json', function(){$$$f2=$$$f2.split(",");$$$when_data_loaded()});
}

function $$$add_key_intercepts(){
	$$$Mousetrap_bind('e', $$$set_era_vertex);
	$$$Mousetrap_bind('p', $$$view_files_as_playlist);
	$$$Mousetrap_bind('v', e => $$$toggle('file2-container'));
	$$$Mousetrap_bind('b', $$$toggle_file_add_backup_dialog);
	$$$Mousetrap_bind('m', $$$merge_files);
	$$$Mousetrap_bind('q', e => $$$focus('qry'));
}
function $$$del_key_intercepts(){
	$$$Mousetrap_unbind('e');
	$$$Mousetrap_unbind('p');
	$$$Mousetrap_unbind('v');
	$$$Mousetrap_unbind('b');
	$$$Mousetrap_unbind('m');
	$$$Mousetrap_unbind('q');
}
