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

function $$$ajax(mthd,resp_type,url,fn){
	$.ajax({
		type:mthd,
		dataType:resp_type,
		url:url,
		success:fn,
		//headers:{
		//	'Expect': '' // Prevent "Expect: 100-continue" behaviour
		//},
		error:$$$err_alert
	});
}
function $$$ajax_w_JSON_response(mthd,url,fn){
	$$$ajax(mthd,"json",url,fn);
}

function $$$ajax_data_w_err(mthd,url,resp_type,data,succ,err){
	$.ajax({
		type:mthd,
		data:data,
		dataType:resp_type,
		url:url,
		success:succ,
		error:err
	});
}

function $$$ajax_data_w_JSON_response(mthd,url,data,fn){
	$$$ajax_data_w_err(mthd,url,"json",data,fn,$$$err_alert);
}

function $$$ajax_POST_w_JSON_response(url,fn){
	$$$ajax_w_JSON_response("POST",url,fn);
}
function $$$ajax_POST_data_w_err(url,resp_type,data,succ,err){
	$$$ajax_data_w_err("POST",url,resp_type,data,succ,err);
}
function $$$ajax_POST_data_w_JSON_response_and_err(url,data,succ,err){
	$$$ajax_POST_data_w_err(url,"json",data,succ,err);
}
function $$$ajax_POST_data(url,resp_type,data,fn){
	$$$ajax_POST_data_w_err(url,resp_type,data,fn,$$$err_alert);
}
function $$$ajax_POST_data_w_text_response(url,data,fn){
	$$$ajax_POST_data(url,"text",data,fn);
}
function $$$ajax_POST_data_w_JSON_response(url,data,fn){
	$$$ajax_POST_data(url,"json",data,fn);
}

function $$$ajax_POST_data_w_text_response_generic_success(url,data){
	$$$ajax_POST_data_w_text_response(url,data,$$$alert_success);
}

function $$$ajax_GET_w_JSON_response(url,fn){
	$$$ajax_w_JSON_response("GET",url,fn);
}

function $$$ajax_GET_w_text_response(url,fn){
	$$$ajax("GET","text",url,fn);
}

function $$$ajax_POST_w_text_response(url,fn){
	$$$ajax("POST","text",url,fn);
}

function $$$ajax_POST_w_json_response(url,fn){
	$$$ajax("POST","json",url,fn);
}

function $$$ajax_POST_w_text_response_generic_success(url){
	$$$ajax_POST_w_text_response(url,$$$alert_success);
}
