function $$$timestamp2dt(t){
	try{
		return new Date(t*1000).toISOString().slice(-24, -5)
	}catch(e){
		return "N/A";
	}
}
function $$$t2human(t){
	return $$$timestamp2dt(t).substr(11);
}

function $$$set_t(id,t){
	$$$document_getElementById(id).innerText=$$$timestamp2dt(t);
}
function $$$set_t_added(t){
	$$$set_t('t-added', t);
}
function $$$set_t_origin(t){
	$$$set_t('t-origin', t);
}

function $$$dt2timestamp(dt){
	return Date.parse(dt);
}
