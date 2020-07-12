function $$$edit_title_path(){
	switch($$$currently_viewing_object_type){
		case "f":
			return "/f/title/";
		default:
			return null;
	}
}
function $$$edit_title(){
	// Basically edit_name() for tags, directories, etc.
	// Only edit_title() for files in reality, as the file names are not for human parsing
	const s = $$$prompt("New title");
	if((s==="")||(s===null))
		return;
	const url = $$$edit_title_path();
	if(url===null)
		return;
	$$$ajax_POST_data_w_text_response(
		url+$$$file_id,
		s,
		function(){
			$$$set_profile_name(s);
		}
	);
}
