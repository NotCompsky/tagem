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
function $$$switch_stylesheet(s){
	for(let e of $$$document_getElementsByClassName('table')){
		for(let c of $$$stylesheet_opts)
			e.classList.remove('tbl-'+c);
		e.classList.add('tbl-'+s);
	}
	$$$local_storage_store("css", s);
}
function $$$rotate_stylesheet(){
	let i = $$$stylesheet_opts.indexOf($$$local_storage_get("css")) + 1;
	if(i===$$$stylesheet_opts.length)
		i=0;
	$$$switch_stylesheet($$$stylesheet_opts[i]);
}
