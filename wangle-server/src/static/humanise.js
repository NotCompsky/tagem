const $$$BYTES_UNITS=['','K','M','G','T','P'];

function $$$bytes2human(n){
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
	let i = 0;
	while(n > 1024){
		++i;
		n /= 1024;
	}
	return n.toFixed(1) + $$$NUMBER_UNITS[i];
}
