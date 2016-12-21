#ifndef MDMC_API_NETWORK_H                                                
#define MDMC_API_NETWORK_H                                                
                                                                          
// =======================================================                
//  MDMC Network API Argument Structure                                   
// =======================================================                
//#ifdef MDMC_SET_NET_CONN_DEFAULT_VAL                                    
//#define MDMC_NET_CONN_TIME_OUT_SEC    10                                
//#define MDMC_NET_CONN_FAIL_RESET  MDMC_TRUE                             
//#endif                                                                  
                                                                          
#define MDMC_MODULE_CHK_API_MAX_NET_RETRY   20                              
#define MDMC_MODULE_CHK_API_MAX_NET_RETRY_1 10                              
                                                                          
                                                                                                         
#define MDMC_MAX_IP_ADDR_LEN 32                                           
                                                                          
#define MDMC_FALSE  0                                                     
#define MDMC_TRUE   1                                                     


typedef unsigned char bool_t;                                             
                                                                          
struct net_manage_info                                                    
{                                                                         
	    bool_t net_conn;                                                      
		    int  timeout_sec;                                                     
			    bool_t reset;                                                         
};                                                                        
typedef struct net_manage_info net_manage_info_t;                         
                                                                          
struct net_manage_ret                                                     
{                                                                         
	    bool_t net_stat;                                                      
		    char ip_addr[MDMC_MAX_IP_ADDR_LEN];                                   
};                                                                        
typedef struct net_manage_ret net_manage_ret_t;                           

#define DEBUG

#ifdef DEBUG
#define MDMC_DEBUG_FUNC_TRACE(fmt, arg...)  printf(fmt, ##arg)
#else
#define MDMC_DEBUG_FUNC_TRACE(fmt, arg...)  do {} while (0)
#endif

int MNet_getDomainIP(char* domain, unsigned char* ip);
extern int network_act;

bool_t is_found_ppp_device( );
bool_t mdmc_api_get_net_info(net_manage_ret_t * result);
#endif // MDMC_API_NETWORK_H

