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
function $$$edit_title_path(){
	switch($$$currently_viewing_object_type){
		case "f":
			return "/f/title/";
		case "t":
			return "/t/name/";
		default:
			return null;
	}
}
function $$$edit_title(){
	// Basically edit_name() for tags, directories, etc.
	// Only edit_title() for files in reality, as the file names are not for human parsing
	const url = $$$edit_title_path();
	if(url===null)
		return;
	const s = $$$prompt("New title", $$$get_profile_name());
	if((s==="")||(s===null))
		return;
	$$$ajax_POST_data_w_text_response(
		url+$$$id_of_currently_viewed_page(),
		s,
		function(){
			$$$set_profile_name(s);
		}
	);
}
