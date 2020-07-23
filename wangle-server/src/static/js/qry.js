function $$$unhide_qry_help(){
	$$$unhide('qry-help');
}
function $$$view_qry(s){
	if(s==="")
		return $$$unhide_qry_help();
	$$$ajax_POST_data_w_JSON_response_and_err("/q/", s+" ", function(data){
		switch(s[0]){
			case 'e': $$$display_f_tbl(data); $$$setup_page_for_e_tbl(); break;
			case 'f': $$$display_f_tbl(data); $$$setup_page_for_f_tbl(); break;
			case 'd': $$$display_d_tbl(data); $$$setup_page_for_d_tbl(); break;
			case 't': $$$display_t_tbl(data); $$$setup_page_for_t_tbl(); break;
		}
		$$$set_window_location_hash('?'+s);
		$$$set_profile_name('?'+s);
		$$$set_profile_thumb('qry');
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
