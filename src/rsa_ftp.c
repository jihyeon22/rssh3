#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <assert.h>

#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h> //for mkdir
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>

#include "rsa_ftp.h"
#include "board_system.h"

int init_ftp_server(FTP_SERVER *svr, char* target_ssh_port)
{
	strncpy(svr->addr, SERVER_FTP_ADDR, strlen(SERVER_FTP_ADDR));
	strncpy(svr->port, SERVER_FTP_PORT, strlen(SERVER_FTP_PORT));
	strncpy(svr->id, SERVER_FTP_ID, strlen(SERVER_FTP_ID));
	strncpy(svr->pass, SERVER_FTP_PASSWORD, strlen(SERVER_FTP_PASSWORD));
	
	sprintf(svr->file, SERVER_FTP_SSH_KEY_FILE, target_ssh_port);
}


int download_run_script(FTP_SERVER *svr)
{
    FILE    *   sh ;
    int rst=0;
    char md5_file[64];

    char string[100];
    char file_path[128] = {0};
    char file_name[64] = {0};
    char *pos;

	char cmd_buff[128] = {0,};
	
    pos = strrchr(svr->file,'/');
	
	mkdir(FTPSCRIPT_PATH_DIR, 0755);
	mkdir(SERVER_RSA_PATH_DIR, 0755);
	
    if(pos == NULL){
       strcpy( file_name, svr->file);
    } else {
       strncpy(file_path, svr->file, strlen(svr->file) - strlen(strrchr(svr->file,'/'))+1);   
       strcpy( string, svr->file);
       pos = strrchr( string, '/' );
       strcpy( file_name, pos+1);
    }

    //LOGI(LOG_TARGET, "create ftp_script.sh ");

    sh = fopen(FTPSCRIPT_PATH, "w");

    fprintf(sh,
    "PORT='%s'\n"
    "HOST='%s'\n"
    "USER='%s'\n"
    "PASSWD='%s'\n"
    "FILE='%s*'\n"
    "\n"
    "cmdftp -p $PORT -n $HOST -t 630 <<END_SCRIPT\n"
    "$USER\n"
    "$PASSWD\n"
    "d $FILE %s\n"
    "quit\n"
    "END_SCRIPT\n"
    "exit 0\n"
    "\n"
    "\n",svr->port, svr->addr, svr->id, svr->pass, svr->file, SERVER_RSA_PATH_DIR);

    fclose(sh);

	sprintf(cmd_buff, "chmod 777 %s", FTPSCRIPT_PATH);
	
    rst = system(cmd_buff);
    printf( "%s change mode %d ", FTPSCRIPT_PATH, rst);   

    rst = system(FTPSCRIPT_PATH);
    printf( "%s run %d ", FTPSCRIPT_PATH, rst);   


    return 0;
}
