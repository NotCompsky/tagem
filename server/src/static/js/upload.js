var $$$imgcanvas;


// NOTE: Use lvh.me - it is an alias for localhost, and therefore browsers allow CORS from it
// For some reason, browsers decided that your own machine (localhost) is actually *less* trustworthy than every other machine on the planet (every other server), so requests made from localhost are only allowed to send, and not receive, information. lvh.me works around that genius restriction.



function $$$render_img_canvas_as_png(){
	$$$imgcanvas.getContext("2d").drawImage($$$document_getElementById_view_img, 0,0,256,256);
}

function $$$render_yandex_results(json){
	console.log("$$$render_yandex_results");
	console.log(json);
}


function $$$upload_img_given_serpid(blob, serpid){
	console.log("$$$upload_img_given_serpid");
	const fd = new FormData();
	fd.append("french.png", blob);
	fetch(
		`https://cors-anywhere.herokuapp.com/https://yandex.com/images/search?serpid=${serpid}&serpListType=horizontal&thumbSnippet=0&uinfo=sw-1920-sh-1080-ww-1920-wh-612-pd-1-wp-16x9_1920x1080&rpt=imageview&format=json&request=%7B%22blocks%22%3A%5B%7B%22block%22%3A%22cbir-controller__upload%3Aajax%22%7D%5D%7D`,
		{
			method:"POST",
			mode:'cors',
			credentials:'include',
			body:fd,
			headers: [
				['Accept',"application/json"],
				["Content-Type","multipart/form-data"],
				["Origin","https://yandex.com"],
				["Referer","https://yandex.com/images/"],
				["Sec-GPC","1"],
			],
		}
	).then(r => {
		if(!r.ok)
			throw Error(`Server returned ${r.status}: ${r.statusText}`);
		r.json().then(j => {
			$$$render_yandex_results(j);
		});
	}).catch($$$err_alert);
}

function $$$upload_img_callback(blob){
	console.log("$$$upload_img_callback");
	fetch(
		`https://cors-anywhere.herokuapp.com/https://yandex.com/images/search`,
		{
			method:"GET",
			mode:'cors',
			credentials:'include',
			headers: [
				['Accept',"text/html"],
				["Origin","https://yandex.com"],
				["Sec-GPC","1"],
			],
		}
	).then(r => {
		if(!r.ok)
			throw Error(`Server returned ${r.status}: ${r.statusText}`);
		r.text().then(s => {
			console.log("s ==");
			console.log(s);
			$$$upload_img_given_serpid(blob, s.match(/"serpid":"([^"]+)",/)[1]);
		});
	}).catch($$$err_alert);
}

function $$$upload_img_to_yandex(){
	$$$render_img_canvas_as_png();
	console.log("rendered canvas");
	$$$imgcanvas.toBlob($$$upload_img_callback,"image/png",0.9);
}
