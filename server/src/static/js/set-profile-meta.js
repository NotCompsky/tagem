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

function $$$set_profile_name(s){
	$$$document.title=s;
	$$$document_getElementById_profile_name.innerText = s;
}

function $$$get_profile_name(){
	return $$$document_getElementById_profile_name.innerText;
}

function $$$set_profile_thumb(url){
	if(url==='d'){
		$$$unhide('profile-img-d');
		$$$hide('profile-img-qry');
		$$$hide('profile-img-etc');
		return;
	}
	if(url==='qry'){
		$$$hide('profile-img-d');
		$$$unhide('profile-img-qry');
		$$$hide('profile-img-etc');
		return;
	}
	$$$hide('profile-img-d');
	$$$hide('profile-img-qry');
	$$$unhide('profile-img-etc');
	$$$document_getElementById('profile-img-etc').src = url;
}
