var $$$descr_contents; // NOTE: Only used within this file

function $$$get_descr_obj(){
	return $$$document_getElementById_descr;
}
function $$$set_description(s){
	// Called when displaying files, tags and dirs, not when the user edits it
	$$$get_descr_obj().innerText = s;
	$$$descr_contents = s;
}
function $$$get_description(){
	return $$$get_descr_obj().innerText;
}

function $$$mousedown_on_descr(){
	$$$descr_contents = $$$get_description();
}

function $$$mouseleave_descr(){
	const s = $$$get_description();
	if(s != $$$descr_contents){
		if (['f','d','t'].includes($$$currently_viewing_object_type)){
			$$$ajax_POST_data_w_text_response(
				$$$currently_viewing_object_type+"/descr/"+$$$id_of_currently_viewed_page(),
				$$$get_description(),
				$$$mousedown_on_descr
			);
		} else {
			$$$alert("Current object cannot have a description updated");
		}
	}
}
