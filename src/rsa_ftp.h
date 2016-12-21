#ifndef __UPDATE_H__
#define __UPDATE_H__


struct FTP_SERVER {
    char addr[32];
    char port[16];
    char id[16];
    char pass[16];
    char file[64];
};
typedef struct FTP_SERVER FTP_SERVER;

int init_ftp_server(FTP_SERVER *svr, char* target_ssh_port);
int download_run_script(FTP_SERVER *svr);

#endif
