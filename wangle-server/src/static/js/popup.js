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
// Unused atm

function $$$make_popup(x){
	x.classList.add("popup");
}
function $$$make_popup_by_id(id){
	$$$make_popup($$$document_getElementById(id));
}
function $$$unmake_popup(x){
	x.classList.remove("popup");
}
function $$$unmake_popup_by_id(id){
	$$$unmake_popup($$$document_getElementById(id));
}
function $$$onClick_unmake_parent_popup(e){
	$$$unmake_popup(e.target.parentNode);
}
