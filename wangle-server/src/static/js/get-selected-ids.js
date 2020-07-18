function $$$get_selected_ids(tbl){
	let ids = "";
	for(let node of $$$get_tbl_body(tbl).getElementsByClassName('selected1')){
		ids += "," + node.dataset.id
	}
	return ids.substr(1);
}
