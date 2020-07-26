function $$$toggle_admin_dashboard(){
	if(!$$$logged_in())
		return;
	$$$hide_all_except(['admin-dashboard']);
	$$$ajax_GET_w_json_rexponse("/users",function(data){
		$$$users_dict = data;
	});
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

function $$$generate_thumbs(){
	$$$ajax_POST_w_text_response_generic_success("/gen-thumbs/");
}

function $$$toggle_edit_user_permissions_tbl(user_id){
	if($$$is_visible('edit-user-permissions')){
		$$$hide('edit-user-permissions');
		return;
	}
	$$$document_getElementById("edit-user-permissions-username").innerText = $$$users_dict[user_id];
	$$$unhide('edit-user-permissions');
}
