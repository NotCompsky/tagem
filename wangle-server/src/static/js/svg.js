function $$$svg_as_src(s){
	return "data:image/svg+xml;charset=utf-8,"+s;
}
function $$$svg_as_node_src(node,s){
	node.src=$$$svg_as_src(s);
}

function $$$set_src_to_file_svg(node){
	$$$svg_as_node_src(node, `!!!MACRO!!!SVG_FILE`);
}
function $$$set_src_to_tag_svg(node){
	$$$svg_as_node_src(node, `!!!MACRO!!!SVG_TAG`);
}
