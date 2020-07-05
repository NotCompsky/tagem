function $$$set_profile_name(s){
	$$$document.title=s;
	$$$document_getElementById('profile-name').innerText = s;
}
function $$$set_profile_thumb(url){
	$$$document_getElementById('profile-img').src = url;
}
function $$$set_profile_cover(url){
	$$$document_getElementById('profile-cover').src = url;
}
