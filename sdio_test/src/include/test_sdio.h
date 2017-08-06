typedef struct wave_ioctl {
	uint cmd; /* common ioctl definition */
	void *buf; /* pointer to user buffer */
	uint len; /* length of user buffer */
	bool set; /* get or set request (optional) */
	uint used; /* bytes read or written (optional) */
	uint needed; /* bytes needed (optional) */
	uint driver; /* to identify target driver */
	msg_frag_t fragment; /* the fragment struct */
} wave_ioctl_t;

struct wave_driver_data {
	void *ctx;			// context
	int ioctl_sock;
	int event_sock;
	char ifname[IFNAMSIZ + 1];
	void * wl;
	unsigned int ifindex; /* interface index */
//	int		flags;			// delete ?
};


/* 
 * Socket関係
 */

#ifdef NL80211_WAVE
#ifdef WAVE_DUAL_PHY
#define ETH_IF_NAME	(g_wlan_chg_flag == 0)?"wlan0":"wlan1"
#else
#define ETH_IF_NAME						"wlan0"
#endif /* WAVE_DUAL_PHY */
#else
#ifdef WAVE_DUAL_PHY
#define ETH_IF_NAME	(g_wlan_chg_flag == 0)?"wave0":"wave1"
#else
#define ETH_IF_NAME						"wave0"
#endif /* WAVE_DUAL_PHY */
//#define ETH_IF_NAME						"eth1"
#endif

#define	IFNAMSIZ						16

/* ko biết qnx có hay không
struct ifdrv {
    char ifd_name[16];
    unsigned long ifd_cmd;
    size_t ifd_len;
    void *ifd_data;
};
*/


