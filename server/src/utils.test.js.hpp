/*
Copyright 2020 Adam Gray
This file is part of the tagem program.
The tagem program is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by the
Free Software Foundation version 3 of the License.
The tagem program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
This copyright notice should be included in any copy or substantial copy of the tagem source code.
The absense of this copyright notices on some other files in this project does not indicate that those files do not also fall under this license, unless they have a different license written at the top of the file.
*/
#define MINIMISED_JS_DECL_sleep "a"
#define MINIMISED_JS_DECL_YOUTUBE_DEVICE_ID "b"
#define MINIMISED_JS_DECL_focus "c"
#define MINIMISED_JS_DECL_init_selects "d"
#define MINIMISED_JS_DECL_refetch_json "e"
#define MINIMISED_JS_DECL_login "f"
#define MINIMISED_JS_DECL_logout "g"
#define MINIMISED_JS_UTILS_TEST_JS "function a(ms){return new Promise(resolve => setTimeout(resolve, ms));}}const b=\"1\";function c(id){document.getElementById(id).focus();}}function d(var_name){let s = \"\";const col = tbl2namecol[var_name];$(tbl2selector[var_name]).select2({placeholder: nickname2fullname[var_name] + (use_regex)?\" pattern\":\"\",ajax:{transport: function (params, success, failure){let arr = Object.entries(window[var_name]); // WARNING: I don't see why there aren't scope issuesif(params.data.q !== undefined){const pattern = (use_regex) ? new RegExp(params.data.q) : params.data.q;arr = arr.filter(x => x[1][0].search(pattern)>=0);}if(arr.length > 50){arr = arr.slice(0, 50);arr.unshift(['0', ['Truncated to 50 results']]);}success(arr);},processResults: function(data){return{results: data.map(([id,tpl]) => ({id:id, text:((col===null)?tpl:tpl[col])}))};}}}); }}function e(var_name, url, fn){get_json(url + '?' + (new Date().getTime()), function(data){console.log(\"Cache busting\", var_name);window[var_name] = data;if(fn !== undefined)fn();d(var_name);});}}function f(){const uname = prompt(\"Username\");set_cookie(\"username\", uname, 3600*24); document.getElementById('username').innerText = uname;refetch_all_jsons();}}function g(){unset_cookie(\"username\");document.getElementById('username').innerText = \"GUEST\";refetch_all_jsons();}}"