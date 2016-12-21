#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <linux/unistd.h>

#include <pthread.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netdb.h>
#include <netinet/ether.h>

#include <linux/ioctl.h>

#include "network.h"
#include "board_system.h"
#include <at/at_util.h>
#include <at/at_log.h>

//#define MDMC_NET_API_DEBUG
void network_device_up()
{
	printf("%s ++\n", __func__);
	system(NETIF_DOWN_CMD);
	send_at_cmd("at$$apcall=0");
	sleep(2);
	send_at_cmd("at$$apcall=1");
	system(NETIF_UP_CMD);
	printf("%s +--\n", __func__);
}

void network_device_down()
{
	printf("%s ++\n", __func__);
	system(NETIF_DOWN_CMD);
	send_at_cmd("at$$apcall=0");
	printf("%s +--\n", __func__);
}
// ------------------------------------------------------------------
//  Function : Network Check..
// ------------------------------------------------------------------
bool_t is_found_ppp_device( )
{

	int             sock, sock_ret;
	struct ifreq    ifr;
    bool_t          ret = MDMC_FALSE;

#ifdef MDMC_NET_API_DEBUG
    MDMC_DEBUG_FUNC_TRACE("Start \r\n");
#endif    
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
	if (sock < 0) {
		MDMC_DEBUG_FUNC_TRACE("%s Get socket() Create error\n", MDMC_NET_INTERFACE);
		return MDMC_FALSE;
	}

	sprintf((char *)&ifr.ifr_name, "%s", MDMC_NET_INTERFACE );

	// Get IP Adress
	sock_ret = ioctl(sock, SIOCGIFADDR, &ifr);
    
	if (sock_ret < 0) {
#ifdef MDMC_NET_API_DEBUG
		MDMC_DEBUG_FUNC_TRACE("%s not found\n", MDMC_NET_INTERFACE);
#endif
		close(sock);
#ifdef MDMC_NET_API_DEBUG
       MDMC_DEBUG_FUNC_TRACE("End [%d] \r\n",ret);
#endif
		return ret;
	}
    
    ret = MDMC_TRUE;
    
	close(sock);
#ifdef MDMC_NET_API_DEBUG
    MDMC_DEBUG_FUNC_TRACE("End [%d] \r\n",ret);
#endif
    
	return ret;
}

// -------------------------------------------------------------------
//  Function : Try Network Connect..
// -------------------------------------------------------------------
bool_t mdmc_api_net_conn (net_manage_info_t* net_manage_arg)
{
    int max_try_cnt;
    bool_t net_conn_success = MDMC_FALSE;
    bool_t ret = MDMC_FALSE;
    
#ifdef MDMC_NET_API_DEBUG
    MDMC_DEBUG_FUNC_TRACE("Start \r\n");
#endif    

    // Argument Check...
    if (net_manage_arg->timeout_sec >= 0)
    {
        max_try_cnt = net_manage_arg->timeout_sec;
#ifdef MDMC_NET_API_DEBUG
        MDMC_DEBUG_FUNC_TRACE("Network Retry Timeout is [%d]\r\n",max_try_cnt);
#endif
    }
    else
    {
#ifdef MDMC_NET_API_DEBUG 
        MDMC_DEBUG_ERROR("Invalid Argument [%d]\r\n",net_manage_arg->timeout_sec);
#endif
    }
    
    
    // Try Connect...
    if (net_manage_arg->net_conn == MDMC_TRUE)
    {
#ifdef MDMC_NET_API_DEBUG
        MDMC_DEBUG_FUNC_TRACE("Try Network Connect... \r\n");
#endif
        do
        {
            if( is_found_ppp_device() == MDMC_TRUE )
            {
                net_conn_success = MDMC_TRUE;
#ifdef MDMC_NET_API_DEBUG 
                MDMC_DEBUG_FUNC_TRACE("Network Connect Success!!!!! \r\n");
#endif
                ret = MDMC_TRUE;
                break;
            }
            //system(MDMC_CMD_PPP_UP);
			network_device_down();
			sleep(1);
			network_device_up();
			// if don't wait ... end loop
			if (max_try_cnt == 0)
				break;
            sleep(5);
        }
        while(max_try_cnt--);
        
        if ((net_manage_arg->reset == MDMC_TRUE) && (net_conn_success == MDMC_FALSE))
        {
        #if 0
            reset_info_t reset_info;
            
            reset_info.delay_sec=0;
            reset_info.notify_other_service = MDMC_FALSE;
#ifdef MDMC_NET_API_DEBUG
            MDMC_DEBUG_ERROR("Cannot Connect Network.. Try Reboot..!!!! \r\n");
#endif
            mdmc_api_reset(&reset_info);
        #endif
        }
        
    }
    // Try Disconect ...
    else
    {
        //system(MDMC_CMD_PPP_DOWN);
        do
        {
            if( is_found_ppp_device() == MDMC_FALSE )
            {
                net_conn_success = MDMC_FALSE;
#ifdef MDMC_NET_API_DEBUG
                MDMC_DEBUG_FUNC_TRACE("Network Disconnect Success!!!!! \r\n");
#endif
                ret = MDMC_TRUE;
                break;
            }
            //system(MDMC_CMD_PPP_DOWN);
			network_device_down();
            sleep(1);
        }
        while(max_try_cnt--);
#ifdef MDMC_NET_API_DEBUG        
        MDMC_DEBUG_FUNC_TRACE("Try Network Disconnect\r\n");
#endif
    }
    
#ifdef MDMC_NET_API_DEBUG
    MDMC_DEBUG_FUNC_TRACE("End [%d]\r\n", ret);
#endif
    return ret;
}

// -----------------------------------------------------------------
//  Function : Get Network Info...
// -----------------------------------------------------------------
bool_t mdmc_api_get_net_info(net_manage_ret_t * result)
{
    int                 sock = 0;
    int                 sock_ret = 0;
    struct ifreq        ifr;
    struct sockaddr     *sa;
    unsigned char       *ptr;
    
    bool_t                ret;
   
#ifdef MDMC_NET_API_DEBUG
    MDMC_DEBUG_FUNC_TRACE("Start [%d]\r\n",result);    
#endif

    // Check Argument...
    if (result == NULL)
    {
#ifdef MDMC_NET_API_DEBUG
        MDMC_DEBUG_ERROR("Invalid Argument \n");
#endif
        ret = MDMC_FALSE;
        goto exit;
    }
    else
    {
        // initialize Argument
        (result)->net_stat = MDMC_FALSE;
        strcpy((result)->ip_addr,"000.000.000.000");
    }
    
    // Check Network Stat
    if( is_found_ppp_device() == MDMC_FALSE )
    {
#ifdef MDMC_NET_API_DEBUG
        MDMC_DEBUG_FUNC_TRACE("Not Connected Network!!!!! \r\n");
#endif
        (result)->net_stat = MDMC_FALSE;
        
        ret = MDMC_FALSE;
        goto exit;
    }
       
    // Start Get Network Infomation
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock < 0) 
    {
#ifdef MDMC_NET_API_DEBUG
        MDMC_DEBUG_ERROR("Failed to Open Socket\n");
#endif
        ret = MDMC_FALSE;
        goto exit;
    }
    
    sprintf((char *)&ifr.ifr_name, "%s", MDMC_NET_INTERFACE );
    
    //  - Get IP Adress
    sock_ret = ioctl(sock, SIOCGIFADDR, &ifr);

    if (sock_ret < 0) 
    {
#ifdef MDMC_NET_API_DEBUG
        MDMC_DEBUG_ERROR("ioctl(SIOCGIFADDR) error\n");
#endif
        ret = MDMC_FALSE;
        goto exit;
    }
    
    strcpy((result)->ip_addr, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
#ifdef MDMC_NET_API_DEBUG
    MDMC_DEBUG_FUNC_TRACE("Get IP Addr : [%s] \r\n", (result)->ip_addr);    
#endif

    //  -  Get Hardware Adress (MAC Address)
    sock_ret = ioctl(sock, SIOCGIFHWADDR, (char *)&ifr);
    if (sock_ret < 0) 
    {
#ifdef MDMC_NET_API_DEBUG
        MDMC_DEBUG_ERROR("ioctl(SIOCGIFHWADDR) error\n");
#endif

        ret = MDMC_FALSE;
        goto exit;
	}
    
    sa = &ifr.ifr_hwaddr;
    ptr = &sa->sa_data[0];

    MDMC_DEBUG_FUNC_TRACE("HW Addr is : %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X \n", *ptr, *(ptr+1),*(ptr+2), *(ptr+3),*(ptr+4),*(ptr+5));

    //  -  Get Netmask
    sock_ret = ioctl(sock, SIOCGIFNETMASK, &ifr);
    if (sock_ret < 0) 
    {
#ifdef MDMC_NET_API_DEBUG
        MDMC_DEBUG_ERROR("ioctl(SIOCGIFNETMASK) error\n");
#endif
       
        ret = MDMC_FALSE;
        goto exit;
    }
    
#ifdef MDMC_NET_API_DEBUG
    MDMC_DEBUG_FUNC_TRACE("Netmask = [%s]\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr/*.s_addr*/));
#endif
   
	ret = MDMC_TRUE; 
    (result)->net_stat = MDMC_TRUE;
    
exit:
    if (ret == MDMC_FALSE)
    {
        (result)->net_stat = MDMC_FALSE;
        strcpy((result)->ip_addr,"000.000.000.000");
    }
    
    if (sock > 0)
        close(sock);

    //result->net_stat = MDMC_TRUE;
    
#ifdef MDMC_NET_API_DEBUG
    MDMC_DEBUG_FUNC_TRACE("End [%d] \r\n",ret);    
#endif
    
    return ret;
}

int _net_check(int log_enable)
{
    int max_try_cnt = MDMC_MODULE_CHK_API_MAX_NET_RETRY_1;
///#ifdef MDMC_NET_API_DEBUG
	if (log_enable)
	{
	    MDMC_DEBUG_FUNC_TRACE("Start \r\n");
	}
//#endif
    do
    {
		if (log_enable)
		{
    		MDMC_DEBUG_FUNC_TRACE("MDMC Network check routine cnt [%d] \r\n", max_try_cnt);
		}
        // ======================================
        // check Network Interface...
        // ======================================
        if( is_found_ppp_device() == MDMC_FALSE )
        {
			if (log_enable)
			{
    			MDMC_DEBUG_FUNC_TRACE("MDMC Network check routine : Network Conn fail try connect... \r\n");
			}
            net_manage_info_t net_manage_arg;
            net_manage_arg.net_conn = MDMC_TRUE;
            //net_manage_arg.timeout_sec = MDMC_NETWORK_CHECK_INTERVAL_SEC;
            net_manage_arg.timeout_sec = 0; // don't wait... try conn
            net_manage_arg.reset = MDMC_FALSE;
            mdmc_api_net_conn (&net_manage_arg);
        }
        else
		{
			if (log_enable == 1)
			{
    			MDMC_DEBUG_FUNC_TRACE("MDMC Network check routine  : MDMC Network Conn success \r\n");
			}
//            return MDMC_TRUE;
			break;
		}
        sleep(1);
    } while(max_try_cnt--);

	// 여기 까지 왔으면 기존 루틴상의  네트워크 연결은 문제가없다고 판단된다.
	
	{
		FILE *fp;
		int found = 0;

		char returnData[64];

		// 일단 ifconfig 명령어를 통해서 실제로 활성화된 네트워크 인터페이스를 찾는다.
		fp = popen("/sbin/ifconfig", "r");

		while (fgets(returnData, 64, fp) != NULL)
		{
		    if ( strstr(returnData,MDMC_NET_INTERFACE) ) // found PPP0 interface
				found = 1;
//			printf("%s", returnData);
		}

		pclose(fp);

		// 인터페이스를 찾았는지 체크
		if (found)
		{
			return MDMC_TRUE;
		}
		else
		{
			MDMC_DEBUG_FUNC_TRACE("MDMC Network check routine  : Cannot Find PPP0 Network!!!!!!!!!! kill pppd process. \r\n");

			//system(MDMC_CMD_PPP_DOWN);
			network_device_down();
			sleep(1);
			return MDMC_FALSE;
		}
			
		return MDMC_TRUE;
	}
	

//#ifdef MDMC_NET_API_DEBUG
	if (log_enable == 1)
	{
    	MDMC_DEBUG_FUNC_TRACE("End \r\n");
	}
//#endif

	return MDMC_FALSE;
}

int at_channel_recovery(void)
{
	send_at_cmd("at");
	return 0;

}

void mdmc_module_network_check(void)
{
    static int try_cnt = 0;
	static int network_recovery = 1;

    if (!_net_check(1)) {
        /* 네트워크 실패시 처리 */
        try_cnt ++;
		network_recovery = 1;
    }
    else
	{
		network_act = 1;
		if (network_recovery)
		{
			at_channel_recovery();
			network_recovery = 0;
		}

        try_cnt = 0;
	}

    if (try_cnt > MDMC_MODULE_CHK_API_MAX_NET_RETRY)
    {
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
        printf("!!!! network cannot connect! !!!!!!\r\n");
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");

        MDMC_DEBUG_FUNC_TRACE("Power Off");

        system("poweroff &");
        system("poweroff &");
        system("poweroff &");
        system("poweroff &");
        system("poweroff &");
    }

}


int ip_check(char *ip)
{
	if(strcmp(ip, " ") == 0)
	{
		//printf("IP address NULL!\n");
		return 0;
	}

	int len = strlen(ip);

	if( len > 15 || len < 7 )
		return 0;

	int nNumCount = 0;
	int nDotCount = 0;
	int i = 0;

	for( i=0; i<len; i++)
	{
		if(ip[i] < '0' || ip[i] > '9')
		{
			if(ip[i] == '.')
			{
				++nDotCount;
				nNumCount = 0;
			}
			else
				return 0;
		}
		else
		{
			if(++nNumCount > 3)
				return 0;
		}
	}

	if(nDotCount != 3)
		return 0;

	return 1;
}

int MNet_getDomainIP(char* domain, unsigned char* ip)
{
	struct hostent *host_entry;

	host_entry = gethostbyname(domain);

	if ( !host_entry)
	{
		//printf( "gethostbyname() fail/n");
		return 0;
	}

	//for ( ndx = 0; NULL != host_entry->h_addr_list[ndx]; ndx++)
	sprintf( ip, "%s", inet_ntoa( *(struct in_addr*)host_entry->h_addr_list[0]));

	return ip_check(ip);

}
