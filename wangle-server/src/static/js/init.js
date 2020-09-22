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
		default:
			!!!MACRO!!!ACTION_ON_DOCUMENT_LOAD
			; // In case the macro is empty
	}
}

function $$$init_file2_input_for_file2_var_of_index(id){
	const [name,min,max,conversion] = $$$f2[id];
	const inp = $$$document_getElementById('file2-value');
	$$$set_visibility('file2-value-dt', (conversion===2));
	$$$set_visibility('file2-value', (conversion!==2));
	switch(conversion){
		// NOTE: Enum values are listed in file2.hpp
		case 0: // integer
			inp.type = "text";
			inp.pattern = "\d+";
			inp.placeholder = "12345";
			inp.min = min;
			inp.max = max;
			break;
		case 2: // datetime
			//inp.type = "datetime-local"; // Chrome-only
			break;
		default:
			$$$alert("Not implemented yet");
		//case 1: // string
		//case 3: // multiline string // TODO: Use <textarea> instead	
		//	inp.type = "text";
		//	inp.removeAttribute("pattern");
		//	break;
	}
}

function $$$when_data_loaded(){
	if($$$D === undefined || $$$P === undefined || $$$t2p === undefined || $$$x === undefined || $$$mt === undefined || $$$f2 === undefined || $$$yt_player === undefined)
		return;
	$$$init_tbls();
	$$$load_page_from_a_hash_string($$$window.location.hash);
	
	$('#file2-select').on('select2:selecting', function(e){
		$$$init_file2_input_for_file2_var_of_index(e.params.args.data.id);
	});
	
	$('#file2-value-dt').datetimepicker();
}

function $$$refetch_all_jsons(){
	$$$refetch_json('D', '!!!MACRO!!!SERVER_ROOT_URL/a/D.json', $$$when_data_loaded);
	$$$refetch_json('P', '!!!MACRO!!!SERVER_ROOT_URL/a/P.json', $$$when_data_loaded);
	$$$refetch_json('t2p', '!!!MACRO!!!SERVER_ROOT_URL/a/t2p.json', $$$when_data_loaded);
	$$$refetch_json('x', '!!!MACRO!!!SERVER_ROOT_URL/a/x.json', $$$when_data_loaded);
	$$$refetch_json('mt', '!!!MACRO!!!SERVER_ROOT_URL/a/mt.json', $$$when_data_loaded);
	$$$refetch_json('f2', '!!!MACRO!!!SERVER_ROOT_URL/a/f2.json', function(){
		$$$f2_as_array = Object.entries($$$f2).sort(x => x[0]).reverse();
		$$$f2_as_array.unshift(null); // Index begins at 1, to simplify server's SQL statement
		$$$when_data_loaded();
	});
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
