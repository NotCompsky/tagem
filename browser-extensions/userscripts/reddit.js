// ==UserScript==
// @name	tagem-archive-reddit-posts
// @namespace	https://goooooooooooooooooooooooooooooooooooooooooooooooooooooooogle.com
// @description	Adds buttons to new.reddit.com (eugh!) that will tell the tagem server to archive the submissions
// @include	https://new.reddit.com/*
// @require https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js 
// @version 0.0.1
// ==/UserScript==

const tagem_server = "localhost:1999";

function add_post_to_db(){
	const score    = this.parentNode.childNodes[0].childNodes[0].childNodes[1].innerText;
	
	const username = this.parentNode.childNodes[1].getElementsByTagName('div')[2].getElementsByTagName('div')[0].getElementsByTagName('a')[0].innerText;
	const title    = this.parentNode.childNodes[1].childNodes[0].childNodes[0].childNodes[1].childNodes[1].childNodes[0].childNodes[0].childNodes[0].innerText;
	const link     = this.parentNode.childNodes[1].childNodes[0].childNodes[0].childNodes[2].childNodes[0].href;
	
	const permalink = this.parentNode.childNodes[1].childNodes[1].childNodes[1].childNodes[0].href;
	const n_cmnts   = this.parentNode.childNodes[1].childNodes[1].childNodes[1].childNodes[0].childNodes[1].innerText;
	
	$.ajax({
		method: "POST",
		url:"http://" + tagem_server + "/x/dl/reddit/p/" + permalink,
		success: function(){ alert("Success")},
		error:   function(){ alert("Error")}
	});
}

const post_containers = document.evaluate('/html/body/div[1]/div/div[2]/div[2]/div/div/div/div[2]/div[3]/div[1]/div[5]/div', document, null, XPathResult.ANY_TYPE, null);
var btn;
const posts = [];
var post;

window.addEventListener('load', function(){
	while(true){
		post = post_containers.iterateNext();
		if(post === null)
			break;
		posts.push(post.getElementsByTagName('div')[0].getElementsByTagName('div')[0]);
	}
	for(post of posts){
		btn = document.createElement("button");
		btn.innerText = "Record in DB";
		btn.onclick = add_post_to_db;
		post.appendChild(btn);
	}
}, false);
