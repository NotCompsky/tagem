var $$$is_editing_existing_file = false; // Either we are creating a new file, or editing the file given by file_id

function $$$view_text_editor(){
	$$$hide_all_except(['text-editor','dirselect-container','tagselect-files-container']);
}

function $$$text_editor_save(){
	const title = $$$document_getElementById('text-editor').getElementsByTagName('h2')[0].innerText.replace(/\n$/,'');
	if(title===""){
		$$$alert("Must name file");
		return;
	}
	if(/[\r\n\/\\]/.test(title)){
		// WARNING: Different filesystems will require different restrictions.
		// NOTE: This is a very lazy test
		$$$alert("File name must not contain newline, carriage return, or slashes of any kind");
		return;
	}
	const tagselect = $('#tagselect-files');
	const tag_ids = tagselect.val();
	if(tag_ids.length === 0){
		$$$alert("Must tag file");
		return;
	}
	const dir_inp = $$$document_getElementById("dirselect");
	const _dir_id = dir_inp.value;
	if(_dir_id === null){
		$$$alert("Must assign file to directory");
		return;
	}
	const _dir_name = dir_inp.textContent;
	if(!_dir_name.startsWith("/")){
		$$$alert("Directory must be local");
		return;
	}
	$$$ajax_POST_data_w_text_response_generic_success(
		"/f/save/"+(($$$is_editing_existing_file)?$$$file_id:0)+"/"+_dir_id+"/"+tag_ids.join(","),
		title+"\n"+$$$document_getElementById('text-edit').innerText
	);
}
