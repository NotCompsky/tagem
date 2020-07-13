function $$$set_user_setting_as(key,val){
	$$$set_cookie(key,val,3600);
	$$$document_getElementById('setting-'+key).textContent = val
}

function $$$set_sleep_on_inanimate_media(){
	const n = $$$get_int();
	$$$set_user_setting_as('sleep_on_inanimate_media',n,3600);
}

function $$$set_sleep_after_media_err(){
	const n = $$$get_int();
	$$$set_user_setting_as('sleep_after_media_err',n,3600);
}

function $$$set_use_regex_tbl_entry(b){
	$$$document_getElementById('setting-use_regex').textContent = ($$$use_regex)?"1":"0";
}

function $$$set_use_regex(){
	const b = $$$confirm("Use RegEx?\nSay no if you don't know what they are");
	$$$use_regex = (b===true);
	$$$set_cookie('use_regex', ($$$use_regex)?'1':'0', 3600);
	$$$set_use_regex_tbl_entry();
}
