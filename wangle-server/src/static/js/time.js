function $$$timesetamp2dt_ls(t){
	try{
		return new Date(t*1000).toISOString().slice(-24, -5).split("T");
	}catch(e){
		return null;
	}
}

function $$$timestamp2dt(t){
	try{
		const ls = $$$timesetamp2dt_ls(t);
		return "<div class='date'>"+ls[0]+"</div><div class='time'>"+ls[1]+"</div>";
	}catch(e){
		return "N/A";
	}
}

function $$$t2human(t){
	try{
		const ls = $$$timesetamp2dt_ls(t);
		return "<div class='time'>"+ls[1]+"</div>";
	}catch(e){
		return "N/A";
	}
}

function $$$set_t(id,t){
	$$$document_getElementById(id).innerHTML=$$$timestamp2dt(t);
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
