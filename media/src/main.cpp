#include <QApplication>

#ifdef VID
  #include <QtAVWidgets>
#endif

#include <compsky/mysql/mysql.hpp> // for compsky::mysql::*, BUF, BUF_INDX

#include "mainwindow.hpp"

#include <cstdlib> // for atof



namespace _mysql {
	MYSQL* obj;
	constexpr static const size_t auth_sz = 512;
	char auth[auth_sz];
}


int dummy_argc = 0;
char** dummy_argv;


int main(const int argc,  const char** argv){
    compsky::mysql::init(_mysql::obj, _mysql::auth, _mysql::auth_sz, getenv("TAGEM_MYSQL_CFG"));
	
# ifdef VID
	QtAV::Widgets::registerRenderers();
# endif
	QApplication app(dummy_argc, dummy_argv);
	MainWindow player;
	
	bool show_gui = true;
	double volume = 1.0;
	while((*(++argv)) != 0){
		const char* const arg = *argv;
		
		if (arg[0] != '-'  ||  arg[1] == 0  ||  arg[2] != 0)
			break; // i.e. goto after_opts_parsed
		switch(arg[1]){
			case '-':
				goto after_opts_parsed;
			case 'a':
				player.auto_next = true;
				break;
			case 'v':
				// tmp - common flags such as "-v" and "-q" are misleading
				player.set_volume(atof(*(++argv)));
				break;
			case 's':
				show_gui = false;
				break;
			default:
				return 33;
		}
	}
	after_opts_parsed:
	
	const char* const inlist_filter_rule = *argv;
	
	if (inlist_filter_rule != nullptr)
		player.inlist_filter_dialog->load_and_apply_from(inlist_filter_rule);
    player.show();
    
    int rc = app.exec();
    compsky::mysql::wipe_auth(_mysql::auth, _mysql::auth_sz);
    return rc;
}
