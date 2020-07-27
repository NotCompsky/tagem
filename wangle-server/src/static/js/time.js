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
