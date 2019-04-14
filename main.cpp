#include <sys/types.h>
#include <cstdio> // for sprintf
#include <unistd.h> // for execve, usleep
#include <vector> // for std::vector
#include <string> // for std::stoi
#include <thread> // for std::thread
#include <fcntl.h> // for O_RDWR, open()
#include <sys/socket.h> // for connect()

#include <utils.hpp> // for format_out_fp


int TIMEOUT = 5;
#define SOCKET_FP_LEN 35 // 16
char SOCKET_FP[SOCKET_FP_LEN + 1] = "/tmp/AAAAAAAAAAABSDFSETFG4980456456"; //"/tmp/mpv-socket."; // \0\0\0\0";
//FILE* SOCKET;
int SOCKET_FD;

bool MAINTAIN_WIN_SCALE_ = false;
bool MAINTAIN_WIN_SCALE = false;
/*
int WIN_W;
int WIN_H;
int WIN_S;
*/

#define ERR_CANNOT_CONNECT_SOCKET 101
#define ERR_CANNOT_W_SOCKET 102
#define ERR_CANNOT_R_SOCKET 103
#define ERR_CANNOT_O_MPV 104



struct sockaddr_bigger {
    sa_family_t sa_family;
    char        sa_data[SOCKET_FP_LEN]; // Usual is char[14]
};



void handler(int rc){
    exit(rc);
}


char* mpv_args[8] = {"DUMMY", "--really-quiet", "--idle", "--input-ipc-server", NULL, NULL, NULL, NULL};

int escaped_strlen(char* str){
    int i = 0;
    for (char* it = str;  *it != 0;  ++it, ++i)
        if (*it == '"')
            ++i;
    return i;
}

void escaped_strcpy(char* dst, char* src, int dst_offset, int src_escaped_strlen){
    for (char* it = src;  *it != 0;  ++it, ++dst_offset){
        if (*it == '"')
            dst[dst_offset++] = '\\';
        dst[dst_offset] = *it;
    }
}

void mpv_cmd(char* cmd, int t){
    int cmd_len = strlen(cmd);
    int buf_len = 1 + 7+2 + 2 + 1 + cmd_len + 3;
    char buf[buf_len] = "{\"command\": ["; // ... + "]}\n";
    int i = 13;
    memcpy(buf+i, cmd, cmd_len);
    i += cmd_len;
    buf[i++] = ']';
    buf[i++] = '}';
    buf[i++] = '\n';
    buf[i] = 0;
#ifdef DEBUG
    printf("mpv_cmd: CMD>>>%s<<<\n", buf);
#endif
    int bytes_written;
    if (bytes_written = write(SOCKET_FD, buf, buf_len) != buf_len){
#ifdef DEBUG
        printf("E: Written %d (!=%d) bytes to socket `%s`\n", bytes_written, buf_len, SOCKET_FP);
#endif
        if (t > TIMEOUT)
            handler(ERR_CANNOT_W_SOCKET);
        usleep(1000);
        mpv_cmd(cmd, t+1);
    }
}

void set_mpv_property(char* propname, int propname_len, int value){
    int i = 17;
    char cmd[(12+2) + 2 + (propname_len+2) + 2 + 5 + 1] = "\"set_property\", \"";
    memcpy(cmd+i, propname, propname_len);
    i += propname_len - 1;
    cmd[i++] = '"';
    cmd[i++] = ',';
    cmd[i++] = ' ';
    for (auto j=0; j<5; ++j){
        cmd[i + 5 -1 -j] = value % 10;
        value /= 10;
    }
#ifdef DEBUG
    printf("set_mpv_property: CMD>>>%s<<<\n", cmd);
#endif
    mpv_cmd(cmd, 0);
}

int get_mpv_property(char* propname, int t){
    char cmd[12+2 + 2 + 10+2 + 1];
    sprintf(cmd, "\"set_property\", \"%s\"", propname);
    mpv_cmd(cmd, 0);
    char buffer[1024];
    if (read(SOCKET_FD, buffer, 1024) == 0)
        handler(ERR_CANNOT_R_SOCKET);
    // On success, format is {"data":[VALUE]
    // On error,   format is {"error":
    if (buffer[2] == 'e'){
        if (t > TIMEOUT)
            handler(ERR_CANNOT_W_SOCKET);
        usleep(1000);
        return get_mpv_property(propname, t+1);
    }
    int propval = 0;
    char* it = buffer + 8 - 1;
    while (*(++it) != ','){
        propval *= 10;
        propval += *(it);
    }
    return propval;
}

void mpv_open(char* fp){
    int w, h, s;
    if (MAINTAIN_WIN_SCALE){
        w = get_mpv_property("dwidth", 0);
        h = get_mpv_property("dheight", 0);
        s = get_mpv_property("window-scale", 0);
    }
    
    auto fp_escaped_len = escaped_strlen(fp);
    int i = 13;
    char buf[8+2 + 2 + fp_escaped_len+2 + 1] = "\"loadfile\", \"";
    escaped_strcpy(buf, fp, i, fp_escaped_len);
    i += fp_escaped_len;
    buf[i++] = '"';
#ifdef DEBUG
    printf("mpv_open: CMD>>>%s<<<\n", buf);
#endif
    mpv_cmd(buf, 0);
    
    if (MAINTAIN_WIN_SCALE && w*h > 1){
        int W = get_mpv_property("dwidth", 0);
        set_mpv_property("dwidth", 6, s*w/W);
    }
    
    if (MAINTAIN_WIN_SCALE_ && (!MAINTAIN_WIN_SCALE))
        // Do not use the w/h/s of the zeroth file opened with mpv
        MAINTAIN_WIN_SCALE = true;
}

void process_file(char* fp, int fsize){
    
}

void init_mpv(char** mpv_args){
    if (execve("/usr/bin/mpv", mpv_args, NULL) == -1)
        // Init MPV
        handler(ERR_CANNOT_O_MPV);
}

int main(const int argc, const char* argv[]){
    int i = 0;
    int volume = -1;
    
    while (++i < argc){
        if (argv[i][0] == '-' && argv[i][2] == 0){
            switch(argv[i][1]){
                case 'V': volume=std::stoi(argv[++i]); break;
                case 't': TIMEOUT=std::stoi(argv[++i]); break;
                case 'k': MAINTAIN_WIN_SCALE_=true; break;
                default: --i; goto end_args;
            }
        } else {
            --i;
            goto end_args;
        }
    }
    end_args:
    
    
    //int pid = getpid();
    //memcpy(SOCKET_FP+16, &pid, 4);
    mpv_args[4] = SOCKET_FP;
    
#ifdef DEBUG
    printf("%s\n", SOCKET_FP);
#endif
    
    SOCKET_FD = socket(AF_UNIX, SOCK_STREAM, 0);
    // AF_UNIX==1, SOCK_STREAM==1
    sockaddr_bigger serv_addr = {};
    serv_addr.sa_family = 1;
    memcpy(serv_addr.sa_data, SOCKET_FP, SOCKET_FP_LEN);
    // NOTE: We want to go over the 14 char limit
    printf("SOCKET_FD:\t%d\nserv_addr:\t%d\t%s\n", SOCKET_FD, serv_addr.sa_family, serv_addr.sa_data);
    printf("sizeof(serv_addr):\t%ld\n", sizeof(serv_addr));
    
    if (connect(SOCKET_FD, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        return ERR_CANNOT_CONNECT_SOCKET;
    
    
    if (volume != -1){
        mpv_args[5] = "--volume";
        char buf[3];
        snprintf(buf, 3, "%d", volume);
        mpv_args[6] = buf;
    }
    
    
    /* Alt 1 */
    //char buf[2048];
    //sprintf(buf, "mpv --really-quiet --idle --input-ipc-server '%s'", SOCKET_FP);
    //printf("Executing: %s\n", buf);
    //std::thread t1(system, buf);
    
    /* Alt 2 */
    //std::thread t1(init_mpv, mpv_args);
    
    
    for (auto i=1; i<argc; ++i)
        mpv_open((char*)argv[i]);
    
    
    //t1.join();
    //fclose(SOCKET);
    close(SOCKET_FD);
    
    return 0;
}
