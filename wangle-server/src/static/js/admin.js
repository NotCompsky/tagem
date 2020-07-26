function $$$toggle_admin_dashboard(){
	if(!$$$logged_in())
		return;
	$$$hide_all_except(['admin-dashboard']);
	$$$ajax_GET_w_JSON_response("/users",function(data){
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

function $$$get_id_of_user_currently_being_edited(){
	return $$$document_getElementById('select-user-for-edit').value;
}

function $$$load_user_for_edit(){
	const [name,vals] = $$$users_dict[$$$get_id_of_user_currently_being_edited()];
	$$$document_getElementById("edit-user-permissions-username").innerText = name;
	let s = "";
	const ls = $$$get_tbl_body("edit-user-permissions-tbl").getElementsByClassName("tr");
	const vals_ls = vals.split(",");
	for(let i=0, n=vals_ls.length;  i<n;  ++i){
		ls[i].getElementsByClassName('td')[1].textContent = (vals_ls[i]==="1")?"TRUE":"FALSE";
	}
}

function $$$edit_permission_value(e){
	const par = e.parentNode.parentNode;
	const field_name = par.childNodes[0].textContent;
	const b = $$$confirm("New value: True or false");
	$$$ajax_POST_data_w_text_response_generic_success("/user/permission/"+$$$get_id_of_user_currently_being_edited()+"/"+(b?"1":"0"), field_name);
	par.childNodes[1].textContent = b?"TRUE":"FALSE";
}

function $$$toggle_edit_user_permissions_tbl(user_id){
	if($$$is_visible('edit-user-permissions')){
		$$$hide('edit-user-permissions');
		return;
	}
	$$$unhide('edit-user-permissions');
}
