#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

#include <getopt.h>
#include <dirent.h>

#include "rsa_ftp.h"
#include "network.h"
#include "board_system.h"
#include <at/at_util.h>
#include <at/at_log.h>
#include <logd_rpc.h>

// ========================================================
//  program argument 
// ========================================================
struct option   long_opt[] =
{
  {"help",		no_argument,		NULL,	'h'},
  {"port",		required_argument,	NULL,	's'},
  {"server",	required_argument,	NULL,	'p'},
  {"target_ssh_port",	required_argument,	NULL,	't'},
  {NULL,		0,					NULL,	0  }
};

const char	* short_opt = "hs:p:t:";

#define LOG_TARGET eSVC_COMMON

// ========================================================
//  ssh info for connect to server.
// ========================================================
typedef struct ssh_tunnel_info
{
	int dropbear_port;
	char server_addr[256];
	char server_port[16];
	char server_login_id[128];
	char server_rsa_path[256];
	char server_target_ssh_port[16];
	char check_cmd[256];
}SSH_TUNNEL_INFO_T;

void set_ssh_info(SSH_TUNNEL_INFO_T* ssh_info)
{
	// set m2m board dropbear port
	ssh_info->dropbear_port = DROPBEAR_PORT;

	// set server address
	// get from argument  : 219.251.4.177
	strcpy(ssh_info->server_addr,"null");

	// set server port
	// get from argument : 30010
	strcpy(ssh_info->server_port,"null");
	//ssh_info->server_port = 30010;

	strcpy(ssh_info->server_target_ssh_port,"null");

	// set server login id
	strcpy(ssh_info->server_login_id,SERVER_LOGIN_ID);

	// set server rsa path for auto login
	strcpy(ssh_info->server_rsa_path,SERVER_RSA_PATH);

	// if server connect success when run script
	//strcpy(ssh_info->check_cmd,"./test.sh");
	strcpy(ssh_info->check_cmd,"");
}

// ========================================================
//  remove file.
// ========================================================

void remove_rsa_file()
{
	struct dirent *next_file;
    DIR *folder;

    char filepath[256];

    folder = opendir(SERVER_RSA_PATH_DIR);
	
	if (NULL == folder)
		return;

    while ( next_file = readdir(folder) )
    {
        // build the full path for each file in the folder
        sprintf(filepath, "%s/%s", SERVER_RSA_PATH_DIR, next_file->d_name);
        remove(filepath);
    }
}

void remove_ftp_script()
{
	remove(FTPSCRIPT_PATH);
}


// ========================================================
//  network check thread
// ========================================================
int network_act = 0;
int net_thread_run = 0;

void* network_chk_loop(void * arg)
{
	static int sleep_sec = 0;

	sleep_sec = 1;

    while(net_thread_run)
    {
        mdmc_module_network_check();

		// if network connected than increase network check interval.
		if (network_act)
			sleep_sec = 60;

        sleep(sleep_sec);
    }
}


int main(int argc, char* argv[])
{
	FILE *fp;

	char cmd_buff[256]={0,};
	char cmd_return[512]={0,};
	
	int pid, sid;
	int count = 0;

	SSH_TUNNEL_INFO_T ssh_info = {0,};
	FTP_SERVER svr={0,};

	int c = 0;
	int ret;
	
	pthread_t p_thread1;

	int i = 0;
	int str_len = 0;

	logd_init();

	for ( i = 0 ; i < argc ; i++ )
		str_len += sprintf(cmd_buff + str_len, "%s ", argv[i]);

	LOGI(LOG_TARGET, "[rssh3] input command [%s] \n", cmd_buff);
	memset(cmd_buff, 0x00, 256);

	

	//ret = at_open(e_DEV_TX501_BASE, NULL, NULL, "/tmp/rssi_at.log");	//file debug msg
	//ret = at_open(e_DEV_TX501_BASE, NULL, NULL, "console");			//console debug msg
	ret = at_open(AT_LIB_TARGET_DEV, NULL, NULL, NULL);					//no debug msg
	// --------------------------------------------
	// std input / output init, redirect
	// --------------------------------------------
//*
	close(0);
	close(1);
	close(2);

	stdin = freopen("/dev/null", "r", stdin);
	stdout = freopen("/dev/null", "w", stdout);
	stderr = freopen("/dev/null", "rw", stderr);
//*/	

	// --------------------------------------------
	// kill already run dbclient process 
	// --------------------------------------------
	system("killall dbclient");
	
	// --------------------------------------------
	// delete rsa file , ftp download script
	// --------------------------------------------
	mkdir(FTPSCRIPT_PATH_DIR, 0755);
	mkdir(SERVER_RSA_PATH_DIR, 0755);
	
	remove_rsa_file();
	remove_ftp_script();
	
	// --------------------------------------------
	// init ssh connect info
	// --------------------------------------------
	memset(&ssh_info, 0x00, sizeof(SSH_TUNNEL_INFO_T));
	set_ssh_info(&ssh_info);
	
	// --------------------------------------------
	// get argument server info
	// --------------------------------------------
	while((c = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1)
	{
		switch(c)
		{
			case -1:       /* no more arguments */
			case 0:        /* long options toggles */
			{
				break;
			}

			case 'p':
			{
				LOGI(LOG_TARGET, "\t get arg : port is \"%s\"\n", optarg);
				strcpy(ssh_info.server_port, optarg);
				break;
			}

			case 's':
			{
				LOGI(LOG_TARGET, "\t get arg : server is \"%s\"\n", optarg);
				strcpy(ssh_info.server_addr, optarg);
				break;
			}

			case 't':
			{
				LOGI(LOG_TARGET, "\t get arg : server target ssh port is \"%s\"\n", optarg);
				strcpy(ssh_info.server_target_ssh_port, optarg);
				break;
			}

			case 'h':
			{
				printf("Usage: %s [OPTIONS]\n", argv[0]);
				printf("  -p, port setting\n");
				printf("  -s, server setting\n");
				printf("  -h, --help                print this help and exit\n");
				printf("\n");
				return (0);
			}

			case ':':
			case '?':
			{
				fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
				return (-2);
			}
			default:
			{
				LOGE(LOG_TARGET, "%s: invalid option -- %c\n", argv[0], c);
				fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
				return (-2);
			}
		};
	};
	
	// --------------------------------------------
	// check argument
	// --------------------------------------------
	if ( !strcmp(ssh_info.server_addr,"null") )
	{	
		LOGE(LOG_TARGET, "need more argument : port and server info\r\n");
		exit(0);
	}
	
	if ( !strcmp(ssh_info.server_port,"null") )
	{
		LOGE(LOG_TARGET, "need more argument : port and server info\r\n");
		exit(0);
	}
	
   
    // --------------------------------------------
	// make daemon...
	// --------------------------------------------
	// make deamon Process...
	pid = fork();
	while (pid < 0 )
	{
		perror("fork error : ");
		pid = fork();
		if (count == 10)
			exit(0);
		sleep(10);
		count++;
	}
	if (pid > 0)
		exit(EXIT_SUCCESS);

	sid = setsid();
	if (sid < 0)
		exit(EXIT_FAILURE);

	chdir("/");


	// --------------------------------------------
	// program start : init network thread
	// --------------------------------------------
	net_thread_run = 1;
	pthread_create(&p_thread1, NULL, network_chk_loop, NULL);

	// --------------------------------------------
	// wait for network init.
	// --------------------------------------------
	while(network_act != 1)
	{
		printf("wait for network activate....\r\n");
		sleep(1);
	}
	
	// --------------------------------------------
	// init ftp info for rsa key.
	// --------------------------------------------
	init_ftp_server(&svr,ssh_info.server_port);
	download_run_script(&svr);
	
#if 0  	// use tty console
	sprintf(cmd_buff,
			"%s -R %s:localhost:%d %s@%s -i %s \"%s\"",
			DROPBEAR_CLIENT_PATH,
			ssh_info.server_port,
			ssh_info.dropbear_port,
			ssh_info.server_login_id,
			ssh_info.server_addr,
			ssh_info.server_rsa_path,
			ssh_info.check_cmd);
#else	// not use tty console, back ground
	sprintf(cmd_buff,
			"%s -y -I 600 -f -N -R %s:localhost:%d -p %s %s@%s -i %s&",
			DROPBEAR_CLIENT_PATH,
			ssh_info.server_port,
			ssh_info.dropbear_port,
			ssh_info.server_target_ssh_port,
			ssh_info.server_login_id,
			ssh_info.server_addr,
			ssh_info.server_rsa_path);
#endif

	LOGI(LOG_TARGET, "[rssh3] run dbclient cmd is [%s]\r\n",cmd_buff);
	
	//printf("run cmd is [%s]\r\n",cmd_buff);
	//system("dbclient -f -N -R 30010:localhost:1022 administrator@219.251.4.177 -i ~/.ssh/id_rsa");

	fp = popen(cmd_buff, "r");

	while (fgets(cmd_return, 256, fp) != NULL)
	{
		printf(">> [%s] \r\n", cmd_return);
	}

	pclose(fp);

	// --------------------------------------------
	// delete rsa file , ftp download script
	// --------------------------------------------
	remove_rsa_file();
	remove_ftp_script();
	
	LOGT(LOG_TARGET, "[rssh3] program end : bye bye ~~\r\n");
	exit(0);
}
