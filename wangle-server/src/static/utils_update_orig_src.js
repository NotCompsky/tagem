function $$$show_update_orig_src_dialog(){
	$$$unhide('dirselect-container');
	$$$unhide('orig-src-dialog');
}
function $$$hide_update_orig_src_dialog(){
	$$$hide('dirselect-container');
	$$$hide('orig-src-dialog');
}

function $$$update_orig_src(){
	const inp = $$$document_getElementById("orig-src-input");
	const url = inp.value;
	const dir_inp = $$$document_getElementById("dirselect");
	let dir = dir_inp.value;
	if(dir === ""){
		dir = $$$guess_parenty_thing_from_name('d', url);
		if(tpl === undefined){
			$$$alert("Cannot guess a suitable directory.\nPlease set the directory manually - it should be a prefix of the URL");
			return;
		}
	}
	const _dir_name = dir_inp.textContent;
	if(!url.startsWith(_dir_name)){
		$$$alert("The directory '" + _dir_name + "' is not a prefix of the URL");
		return;
	}
	$$$ajax_POST_data_w_text_response(
		"/f/orig/" + $$$file_id + "/" + dir,
		url.substr(_dir_name.length),
		$$$hide_update_orig_src_dialog
	);
}
