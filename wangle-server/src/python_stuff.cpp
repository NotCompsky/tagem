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

#include "python_stuff.hpp"
#include "os.hpp"
#include "errors.hpp"
#include "python.hpp"
#include <compsky/macros/likely.hpp>
#include <stdexcept>
#include <mutex>


extern const char* YTDL_FORMAT;


namespace python {

// youtube_dl.YoutubeDL(options).download(args)
// MODULE     INSTANTIATION      FUNCTION

static PyObj ytdl_obj;

namespace tagem_module {
	static PyObject* to_stdout;
	
	static PyObject* modul;
	
	static PyObj file_path;
	
	static
	PyObject* to_stdout_fn(PyObject* const self,  PyObject* const args,  PyObject* const keyword_args){
		// Overrides to_stdout - saves file path instead of writing to stdout
		
		// NOTE: self is the module, not the class instantiation, for some reason.
		// So as a workaround, static C-string is set, and mutex lock is needed in the calling function
		// In reality, the download() function could take many minutes, so a mutex lock is completely infeasible. Hence we must *hope* that there won't be a data race.
		
		PyObject* const _file_path = PyTuple_GetItem(args, 0);
		if (unlikely(_file_path == nullptr)){
			Py_RETURN_NONE;
		}
		file_path.obj = _file_path;
		const char* const str = file_path.as_str();
		if (not os::is_local_file_or_dir(str))
			Py_RETURN_NONE;
		log("filepath == ", str);
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

void init_ytdl(){
	Py_SetProgramName(L"tagem");
	Py_Initialize();
	PyEval_InitThreads();
	tagem_module::modul = PyModule_Create(&tagem_module::_module);
	tagem_module::to_stdout = PyObject_GetAttrString(tagem_module::modul, "to_stdout");
	
	PyObject* ytdl_module = PyImport_ImportModule("youtube_dl");
	PY_ASSERT_NOT_NULL(ytdl_module, "Cannot import youtube_dl");
	ytdl_obj.obj = PyObject_GetAttrString(ytdl_module, "YoutubeDL");
	
	PyEval_SaveThread(); // i.e. Py_BEGIN_ALLOW_THREADS
}

bool ytdl(char* const out_fmt_as_input__resulting_fp_as_output,  const char* const url){
	GILLock gillock();
	
	PyDict<4> opts(
		"quiet", Py_True,
		"forcefilename", Py_True,
		"outtmpl", PyUnicode_FromString(out_fmt_as_input__resulting_fp_as_output),
		"format", PyUnicode_FromString(YTDL_FORMAT)
	);
	Py_INCREF(Py_True);
	Py_INCREF(Py_True);
	PyObj ytdl_instantiation(ytdl_obj.call(opts.obj));
	// Override to_stdout, so that the file path is written to a buffer instead of stdout
	ytdl_instantiation.set_attr("to_stdout", tagem_module::to_stdout);
	Py_INCREF(tagem_module::to_stdout);
	/*PyObject_SetAttrString(tagem_module::modul, "ytdl_instantiation", ytdl_instantiation);
	PyRun_String("tagem.ytdl_instantiation.to_stdout = tagem.to_stdout", Py_single_input, nullptr, nullptr);
	// The C API call acts differently - the first argument is NOT a pointer to the 'self' object, but remains a pointer to the module object*/
	
	ytdl_instantiation.call_fn_void("download", PyObj(Py_BuildValue("[s]", url)).obj); // NOTE: Segfaults the second time it is called; appears to be caused by the garbage collector, most likely threading issues.
	
	const bool failed = (unlikely(PyErr_Occurred() != nullptr));
	if (not failed){
		tagem_module::file_path.copy_str(out_fmt_as_input__resulting_fp_as_output);
		Py_DECREF(tagem_module::file_path.obj);
	}
	
	return failed;
}

} // namespace python
