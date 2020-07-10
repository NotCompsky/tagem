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
