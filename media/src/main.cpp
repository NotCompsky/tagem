#ifdef PYTHON
# include <Python.h>
// WARNING: Must be included before any Qt includes, because Qt is greedy and slots is a macro name.
#endif

#include "mainwindow.hpp"
#include "file2.hpp"
#include <compsky/mysql/query.hpp>

#include <QApplication>
#include <QCompleter>
#include <QStringList>

#ifdef VID
  #include <QtAVWidgets>
#endif

#include <map> // for std::map
#include <cstdlib> // for atof



namespace _mysql {
	MYSQL* obj;
	constexpr static const size_t auth_sz = 512;
	char auth[auth_sz];
}
MYSQL_RES* RES1;
MYSQL_ROW ROW1;
MYSQL_RES* RES2;
MYSQL_ROW ROW2;


int dummy_argc = 0;
char** dummy_argv;


QCompleter* tagcompleter;
QStringList tagslist;
std::map<uint64_t, QString> tag_id2name;


MainWindow* player_ptr;


#ifdef PYTHON
static
PyObject* pymodule_tagem_jump_to_file(PyObject* self,  PyObject* args){
	uint64_t file_id;
	
	if (!PyArg_ParseTuple(args, "K", &file_id))
		return nullptr;
	
	player_ptr->jump_to_file(file_id);
	
	Py_RETURN_NONE;
}

# ifdef ERA
static
PyObject* pymodule_tagem_jump_to_era(PyObject* self,  PyObject* args){
	uint64_t  era_id;
	
	if (!PyArg_ParseTuple(args, "K", &era_id))
		return nullptr;
	
	player_ptr->jump_to_era(era_id);
	
	Py_RETURN_NONE;
}
# endif

static
PyMethodDef pymodule_tagem_methods[] = {
	{"jump_to_file", (PyCFunction)pymodule_tagem_jump_to_file, METH_VARARGS, "Description."},
# ifdef ERA
	{"jump_to_era",  (PyCFunction)pymodule_tagem_jump_to_era,  METH_VARARGS, "Description."},
# endif
	{nullptr, nullptr, 0, nullptr}
};

struct module_state {
	PyObject *error;
};

static
struct PyModuleDef pymodule_tagem = {
	PyModuleDef_HEAD_INIT,
	"tagem",
	nullptr,
	sizeof(struct module_state),
	pymodule_tagem_methods,
	nullptr,
	0,
	0,
	nullptr
};

static
PyObject* PyInit_tagem(){
	return PyModule_Create(&pymodule_tagem);
}
#endif


int main(const int argc,  const char** argv){
# ifdef PYTHON
	Py_SetProgramName(L"tagem");
	PyImport_AppendInittab("tagem", &PyInit_tagem);
	Py_Initialize();
# endif
	
    compsky::mysql::init(_mysql::obj, _mysql::auth, _mysql::auth_sz, getenv("TAGEM_MYSQL_CFG"));
	
	file2::initialise();
	
# ifdef VID
	QtAV::Widgets::registerRenderers();
# endif
	QApplication app(dummy_argc, dummy_argv);
	MainWindow player;
	player_ptr = &player;
	
	bool show_gui = true;
	double volume = 1.0;
# ifdef VID
	bool display_video = true;
# endif
	while((*(++argv)) != 0){
		const char* const arg = *argv;
		
		if (arg[0] != '-'  ||  arg[1] == 0  ||  arg[2] != 0)
			break; // i.e. goto after_opts_parsed
		switch(arg[1]){
			case '-':
				goto after_opts_parsed;
		  #ifdef VID
			case 'a':
				player.auto_next = true;
				break;
			case 'v':
				// tmp - common flags such as "-v" and "-q" are misleading
				player.set_volume(atof(*(++argv)));
				break;
			case 'V':
				display_video = false;
				break;
		  #endif
			case 's':
				show_gui = false;
				break;
			default:
				return 33;
		}
	}
	after_opts_parsed:

# ifdef VID
	if (display_video)
		player.init_video_output();
# endif
	
	compsky::mysql::query_buffer(_mysql::obj,  RES1,  "SELECT id, name FROM tag");
	{
		uint64_t id;
		const char* name;
		while (compsky::mysql::assign_next_row(RES1,  &ROW1,  &id, &name)){
			const QString s = name;
			tag_id2name[id] = s;
			tagslist << s;
		}
	}
	tagcompleter = new QCompleter(tagslist);
	
	const char* const inlist_filter_rule = *argv;
	
	if (inlist_filter_rule != nullptr){
		if (player.inlist_filter_dialog->load(inlist_filter_rule))
			player.inlist_filter_dialog->get_results();
	}
    player.show();
	if (player.auto_next)
		player.media_next();
    
    int rc = app.exec();
    compsky::mysql::wipe_auth(_mysql::auth, _mysql::auth_sz);
# ifdef PYTHON
	Py_Finalize(); // Frees
# endif
    return rc;
}
