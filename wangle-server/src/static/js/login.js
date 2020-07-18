function $$$login(){
	const uname = $$$prompt("Username");
	if((uname==="")||(uname===null))
		return;
	$$$set_cookie("username", uname, 3600*24); // Super 100% secure login with inbuilt blockchain neural networks
	$$$document_getElementById('username').innerText = uname;
	$$$refetch_all_jsons();
}
function $$$logout(){
	if(!$$$logged_in())
		return;
	$$$unset_cookie("username");
	$$$document_getElementById('username').innerText = "GUEST";
	$$$refetch_all_jsons();
}


function $$$logged_in(){
	return ($$$get_cookie("username")!==undefined);
}
