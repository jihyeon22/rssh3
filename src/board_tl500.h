#pragma once

#define CONCAT_STR(X, Y)		X Y
#define SYSTEM_DIR				"/system/mds"


#define DROPBEAR_SERVER_PATH	CONCAT_STR(SYSTEM_DIR, "/usr/bin/dbclient")
#define DROPBEAR_PORT			1022
#define SERVER_LOGIN_ID			"administrator"

#define SERVER_RSA_PATH_DIR		CONCAT_STR(SYSTEM_DIR, "/.ssh")
#define SERVER_RSA_PATH			CONCAT_STR(SYSTEM_DIR, "/.ssh/id_rsa")

#define DROPBEAR_CLIENT_PATH	CONCAT_STR(SYSTEM_DIR, "/usr/bin/dbclient")

#define FTPSCRIPT_PATH_DIR		CONCAT_STR(SYSTEM_DIR, "/.ftp")
#define FTPSCRIPT_PATH			CONCAT_STR(SYSTEM_DIR, "/.ftp/ftp_script.sh")

/////////////////////////////////////////////////////////////////////////
//Server Connection Infomation
/////////////////////////////////////////////////////////////////////////
#define SERVER_FTP_ADDR			"virtual.mdstec.com"
#define SERVER_FTP_PORT			"21"
#define SERVER_FTP_ID			"openm2m"
#define SERVER_FTP_PASSWORD		"openm2m.open"
#define SERVER_FTP_SSH_KEY_FILE	"/home/rsa_keys/TX501/%s/.ssh/"


//#define MDMC_CMD_PPP_UP     "pppd /dev/smd0"                              
//#define MDMC_CMD_PPP_DOWN   "kill -SIGTERM `cat /var/run/ppp0.pid`"     
#define MDMC_NET_INTERFACE		"rmnet_data0"     
#define NETIF_DOWN_CMD			"/sbin/ifconfig rmnet_data0 down &"
#define NETIF_UP_CMD			"/sbin/ifconfig rmnet_data0 up &"

#define AT_DEV_FILE				"/dev/smd8"

#define AT_LIB_TARGET_DEV   e_DEV_TL500_BASE
