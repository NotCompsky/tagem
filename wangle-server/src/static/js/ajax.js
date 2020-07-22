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
