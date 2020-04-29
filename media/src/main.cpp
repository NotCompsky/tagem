#ifdef PYTHON
# ifndef MAINWINDOW
#  error "So far Python functions only exist for MainWindow"
# endif
# include <Python.h>
// WARNING: Must be included before any Qt includes, because Qt is greedy and slots is a macro name.
#endif

#ifdef MAINWINDOW
# include "mainwindow.hpp"
#else
# define BUF_SZ (2 * 4096)
  char BUF[BUF_SZ];
#endif
#ifdef FILE2
# include "file2.hpp"
#endif
#if (defined TAG_MANAGER || defined MAINWINDOW)
# include "tag-manager/mainwindow.hpp"
#endif
#include <compsky/mysql/query.hpp>

#include <QApplication>
#include <QStringList>

#ifdef VID
  #include <QtAVWidgets>
#endif

#include <map> // for std::map
#include <cstdlib> // for atof

#include "completer/completer.hpp"



namespace _mysql {
	MYSQL* obj;
	constexpr static const size_t auth_sz = 512;
	char auth[auth_sz];
}
MYSQL_RES* RES1;
MYSQL_ROW ROW1;
MYSQL_RES* RES2;
MYSQL_ROW ROW2;


TagManager* tag_manager;


int dummy_argc = 0;
char** dummy_argv;


Completer tagcompleter;
QStringList tagslist;
std::map<uint64_t, QString> tag_id2name;


#ifdef MAINWINDOW
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

static
PyObject* pymodule_tagem_jump_to_era_tagged(PyObject* self,  PyObject* args){
	/*
	Python call:
	jump_to_era_tagged( [REQUIRED_TAGS], [FORBIDDEN_TAGS] )
	*/
	
	std::vector<MainWindow::TagstrAndNot> tagstrs;
	
	constexpr static const char* const reverse_indx__to__andnot[2] = {"", "NOT"};
	Py_ssize_t n_args = PyTuple_Size(args);
	
	while(n_args != 0){
		--n_args;
		const char* const andnot = reverse_indx__to__andnot[n_args];
		
		PyObject* const py_list = PyTuple_GetItem(args, n_args);
		
		Py_ssize_t n = PyList_Size(py_list);
		while (n != 0) {
			PyObject* const py_item = PyList_GetItem(py_list, --n);
			const char* const tagstr = const_cast<const char*>(PyUnicode_AsUTF8(py_item));
			tagstrs.emplace_back(tagstr, andnot);
		}
	}
	
	player_ptr->jump_to_era_tagged(tagstrs);
	Py_RETURN_NONE;
}
# endif

static
PyMethodDef pymodule_tagem_methods[] = {
	{"jump_to_file", (PyCFunction)pymodule_tagem_jump_to_file, METH_VARARGS, "Description."},
# ifdef ERA
	{"jump_to_era",  (PyCFunction)pymodule_tagem_jump_to_era,  METH_VARARGS, "Description."},
	{"jump_to_era_tagged",  (PyCFunction)pymodule_tagem_jump_to_era_tagged,  METH_VARARGS, "Description."},
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
#else
nullptr_t player_ptr;
#endif // #ifdef MAINWINDOW


int main(const int argc,  const char** argv){
# ifdef PYTHON
	Py_SetProgramName(L"tagem");
	PyImport_AppendInittab("tagem", &PyInit_tagem);
	Py_Initialize();
# endif
	
    compsky::mysql::init(_mysql::obj, _mysql::auth, _mysql::auth_sz, getenv("TAGEM_MYSQL_CFG"));

#ifdef FILE2
	file2::initialise();
#endif
# ifdef VID
	QtAV::Widgets::registerRenderers();
# endif
	QApplication app(dummy_argc, dummy_argv);
	
#ifdef MAINWINDOW
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
			case 'e':
				player.readonly = false;
				break;
		  #ifdef ERA
			case 'E': {
				const char* const _tblname = *(++argv);
				memcpy(era2s_tblname,  _tblname,  strlen(_tblname) + 1);
				break;
			}
		  #endif
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
			case 'o':
				player.file_stuff_to_stdout_not_db = true;
				break;
			case 's':
				show_gui = false;
				break;
			case 'T':
				player.titles = true;
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
#else
	player_ptr = nullptr; // For use with tag_manager
#endif // #ifdef MAINWINDOW
	
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
	tagcompleter.reset(tagslist);
	
#ifdef TAG_MANAGER
	tag_manager = new TagManager(player_ptr);
	tag_manager->show();
#endif
#ifdef MAINWINDOW
	const char* const inlist_filter_rule = *argv;
	
	if (inlist_filter_rule != nullptr){
		if (player.inlist_filter_dialog->load(inlist_filter_rule))
			player.inlist_filter_dialog->get_results();
	}
    player.show();
	if (player.auto_next)
		player.media_next();
#endif
	
    int rc = app.exec();
    compsky::mysql::wipe_auth(_mysql::auth, _mysql::auth_sz);
# ifdef PYTHON
	Py_Finalize(); // Frees
# endif
    return rc;
}
