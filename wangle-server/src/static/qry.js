function $$$view_qry(s){
	$$$ajax_POST_data_w_JSON_response("/q/", s+" ", function(data){
		switch(s[0]){
			case 'f': $$$view_files(data); break;
			case 'd': $$$view_dirs(data.map(x => x.toString()));  break;
			case 't': $$$view_tags(data.map(x => x.toString()));  break;
		}
		$$$set_window_location_hash('?'+s);
		$$$set_profile_name('?'+s);
	});
}

function $$$init_qry(){
	$$$document_getElementById("qry").addEventListener("keyup", function(e){
		e.preventDefault();
		if (e.keyCode === 13){ // Enter
			$$$view_qry(this.value);
		}
	});
}
