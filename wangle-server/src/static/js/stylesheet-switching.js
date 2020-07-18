function $$$switch_stylesheet(s){
	for(let e of $$$document_getElementsByClassName('table')){
		for(let c of $$$stylesheet_opts)
			e.classList.remove('tbl-'+c);
		e.classList.add('tbl-'+s);
	}
	$$$set_cookie("css", s, 3600);
}
function $$$rotate_stylesheet(){
	let i = $$$stylesheet_opts.indexOf($$$get_cookie("css")) + 1;
	if(i===$$$stylesheet_opts.length)
		i=0;
	$$$switch_stylesheet($$$stylesheet_opts[i]);
}
