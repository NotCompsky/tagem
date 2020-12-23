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
function $$$login(){
	const uname = $$$prompt("Username");
	if((uname==="")||(uname===null))
		return;
	$$$set_cookie_forever("username", uname); // Super 100% secure login with inbuilt blockchain neural networks
	$$$document_getElementById_username.innerText = uname;
	$$$refetch_all_jsons();
}
function $$$logout(){
	if(!$$$logged_in())
		return;
	$$$unset_cookie("username");
	$$$document_getElementById_username.innerText = "GUEST";
	$$$refetch_all_jsons();
}


function $$$logged_in(){
	return ($$$get_cookie("username")!==undefined);
}
