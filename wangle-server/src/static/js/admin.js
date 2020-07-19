function $$$toggle_admin_dashboard(){
	if($$$logged_in())
		$$$hide_all_except(['admin-dashboard']);
}


function $$$recursively_record_filesystem_dir_dialog(){
	$$$toggle('record-fs-dir');
}

function $$$recursively_record_filesystem_dir(){
	const s = $$$prompt("/path/of/directory");
	if((s==="")||(s===null))
		return;
	const max_depth = $$$get_int_with_prompt("Maximum depth - 0 being 'no limit'");
	if (max_depth === null)
		return;
	let tag_ids = $('#tagselect-record-fs-dir').val().join(",");
	if (tag_ids === "")
		tag_ids = "0";
	$$$ajax_POST_data_w_text_response(
		"/record-local-dir/"+max_depth+"/"+tag_ids,
		s,
		function(){
			$$$hide('record-fs-dir');
		}
	);
}
