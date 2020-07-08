const $$$Mousetrap_bind   = Mousetrap.bind;
const $$$Mousetrap_unbind = Mousetrap.unbind;
const $$$document = document;
const $$$document_getElementById = $$$document.getElementById.bind($$$document);
const $$$document_getElementsByClassName = $$$document.getElementsByClassName.bind($$$document);
const $$$confirm = confirm;
const $$$prompt = prompt;

function $$$for_node_in_document_getElementsByClassName(s, fn){
	for(let e of $$$document_getElementsByClassName(s))
		fn(e);
}

function $$$get_tbl_body(id){
	return $$$document_getElementById(id).getElementsByClassName('tbody')[0];
}
