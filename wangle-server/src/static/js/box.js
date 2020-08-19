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

// WARNING: Boxes are messed up for images displayed within <iframe>. I do not think there is an easy fix to this, and no fixed is currently planned. Images should only be displayed in <img> tag, and if they are displayed in an <iframe> that is because the wrong mimetype has been set.

function $$$add_empty_box_given_tags(tags){
	$$$draw_box(["0", 0, 0.25, 0.25, 0.5, 0.5, tags]);
}

function $$$add_empty_box(){
	$$$prompt_for_tags($$$add_empty_box_given_tags);
}

function $$$draw_box(box){
	const [id,frame,x,y,w,h,tags] = box;
	const e = $$$document.createElement("div");
	e.dataset.id = id;
	
	// e.style.transform = "translateX("+100*x+"%) translateY("+100*y+"%)";
	// The insane thing is that CSS translate rules use the node's own dimensions for translates here, for no apparent reason. Even animation translates use the parent node's dimensions!
	// So to get around this, we create a container <div>, set its width to 100% of the parent, and translate that instead.
	const why = $$$document.createElement("div");
	why.style.transform = "translateX("+100*x+"%) translateY("+100*y+"%)";
	why.classList = "box-container unselectable";
	
	// transforms due to https://www.w3.org/TR/css-transforms-1/#containing-block-for-all-descendants (see CSS)
	e.style.width  = 100*w+"%"
	e.style.minHeight = 100*h+"%";
	e.style.maxHeight = 100*h+"%";
	// Both Firefox and Chrome treat height attribute on its own as min-height, and max-height+height as min-height+min-height.
	e.classList = "box";
	e.dataset.frame_n = frame;
	e.dataset.w = w;
	e.dataset.h = h;
	e.dataset.x = x;
	e.dataset.y = y;
	e.innerHTML =
		'<div class="resizer topL"></div><div class="resizer topR"></div>' +
		tags.map(([id,name]) => $$$display_tag(id, name, null, "$$$unlink_this_tag_from_this_box", 'b')).join("") +
		'<div class="bottom-anchor"><div class="resizer botL"></div><div class="resizer botR"></div></div>'
	;
	
	$$$make_resizeable(e);
	why.prepend(e);
	$$$document_getElementById("view").appendChild(why);
}


function $$$make_resizeable(box){
	// NOTE: Although we are dragging the item by an amount in pixels, we must only set the width to a % of it's parent (image) width.
	const min_size_px = 10.0;
	for (const resizer of box.getElementsByClassName("resizer")){
		resizer.addEventListener('mousedown', function(e){
			const corner_orig_x_px = e.pageX;
			const corner_orig_y_px = e.pageY;
			
			const box_orig_w_px = parseFloat(getComputedStyle(box).width.replace('px', ''));
			const box_orig_h_px = parseFloat(getComputedStyle(box).height.replace('px', ''));
			
			let x = parseFloat(box.dataset.x);
			let y = parseFloat(box.dataset.y);
			let w = parseFloat(box.dataset.w);
			let h = parseFloat(box.dataset.h);
			
			const xscale = parseFloat(box.dataset.w) / box_orig_w_px;
			const yscale = parseFloat(box.dataset.h) / box_orig_h_px;
			const min_x = min_size_px * xscale;
			const min_y = min_size_px * yscale;
			
			function user_dragging_box_event(e){
				console.log(xscale, yscale);
				console.log(e.pageX, e.pageY);
				console.log(corner_orig_x_px, corner_orig_y_px);
				
				const delta_y_norm = yscale * (e.pageY - corner_orig_y_px);
				if (resizer.classList.contains('topL') || resizer.classList.contains('topR')){
					console.log("Moving up by", delta_y_norm);
					y = parseFloat(box.dataset.y) + delta_y_norm;
					h = parseFloat(box.dataset.h) - delta_y_norm;
				} else {
					h = parseFloat(box.dataset.h) + delta_y_norm;
				}
				if(h >= min_y)
					box.style.minHeight = box.style.maxHeight = box.style.height = 100*h+"%";
				
				const delta_x_norm = xscale * (e.pageX - corner_orig_x_px);
				if (resizer.classList.contains('topL') || resizer.classList.contains('botL')){
					console.log("Moving left by", delta_x_norm);
					x = parseFloat(box.dataset.x) + delta_x_norm;
					w = parseFloat(box.dataset.w) - delta_x_norm;
				} else {
					w = parseFloat(box.dataset.w) + delta_x_norm;
				}
				if(w >= min_x)
					box.style.width = 100*w+"%";
				
				if ((!resizer.classList.contains('botR'))&&(h >= min_y)&&(w >= min_x))
					box.parentNode.style.transform = "translateX("+100*x+"%) translateY("+100*y+"%)";
			}
			
			window.addEventListener('mousemove', user_dragging_box_event);
			window.addEventListener('mouseup', function(){
				if(w >= min_x)
					box.dataset.w = w;
				if(h >= min_y)
					box.dataset.h = h;
				if((x >= 0)&&(x <= 1))
					box.dataset.x = x;
				if((y >= 0)&&(y <= 1))
					box.dataset.y = y;
				window.removeEventListener('mousemove', user_dragging_box_event);
			});
		});
	}
}

function $$$get_box_tags(node){
	return Array.from(node.getElementsByClassName("tag")).map(e => e.dataset.id).join(",");
}

function $$$save_boxes(){
	$$$ajax_POST_data_w_JSON_response(
		"/box/add/"+$$$file_id,
		Array.from($$$document_getElementsByClassName("box")).map(e => e.dataset.id+";"+e.dataset.frame_n+";"+e.dataset.x+";"+e.dataset.y+";"+e.dataset.w+";"+e.dataset.h+";"+$$$get_box_tags(e)).join("\n")+"\n",
		function(box_ids){
			// Update the IDs of the newly recorded boxes
			let i = 0;
			for(let e of $$$document_getElementsByClassName("box")){
				if(e.dataset.id !== "0")
					continue;
				e.dataset.id = box_ids[i++];
			}
			$$$alert_success();
		}
	);
}
