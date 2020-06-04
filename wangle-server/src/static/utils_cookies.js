R"=========(
function set_cookie(name, value, lifetime){
	document.cookie = name + '=' + value + ';max-age=' + lifetime + ';path=/;samesite=strict;';
	// NOTE: Previous cookies are NOT overwritten
}
function get_cookie(name){
	const regexp = new RegExp('(?:^|;)' + name + "=([^;]+)");
	const x = regexp.exec(document.cookie)
	if (x === null)
		return undefined;
	return x[1];
}
function unset_cookie(name){   
	document.cookie = name + '=;max-age=-1;';  
}

function merge_into(dict, d2){
	for (const [key, value] of Object.entries(d2)){
		console.log("dict[key] =", dict[key]);
		console.log("merging", key, value);
		dict[key] = value;
		console.log("dict[key] =", dict[key]);
		if (window["t2p"] === undefined){
			console.log("window[t2p] undefined");
		} else {
			console.log("window[t2p][key] =", window["t2p"][key]);
		}
		if (t2p === undefined){
			console.log("t2p undefined");
		} else {
			console.log("t2p[key] =", t2p[key]);
		}
	}
}

function del_keys(dict, keys){
	for (const key of keys){
		delete dict[key];
	}
}

function add_to_json_then(var_name, dict, url, fn){
	console.log("add_to_json_then", var_name, dict);
	//console.log(t2p.filter(x => x[0] == tag_id).map(([x,y]) => [t[x][0], t[y][0]]));
	// dict is the dictionary of additions to the main dictionary
	// The main dictionary is referred to by window[var_name]
	if (dict instanceof Array){
		window[var_name] = Array.prototype.concat(window[var_name], dict);
	} else {
		merge_into(window[var_name], dict);
	}
	//console.log(t2p.filter(x => x[0] == tag_id).map(([x,y]) => [t[x][0], t[y][0]]));
	
	const cookie_name = var_name + '_adds';
	const cookie_val = get_cookie(cookie_name);
	
	if (cookie_val !== undefined){
		if (dict instanceof Array){
			dict = Array.prototype.concat(dict, JSON.parse(cookie_val));
		} else {
			merge_into(dict, JSON.parse(cookie_val));
		}
	}
	
	const str = JSON.stringify(dict);
	
	if (str.length > 4000){
		// Most browsers only support cookies of max length 4096-ish
		// If above this limit, just get an entirely new JSON
		get_json(url + '?' + (new Date().getTime()), function(data){
			// Cache buster url parameter
			window[var_name] = data;
			fn();
		});
		unset_cookie(cookie_name);
		return;
	}
	
	set_cookie(cookie_name, str, 3600*24);
	fn();
}

function rm_from_json_then(var_name, keys, url, fn){
	// keys is the list of deletions from the main dictionary
	// The main dictionary is referred to by window[var_name]
	if (window[var_name] instanceof Array){
		window[var_name] = window[var_name].filter(x => !keys.includes(x));
	} else {
		del_keys(window[var_name], keys);
	}
	
	const cookie_name = var_name + '_dels';
	const cookie_val = get_cookie(cookie_name);
	
	if (cookie_val !== undefined){
		keys = Array.prototype.concat(keys, JSON.parse(cookie_val));
	}
	
	const str = JSON.stringify(keys);
	
	if (str.length > 4000){
		// Most browsers only support cookies of max length 4096-ish
		// If above this limit, just get an entirely new JSON
		get_json(url + '?' + (new Date().getTime()), function(data){
			// Cache buster url parameter
			window[var_name] = data;
			fn();
		});
		unset_cookie(cookie_name);
		return;
	}
	
	set_cookie(cookie_name, str, 3600*24);
	fn();
}
)========="
