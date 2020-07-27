var $$$users_tags_dict;

function $$$toggle_admin_dashboard(){
	if(!$$$logged_in())
		return;
	$$$hide_all_except(['admin-dashboard']);
	$$$ajax_GET_w_JSON_response("/users",function(data){
		[$$$users_dict, $$$users_tags_dict] = data;
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

function $$$user_tag_bl_row(id,name){
	return "<div class='tr' data-id='"+id+"'><div class='td username'>"+name+"</div><div class='td' onclick='$$$rm_user_bl_tag(this.parentNode)'>Edit</div></div>";
}

function $$$load_user_for_edit(){
	const [name,vals,bl] = $$$users_dict[$$$get_id_of_user_currently_being_edited()];
	$$$document_getElementById("edit-user-permissions-username").innerText = name;
	let s = "";
	const ls = $$$get_tbl_body("edit-user-permissions-tbl").getElementsByClassName("tr");
	const vals_ls = vals.split(",");
	for(let i=0, n=vals_ls.length;  i<n;  ++i){
		ls[i].getElementsByClassName('td')[1].textContent = (vals_ls[i]==="1")?"TRUE":"FALSE";
	}
	
	s = "";
	for(let id of $$$split_on_commas_but_make_empty_if_empty(bl)){
		s += $$$user_tag_bl_row(id,$$$users_tags_dict[id][0]);
	}
	$$$get_tbl_body("edit-user-tag-bl-tbl").innerHTML = s;
}

function $$$edit_permission_value(e){
	const par = e.parentNode.parentNode;
	const field_name = par.childNodes[0].textContent;
	const b = $$$confirm("Allow "+$$$users_dict[$$$get_id_of_user_currently_being_edited()][0]+" access to "+field_name+"?");
	$$$ajax_POST_data_w_text_response_generic_success("/user/permission/"+$$$get_id_of_user_currently_being_edited()+"/"+(b?"1":"0"), field_name);
	par.childNodes[1].textContent = b?"TRUE":"FALSE";
}

function $$$toggle_edit_user_thing(id){
	if($$$is_visible(id)){
		$$$hide(id);
		return;
	}
	$$$unhide(id);
}

function $$$rm_user_bl_tag(e){
	$$$ajax_POST_w_text_response_generic_success("/user/bl/t-/"+$$$get_id_of_user_currently_being_edited()+"/"+e.dataset.id);
	e.remove();
}

function $$$add_new_user_tag_blacklist_rule(){
	const ls = $('#tagselect-useredit').val();
	if(ls.length===0)
		return;
	$$$ajax_POST_w_text_response("/user/bl/t+/"+$$$get_id_of_user_currently_being_edited()+"/"+ls.join(","), function(){
		let s = "";
		for(let x of $('#tagselect-useredit').select2('data'))
			s += $$$user_tag_bl_row(x.id,x.text);
		$$$get_tbl_body("edit-user-tag-bl-tbl").innerHTML += s;
		$('#tagselect-useredit').val(null).trigger('change');
	});
}

function $$$add_user(){
	const name = $$$prompt("User name");
	if((name===null)||(name===""))
		return;
	$$$ajax_POST_data_w_text_response_generic_success("/user/+",name);
}
