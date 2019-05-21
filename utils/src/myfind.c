#include <dirent.h> // for opendir, readdir
#include <string.h> // for strlen, memcpy
#include <stdio.h> // for write
#include <stdlib.h> // for malloc, realloc
#include <sys/stat.h> // for stat

#define STDOUT_FILENO 1

#ifdef PRINT_FSIZE
#ifdef NEWLINES
#error "Cannot print file sizes if printing in newline mode"
#endif
#include <unistd.h> // for stat
#endif

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

#ifdef FIND_NOEXT
int has_ext(const char* str, int len){
    for (auto i = 0;  i < len;  ++i)
        if (str[i] == '.')
            return 1;
    return 0;
}
#endif

int media_exts(const char* str, int i){
	switch(str[--i]){
		case '.':
			return 1; // .
#ifdef FIND_MUSIC
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
#endif
#ifdef FIND_VID
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
#endif
#ifdef FIND_TXT
		case 'c':
			switch(str[--i]){
				case '.':
					return 1; // c.
				default: return 0;
			}
#endif
#ifdef FIND_VID
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
#endif
		case 'g':
			switch(str[--i]){
#ifdef FIND_IMG
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
#endif
#if defined (FIND_IMG) || defined (FIND_VID)
				case 'p':
					switch(str[--i]){
#ifdef FIND_IMG
						case 'j':
							switch(str[--i]){
								case '.':
									return 1; // gpj.
								default: return 0;
							}
#endif
						case 'm':
							switch(str[--i]){
								case '.':
									return 1; // gpm.
								default: return 0;
							}
						default: return 0;
					}
#endif
				default: return 0;
			}
#ifdef FIND_TXT
		case 'h':
			switch(str[--i]){
				case '.':
					return 1; // h.
				default: return 0;
			}
#endif
#ifdef FIND_VID
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
#ifdef FIND_TXT
		case 'p':
			switch(str[--i]){
				case 'p':
					switch(str[--i]){
						case 'c':
							switch(str[--i]){
								case '.':
									return 1; // ppc.
								default: return 0;
							}
						case 'h':
							switch(str[--i]){
								case '.':
									return 1; // pph.
								default: return 0;
							}
						default: return 0;
					}
			}
		case 't':
			switch(str[--i]){
				case 'x':
					switch(str[--i]){
						case 't':
							switch(str[--i]){
								case '.':
									return 1; // txt.
								default: return 0;
							}
						default: return 0;
					}
			}
#endif
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
#endif
		default: return 0;
	}
}


const int zero = 0;
const char endline = '\n';
int last_depth = 0;
struct stat sb;


void find_medias(const char* path, const int path_len, const int depth){
    DIR* dir;
    dir = opendir(path);
    
    last_depth = depth;
    
    if (dir == 0)
        return;
    struct dirent* e;
    
#ifndef NEWLINES
    goto goto__print_dirpath;
#endif
    
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
        const size_t epath_len = path_len + ename_len + 1;
        {
            // Scope to keep non-const 'epath' out of scope of goto jump destination
        char epath[epath_len + 1];
        memcpy(epath,  path,  path_len);
        epath[path_len] = '/';
        memcpy(epath + path_len + 1,  ename,  ename_len);
        epath[path_len + 1 + ename_len] = 0;
        
        if (e->d_type == DT_DIR){
            find_medias(epath, epath_len, depth + 1);
            continue;
        }
        
#ifdef FIND_NOEXT
        if (!has_ext(ename, ename_len));
        else
#endif
        if (media_exts(ename, ename_len) == 0)
            continue;
#ifdef NEWLINES
        write(STDOUT_FILENO, epath, epath_len);
        write(STDOUT_FILENO, &endline, sizeof(char));
#else
        write(STDOUT_FILENO, &ename_len, sizeof(int));
        write(STDOUT_FILENO, ename, ename_len);
#endif
#ifdef PRINT_FSIZE
#pragma message("Print FSize")
        if (stat(epath, &sb) != 0)
            goto goto__end;
        
        write(STDOUT_FILENO, &sb.st_size, sizeof(size_t));
#endif
        }
#ifndef NEWLINES
        if (depth < last_depth){
            // We've just come back up from a recursive find_medias down into a sibling directory
            goto__print_dirpath:
            last_depth = depth;
            write(STDOUT_FILENO, &zero, sizeof(int));
            write(STDOUT_FILENO, &path_len, sizeof(int));
            write(STDOUT_FILENO, path, path_len);
        }
#endif
    }
    goto__end:
    closedir(dir);
}

int main(const int argc, const char* argv[]){
    for (int i=1; i<argc; ++i)
        find_medias(argv[i], strlen(argv[i]), 0);
}
