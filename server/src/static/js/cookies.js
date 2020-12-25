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
