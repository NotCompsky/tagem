function $$$set_profile_name(s){
	$$$document.title=s;
	$$$document_getElementById('profile-name').innerText = s;
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
function $$$set_profile_cover(url){
	$$$document_getElementById('profile-cover').src = url;
}
