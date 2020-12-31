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

function $$$select2_ids_default(){
	return '0';
}

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

function $$$init_selects(var_name){
	let s = "";
	const col = $$$tbl2namecol[var_name];
	$($$$tbl2selector[var_name]).select2({
		placeholder: $$$nickname2fullname(var_name) + (($$$use_regex)?" regexp":""),
		allowClear:true,
		ajax:{
			transport: function (params, success, failure){
				let arr = Object.entries($$$window[var_name]); // WARNING: I don't see why there aren't scope issues
				if(params.data.q !== undefined){
					const pattern = ($$$use_regex) ? new RegExp(params.data.q) : params.data.q;
					arr = arr.filter(x => x[1][0].search(pattern)>=0);
				}
				if(arr.length > 50){
					arr = arr.slice(0, 50);
					arr.unshift(['0', ['Truncated to 50 results']]);
				}
				success(arr);
			},
			processResults: function(data){
				return{
					results: data.map(([id,tpl]) => ({id:id, text:((col===null)?tpl:tpl[col])}))
				};
			}
		}
	}); // Initialise
}

function $$$init_selects__ajax(var_name){
	let s = "";
	const col = $$$tbl2namecol[var_name];
	$($$$tbl2selector[var_name]).select2({
		minimumInputLength:1,
		placeholder: $$$nickname2fullname(var_name) + (($$$use_regex)?" regexp":""),
		allowClear:true,
		ajax:{
			url: !!!MACRO!!!SELECT2_URL_ENDPOINT,
			// Basically "a/select2/regex/" + var_name, but customisable because select2 needs a workaround for GitHub pages deployment, as GH Pages discards parameters.
			dataType:"json",
			data:function(params){
				// The server filters out tags that are already applied to all ids
				return ($$$use_regex) ? {'q': $$$select2_ids()+'+'+params.term} : {'q': $$$select2_ids()+'+'+$$$regexp_esc_for_mysql(params.term)}
			},
			processResults:function(data){
				return{
					results:Object.entries(data).map(([id,name]) => ({id:id, text:name}))
				};
			}
		}
	});
}

function $$$refetch_json(var_name, url, fn){
	$$$ajax_GET_w_JSON_response(url + '?' + (new Date().getTime()), function(data){
		// Cache buster url parameter
		console.log("Cache busting", var_name);
		$$$window[var_name] = data;
		if(fn !== undefined)
			fn();
		$$$init_selects(var_name);
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
