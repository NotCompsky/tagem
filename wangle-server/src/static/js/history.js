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
function $$$add_window_location_hash_to_history(s){
	if((s==="")||($$$document.title===""))
		return;
	const x = $$$document_getElementById('recent-pages');
	for(node of x.childNodes){
		if(node.innerText===$$$document.title)
			return;
	}
	if(x.childNodes.length===10)
		x.removeChild(x.childNodes[9]);
	x.innerHTML = "<a onclick='$$$load_page_from_a_hash_string(\""+s+"\")'>" + $$$document.title + "</a>" + x.innerHTML; // WARNING: Not escaped
}
function $$$set_window_location_hash(s){
	const S = "#" + s;
	$$$add_window_location_hash_to_history(S);
	$$$currently_viewing_object_type = s[0];
	if(S!=history.state)
		history.pushState(S, "", S);
}
function $$$unset_window_location_hash(){
	$$$set_window_location_hash("");
}
