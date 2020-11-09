/*
Copyright 2020 Adam Gray
This file is part of the tagem program.
The tagem program is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by the
Free Software Foundation version 3 of the License.
The tagem program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
This copyright notice should be included in any copy or substantial copy of the tagem source code.
The absense of this copyright notices on some other files in this project does not indicate that those files do not also fall under this license, unless they have a different license written at the top of the file.
*/

#pragma once

#include "python_stuff.hpp"
#include "os.hpp"
#include "errors.hpp"
#include "python.hpp"
#include <compsky/macros/likely.hpp>
#include <stdexcept>


extern const char* YTDL_FORMAT;
extern const char* FFMPEG_LOCATION;


namespace python {

// youtube_dl.YoutubeDL(options).download(args)
// MODULE     INSTANTIATION      FUNCTION

static PyObj ytdl_obj;
static PyObj view_remote_dir__parse_url_fn;

namespace tagem_module {
	static PyObject* to_stdout;
	
	static PyObject* modul;
	
	static PyObject* file_path;
	static PyObject* json_metadata;
	
	static PyObject* ffmpeg_location;
	
	static
	PyObject* to_stdout_fn(PyObject* const self,  PyObject* const args,  PyObject* const keyword_args){
		// Overrides to_stdout - saves file path instead of writing to stdout
		
		// NOTE: self is the module, not the class instantiation, for some reason.
		// So as a workaround, static C-string is set, and mutex lock is needed in the calling function
		// In reality, the download() function could take many minutes, so a mutex lock is completely infeasible. Hence we must *hope* that there won't be a data race.
		
		const PyObj data(PyTuple_GetItem(args, 0));
		if (unlikely(data.obj == nullptr))
			Py_RETURN_NONE;
		const char* const str = data.as_str();
		Py_INCREF(data.obj);
		if (str[0] == '{')
			json_metadata = data.obj;
		else if (os::is_local_file_or_dir(str))
			file_path = data.obj;
		else
			Py_DECREF(data.obj);
		Py_RETURN_NONE;
	}
	
	static
	PyMethodDef _methods[] = {
		{"to_stdout", (PyCFunction)to_stdout_fn, METH_VARARGS | METH_KEYWORDS, "Description."},
		{nullptr, nullptr, 0, nullptr}
	};
	
	struct _state {
		PyObject* error;
	};
	
	static
	struct PyModuleDef _module = {
		PyModuleDef_HEAD_INIT,
		"tagem",
		nullptr,
		sizeof(struct _state),
		_methods,
		nullptr,
		0,
		0,
		nullptr
	};
}


bool is_valid_ytdl_url(const char* const url){
	PyDict<3> opts(
		"quiet", Py_False,
		"simulate", Py_True,
		"skip_download", Py_True
	);
	Py_INCREF(Py_False);
	Py_INCREF(Py_True);
	Py_INCREF(Py_True);
	
	PyObj iterator(PyObject_GetIter(PyObj(ytdl_obj.call(opts.obj), "_ies").obj));
	if (unlikely(iterator.obj == nullptr)){
		log("Cannot validate ytdl URL");
		return true;
	}
	PyStr pyurl(url);
	while(true){
		PyObj item(PyIter_Next(iterator.obj));
		if (unlikely(item.obj == nullptr))
			break;
		if (strcmp(PyObj(item, "__name__").as_str(), "GenericIE") == 0)
			continue;
		if (item.call_fn_bool("suitable", pyurl.obj))
			return true;
	}
	log("Invalid ytdl URL: ", url);
}


PyObject* attempt_import_failover(const char* const name){
	log("Unable to import ", name);
	abort();
}

template<typename... Strings>
PyObject* attempt_import_failover(const char* const name,  const char* const module_name,  Strings... module_names){
	PyObject* o = PyImport_ImportModule(module_name);
	return o ? o : attempt_import_failover(name, module_names...);
}


void init_ytdl(){
	Py_SetProgramName(L"tagem");
	Py_Initialize();
	tagem_module::modul = PyModule_Create(&tagem_module::_module);
	tagem_module::to_stdout = PyObject_GetAttrString(tagem_module::modul, "to_stdout");
	tagem_module::ffmpeg_location = PyUnicode_FromString(FFMPEG_LOCATION);
	
	PyObject* const ytdl_module = attempt_import_failover("youtube-dl", "youtube_dlc", "youtube_dl");
	ytdl_obj.obj = PyObject_GetAttrString(ytdl_module, "YoutubeDL");
}

PyObject* create_mysql_obj(const DatabaseInfo& db_info){
	PyObject* _module = PyImport_ImportModule("pymysql");
	if (unlikely(_module == nullptr))
		throw std::runtime_error("Python: Cannot import pymysql");
	
	return PyObj(_module, "connect").call(PyStr(db_info.host()).obj, PyStr(db_info.user()).obj, PyStr(db_info.pwrd()).obj, PyStr(db_info.name()).obj, PyInt(db_info.port()).obj, PyStr(db_info.path()).obj);
}

void import_view_remote_dir(const DatabaseInfo& db_info){
	/*
	PyObject* _module = PyImport_ImportModule("youtube_dl");
	if (unlikely(_module == nullptr))
		throw std::runtime_error("Python: Cannot import youtube_dl");
	*/
	
	PyObj conn(create_mysql_obj(db_info), "cursor");
	Py_INCREF(conn.obj);
}

bool view_remote_dir(char* buf,  const char* const url){
	return false;
}

} // namespace python
