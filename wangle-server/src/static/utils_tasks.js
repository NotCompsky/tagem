function $$$view_tasks(){
	$$$hide_all_except(['tasks-container']);
	$.get({
		url:"/a/exec.json",
		success:function(data){
			let _s = "";
			for(const [id,name,descr,content] of data){
				_s += `
					<div class='task'>
						<h3 class='task-name'>" + name + "</h3>
						<p class='task-descr'>" + descr + "</p>
						<a onclick='$$$toggle(\\"task-content" + id + "\\")'>Content</a>
						<div id='task-content" + id + "' class='hidden task-content'>
							` + content + `
						</div>
						<a onclick='$$$exec_task(" + id + ")'>Exec</a>
					</div>
				`; // WARNING: No HTML escaping. TODO: Fix this.
			}
			$$$document_getElementById('tasks').innerHTML = _s;
		},
		error: function(){
			alert("Not authorised. Please log in first.");
		}
	});
}

function $$$exec_task(task_id){
	$.post({
		url: "/exec/" + task_id,
		success:function(){
			alert("Done");
		},
		error:$$$err_alert
	});
}
