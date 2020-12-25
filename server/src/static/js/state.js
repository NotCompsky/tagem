function $$$tbl_alias_to_id_value(alias){
	switch(alias){
		case 'f':
			return $$$file_id;
		case 'd':
			return $$$dir_id;
		case 't':
			return $$$tag_id;
	}
}
function $$$id_of_currently_viewed_page(){
	return $$$tbl_alias_to_id_value($$$currently_viewing_object_type);
}
