function $$$unhide_qry_help(){
	$$$unhide('qry-help');
}
function $$$view_qry(s){
	if(s==="")
		return $$$unhide_qry_help();
	$$$ajax_POST_data_w_JSON_response_and_err("/q/", s+" ", function(data){
		switch(s[0]){
			case 'e': $$$view_eras(data); break;
			case 'f': $$$view_files(data); break;
			case 'd': $$$view_dirs(data.map(x => x.toString()));  break;
			case 't': $$$view_tags(data.map(x => x.toString()));  break;
		}
		$$$set_window_location_hash('?'+s);
		$$$set_profile_name('?'+s);
		$$$set_profile_thumb('qry');
		$$$set_profile_cover('https://ichef.bbci.co.uk/images/ic/1008x567/p07nzhmt.jpg');
	},
	$$$unhide_qry_help
	);
}

function $$$init_qry(){
	$$$document_getElementById("qry").addEventListener("keyup", function(e){
		e.preventDefault();
		if (e.keyCode === 13){ // Enter
			$$$view_qry(this.value);
		}
	});
}
