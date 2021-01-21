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

function $$$set_css_colour(name,val){
	const x = "--custom-"+name;
	if(val === undefined)
		val = $$$css_root.getPropertyValue(x);
	else
		$$$css_root.setProperty(x,val);
	$$$local_storage_store("css_colour_"+name, val);
}

function $$$on_css_colour_picker_change(e){
	const x = e.target;
	$$$set_css_colour(x.dataset.for,x.value);
}

function $$$set_css_theme(name){
	$$$local_storage_store("css-theme",name);
	$$$document_body.classList.add(name);
	for(let opt of $$$document_getElementById("css-theme").childNodes){
		if(opt.value !== name){
			$$$document_body.classList.remove(opt.value);
		}
	}
}
