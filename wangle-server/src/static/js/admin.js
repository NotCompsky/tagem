function $$$toggle_admin_dashboard(){
	$$$hide_all_except(['record-fs-dir','tagselect-files']);
}


function $$$recursively_record_filesystem_dir_dialog(){
	if($$$is_visible('record-fs-dir')){
		$$$hide('record-fs-dir');
		$$$hide('tagselect-files');
	} else {
		$$$unhide('record-fs-dir');
		$$$unhide('tagselect-files');
	}
}

function $$$recursively_record_filesystem_dir(){
	const s = $$$prompt("/path/of/directory");
	if((s==="")||(s===null))
		return;
	const max_depth = $$$get_int_with_prompt("Maximum depth - 0 being 'no limit'");
	if (max_depth === null)
		return;
	$$$ajax_POST_data_w_text_response(
		"!!!MACRO!!!SERVER_ROOT_URL/record-local-dir/"+max_depth+"/"+tag_ids,
		s,
		function(){
			$$$hide('record-fs-dir');
		}
	);
}
