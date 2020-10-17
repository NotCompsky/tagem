// 
// Copyright 2020 Adam Gray
// This file is part of the tagem program.
// The tagem program is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published by the
// Free Software Foundation version 3 of the License.
// The tagem program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// This copyright notice should be included in any copy or substantial copy of the tagem source code.
// The absense of this copyright notices on some other files in this project does not indicate that those files do not also fall under this license, unless they have a different license written at the top of the file.
// 
function $$$unhide_qry_help(){
	$$$unhide('qry-help');
}
function $$$view_qry(s){
	if(s==="")
		return $$$unhide_qry_help();
	$$$ajax_POST_data_w_JSON_response_and_err("/q/", s+" ", function(data){
		if ($$$is_integer(data)){
			$$$set_profile_name("="+data);
			return;
		}
		if ($$$is_string(data)){
			$$$document_getElementById("qry-textarea").innerText = data;
			$$$unhide("qry-textarea");
			return;
		}
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
