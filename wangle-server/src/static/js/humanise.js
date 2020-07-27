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
const $$$BYTES_UNITS=['','K','M','G','T','P'];

function $$$bytes2human(n){
	if(n===null)
		return "N/A";
	let i = 0;
	while(n > 1024){
		++i;
		n /= 1024;
	}
	return n.toFixed(1) + ' ' + $$$BYTES_UNITS[i] + 'iB';
}

// Dehumanise
function $$$_abs_human2bytes(n, unit, magnitude){
	return n*(magnitude**$$$BYTES_UNITS.indexOf(unit))
}
function $$$human2bytes(s){
	const [_, n, unit, ibi_or_iga] = new RegExp('^([0-9]+(?:\.[0-9]+)?)(|[KMGTP])(i)?B$').exec(s);
	return $$$_abs_human2bytes(parseFloat(n), unit, (ibi_or_iga===undefined)?1000:1024)
}


const $$$NUMBER_UNITS=['','K','M','B','T'];

function $$$n2human(n){
	if(n===null)
		return "N/A";
	let i = 0;
	while(n > 1024){
		++i;
		n /= 1024;
	}
	return n.toFixed(1) + $$$NUMBER_UNITS[i];
}
