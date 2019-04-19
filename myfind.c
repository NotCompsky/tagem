#include <dirent.h> // for opendir, readdir
#include <string.h> // for strlen
#include <stdio.h> // for printf

#define STDOUT_FILENO 1

void find(const char* path, unsigned int depth){
    DIR* dir;
    dir = opendir(path);
    if (dir == 0)
        return;
    struct dirent* e;
    while (e=readdir(dir) != 0){
        const char* ename = e->d_name;
        if (ename == 0)
            continue;
        if ((ename[0]=='.') &&
            (ename[1]==0 || (ename[1]=='.' && ename[2]==0)))
            // Ignore . and ..
            continue;
        if (e->d_type == DT_DIR){
            find(ename, depth+1);
            continue;
        }
    }
    closedir(dir);
}

int media_exts(const char* str, int i){
	switch(str[--i]){
		case '.':
			return 1; // .
		case '3':
			switch(str[--i]){
				case 'p':
					switch(str[--i]){
						case 'm':
							switch(str[--i]){
								case '.':
									return 1; // 3pm.
								default: return 0;
							}
						default: return 0;
					}
				default: return 0;
			}
		case '4':
			switch(str[--i]){
				case 'p':
					switch(str[--i]){
						case 'm':
							switch(str[--i]){
								case '.':
									return 1; // 4pm.
								default: return 0;
							}
						default: return 0;
					}
				default: return 0;
			}
		case 'b':
			switch(str[--i]){
				case 'o':
					switch(str[--i]){
						case 'v':
							switch(str[--i]){
								case '.':
									return 1; // bov.
								default: return 0;
							}
						default: return 0;
					}
				default: return 0;
			}
		case 'f':
			switch(str[--i]){
				case 'i':
					switch(str[--i]){
						case 'g':
							switch(str[--i]){
								case '.':
									return 1; // fig.
								default: return 0;
							}
						default: return 0;
					}
				default: return 0;
			}
		case 'g':
			switch(str[--i]){
				case 'e':
					switch(str[--i]){
						case 'p':
							switch(str[--i]){
								case 'j':
									switch(str[--i]){
										case '.':
											return 1; // gepj.
										default: return 0;
									}
								case 'm':
									switch(str[--i]){
										case '.':
											return 1; // gepm.
										default: return 0;
									}
								default: return 0;
							}
						default: return 0;
					}
				case 'g':
					switch(str[--i]){
						case 'o':
							switch(str[--i]){
								case '.':
									return 1; // ggo.
								default: return 0;
							}
						default: return 0;
					}
				case 'n':
					switch(str[--i]){
						case 'p':
							switch(str[--i]){
								case '.':
									return 1; // gnp.
								default: return 0;
							}
						default: return 0;
					}
				case 'p':
					switch(str[--i]){
						case 'j':
							switch(str[--i]){
								case '.':
									return 1; // gpj.
								default: return 0;
							}
						case 'm':
							switch(str[--i]){
								case '.':
									return 1; // gpm.
								default: return 0;
							}
						default: return 0;
					}
				default: return 0;
			}
		case 'i':
			switch(str[--i]){
				case 'v':
					switch(str[--i]){
						case 'a':
							switch(str[--i]){
								case '.':
									return 1; // iva.
								default: return 0;
							}
						default: return 0;
					}
				default: return 0;
			}
		case 'm':
			switch(str[--i]){
				case 'b':
					switch(str[--i]){
						case 'e':
							switch(str[--i]){
								case 'w':
									switch(str[--i]){
										case '.':
											return 1; // mbew.
										default: return 0;
									}
								default: return 0;
							}
						default: return 0;
					}
				default: return 0;
			}
		case 'v':
			switch(str[--i]){
				case '2':
					switch(str[--i]){
						case 'm':
							switch(str[--i]){
								case '.':
									return 1; // v2m.
								default: return 0;
							}
						default: return 0;
					}
				case '4':
					switch(str[--i]){
						case 'm':
							switch(str[--i]){
								case '.':
									return 1; // v4m.
								default: return 0;
							}
						default: return 0;
					}
				case 'g':
					switch(str[--i]){
						case 'o':
							switch(str[--i]){
								case '.':
									return 1; // vgo.
								default: return 0;
							}
						default: return 0;
					}
				case 'k':
					switch(str[--i]){
						case 'm':
							switch(str[--i]){
								case '.':
									return 1; // vkm.
								default: return 0;
							}
						default: return 0;
					}
				case 'l':
					switch(str[--i]){
						case 'f':
							switch(str[--i]){
								case '.':
									return 1; // vlf.
								default: return 0;
							}
						default: return 0;
					}
				case 'm':
					switch(str[--i]){
						case 'w':
							switch(str[--i]){
								case '.':
									return 1; // vmw.
								default: return 0;
							}
						default: return 0;
					}
				case 'o':
					switch(str[--i]){
						case 'm':
							switch(str[--i]){
								case '.':
									return 1; // vom.
								default: return 0;
							}
						default: return 0;
					}
				default: return 0;
			}
		default: return 0;
	}
}


const char* endline = "\n";
void find_medias(const char* path){
    DIR* dir;
    dir = opendir(path);
    int path_len = strlen(path);
    if (dir == 0)
        return;
    struct dirent* e;
    while ((e=readdir(dir)) != 0){
        const char* ename = e->d_name;
        
        if (ename == 0)
            continue;
        
        if (ename[0] == '.'){
            if (ename[1] == 0)
                continue;
            if (ename[1] == '.')
                continue;
        }
        
        const int ename_len = strlen(ename);
        
        const int epath_len = path_len + ename_len + 1;
        char epath[epath_len + 1];
        memcpy(epath,  path,  path_len);
        epath[path_len] = '/';
        memcpy(epath + path_len + 1,  ename,  ename_len);
        epath[path_len + 1 + ename_len] = 0;
        
        if (e->d_type == DT_DIR){
            find_medias(epath);
            continue;
        }
        
        if (media_exts(ename, ename_len) == 0)
            continue;
        write(STDOUT_FILENO, epath, epath_len);
        write(STDOUT_FILENO, endline, 2);
    }
    closedir(dir);
}

int main(const int argc, const char* argv[]){
    for (int i=1; i<argc; ++i)
        find_medias(argv[i]);
}
