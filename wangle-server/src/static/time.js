function $$$timestamp2dt(t){
	return new Date(t*1000).toISOString().slice(-24, -5)
}

function $$$set_t_added(t){
	$$$document_getElementById('t-added').innerText = $$$timestamp2dt(t);
}
