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

function $$$is_integer(x){
	return Number.isInteger(x);
}

function $$$is_string(x){
	return (typeof x === 'string' || x instanceof String);
}

function $$$node_exists_w_id(s){
	return ($$$document_getElementById(s)!==null);
}

function $$$sleep(ms){
	return new Promise(resolve => setTimeout(resolve, ms));
}

function $$$refetch_json(var_name, url, fn){
	$$$ajax_GET_w_JSON_response(url + '?' + (new Date().getTime()), function(data){
		// Cache buster url parameter
		console.log("Cache busting", var_name);
		$$$window[var_name] = data;
		if(fn !== undefined)
			fn();
	});
}

function $$$set_var_to_json_then(var_name, url, fn){
	// All global variable are set in the window object
	if ($$$window[var_name] !== undefined){
		fn();
		return;
	}
	$$$ajax_GET_w_JSON_response(url, function(data){
		$$$window[var_name] = data;
		fn();
	});
}
function $$$link_to_named_fn_w_param_id_w_human_name(fn,id,x){
	return "<a onclick='"+fn+"(\""+id+"\")'>"+((x instanceof Array)?x[0]:x)+"</a> ";
}
function $$$sub_into(data, node, fn_name){
	let s = "";
	if (data instanceof Array)
		data = Object.fromEntries(data.entries()); // Convert ["foo","bar"] -> {0:"foo", 1:"bar"}
	for (var tagid of $$$split_on_commas_but_make_empty_if_empty(node.textContent)){
		s += $$$link_to_named_fn_w_param_id_w_human_name(fn_name,tagid,data[tagid]);
	}
	node.innerHTML = s;
}

function $$$invert_dict(data){
	return Object.fromEntries(Object.entries(data).map(([k, v]) => [v, k]));
}

function $$$zipsplitarr(keys, vals){
	// Convert two arrays into a dictionary
	// [foo, bar], [ree, gee] -> [[foo,ree], [foo,gee], [bar,ree], [bar,gee]]
	const arr = [];
	for (const key of keys){
		for (const val of vals){
			arr.push([key, val]);
		}
	}
	return arr;
}
