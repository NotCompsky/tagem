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
function $$$set_cookie(name, value, lifetime){
	document.cookie = name + '=' + value + ';max-age=' + lifetime + ';path=/;samesite=strict;';
	// NOTE: Previous cookies are NOT overwritten
}
function $$$set_cookie_forever(name, value){
	$$$set_cookie(name,value,999999999);
}
function $$$get_cookie(name){
	const regexp = new RegExp('(?:^|;) ?' + name + "=([^;]+)");
	const x = regexp.exec(document.cookie);
	if (x === null)
		return undefined;
	return x[1];
}
function $$$unset_cookie(name){
	document.cookie = name + '=;max-age=-1;';
}

function $$$merge_into(dict, d2){
	for (const [key, value] of Object.entries(d2)){
		dict[key] = value;
	}
}

function $$$del_keys(dict, keys){
	for (const key of keys){
		delete dict[key];
	}
}

function $$$add_to_json_then(var_name, dict, url, fn){
	// dict is the dictionary of additions to the main dictionary
	// The main dictionary is referred to by window[var_name]
	if (dict instanceof Array){
		$$$window[var_name] = Array.prototype.concat($$$window[var_name], dict);
	} else {
		$$$merge_into($$$window[var_name], dict);
	}
	
	const cookie_name = var_name + '_adds';
	const cookie_val = $$$get_cookie(cookie_name);
	
	if (cookie_val !== undefined){
		if (dict instanceof Array){
			dict = Array.prototype.concat(dict, JSON.parse(cookie_val));
		} else {
			$$$merge_into(dict, JSON.parse(cookie_val));
		}
	}
	
	const str = JSON.stringify(dict);
	
	if (str.length > 4000){
		// Most browsers only support cookies of max length 4096-ish
		// If above this limit, just get an entirely new JSON
		$$$refetch_json(var_name, url, fn);
		$$$unset_cookie(cookie_name);
		return;
	}
	
	$$$set_cookie(cookie_name, str, 3600*24);
	fn();
}

function $$$rm_from_json_then(var_name, keys, url, fn){
	// keys is the list of deletions from the main dictionary
	// The main dictionary is referred to by window[var_name]
	if ($$$window[var_name] instanceof Array){
		$$$window[var_name] = $$$window[var_name].filter(x => !keys.includes(x));
	} else {
		$$$del_keys($$$window[var_name], keys);
	}
	
	const cookie_name = var_name + '_dels';
	const cookie_val = $$$get_cookie(cookie_name);
	
	if (cookie_val !== undefined){
		keys = Array.prototype.concat(keys, JSON.parse(cookie_val));
	}
	
	const str = JSON.stringify(keys);
	
	if (str.length > 4000){
		// Most browsers only support cookies of max length 4096-ish
		// If above this limit, just get an entirely new JSON
		$$$ajax_GET_w_JSON_response(url + '?' + (new Date().getTime()), function(data){
			// Cache buster url parameter
			console.log("Cache busting", var_name);
			$$$window[var_name] = data;
			fn();
		});
		$$$unset_cookie(cookie_name);
		return;
	}
	
	$$$set_cookie(cookie_name, str, 3600*24);
	fn();
}
