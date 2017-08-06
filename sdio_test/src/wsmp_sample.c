/******************************************************************************
* File Name    : wsmp_sample.c
******************************************************************************/

/***************************** MACRO DEFINITIONS ****************************/
#ifndef	TRUE
#define	TRUE			1
#endif
#ifndef	FALSE
#define	FALSE			0
#endif

#define	MAX_RCV_BUFF_SIZE	2048

#define UNUSED(x)		( (void)x )
#define	IFNAMSIZ						16
#define	IFMACSIZ						6

#if !defined(CONFIG_LIBNL20) && !defined(CONFIG_LIBNL30)
#define nl_sock nl_handle
#endif


/********************************** Include ************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <net/if_ether.h>    
#if 0
    #include <linux/if_ether.h>
    #include <linux/if_packet.h>
    #include <linux/netlink.h>
    #include <linux/rtnetlink.h>
    #include <linux/wireless.h>

    #include <linux/ip.h>
#else
    #include <sys/iofunc.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <sys/ioctl.h>
    #include <sys/iomsg.h>
    #include <net/if.h>
    #include <sys/pps.h>
    #include <sys/slog.h>
    #include <sys/slogcodes.h>
    #include <net/if.h>
    #include <net/route.h>
    #include <net80211/ieee80211_netbsd.h>
    #include <net80211/ieee80211.h>
    #include <stddef.h>
#endif    
#include <ifaddrs.h>

#include <stdarg.h>
//#include <sys/timerfd.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "nl80211.h"
#include "ks_api.h"

// #include <netlink/genl/genl.h>
// #include <netlink/genl/family.h>
// #include <netlink/genl/ctrl.h>
// #include <netlink/msg.h>
// #include <netlink/attr.h>
// #include <stdbool.h>
// #include <netlink/netlink.h>
//#include <endian.h>

//#include "ks_api.h"


/***************************** TYPE DEFINITIONS ****************************/
typedef	unsigned char	UCHAR;
typedef	unsigned short 	USHORT;
typedef	unsigned int	UINT;
typedef	unsigned long	ULONG;
typedef	long long		DLONG;

typedef	signed char		SCHAR;
typedef	signed short	SSHORT;
typedef	signed int		SINT;
typedef	signed long		SLONG;

typedef char			CHAR_T;
typedef signed int		BOOL;

typedef enum {
	SOCKET_RAW = 0,
	SOCKET_MAX
} SOCKET_TYPE;

typedef struct {
	SINT			If_Index;
	UCHAR			CCH_MAC_Address0[IFMACSIZ];
	UCHAR			SCH_MAC_Address0[IFMACSIZ];
	UCHAR			CCH_MAC_Address1[IFMACSIZ];
	UCHAR			SCH_MAC_Address1[IFMACSIZ];
} SRC_INFO;

typedef enum {
	IOCTL_STATUS_OK = 0,
	IOCTL_STATUS_ARG_ERR,
	IOCTL_STATUS_INVALID_REQUEST,
	IOCTL_STATUS_INVALID_PARAM
} IOCTL_STATUS;

struct family_data {
	const char *group;
	int id;
};

#ifdef NL80211_WAVE
struct nl80211_state {
	struct nl_sock *nl;
	struct nl_sock *nl_sock;
	int nl80211_id;
	struct nl_cb *nl_cb_event;
	struct nl_sock *nl_event;
	int err;
};

struct nl_handle
{
	struct sockaddr_nl	h_local;
	struct sockaddr_nl	h_peer;
	int			h_fd;
	int			h_proto;
	unsigned int		h_seq_next;
	unsigned int		h_seq_expect;
	int			h_flags;
	struct nl_cb *		h_cb;
};
#endif

struct us_txpbp{
	UCHAR	format;
	UCHAR	size;
	UCHAR	Priority;
	UCHAR	Data_rate;
	UCHAR	Tx_Power;
	UCHAR	Channel;
	UCHAR	Tx_Parameter;
	UCHAR	Reserved;
	DLONG   ExpiryTime;
}__attribute__((packed));

struct wsm_frame_form {
	struct ethhdr	Hdr;
	struct us_txpbp PbP;
	UCHAR		Buff[ 2048 ];
}__attribute__((packed));


/**************************** PRIVATE PROTOTYPE ****************************/
#ifdef DEBUG_PRINT_ENABLE
void Debug_Print( CHAR_T *Format_p, ... );
#else
#define Debug_Print( ARG, ... )
#endif

extern unsigned int if_nametoindex(const char *ifname);
static void Close_Soket( void );
static SINT Create_Socket(void);
static SINT Bind_Socket(void);
static void NET_Terminate_Socket( void );
static SINT NET_Init_Socket( void );
static void Init_Main(void);
void Print_Dump( UCHAR *Data_p, SINT Length );
IOCTL_STATUS Io_Ctrl_Wlan_Private( SINT Socket, enum wave_sub_command Request, SINT Argc, CHAR_T **Argv );
static void Snd_Frame_Func(UINT cnt, UINT len);

#ifdef NL80211_WAVE
static int nl80211_init(struct nl80211_state *state);
static int nl80211_exit(struct nl80211_state *state);
static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg);
static int finish_handler(struct nl_msg *msg, void *arg);
static int ack_handler(struct nl_msg *msg, void *arg);
static int nl_get_multicast_id(struct nl80211_state *state,
			       const char *family, const char *group);
static int family_handler(struct nl_msg *msg, void *arg);
int os_strncmp(const char *s1, const char *s2, size_t n);
static int send_and_recv( struct nl_handle *nl_handle, struct nl_msg *msg,
			 int (*valid_handler)(struct nl_msg *, void *), void *valid_data);
static int process_event(struct nl_msg *msg, void *arg);
static int no_seq_check(struct nl_msg *msg, void *arg);
#endif
static void set_SETCH_REQ( unsigned short value );
static void set_MACADDR_EX_REQ(void);


/**************************** PUBLIC VARIABLES ****************************/
CHAR_T			g_If_Name_Buff[ IFNAMSIZ ];
SINT				g_Socket_Fd[ SOCKET_MAX ];
struct nl80211_state	nlstate;
const UCHAR	dst_addr[6]	= {0xff,0xff,0xff,0xff,0xff,0xff};
struct us_txpbp			g_us_PbP;
pthread_t		Rcv_Socket_Thread;
SRC_INFO	g_Src_Info;
BOOL	event_flag;

IWREQ_CH							priv_ch;
IWREQ_SET_MAC_ADDR_EX				priv_set_mac_ex;

/**************************** MAIN PROCESS ****************************/
int main( int argc, char **argv )
{

	Init_Main();

	if( 2 == argc ) {
		strncpy(g_If_Name_Buff,argv[0],32U);
	}else{
		strncpy(g_If_Name_Buff,"wave0",32U);
	}
#ifdef NL80211_WAVE
	if (nl80211_init(&nlstate)){
		printf("nl80211 init failed\n");
		return 1;
	}
#endif
	if ( 0 != NET_Init_Socket() ) {
		return -1;
	}

	/* set channel */
	set_SETCH_REQ( 172 );

	/* set CCH and SCH MAC Address */
	set_MACADDR_EX_REQ();

	/*send 100byte sequencial data as WSM frame three times */
	Snd_Frame_Func(30000, 100);

	NET_Terminate_Socket();
#ifdef NL80211_WAVE
	nl80211_exit(&nlstate);
#endif
	return( 1 );
}

int wave_ioctl(struct wave_driver_data *drv, int cmd, void *buf, int len,
		int set) {
	io_devctl_t msg;
	struct ifdrv *ifd = (struct ifdrv *) drv->wl;
	iov_t siov[4];
	iov_t riov[4];
	int sparts;
	int rparts;
	/* A hack, or improvisation if you prefer */
	wave_ioctl_t ioc;
	int ret = 0;
	int stack_ioctl = set ? SIOCSDRVSPEC : SIOCGDRVSPEC;
	//SIOCSDRVSPEC : Set driver-specific parameters
	//SIOCGDRVSPEC : Get driver-specific parameters
	int nbytes;
	int i;
	char buff[2048];
	memset(buff, 0, 2048);
	int frag_cnt, j, per_len;

	/*When the buff length exceed the MAX MSG SIZE 1024, we should use fragment */
	frag_cnt = len / (MAX_MSG_LENGTH + 1) + 1;

	for (j = 0; j < frag_cnt; j++) {
		if (frag_cnt > 1) {
			per_len =
					(j == frag_cnt - 1) ?
							(len - (j * MAX_MSG_LENGTH)) : MAX_MSG_LENGTH;

			/* do it */
			ifd->ifd_cmd = cmd;
			ifd->ifd_len = sizeof(ioc) + per_len; /* stack will use this */
			ifd->ifd_data = 0; /* valid only for our space */

			ioc.cmd = cmd;
			ioc.len = per_len; /* driver will use this */
			//ioc.buf = buf;      /* invalid in the driver mem-space but used as bool */
			ioc.set = set;
			ioc.driver = 0;
			ioc.fragment.frag_t = frag_cnt;
			ioc.fragment.frag_num = j + 1;
		} else {
			ifd->ifd_len = sizeof(ioc) + len;
			/* do it */
			ifd->ifd_cmd = cmd;
			ifd->ifd_data = 0; /* valid only for our space */

			ioc.cmd = cmd;
			ioc.len = len;
			//ioc.buf = buf;      /* invalid in the driver mem-space but used as bool */
			ioc.set = set;
			ioc.driver = 0;
			ioc.fragment.frag_t = frag_cnt;
			ioc.fragment.frag_num = 1;
		}

		SETIOV(&siov[0], &msg, sizeof(io_devctl_t)); //32Bytes
		SETIOV(&siov[1], ifd, sizeof(struct ifdrv)); //12Bytes
		SETIOV(&siov[2], &ioc, sizeof(ioc));
		memcpy(&riov[0], &siov[0], 3 * sizeof(iov_t));

		if (buf) {
			if (frag_cnt > 1) {
				//per_len = (j == frag_cnt - 1) ? (len - (j * MAX_MSG_LENGTH)) : MAX_MSG_LENGTH;
				SETIOV(&siov[3], buf + (j * MAX_MSG_LENGTH), per_len);
			} else {
				SETIOV(&siov[3], buf, len);
			}
			SETIOV(&riov[3], buf, len);
			sparts = 4;
		} else {
			sparts = 3;
		}
		rparts = sparts;
		Debug_Print("sparts=%d\n", sparts);
		for (nbytes = 0, i = 1; i < sparts; i++) {
			nbytes += GETIOVLEN(&siov[i]);
		}
		Debug_Print("nbytes=%d\n", nbytes);
		memset(&msg, 0, sizeof(msg));
		msg.i.type = _IO_DEVCTL;
		msg.i.combine_len = sizeof(msg.i);
		msg.i.dcmd = stack_ioctl;

		msg.i.nbytes = nbytes;
		msg.i.zero = 0;

		if ((ret = MsgSendv_r(drv->ioctl_sock, siov, sparts, riov, rparts))
				< 0) {
			perror("MsgSendv_r");
			ret = -1;
		}
	}
	return ret;
}

/******************************************************************************
* Outline      : *Rcv_Socket_Loop
* Description  : The main thread of reception socket
* Argument     : *Arg_p -- Arument of thread
* Return Value : NULL
******************************************************************************/
static void *Rcv_Socket_Loop( void *Arg_p )
{
	static UCHAR		rcv_buff[ MAX_RCV_BUFF_SIZE ];
	BOOL				run = TRUE;
	fd_set				fds, read_fds;
	SINT				max_fd = -1;
	SINT				rcv_bytes;
	SINT				i;

	UNUSED(Arg_p);
	FD_ZERO( &read_fds );
	for ( i = 0; i < SOCKET_MAX; i++ ) {
		/* register Socket_Fd */
		FD_SET( g_Socket_Fd[ i ], &read_fds );
		/* register maximum number of Socket Fd */
		if ( max_fd < g_Socket_Fd[ i ] ) {
			max_fd = g_Socket_Fd[ i ];
		}
	}
	
	FD_SET(nlstate.nl_event->h_fd, &read_fds);
	if ( max_fd < nlstate.nl_event->h_fd) {
		max_fd = nlstate.nl_event->h_fd;
	}

	while ( TRUE == run ) {
		/* initialize Fds */
		memcpy( &fds, &read_fds, sizeof( fd_set ) );
		/* wait to receive */
		if ( -1 != select( max_fd + 1, &fds, NULL, NULL, NULL ) ) {

			/* receive NetLink */
			if ( 0 != FD_ISSET( nlstate.nl_event->h_fd, &fds ) ) {
				nlstate.err = 1;
				while (nlstate.err > 0)	{
					nl_recvmsgs(nlstate.nl_event, nlstate.nl_cb_event);
				}
			}

			/* receive raw data */
			if ( 0 != FD_ISSET( g_Socket_Fd[ SOCKET_RAW ], &fds  )){
				/* receive Linklevel data */
				rcv_bytes = recv( g_Socket_Fd[ SOCKET_RAW ], rcv_buff, sizeof( rcv_buff ), 0 );
				if(rcv_bytes >0 ){
//					Debug_Print( "RAW Socket Data Recieved %dByte\n", rcv_bytes );
//					Print_Dump((UCHAR*)rcv_buff, rcv_bytes );
				}
			}

		} else {
			run = FALSE;
		}
	}
	return( NULL );
}


/******************************************************************************
* Outline      : Init_Main
* Description  : Main initialization
* Argument     : none
* Return Value : none
******************************************************************************/
static void Init_Main(void)
{
	
	memset( g_If_Name_Buff, 0, sizeof( g_If_Name_Buff ) );

	g_us_PbP.format= 0;
	g_us_PbP.size= 16;
	g_us_PbP.Priority= 0;
	g_us_PbP.Data_rate= 24;
	g_us_PbP.Tx_Power= 40;
	g_us_PbP.Channel= 172;
	g_us_PbP.Tx_Parameter= 0;
	g_us_PbP.Reserved= 0;
	g_us_PbP.ExpiryTime= 0;

}



/******************************************************************************
* Outline      : NET_Init_Socket
* Description  : Initialization of Socket processing
* Argument     : none
* Return Value : Ret 0 -- Success
*			   :    -1 -- Faile
*			   :    -2 -- Fails in the acquisition of IF information. 
*			   :    -3 -- Fails in the socket bind
******************************************************************************/
SINT NET_Init_Socket( void )
{
	SINT	ret = -1;

	/* create Socket */
	ret = Create_Socket();
	
	if ( 0 == ret ) {
		ret = -4;
		Debug_Print( "Creating Data Recieve Thread\n" );
		if ( 0 == pthread_create( &Rcv_Socket_Thread, NULL, Rcv_Socket_Loop, NULL ) ) {
			/* complete soecket binding */
			puts( "#>>Socket Initialize Done" );
			ret = 0;
		} else {
			printf(">>RcvSocketLoop Thread Create Error: %d, %s \n", errno, strerror( errno ) );
		}
	}
	g_Src_Info.If_Index = if_nametoindex(g_If_Name_Buff);
	if(g_Src_Info.If_Index == 0){
		return -1;
	}

	//NL80211_VENDOR_SUBCMD_GET_MAC_ADDRESS_EX
	//ret = Io_Ctrl_Wlan_Private( g_Socket_Fd[ SOCKET_RAW ], NL80211_VENDOR_SUBCMD_GET_MAC_ADDRESS_EX , 0, NULL);
    ret = Io_Ctrl_Wlan_Private( g_Socket_Fd[ SOCKET_RAW ], SIOCGIFHWADDR, 0, NULL)
	if ( 0 != ret ) {
		printf(">>Get Mac Address Error: %d, %s \n", errno, strerror( errno ) );
		return ret;
	}

	/*
	 * Bind
	 */
	if ( 0 == ret ) {
		ret = Bind_Socket();
	}

	/*
	 * close socket if error status was returned 
	 */
	if ( 0 != ret ) {
		Close_Soket();
	}
	return ret;
}

/******************************************************************************
* Outline      : Create_Socket
* Description  : Creste Each Sockets
* Argument     : none
* Return Value : ret 0 -- Success
*			   :    -1 -- Faile
******************************************************************************/
static SINT Create_Socket(void)
{
	SINT	ret = -1;
	for(;;){
	
		Debug_Print( "Creating Socket for RAW\n" );
		//if((g_Socket_Fd[ SOCKET_RAW ] = socket( PF_PACKET, SOCK_RAW, (SINT)htons( ETH_P_ALL ))) == -1){
        if((g_Socket_Fd[ SOCKET_RAW ] = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
			printf(">>Can't open RAW socket: %d, %s \n", errno, strerror( errno ) );
			break;
		}
		ret = 0;
		break;
	}
	
	return ret;
}


/******************************************************************************
* Outline      : Bind_Socket
* Description  : Bind Each Sockets
* Argument     : none
* Return Value : ret 0 -- Success
*			   :    -4 -- Faile
******************************************************************************/
static SINT Bind_Socket(void)
{
	struct sockaddr_ll	sock_addr_link_level;
	SINT ret = 0;

	memset( &sock_addr_link_level, 0, sizeof( sock_addr_link_level ) );
	sock_addr_link_level.sll_family   = AF_PACKET;
	sock_addr_link_level.sll_protocol = htons( ETH_P_ALL );
	sock_addr_link_level.sll_ifindex  = g_Src_Info.If_Index;
	/* bind raw socket that is used to transmit and receive ioctl and data */
	Debug_Print( "Binding RAW Socket\n" );
	if ( 0 != bind( g_Socket_Fd[ SOCKET_RAW ], ( struct sockaddr* )&sock_addr_link_level, sizeof( sock_addr_link_level ) ) ) {
		printf(">>Can't bind RAW socket: %d, %s \n", errno, strerror( errno ) );
		ret = -3;
	}

	return ret;
}


/******************************************************************************
* Outline      : Close_Soket
* Description  : Close file descriptor
* Argument     : none
* Return Value : none
******************************************************************************/
static void Close_Soket( void )
{
	SINT	i;

	for ( i = 0; i < SOCKET_MAX; i++ ) {
		close( g_Socket_Fd[ i ] );
		g_Socket_Fd[ i ] = -1;
	}
}

/******************************************************************************
* Outline      : NET_Terminate_Socket
* Description  : Terminate of socket reception thread processing
* Argument     : none
* Return Value : none
******************************************************************************/
void NET_Terminate_Socket( void )
{
	/* Socket Close */
	Close_Soket();

	/* terminate Socket receiving thread */
	pthread_cancel( Rcv_Socket_Thread );
	/* wait to complete termination */
	pthread_join( Rcv_Socket_Thread, NULL );
}


/******************************************************************************
* Outline      : Snd_Frame_Func
* Description  : Send sequential data
* Argument     : cnt   -- amount of frames that should be sent
* Argument     : len   -- size of a frame that should be sent
* Return Value : none
******************************************************************************/
static void Snd_Frame_Func(UINT cnt, UINT len)
{
	SINT	i;
	UCHAR	j,data,*pdata;
	struct sockaddr_ll	sock_addr_link_level;
	UINT	sent_bytes,sent_in_packet;
	const UINT	header_size = sizeof(struct ethhdr ) + sizeof(struct us_txpbp );
	struct wsm_frame_form	frame;
	
	if(len > 1500){
		return;
	}

	data = 0x00;

	memset( &sock_addr_link_level, 0, sizeof( sock_addr_link_level ) );
	sock_addr_link_level.sll_ifindex = g_Src_Info.If_Index;

	memcpy( frame.Hdr.h_dest,dst_addr, ETH_ALEN );
	memcpy( frame.Hdr.h_source, &g_Src_Info.SCH_MAC_Address0, ETH_ALEN );
	frame.Hdr.h_proto = htons(0x88DC);	//set protocol number of WSM
	memcpy( &frame.PbP, &g_us_PbP , sizeof(struct us_txpbp));

	for(i=0;i<cnt;i++){
		pdata = &frame.Buff[0];
		for(j=0;j<len;j++){
			*pdata++= data++;
		}
		sent_bytes = 0;
		while(sent_bytes  < header_size + len){
			sent_in_packet = sendto( g_Socket_Fd[ SOCKET_RAW ], (UCHAR *)&frame + sent_bytes, header_size + len - sent_bytes, 
									 0, ( struct sockaddr* )&sock_addr_link_level, sizeof( sock_addr_link_level ) );

			if ( sent_in_packet < 0 ) {
				printf(">>RAW_Send Error: %d, %s \n", errno, strerror( errno ) );
				break;
			}
			else{
				sent_bytes += sent_in_packet;
			}

		}
	}

}

#ifdef NL80211_WAVE
/******************************************************************************
*
*  nl80211_init
*
*
******************************************************************************/
static int no_seq_check(struct nl_msg *msg, void *arg)
{
	return NL_OK;
}

static int sub_command_setch_process(const void *data, int data_len, int event_status, unsigned int phy_info)
{
	if (event_status != 0) {
		printf(">> NL80211_VENDOR_SUBCMD_SET_CH Error, Return: %d\n",  event_status);
	}
	else {
		puts("NL80211_VENDOR_SUBCMD_SET_CH success!");
	}

	return 0;
}

static int sub_command_setmacex_process(const void *data, int data_len, int event_status, unsigned int phy_info)
{
	if (event_status != SETMACADDREX_RESULT_SUCCESS) {
		printf(">> NL80211_VENDOR_SUBCMD_SET_MAC_ADDRESS_EX Error, Return: %d\n",  event_status);
	}
	else {
		puts("NL80211_VENDOR_SUBCMD_SET_MAC_ADDRESS_EX success!");

		if(phy_info == WIPHYNAME_PHY0){

			if( (priv_set_mac_ex.type == SETMACADDREX_TYPE_CCH ) || 
				(priv_set_mac_ex.type == SETMACADDREX_TYPE_BOTH ) ){

				memcpy( g_Src_Info.CCH_MAC_Address0, priv_set_mac_ex.cch_mac_addr, IFMACSIZ );
			}

			if( (priv_set_mac_ex.type == SETMACADDREX_TYPE_SCH ) || 
				(priv_set_mac_ex.type == SETMACADDREX_TYPE_BOTH ) ){

				memcpy( g_Src_Info.SCH_MAC_Address0, priv_set_mac_ex.sch_mac_addr, IFMACSIZ );
			}
		}

		if(phy_info == WIPHYNAME_PHY1){

			if( (priv_set_mac_ex.type == SETMACADDREX_TYPE_CCH ) || 
				(priv_set_mac_ex.type == SETMACADDREX_TYPE_BOTH ) ){

				memcpy( g_Src_Info.CCH_MAC_Address1, priv_set_mac_ex.cch_mac_addr, IFMACSIZ );
			}

			if( (priv_set_mac_ex.type == SETMACADDREX_TYPE_SCH ) || 
				(priv_set_mac_ex.type == SETMACADDREX_TYPE_BOTH ) ){

				memcpy( g_Src_Info.SCH_MAC_Address1, priv_set_mac_ex.sch_mac_addr, IFMACSIZ );
			}
		}
	}

	return 0;
}

static int sub_command_getmacex_process(const void *data, int data_len, int event_status, unsigned int phy_info)
{
	IWREQ_GET_MAC_ADDR_EX		*mac_data;

	if(event_status != 0){
		printf(">> NL80211_VENDOR_SUBCMD_GET_MAC_ADDRESS_EX Error, Return event_status: %d\n", event_status);
	}
	else{
		if((phy_info == WIPHYNAME_PHY0)&&(data_len == sizeof(IWREQ_GET_MAC_ADDR_EX))){
			mac_data = (IWREQ_GET_MAC_ADDR_EX *)data;
			memcpy( g_Src_Info.CCH_MAC_Address0, mac_data->cch_mac_addr, IFMACSIZ );
			memcpy( g_Src_Info.SCH_MAC_Address0, mac_data->sch_mac_addr, IFMACSIZ );

			printf("#wlan0 CCH MAC_Addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
				 mac_data->cch_mac_addr[0],mac_data->cch_mac_addr[1],mac_data->cch_mac_addr[2],
				 mac_data->cch_mac_addr[3],mac_data->cch_mac_addr[4],mac_data->cch_mac_addr[5]);

			printf("#wlan0 SCH MAC_Addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
				 mac_data->sch_mac_addr[0],mac_data->sch_mac_addr[1],mac_data->sch_mac_addr[2],
				 mac_data->sch_mac_addr[3],mac_data->sch_mac_addr[4],mac_data->sch_mac_addr[5]);
		}

		if((phy_info == WIPHYNAME_PHY1)&&(data_len == sizeof(IWREQ_GET_MAC_ADDR_EX))){
			mac_data = (IWREQ_GET_MAC_ADDR_EX *)data;
			memcpy( g_Src_Info.CCH_MAC_Address1, mac_data->cch_mac_addr, IFMACSIZ );
			memcpy( g_Src_Info.SCH_MAC_Address1, mac_data->sch_mac_addr, IFMACSIZ );

			printf("#wlan1 CCH MAC_Addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
				 mac_data->cch_mac_addr[0],mac_data->cch_mac_addr[1],mac_data->cch_mac_addr[2],
				 mac_data->cch_mac_addr[3],mac_data->cch_mac_addr[4],mac_data->cch_mac_addr[5]);

			printf("#wlan1 SCH MAC_Addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
				 mac_data->sch_mac_addr[0],mac_data->sch_mac_addr[1],mac_data->sch_mac_addr[2],
				 mac_data->sch_mac_addr[3],mac_data->sch_mac_addr[4],mac_data->sch_mac_addr[5]);
		}
	}

	return 0;
}

typedef struct sub_command_lookup_tbl {
	wave_sub_command_e sub_cmd;
	int (*process)(const void *data, int data_len, int event_status, unsigned int phy_info);
}SUB_COMMAND_LOOKUP_TBL_T;

static const SUB_COMMAND_LOOKUP_TBL_T sub_command_process_lookup_tbl[] =
{
	{NL80211_VENDOR_SUBCMD_SET_CH,				sub_command_setch_process			},
	{NL80211_VENDOR_SUBCMD_SET_MAC_ADDRESS_EX,	sub_command_setmacex_process		},
	{NL80211_VENDOR_SUBCMD_GET_MAC_ADDRESS_EX,	sub_command_getmacex_process		},

	{NL80211_VENDOR_SUBCMD_MAX, 				NULL								}

};

static int process_event(struct nl_msg *msg, void *arg)
{
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
	int len = 0, err = 0;
	void *buff;
	unsigned int phy_info = 0;
	unsigned int sub_command = 0;
	int event_status = 0;
	int fixed_len = 0;
	int i = 0;

	int *ret = arg;
	*ret = 0;

	printf("%s\n", __func__);

	err = nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
					genlmsg_attrlen(gnlh, 0), NULL);

	if (err < 0)
	  return err;

	if(tb_msg[NL80211_ATTR_VENDOR_DATA]) {
		len = nla_len(tb_msg[NL80211_ATTR_VENDOR_DATA]);
		buff = nla_data(tb_msg[NL80211_ATTR_VENDOR_DATA]);

		fixed_len = sizeof(phy_info) + sizeof(sub_command) + sizeof(event_status);
		
		memcpy(&phy_info, buff, sizeof(phy_info));
		memcpy(&sub_command, buff + sizeof(phy_info), sizeof(sub_command));
		memcpy(&event_status, buff + sizeof(phy_info) + sizeof(sub_command), sizeof(event_status));

		if(sub_command < NL80211_VENDOR_SUBCMD_VSA || 
			sub_command > NL80211_VENDOR_SUBCMD_MAX) {
			printf("%s sub command error!\n", __func__);
		}
		else {
			for(i = 0; NL80211_VENDOR_SUBCMD_MAX != sub_command_process_lookup_tbl[i].sub_cmd; i++){
				if(sub_command ==  sub_command_process_lookup_tbl[i].sub_cmd){
					err = sub_command_process_lookup_tbl[i].process((buff + fixed_len), (len - fixed_len), event_status, phy_info);
				}
			}
		}
	}

	event_flag = FALSE;

	return NL_SKIP;
}

static int send_and_recv(struct nl_handle *nl_handle, struct nl_msg *msg,
			 int (*valid_handler)(struct nl_msg *, void *), void *valid_data)
{
	struct nl_cb *cb;
	int err = -ENOMEM;

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb)
		goto out;

	err = nl_send_auto_complete(nl_handle, msg);
	if (err < 0)
		goto out;

	err = 1;
	nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);

	if (valid_handler)
		nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, valid_handler, valid_data);

	while (err > 0)
		nl_recvmsgs(nl_handle, cb);
 out:
	nl_cb_put(cb);
	nlmsg_free(msg);
	return err;
}

int os_strncmp(const char *s1, const char *s2, size_t n)
{
	if (n == 0)
		return 0;

	while (*s1 == *s2) {
		if (*s1 == '\0')
			break;
		s1++;
		s2++;
		n--;
		if (n == 0)
			return 0;
	}

	return *s1 - *s2;
}

static int family_handler(struct nl_msg *msg, void *arg)
{
	struct family_data *res = arg;
	struct nlattr *tb[CTRL_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *mcgrp;
	int i;

	nla_parse(tb, CTRL_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);
	if (!tb[CTRL_ATTR_MCAST_GROUPS])
		return NL_SKIP;

	nla_for_each_nested(mcgrp, tb[CTRL_ATTR_MCAST_GROUPS], i) {
		struct nlattr *tb2[CTRL_ATTR_MCAST_GRP_MAX + 1];
		nla_parse(tb2, CTRL_ATTR_MCAST_GRP_MAX, nla_data(mcgrp),
			  nla_len(mcgrp), NULL);
		if (!tb2[CTRL_ATTR_MCAST_GRP_NAME] ||
		    !tb2[CTRL_ATTR_MCAST_GRP_ID] ||
		    os_strncmp(nla_data(tb2[CTRL_ATTR_MCAST_GRP_NAME]),
			       res->group,
			       nla_len(tb2[CTRL_ATTR_MCAST_GRP_NAME])) != 0)
			continue;
		res->id = nla_get_u32(tb2[CTRL_ATTR_MCAST_GRP_ID]);
		break;
	};

	return NL_SKIP;
}

static int nl_get_multicast_id(struct nl80211_state *state,
			       const char *family, const char *group)
{
	struct nl_msg *msg;
	int ret = -1;
	struct family_data res = { group, -ENOENT };

	msg = nlmsg_alloc();
	if (!msg)
		return -ENOMEM;
	
	genlmsg_put(msg, 0, 0, genl_ctrl_resolve(state->nl, "nlctrl"),
		    0, 0, CTRL_CMD_GETFAMILY, 0);
	nla_put_string(msg, CTRL_ATTR_FAMILY_NAME, family);

	ret = send_and_recv(state->nl, msg, family_handler, &res);
	if (ret == 0)
		ret = res.id;

	return ret;
}

static int nl80211_init(struct nl80211_state *state)
{
	int ret;

	state->nl_sock = nl_socket_alloc();
	if (!state->nl_sock) {
		printf("Failed to allocate netlink socket for nl80211.\n");
		return -ENOMEM;
	}

	if (genl_connect(state->nl_sock)) {
		printf("Failed to connect to generic netlink.\n");
		ret = -ENOLINK;
		goto out_handle_destroy;
	}

	state->nl80211_id = genl_ctrl_resolve(state->nl_sock, "nl80211");
	if (state->nl80211_id < 0) {
		printf("'nl80211' generic netlink not found.\n");
		ret = -ENOENT;
		goto out_handle_destroy;
	}

	state->nl_cb_event = nl_cb_alloc(NL_CB_DEFAULT);
	if (!state->nl_cb_event)
	{
		printf("Failed to allocate netlink callbacks\n");
		ret = -ENOMEM;
		goto out_handle_destroy;
	}

	state->nl_event = nl_socket_alloc();
	if (!state->nl_event) {
		printf("Failed to allocate netlink socket for event.\n");
		ret = -ENOMEM;
	}

	if (genl_connect(state->nl_event)) {
		printf("Failed to connect to generic netlink for event.\n");
		ret = -ENOLINK;
		goto out_handle_destroy;
	}

	state->nl = nl_socket_alloc();
	if (!state->nl) {
		printf("Failed to allocate netlink socket for multicat id.\n");
		ret = -ENOMEM;
	}

	if (genl_connect(state->nl)) {
		fprintf(stderr, "Failed to connect to generic netlink.\n");
		ret = -ENOLINK;
		goto out_handle_destroy;
	}

	ret = nl_get_multicast_id(state, "nl80211", "vendorRenesasUS");
	if (ret >= 0)
		ret = nl_socket_add_membership(state->nl_event, ret);
	if (ret < 0) {
		printf("nl80211: Could not add multicast "
			   "membership for scan events: %d (%s)",
			   ret, strerror(-ret));
	}

	nl_cb_set(state->nl_cb_event, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, 
				no_seq_check, &state->err);
	nl_cb_set(state->nl_cb_event, NL_CB_VALID, NL_CB_CUSTOM, 
			process_event, &state->err);	

	return 0;

 out_handle_destroy:
 	if(state->nl_sock)
		nl_socket_free(state->nl_sock);
	if(state->nl_cb_event){
		nl_cb_put(state->nl_cb_event);
		state->nl_cb_event = NULL;
	}
	if(state->nl_event)
		nl_socket_free(state->nl_event);
	if(state->nl)
		nl_socket_free(state->nl);
	
	return ret;
}

static int nl80211_exit(struct nl80211_state *state)
{
	 if(state->nl_sock)
		nl_socket_free(state->nl_sock);
	if(state->nl_cb_event){
		nl_cb_put(state->nl_cb_event);
		state->nl_cb_event = NULL;
	}
	if(state->nl_event)
		nl_socket_free(state->nl_event);
	if(state->nl)
		nl_socket_free(state->nl);

	return 0;
}

#endif


/******************************************************************************
* Outline      : Print_Dump
* Description  : Output the dump data to the console
* Argument     : *Data_p -- Address to dump data
*			   : Length  -- Length of dump data
* Return Value : none
******************************************************************************/
void Print_Dump( UCHAR *Data_p, SINT Length )
{
	SINT		i, j;
	CHAR_T		ch;

	printf("      | +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F | ASCII\n");
	printf(" -----+-------------------------------------------------+-----------------\n");
	for ( i = 0; i < Length; i += 16 ) {
		printf(" %04X | ", i);
		for ( j = 0; ( j < 16 ) && ( ( i + j ) < Length ); j++ ) {
			printf("%02X ", Data_p[ i + j ] );
		}
		for ( ; j < 16; j++ ) {
			printf("   ");
		}
		printf("| ");
		for ( j = 0; ( j < 16 ) && ( ( i + j ) < Length ); j++ ) {
			ch = (CHAR_T)toascii( Data_p[ i + j ] );
			printf("%c", ( isalnum( ch ) ? ch : '.' ) );
		}
		printf("\n");
	}
}

/******************************************************************************
* Outline      : Set_If_Name
* Description  : Set the device name
* Argument     : *Iw_Req_p -- Address to device name
* Return Value : none
******************************************************************************/
void Set_If_Name( struct iwreq *Iw_Req_p )
{
	memset( Iw_Req_p, 0, sizeof( struct iwreq ) );
	strncpy( Iw_Req_p->ifr_ifrn.ifrn_name, g_If_Name_Buff, IFNAMSIZ );
}

/******************************************************************************
* Outline 		 : Set_Iw_Req_In_Param
* Description  : Set iw_req In Parameter
* Argument		 : Socket  -- Socket type
*				 : Request -- Request code
*				 : iw_req  -- 
*				 : iw_req_priv
* 						 : Argc 	 -- Number of character strings input to command line
*				 : **Argv  -- Pointer to command name and command parameter character string
* Return Value : IOCTL_STATUS Status
******************************************************************************/
static IOCTL_STATUS Set_Iw_Req_In_Param(enum wave_sub_command Request, struct iwreq *iw_req, union iwreq_priv_u *iw_req_priv, SINT Argc, CHAR_T **Argv)
{
	IOCTL_STATUS	status = IOCTL_STATUS_OK;
	unsigned long	value;
	CHAR_T			*buff_p;
	SINT			i;
	UINT			mac_addr_uint[6];
	
	switch ( Request ) {
		case NL80211_VENDOR_SUBCMD_VSA:
			iw_req_priv->vsa_req.mgmt_id = g_vsa_req.mgmt_id;
//			iw_req_priv->vsa_req.vendor_spec.oid_len = g_vsa_req.vendor_spec.oid_len;
			iw_req_priv->vsa_req.vendor_spec.ven_len= g_vsa_req.vendor_spec.ven_len;
			iw_req_priv->vsa_req.repeat= g_vsa_req.repeat;
			iw_req_priv->vsa_req.ch_identifier.operating_class.class_len = g_vsa_req.ch_identifier.operating_class.class_len;
			iw_req_priv->vsa_req.ch_identifier.operating_class.current_operating_class = g_vsa_req.ch_identifier.operating_class.current_operating_class;
			iw_req_priv->vsa_req.ch_identifier.channel_num = g_vsa_req.ch_identifier.channel_num;
			iw_req_priv->vsa_req.channel_type = g_vsa_req.channel_type;
			for(i = 0; i < ETH_ALEN; i++)
				iw_req_priv->vsa_req.dst_mac[i] = g_vsa_req.dst_mac[i];
			for(i = 0; i < OID_LEN; i++)
				iw_req_priv->vsa_req.vendor_spec.ven_oid[i] = g_vsa_req.vendor_spec.ven_oid[i];
			for(i = 0; i < IEEE16094_VENDOR_SPECIFIC_SIZE; i++)
				iw_req_priv->vsa_req.vendor_spec.ven_specific[i] = g_vsa_req.vendor_spec.ven_specific[i];
			for(i = 0; i < 3; i++)
				iw_req_priv->vsa_req.ch_identifier.country_string[i] = g_vsa_req.ch_identifier.country_string[i];
			for(i = 0; i < IEEE16094_OPERATING_MAX_TRIPLETS_LEN; i++)
				iw_req_priv->vsa_req.ch_identifier.operating_class.operating_classes[i] = g_vsa_req.ch_identifier.operating_class.operating_classes[i];

			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->vsa_req;
			iw_req->u.data.length = sizeof( iw_req_priv->vsa_req );

			break;

		case NL80211_VENDOR_SUBCMD_VSAEND:
			break;

		case NL80211_VENDOR_SUBCMD_SCHSTART:
			iw_req_priv->schstart.ch_identifier.operating_class.class_len = g_schstart.ch_identifier.operating_class.class_len;
			iw_req_priv->schstart.ch_identifier.operating_class.current_operating_class = g_schstart.ch_identifier.operating_class.current_operating_class;
			iw_req_priv->schstart.ch_identifier.channel_num = g_schstart.ch_identifier.channel_num;
//			iw_req_priv->schstart.rate_set.length = g_schstart.rate_set.length;
			iw_req_priv->schstart.edca_params.qos_info.vo_flag = g_schstart.edca_params.qos_info.vo_flag;
			iw_req_priv->schstart.edca_params.qos_info.vi_flag = g_schstart.edca_params.qos_info.vi_flag;
			iw_req_priv->schstart.edca_params.qos_info.bk_flag = g_schstart.edca_params.qos_info.bk_flag;
			iw_req_priv->schstart.edca_params.qos_info.be_flag = g_schstart.edca_params.qos_info.be_flag;
			iw_req_priv->schstart.edca_params.qos_info.q_ack = g_schstart.edca_params.qos_info.q_ack;
			iw_req_priv->schstart.edca_params.qos_info.max_splen = g_schstart.edca_params.qos_info.max_splen;
			iw_req_priv->schstart.edca_params.qos_info.data_ack = g_schstart.edca_params.qos_info.data_ack;
			iw_req_priv->schstart.edca_params.ac_be.aci_aifsn.aifsn = g_schstart.edca_params.ac_be.aci_aifsn.aifsn;
			iw_req_priv->schstart.edca_params.ac_be.aci_aifsn.acm = g_schstart.edca_params.ac_be.aci_aifsn.acm;
			iw_req_priv->schstart.edca_params.ac_be.aci_aifsn.aci = g_schstart.edca_params.ac_be.aci_aifsn.aci;
			iw_req_priv->schstart.edca_params.ac_be.CWmin = g_schstart.edca_params.ac_be.CWmin;
			iw_req_priv->schstart.edca_params.ac_be.CWmax = g_schstart.edca_params.ac_be.CWmax;
			iw_req_priv->schstart.edca_params.ac_be.TXOPLimit = g_schstart.edca_params.ac_be.TXOPLimit;
			iw_req_priv->schstart.edca_params.ac_bk.aci_aifsn.aifsn = g_schstart.edca_params.ac_bk.aci_aifsn.aifsn;
			iw_req_priv->schstart.edca_params.ac_bk.aci_aifsn.acm = g_schstart.edca_params.ac_bk.aci_aifsn.acm;
			iw_req_priv->schstart.edca_params.ac_bk.aci_aifsn.aci = g_schstart.edca_params.ac_bk.aci_aifsn.aci;
			iw_req_priv->schstart.edca_params.ac_bk.CWmin = g_schstart.edca_params.ac_bk.CWmin;
			iw_req_priv->schstart.edca_params.ac_bk.CWmax = g_schstart.edca_params.ac_bk.CWmax;
			iw_req_priv->schstart.edca_params.ac_bk.TXOPLimit = g_schstart.edca_params.ac_bk.TXOPLimit;
			iw_req_priv->schstart.edca_params.ac_vi.aci_aifsn.aifsn = g_schstart.edca_params.ac_vi.aci_aifsn.aifsn;
			iw_req_priv->schstart.edca_params.ac_vi.aci_aifsn.acm = g_schstart.edca_params.ac_vi.aci_aifsn.acm;
			iw_req_priv->schstart.edca_params.ac_vi.aci_aifsn.aci = g_schstart.edca_params.ac_vi.aci_aifsn.aci;
			iw_req_priv->schstart.edca_params.ac_vi.CWmin = g_schstart.edca_params.ac_vi.CWmin;
			iw_req_priv->schstart.edca_params.ac_vi.CWmax = g_schstart.edca_params.ac_vi.CWmax;
			iw_req_priv->schstart.edca_params.ac_vi.TXOPLimit = g_schstart.edca_params.ac_vi.TXOPLimit;
			iw_req_priv->schstart.edca_params.ac_vo.aci_aifsn.aifsn = g_schstart.edca_params.ac_vo.aci_aifsn.aifsn;
			iw_req_priv->schstart.edca_params.ac_vo.aci_aifsn.acm = g_schstart.edca_params.ac_vo.aci_aifsn.acm;
			iw_req_priv->schstart.edca_params.ac_vo.aci_aifsn.aci = g_schstart.edca_params.ac_vo.aci_aifsn.aci;
			iw_req_priv->schstart.edca_params.ac_vo.CWmin = g_schstart.edca_params.ac_vo.CWmin;
			iw_req_priv->schstart.edca_params.ac_vo.CWmax = g_schstart.edca_params.ac_vo.CWmax;
			iw_req_priv->schstart.edca_params.ac_vo.TXOPLimit = g_schstart.edca_params.ac_vo.TXOPLimit;
			iw_req_priv->schstart.immediate_access = g_schstart.immediate_access;
			iw_req_priv->schstart.extended_access = g_schstart.extended_access;
			for(i = 0; i < 3; i++)
				iw_req_priv->schstart.ch_identifier.country_string[i] = g_schstart.ch_identifier.country_string[i];
			for(i = 0; i < IEEE16094_OPERATING_MAX_TRIPLETS_LEN; i++)
				iw_req_priv->schstart.ch_identifier.operating_class.operating_classes[i] = g_schstart.ch_identifier.operating_class.operating_classes[i];
			for(i = 0; i < IEEE16094_MAX_SUPPORTED_RATE; i++)
				iw_req_priv->schstart.rate_set.supported_rate[i] = g_schstart.rate_set.supported_rate[i];

			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->schstart;
			iw_req->u.data.length = sizeof( iw_req_priv->schstart );

			break;

		case NL80211_VENDOR_SUBCMD_SCHEND:
			iw_req_priv->schend.ch_identifier.operating_class.class_len = g_schend.ch_identifier.operating_class.class_len;
			iw_req_priv->schend.ch_identifier.operating_class.current_operating_class = g_schend.ch_identifier.operating_class.current_operating_class;
			iw_req_priv->schend.ch_identifier.channel_num = g_schend.ch_identifier.channel_num;
			for(i = 0; i < 3; i++)
				iw_req_priv->schend.ch_identifier.country_string[i] = g_schend.ch_identifier.country_string[i];
			for(i = 0; i < IEEE16094_OPERATING_MAX_TRIPLETS_LEN; i++)
				iw_req_priv->schend.ch_identifier.operating_class.operating_classes[i] = g_schend.ch_identifier.operating_class.operating_classes[i];

			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->schend;
			iw_req->u.data.length = sizeof( iw_req_priv->schend );

			break;

		case NL80211_VENDOR_SUBCMD_REGISTER:
			iw_req_priv->regtxprofile.ch_identifier.operating_class.class_len = g_regtxprofile.ch_identifier.operating_class.class_len;
			iw_req_priv->regtxprofile.ch_identifier.operating_class.current_operating_class = g_regtxprofile.ch_identifier.operating_class.current_operating_class;
			iw_req_priv->regtxprofile.ch_identifier.channel_num = g_regtxprofile.ch_identifier.channel_num;
			iw_req_priv->regtxprofile.adaptable = g_regtxprofile.adaptable;
			iw_req_priv->regtxprofile.txpwr_level = g_regtxprofile.txpwr_level;
			iw_req_priv->regtxprofile.data_rate = g_regtxprofile.data_rate;
			for(i = 0; i < 3; i++)
				iw_req_priv->regtxprofile.ch_identifier.country_string[i] = g_regtxprofile.ch_identifier.country_string[i];
			for(i = 0; i < IEEE16094_OPERATING_MAX_TRIPLETS_LEN; i++)
				iw_req_priv->regtxprofile.ch_identifier.operating_class.operating_classes[i] = g_regtxprofile.ch_identifier.operating_class.operating_classes[i];

			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->regtxprofile;
			iw_req->u.data.length = sizeof( iw_req_priv->regtxprofile );

			break;

		case NL80211_VENDOR_SUBCMD_DELETE:
			iw_req_priv->deltxprofile.ch_identifier.operating_class.class_len = g_deltxprofile.ch_identifier.operating_class.class_len;
			iw_req_priv->deltxprofile.ch_identifier.operating_class.current_operating_class = g_deltxprofile.ch_identifier.operating_class.current_operating_class;
			iw_req_priv->deltxprofile.ch_identifier.channel_num = g_deltxprofile.ch_identifier.channel_num;
			for(i = 0; i < 3; i++)
				iw_req_priv->deltxprofile.ch_identifier.country_string[i] = g_deltxprofile.ch_identifier.country_string[i];
			for(i = 0; i < IEEE16094_OPERATING_MAX_TRIPLETS_LEN; i++)
				iw_req_priv->deltxprofile.ch_identifier.operating_class.operating_classes[i] = g_deltxprofile.ch_identifier.operating_class.operating_classes[i];

			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->deltxprofile;
			iw_req->u.data.length = sizeof( iw_req_priv->deltxprofile );

			break;

		case NL80211_VENDOR_SUBCMD_CANCELTX:
			iw_req_priv->canceltx.ch_identifier.operating_class.class_len = g_canceltx.ch_identifier.operating_class.class_len;
			iw_req_priv->canceltx.ch_identifier.operating_class.current_operating_class = g_canceltx.ch_identifier.operating_class.current_operating_class;
			iw_req_priv->canceltx.ch_identifier.channel_num = g_canceltx.ch_identifier.channel_num;
			iw_req_priv->canceltx.aci = g_canceltx.aci;
			for(i = 0; i < 3; i++)
				iw_req_priv->canceltx.ch_identifier.country_string[i] = g_canceltx.ch_identifier.country_string[i];
			for(i = 0; i < IEEE16094_OPERATING_MAX_TRIPLETS_LEN; i++)
				iw_req_priv->canceltx.ch_identifier.operating_class.operating_classes[i] = g_canceltx.ch_identifier.operating_class.operating_classes[i];	

			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->canceltx;
			iw_req->u.data.length = sizeof( iw_req_priv->canceltx );

			break;

		case NL80211_VENDOR_SUBCMD_SETUTCTIME:
			for(i = 0; i < IEEE16094_TIME_VALUE_SIZE; i++)
				iw_req_priv->setutctime.value[i] = g_setutctime.value[i];
			for(i = 0; i < IEEE16094_TIME_ERROR_SIZE; i++)
				iw_req_priv->setutctime.error[i] = g_setutctime.error[i];

			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->setutctime;
			iw_req->u.data.length = sizeof( iw_req_priv->setutctime );

			break;

		case NL80211_VENDOR_SUBCMD_SENDPRIMITIVE:
			iw_req_priv->sndprimitive.ch_identifier.operating_class.class_len = g_sndprimitive.ch_identifier.operating_class.class_len;
			iw_req_priv->sndprimitive.ch_identifier.operating_class.current_operating_class = g_sndprimitive.ch_identifier.operating_class.current_operating_class;
			iw_req_priv->sndprimitive.ch_identifier.channel_num = g_sndprimitive.ch_identifier.channel_num;
			iw_req_priv->sndprimitive.channel_type = g_sndprimitive.channel_type;
			iw_req_priv->sndprimitive.primitive_content = g_sndprimitive.primitive_content;
			for(i = 0; i < 3; i++)
				iw_req_priv->sndprimitive.ch_identifier.country_string[i] = g_sndprimitive.ch_identifier.country_string[i];
			for(i = 0; i < IEEE16094_OPERATING_MAX_TRIPLETS_LEN; i++)
				iw_req_priv->sndprimitive.ch_identifier.operating_class.operating_classes[i] = g_sndprimitive.ch_identifier.operating_class.operating_classes[i];

			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->sndprimitive;
			iw_req->u.data.length = sizeof( iw_req_priv->sndprimitive );

			break;

		case NL80211_VENDOR_SUBCMD_GET_CHANNEL_TAB:
			if ((1 == Argc) && (1 == sscanf(Argv[0], "%lu", &value))) {
				iw_req_priv->channelgettable_ch.channel = (unsigned short)value;
				iw_req->u.data.pointer = (caddr_t)&iw_req_priv->channelgettable_ch;
				iw_req->u.data.length = sizeof(iw_req_priv->channelgettable_ch);
			} else {
				/* 引数エラー */
				printf(">>error\n");
			}
			break;

		case NL80211_VENDOR_SUBCMD_SET_CHANNEL_TAB:
			iw_req_priv->channelsettable.ch_number = g_channelsettable.ch_number;
			iw_req_priv->channelsettable.index = g_channelsettable.index;
			iw_req_priv->channelsettable.rate = g_channelsettable.rate;
			iw_req_priv->channelsettable.pw_level = g_channelsettable.pw_level;
			iw_req_priv->channelsettable.power_rate = g_channelsettable.power_rate;
			iw_req_priv->channelsettable.operating_class = g_channelsettable.operating_class;

			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->channelsettable;
			iw_req->u.data.length = sizeof( iw_req_priv->channelsettable );

			break;

		case NL80211_VENDOR_SUBCMD_GET_UTC_STATUS:
			break;

		case NL80211_VENDOR_SUBCMD_GETUTCTIME,:
			break;

		case NL80211_VENDOR_SUBCMD_SET_CTRLMODE:
			if ( ( 1 == Argc ) && ( 1 == sscanf( Argv[ 0 ], "%lu", &value ) ) ) {
				iw_req_priv->control.control = value;
				iw_req->u.data.pointer = (caddr_t)&iw_req_priv->control;
				iw_req->u.data.length = sizeof( iw_req_priv->control );
			} else {
				/* 引数エラー */
				printf (">>\n");
				printf (">> Usage : set_ctrlmode <mode>\n");
				printf (">>   <mode> : 0,1,3\n");
				printf (">>               0 : Idle mode\n");
				printf (">>               1 : OBE  mode\n");
				printf (">>               3 : TP   mode\n");
				printf ("\n");
				status = IOCTL_STATUS_ARG_ERR;
			}
			break;

		case NL80211_VENDOR_SUBCMD_GET_CTRLMODE:
			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->control;
			iw_req->u.data.length = sizeof(iw_req_priv->control);
			break;

		case NL80211_VENDOR_SUBCMD_SET_TX_POWER:
			if ( ( 1 == Argc ) && ( 1 == sscanf( Argv[ 0 ], "%lu", &value ) ) ) {
				iw_req_priv->txpower.txpower = (unsigned short)value;
				iw_req->u.data.pointer = (caddr_t)&iw_req_priv->txpower;
				iw_req->u.data.length = sizeof( iw_req_priv->txpower );
			} else {
				/* 引数エラー */
				printf (">>\n");
				printf (">> Usage : set_txpower <power>\n");
				printf (">>   <power> : 0-127\n");
				printf (">>               0 : -20.0dBm\n");
				printf (">>               N : N/2-20dBm\n");
				printf (">>             127 :  43.5dBm\n");
				printf ("\n");
				status = IOCTL_STATUS_ARG_ERR;
			}
			break;

		case NL80211_VENDOR_SUBCMD_GET_TX_POWER:
			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->txpower;
			iw_req->u.data.length = sizeof( iw_req_priv->txpower );
			break;

		case NL80211_VENDOR_SUBCMD_SET_EDCA:
			for( i = 0; i < IFACPRIO; i++ ) {
				iw_req_priv->edcaparam.CWmin[i] = g_iwreq_edcaparam.CWmin[i];
				iw_req_priv->edcaparam.CWmax[i] = g_iwreq_edcaparam.CWmax[i];
				iw_req_priv->edcaparam.AIFSN[i] = g_iwreq_edcaparam.AIFSN[i];
				iw_req_priv->edcaparam.TXOPLimit[i] = g_iwreq_edcaparam.TXOPLimit[i];
			}
			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->edcaparam;
			iw_req->u.data.length = sizeof( iw_req_priv->edcaparam );
			break;

		case NL80211_VENDOR_SUBCMD_GET_EDCA:
			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->edcaparam;
			iw_req->u.data.length = sizeof( iw_req_priv->edcaparam );
			break;

		case NL80211_VENDOR_SUBCMD_GET_SOFT_VERSION:
			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->softver;
			iw_req->u.data.length = sizeof( iw_req_priv->softver );
			break;

		case NL80211_VENDOR_SUBCMD_SET_PHY_INFO:
			iw_req->u.data.pointer = NULL;
			iw_req->u.data.length = 0;
			break;
			
		case NL80211_VENDOR_SUBCMD_GET_PHY_INFO:
			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->phyinfo;
			iw_req->u.data.length = sizeof( iw_req_priv->phyinfo );
			break;

		case NL80211_VENDOR_SUBCMD_SET_MAC_ADDRESS:
			if ( ( 1 == Argc ) && ( 6 == sscanf( Argv[ 0 ], "%02x:%02x:%02x:%02x:%02x:%02x", 
				&mac_addr_uint[ 0 ], 
				&mac_addr_uint[ 1 ], 
				&mac_addr_uint[ 2 ], 
				&mac_addr_uint[ 3 ], 
				&mac_addr_uint[ 4 ], 
				&mac_addr_uint[ 5 ] ) ) ) {
				iw_req_priv->mac_addr.mac_addr[ 0 ] = ( unsigned char )mac_addr_uint[ 0 ];
				iw_req_priv->mac_addr.mac_addr[ 1 ] = ( unsigned char )mac_addr_uint[ 1 ];
				iw_req_priv->mac_addr.mac_addr[ 2 ] = ( unsigned char )mac_addr_uint[ 2 ];
				iw_req_priv->mac_addr.mac_addr[ 3 ] = ( unsigned char )mac_addr_uint[ 3 ];
				iw_req_priv->mac_addr.mac_addr[ 4 ] = ( unsigned char )mac_addr_uint[ 4 ];
				iw_req_priv->mac_addr.mac_addr[ 5 ] = ( unsigned char )mac_addr_uint[ 5 ];
				iw_req->u.data.pointer = (caddr_t)&iw_req_priv->mac_addr;
				iw_req->u.data.length = sizeof( iw_req_priv->mac_addr );
#ifdef WAVE_DUAL_PHY
				memcpy( &g_Src_Info.Mac_Address_temp, &iw_req_priv->mac_addr.mac_addr[0], ETH_ALEN );
#endif /* WAVE_DUAL_PHY */
			} else {
				/* 引数エラー */
				printf (">>\n");
				printf (">> Usage: set_macaddr <MAC addr>\n");
				printf (">>   <MAC addr> : xx:xx:xx:xx:xx:xx\n");
				printf ("\n");
				status = IOCTL_STATUS_ARG_ERR;
			}
			break;
			
		case NL80211_VENDOR_SUBCMD_GET_MAC_ADDRESS:
			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->mac_addr;
			iw_req->u.data.length = sizeof( iw_req_priv->mac_addr );
			break;
			
		case NL80211_VENDOR_SUBCMD_SET_CH:
			if ( ( 1 == Argc ) && ( 1 == sscanf( Argv[ 0 ], "%lu", &value ) ) ) {
				iw_req_priv->ch.channel = (unsigned short)value;
				iw_req->u.data.pointer = (caddr_t)&iw_req_priv->ch;
				iw_req->u.data.length = sizeof( iw_req_priv->ch );
			} else {
				/* 引数エラー */
				printf (">>\n");
				printf (">> Usage: set_ch <Channel_no>\n");
				printf (">>   <Channel_no> : 172...180\n");
				printf (">>                  172,174,176,178,180\n");
				printf ("\n");
				status = IOCTL_STATUS_ARG_ERR;
			}
			break;
			
		case NL80211_VENDOR_SUBCMD_GET_CH:
			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->ch;
			iw_req->u.data.length = sizeof( iw_req_priv->ch );
			break;
			
		case NL80211_VENDOR_SUBCMD_SET_TX_RATES:
			if ( ( 1 == Argc ) && ( 1 == sscanf( Argv[ 0 ], "%lu", &value ) ) ) {
				iw_req_priv->bitrate.bitrate = value;
				iw_req->u.data.pointer = (caddr_t)&iw_req_priv->bitrate;
				iw_req->u.data.length = sizeof( iw_req_priv->bitrate );
			} else {
				/* 引数エラー */
				printf (">>\n");
				printf (">> Usage: set_txrate <TXrate>\n");
				printf (">>   <TXrate> : 300000...2700000\n");
				printf (">>              3000000,  450000,  600000,  900000,\n");
				printf (">>             12000000, 1800000, 2400000, 2700000\n");
				printf ("\n");
				status = IOCTL_STATUS_ARG_ERR;
			}
			break;
			
		case NL80211_VENDOR_SUBCMD_GET_TX_RATES:
			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->bitrate;
			iw_req->u.data.length = sizeof( iw_req_priv->bitrate );
			break;
			
		case NL80211_VENDOR_SUBCMD_SET_RTS:
			if ( ( 1 == Argc ) && ( 1 == sscanf( Argv[ 0 ], "%lu", &value ) ) ) {
				iw_req_priv->rts.rts_threshold = (unsigned short)value;
				iw_req->u.data.pointer = (caddr_t)&iw_req_priv->rts;
				iw_req->u.data.length = sizeof( iw_req_priv->rts );
			} else {
				/* 引数エラー */
				printf (">>\n");
				printf (">> Usage : set_rts <data size>\n");
				printf (">>   <data size> : 0-2347\n");
				printf ("\n");
				status = IOCTL_STATUS_ARG_ERR;
			}
			break;
			
		case NL80211_VENDOR_SUBCMD_GET_RTS:
			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->rts;
			iw_req->u.data.length = sizeof( iw_req_priv->rts );
			break;
			
		case NL80211_VENDOR_SUBCMD_SET_TPTX:
			memset( &iw_req_priv->tptx, 0, sizeof( iw_req_priv->tptx ) );
			for ( i = 0; i < Argc; i++ ) {
				buff_p = ( CHAR_T * )&iw_req_priv->tptx.tp_tx[ strlen( ( CHAR_T * )iw_req_priv->tptx.tp_tx ) ];
				if ( 0 != i ) {
					*buff_p++ = SP_KEY;
				}
				strcpy( buff_p, Argv[ i ] );
			}
			iw_req->u.data.length = (USHORT)strlen( ( CHAR_T * )iw_req_priv->tptx.tp_tx );
			/* 終端にNULL(0x00)、CR(0x0d)、LF(0x0a)を付加する */
			iw_req_priv->tptx.tp_tx[ iw_req->u.data.length + 1 ] = CR_KEY;
			iw_req_priv->tptx.tp_tx[ iw_req->u.data.length + 2 ] = LF_KEY;
			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->tptx;
			iw_req->u.data.length += 3;	/* 文字列＋NULL(0x00)+CR(0x0d)+LF(0x0a) */
			break;
			
		case NL80211_VENDOR_SUBCMD_SET_DBGTX:
			memset( &iw_req_priv->dbgtx, 0, sizeof( iw_req_priv->dbgtx ) );
			for ( i = 0; i < Argc; i++ ) {
				buff_p = ( CHAR_T * )&iw_req_priv->dbgtx.dbg_tx[ strlen( ( CHAR_T * )iw_req_priv->dbgtx.dbg_tx ) ];
				if ( 0 != i ) {
					*buff_p++ = SP_KEY;
				}
				strcpy( buff_p, Argv[ i ] );
			}
			iw_req->u.data.length = (USHORT)strlen( ( CHAR_T * )iw_req_priv->dbgtx.dbg_tx );
			/* 終端にNULL(0x00)、CR(0x0d)、LF(0x0a)を付加する */
			iw_req_priv->dbgtx.dbg_tx[ iw_req->u.data.length + 1 ] = CR_KEY;
			iw_req_priv->dbgtx.dbg_tx[ iw_req->u.data.length + 2 ] = LF_KEY;
			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->dbgtx;
			iw_req->u.data.length += 3;	/* 文字列＋NULL(0x00)+CR(0x0d)+LF(0x0a) */
			break;
			
		case NL80211_VENDOR_SUBCMD_SET_FRAGMENT:
			iw_req_priv->frag.frag_threshold = g_iwreq_frag.frag_threshold;
			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->frag;
			iw_req->u.data.length = sizeof( iw_req_priv->frag );
			break;

		case NL80211_VENDOR_SUBCMD_GET_FRAGMENT:
			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->frag;
			iw_req->u.data.length = sizeof( iw_req_priv->frag );
			break;

		case NL80211_VENDOR_SUBCMD_SET_CTRLEVENT:
			iw_req_priv->ctrl_event.control_event = 0;

			if(g_ctrl_event_kind.ind_diag == 1){
				iw_req_priv->ctrl_event.control_event |= CONTROL_EVENT_DIAG;
			}
			if(g_ctrl_event_kind.ind_temp == 1){
				iw_req_priv->ctrl_event.control_event |= CONTROL_EVENT_TEMP;
			}
			if(g_ctrl_event_kind.ind_tprx == 1){
				iw_req_priv->ctrl_event.control_event |= CONTROL_EVENT_TPRX;
			}
			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->ctrl_event;
			iw_req->u.data.length = sizeof( iw_req_priv->ctrl_event );
			break;

		case NL80211_VENDOR_SUBCMD_GET_CTRLEVENT:
			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->ctrl_event;
			iw_req->u.data.length = sizeof( iw_req_priv->ctrl_event );
			break;

		case NL80211_VENDOR_SUBCMD_IFNAME:
			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->ifrn_name;
			iw_req->u.data.length = sizeof(iw_req_priv->ifrn_name);
			break;

		case NL80211_VENDOR_SUBCMD_SET_MAC_ADDRESS_EX:
			if ( ( 1 == Argc ) && ( 1 == sscanf( Argv[ 0 ], "%lu", &value ) ) &&
					( (value >= SETMACADDREX_TYPE_CCH) && (value <= SETMACADDREX_TYPE_BOTH) ) ) {
				g_iwreq_set_mac_ex_parame.type = value;
				iw_req_priv->set_mac_ex.type = value;
				memcpy(iw_req_priv->set_mac_ex.cch_mac_addr, g_iwreq_set_mac_ex_parame.cch_mac_addr, IFMACSIZ);
				memcpy(iw_req_priv->set_mac_ex.sch_mac_addr, g_iwreq_set_mac_ex_parame.sch_mac_addr, IFMACSIZ);
				iw_req->u.data.pointer = (caddr_t)&iw_req_priv->set_mac_ex;
				iw_req->u.data.length = sizeof( IWREQ_SET_MAC_ADDR_EX );
			} else {
				/* 引数エラー */
				printf (">>\n");
				printf (">> Usage : set_macaddr_ex <type>\n");
				printf (">>   <type> : 1,2,3\n");
				printf (">>               1 : CCH ONLY\n");
				printf (">>               2 : SCH ONLY\n");
				printf (">>               3 : BOTH\n");
				printf ("\n");
				status = IOCTL_STATUS_ARG_ERR;
			}
			break;

		case NL80211_VENDOR_SUBCMD_GET_MAC_ADDRESS_EX:
			break;

		case NL80211_VENDOR_SUBCMD_SET_SHORT_RETRY:
			if ( ( 1 == Argc ) && ( 1 == sscanf( Argv[ 0 ], "%lu", &value ) ) ) {
				iw_req_priv->sretry.short_retry_limit = value;
				iw_req->u.data.pointer = (caddr_t)&iw_req_priv->sretry;
				iw_req->u.data.length = sizeof( iw_req_priv->sretry );
			} else {
				/* 引数エラー */
				printf (">>\n");
				printf (">> Usage : set_short_retry <count>\n");
				printf (">>   <count> : 0-255\n");
				printf ("\n");
				status = IOCTL_STATUS_ARG_ERR;
			}
			break;
					
		case NL80211_VENDOR_SUBCMD_GET_SHORT_RETRY:
			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->sretry;
			iw_req->u.data.length = sizeof( iw_req_priv->sretry );
			break;
			
		case NL80211_VENDOR_SUBCMD_SET_LONG_RETRY:
			if ( ( 1 == Argc ) && ( 1 == sscanf( Argv[ 0 ], "%lu", &value ) ) ) {
				iw_req_priv->lretry.long_retry_limit = value;
				iw_req->u.data.pointer = (caddr_t)&iw_req_priv->lretry;
				iw_req->u.data.length = sizeof( iw_req_priv->lretry );
			} else {
				/* 引数エラー */
				printf (">>\n");
				printf (">> Usage : set_long_retry <count>\n");
				printf (">>   <count> : 0-255\n");
				printf ("\n");
				status = IOCTL_STATUS_ARG_ERR;
			}
			break;
			
		case NL80211_VENDOR_SUBCMD_GET_LONG_RETRY:
			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->lretry;
			iw_req->u.data.length = sizeof( iw_req_priv->lretry );
			break;
			
		case NL80211_VENDOR_SUBCMD_SET_RETRY_RATE:
			if ( ( 1 == Argc ) && ( 1 == sscanf( Argv[ 0 ], "%lu", &value ) ) ) {
				iw_req_priv->retryrate.retry_rate_mode = value;
				iw_req->u.data.pointer = (caddr_t)&iw_req_priv->retryrate;
				iw_req->u.data.length = sizeof( iw_req_priv->retryrate );
			} else {
				/* 引数エラー */
				printf (">>\n");
				printf (">> Usage : set_retry_rate <sw>\n");
				printf (">>   <sw> : 0,1\n");
				printf (">>          0 : OFF\n");
				printf (">>          1 : ON\n");
				printf ("\n");
				status = IOCTL_STATUS_ARG_ERR;
			}
			break;
			
		case NL80211_VENDOR_SUBCMD_GET_RETRY_RATE:
			iw_req->u.data.pointer = (caddr_t)&iw_req_priv->retryrate;
			iw_req->u.data.length = sizeof( iw_req_priv->retryrate );
			break;

		default:
			/* リクエスト範囲外 */
			status = IOCTL_STATUS_INVALID_REQUEST;
	}
	return status;
}

/******************************************************************************
* Outline      : Io_Ctrl_Wlan_Private
* Description  : Acquires private WLAN information
* Return Value : IOCTL_STATUS Status
******************************************************************************/
IOCTL_STATUS Io_Ctrl_Wlan_Private( SINT Snd_Socket, SINT Rcv_Socket, enum wave_sub_command Request, SINT Argc, CHAR_T **Argv )
{   
    static struct iwreq			iw_req;
	static union iwreq_priv_u	iw_req_priv;
	IOCTL_STATUS	status = IOCTL_STATUS_OK;
	struct nl_msg *msg;
	int err = -ENOMEM;
	int ret = 0;
	event_flag = TRUE;
	int (*valid_handler)(struct nl_msg *, void *);
	valid_handler = NULL;
	unsigned int sub_command = 0;
    struct wave_driver_data *drv;
   
    Set_If_Name( &iw_req );
	/* Requestのチェックとiwreqのinパラメータ設定 */
	status = Set_Iw_Req_In_Param(Request, &iw_req, &iw_req_priv, Argc, Argv);
	if ( IOCTL_STATUS_OK != status ) {
		printf("%s Set_Iw_Req_In_Param failed!\n", __func__);
		return status;
	}
	
	printf("netlink request message:%X\n", Request);
	switch(Request) {
		default:
			status = IOCTL_STATUS_INVALID_REQUEST;
			goto out;
	}
	if(ret < 0){
		status = IOCTL_STATUS_INVALID_REQUEST;
		goto out;
	}
	
	strlcpy(ifr.ifr_name, ETH_IF_NAME, IFNAMSIZ);
	drv = malloc(sizeof(*drv));
	strlcpy(drv->ifname, g_If_Name_Buff, sizeof(drv->ifname));
	drv->ioctl_sock = Snd_Socket;

#ifdef WAVE_DUAL_PHY
	Debug_Print("\nCOMMAND SEND:WAVE%d-%d\n\n",g_wlan_chg_flag,Request);
	if(g_wlan_chg_flag == 0){
		drv->ifindex = g_Src_Info.If_Index_dual[0];
	}
	else{
		drv->ifindex = g_Src_Info.If_Index_dual[1];
	}
#else
	drv->ifindex = g_Src_Info.If_Index;
	//Debug_Print("index=%d,err = %d,%s\n",drv->ifindex,errno,ETH_IF_NAME);
#endif /* WAVE_DUAL_PHY */
	
	ifd = malloc(sizeof(*ifd));
	strlcpy(ifd->ifd_name, ETH_IF_NAME, IFNAMSIZ);
	drv->wl = (void *) ifd;
	
    drv->event_sock = Rcv_Socket;
	Debug_Print("event_sock=%d\n",drv->event_sock);
	
	err = wave_ioctl(drv, Request, iw_req.u.data.pointer, iw_req.u.data.length,1);
	if (err < 0) {
		printf("wave_ioctl err!\n");
		status = IOCTL_STATUS_INVALID_REQUEST;
		if(CAP_MODE != g_Run_Mode && CAP_FTP_MODE != g_Run_Mode) {
			usleep(100);
		}
	}
	else
	{
		Debug_Print("wave_ioctl ok!\n");
	}

	err = send_and_recv(nlstate.nl_sock, msg, valid_handler, NULL);
	if (err < 0){
		goto out;
	}
	
	while(event_flag) {
		usleep(100);
	}
	
    free(drv);
	free(buff);
	return status;

out:
	nlmsg_free(msg);
	free(buff);
	return status;	
}

static int error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err,
			 void *arg)
{
	int *ret = arg;
	*ret = err->error;

	return NL_STOP;
}

static int finish_handler(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	*ret = 0;
	return NL_SKIP;
}

static int ack_handler(struct nl_msg *msg, void *arg)
{
	int *ret = arg;
	*ret = 0;
	return NL_STOP;
}

/******************************************************************************
* Outline      : set_SETCH_REQ
* Return Value : none
******************************************************************************/
static void set_SETCH_REQ( unsigned short value )
{
	IOCTL_STATUS	status;

	priv_ch.channel = value;

	status = Io_Ctrl_Wlan_Private( g_Socket_Fd[ SOCKET_RAW ], NL80211_VENDOR_SUBCMD_SET_CH , 0, NULL);
	
	switch( status ) {
		case IOCTL_STATUS_OK:				/* completed */
			puts("\n>>NL80211 Done");
			break;
		case IOCTL_STATUS_ARG_ERR:			/* found a error in a argument */
			puts("\n>>NL80211 Error -- Illeagal Argument");
			break;
		case IOCTL_STATUS_INVALID_REQUEST:	/* illeagal request */
			printf("\n>>NL80211 Error -- Invalid Request: %4X\n", NL80211_VENDOR_SUBCMD_SET_CH);
			break;
		case IOCTL_STATUS_INVALID_PARAM:	/* found a error in a parameter */
			printf("\n>>NL80211 Error -- Invalid Parameter\n");
			break;
		default:
			break;
	}

}

/******************************************************************************
* Outline      : set_MACADDR_EX_REQ
* Return Value : none
******************************************************************************/
static void set_MACADDR_EX_REQ(void)
{
	IOCTL_STATUS	status;

	/*set_channelsettable*/
	priv_set_mac_ex.type = SETMACADDREX_TYPE_BOTH;
	priv_set_mac_ex.cch_mac_addr[0] = 0x12;
	priv_set_mac_ex.cch_mac_addr[1] = 0x22;
	priv_set_mac_ex.cch_mac_addr[2] = 0x33;
	priv_set_mac_ex.cch_mac_addr[3] = 0x44;
	priv_set_mac_ex.cch_mac_addr[4] = 0x55;
	priv_set_mac_ex.cch_mac_addr[5] = 0x66;

	priv_set_mac_ex.sch_mac_addr[0] = 0x12;
	priv_set_mac_ex.sch_mac_addr[1] = 0x77;
	priv_set_mac_ex.sch_mac_addr[2] = 0x88;
	priv_set_mac_ex.sch_mac_addr[3] = 0x99;
	priv_set_mac_ex.sch_mac_addr[4] = 0xAA;
	priv_set_mac_ex.sch_mac_addr[5] = 0xBB;

	status = Io_Ctrl_Wlan_Private( g_Socket_Fd[ SOCKET_RAW ], NL80211_VENDOR_SUBCMD_SET_MAC_ADDRESS_EX , 0, NULL);
	
	switch( status ) {
		case IOCTL_STATUS_OK:				/* completed */
			puts("\n>>NL80211 Done");
			break;
		case IOCTL_STATUS_ARG_ERR:			/* found a error in a argument */
			puts("\n>>NL80211 Error -- Illeagal Argument");
			break;
		case IOCTL_STATUS_INVALID_REQUEST:	/* illeagal request */
			printf("\n>>NL80211 Error -- Invalid Request: %4X\n", NL80211_VENDOR_SUBCMD_SET_MAC_ADDRESS_EX);
			break;
		case IOCTL_STATUS_INVALID_PARAM:	/* found a error in a parameter */
			printf("\n>>NL80211 Error -- Invalid Parameter\n");
			break;
		default:
			break;
	}

}

/******************************************************************************
* Outline		: Debug_Print
* Description  : Output debug code 
* Argument	  : *Format_p -- Address to debugging code character string
* Return Value : none
******************************************************************************/
#undef Debug_Print
void Debug_Print(CHAR_T *Format_p, ... )
{
	va_list		args;

	va_start( args, Format_p );
	vprintf( Format_p, args );
	va_end( args );
}
