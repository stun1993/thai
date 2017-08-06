/*
 ***************************************************************************
 *
 * Copyright (C) 2015 Renesas Electronics Corp. All rights reserved.
 *
 ***************************************************************************
*/

#ifndef _KS_IEEE16094_API_H_
#define _KS_IEEE16094_API_H_

//#include <linux/netdevice.h>
//#include <stdbool.h>

/* Macro definition */
    
#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif
    
#define IEEE16094_EXT_CAPABILITY_SIZE   15      /* external capability we'll handle */
#define IEEE16094_TIME_VALUE_SIZE   10      /* Time Value */
#define IEEE16094_TIME_ERROR_SIZE   5      /* Time Error */
#define IEEE16094_VENDOR_SPECIFIC_SIZE   1962   /* max vendor specific size we'll handle */   
#define IEEE16094_COUNTRY_MAX_TRIPLETS_LEN 83 /* max country TRIPLET define channel, number of channels and Max tx Power */
#ifdef VSA_OID_5_LEN
#define OID_LEN 5
#else
#define OID_LEN 3
#endif /* VSA_OID_5_LEN */

#define IEEE16094_OPERATING_MAX_TRIPLETS_LEN 253
#define IEEE16094_MAX_SUPPORTED_RATE 	8

#define IFTPTXSIZ 41
#define IFTPRXSIZ 2300
#define IFMACSIZ  6

#define WIPHYNAME_PHY0 0
#define WIPHYNAME_PHY1 1

#define SETMACADDREX_RESULT_SUCCESS        0
#define SETMACADDREX_RESULT_INVALID        1
#define SETMACADDREX_RESULT_IN_PROCESSING  4

#define SETMACADDREX_TYPE_CCH   1
#define SETMACADDREX_TYPE_SCH   2
#define SETMACADDREX_TYPE_BOTH  3

/* Data Type  */
typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

//typedef unsigned int bool;


// Sub command definitions
typedef enum wave_sub_command {
	NL80211_VENDOR_SUBCMD_VSA = 1,
	NL80211_VENDOR_SUBCMD_VSAEND,
	NL80211_VENDOR_SUBCMD_SCHSTART,
	NL80211_VENDOR_SUBCMD_SCHEND,
	NL80211_VENDOR_SUBCMD_REGISTER,/*5*/
	NL80211_VENDOR_SUBCMD_DELETE,
	NL80211_VENDOR_SUBCMD_CANCELTX,
	NL80211_VENDOR_SUBCMD_SETUTCTIME,
	NL80211_VENDOR_SUBCMD_SENDPRIMITIVE,
	NL80211_VENDOR_SUBCMD_GET_CHANNEL_TAB,/*10*/
	NL80211_VENDOR_SUBCMD_SET_CHANNEL_TAB,
	NL80211_VENDOR_SUBCMD_GET_UTC_STATUS,

	NL80211_VENDOR_SUBCMD_TA_IND,
	NL80211_VENDOR_SUBCMD_VSA_IND,
	NL80211_VENDOR_SUBCMD_SCHEND_IND,/*15*/
	NL80211_VENDOR_SUBCMD_UTCSYNC_IND,

	NL80211_VENDOR_SUBCMD_SET_CTRLMODE,
	NL80211_VENDOR_SUBCMD_GET_CTRLMODE,
	NL80211_VENDOR_SUBCMD_SET_TX_POWER,
	NL80211_VENDOR_SUBCMD_GET_TX_POWER,/*20*/
	NL80211_VENDOR_SUBCMD_SET_EDCA,
	NL80211_VENDOR_SUBCMD_GET_EDCA,
	NL80211_VENDOR_SUBCMD_GET_SOFT_VERSION,
	NL80211_VENDOR_SUBCMD_SET_PHY_INFO,
	NL80211_VENDOR_SUBCMD_GET_PHY_INFO,/*25*/
	NL80211_VENDOR_SUBCMD_SET_MAC_ADDRESS,
	NL80211_VENDOR_SUBCMD_GET_MAC_ADDRESS,
	NL80211_VENDOR_SUBCMD_SET_CH,
	NL80211_VENDOR_SUBCMD_GET_CH,
	NL80211_VENDOR_SUBCMD_SET_TX_RATES,/*30*/
	NL80211_VENDOR_SUBCMD_GET_TX_RATES,
	NL80211_VENDOR_SUBCMD_IFNAME,
	NL80211_VENDOR_SUBCMD_SET_RTS,
	NL80211_VENDOR_SUBCMD_GET_RTS,
	NL80211_VENDOR_SUBCMD_SET_TPTX,/*35*/
	NL80211_VENDOR_SUBCMD_IND_TPRX,
	NL80211_VENDOR_SUBCMD_IND_DIAG,
	NL80211_VENDOR_SUBCMD_IND_TEMP,
	NL80211_VENDOR_SUBCMD_SET_FRAGMENT,
	NL80211_VENDOR_SUBCMD_GET_FRAGMENT,/*40*/

	NL80211_VENDOR_SUBCMD_GETUTCTIME,

	NL80211_VENDOR_SUBCMD_SET_CTRLEVENT,
	NL80211_VENDOR_SUBCMD_GET_CTRLEVENT,
	
	NL80211_VENDOR_SUBCMD_SET_MAC_ADDRESS_EX,
	NL80211_VENDOR_SUBCMD_GET_MAC_ADDRESS_EX,
	NL80211_VENDOR_SUBCMD_SET_SHORT_RETRY,
	NL80211_VENDOR_SUBCMD_GET_SHORT_RETRY,
	NL80211_VENDOR_SUBCMD_SET_LONG_RETRY,
	NL80211_VENDOR_SUBCMD_GET_LONG_RETRY,
	NL80211_VENDOR_SUBCMD_SET_RETRY_RATE,
	NL80211_VENDOR_SUBCMD_GET_RETRY_RATE,

	/* add sub command definition here */


	NL80211_VENDOR_SUBCMD_SET_DBGTX,
	NL80211_VENDOR_SUBCMD_IND_DBGRX,

	__NL80211_VENDOR_SUBCMD_AFTER_LAST,
	NL80211_VENDOR_SUBCMD_MAX

} wave_sub_command_e;

// Time Advertisement
typedef struct ieee_ta_info 
{
	u8 							ta_info_len;
	u8 							capability;
	u8 							value[IEEE16094_TIME_VALUE_SIZE];
	u8 							error[IEEE16094_TIME_ERROR_SIZE];
	u8							update;
} __attribute__((packed)) ieee_ta_info_t;

// Extended Capabilities
typedef struct ieee_extended_capability
{
	u8 							ext_cap_len;
	u8 							ext_capability[IEEE16094_EXT_CAPABILITY_SIZE];
} __attribute__((packed)) ieee_ext_capability_t;

// Vendor Specific Information

typedef struct ieee_vendor_specific
{
	u16							ven_len;	/* Vendor specific content length	 */
	u8 							ven_oid[OID_LEN]; /* Organization Identifier */
	u8 							ven_specific[IEEE16094_VENDOR_SPECIFIC_SIZE];
} __attribute__((packed)) ieee_vendor_specific_t;

typedef enum ieee_channel_interval
{
	IEEE16094_CCH_INTERVAL = 0,
	IEEE16094_SCH_INTERVAL,
	IEEE16094_BOTH_INTERVAL
} ieee_channel_interval_e;

typedef struct ieee_operating_class
{
	u8 							class_len;
	u8 							current_operating_class;
	u8 							operating_classes[IEEE16094_OPERATING_MAX_TRIPLETS_LEN];
} __attribute__((packed)) ieee_operating_class_t;

typedef struct ieee_channel_identifier
{
	u8 							country_string[3]; /* Country String */ 
	ieee_operating_class_t 		operating_class;  /* operating class */
	u8 							channel_num;  /* Channel Number */
} __attribute__((packed)) ieee_channel_identifier_t;

typedef struct ieee_country_string
{
    u8 							country_len;
    u8 							country_str[3];
    u8 							country_triplet[IEEE16094_COUNTRY_MAX_TRIPLETS_LEN*3];
} __attribute__((packed)) ieee_country_string_t;

typedef struct ieee_ta_content 
{
	u16 						capability;
	u8 							power_constraint;
	ieee_country_string_t 		string;
	ieee_ta_info_t 				time_adv_info;	
	ieee_ext_capability_t 		ext_capabilities;
	ieee_vendor_specific_t 		vendor_spec;
} __attribute__((packed)) ieee_ta_content_t;

//Supported Rates element
typedef struct ieee_oper_rate_set
{
//	u8 							length;
	u8 							supported_rate[IEEE16094_MAX_SUPPORTED_RATE]; /* 1-8 */
} __attribute__((packed)) ieee_oper_rate_set_t;

// EDCA Parameter Set
typedef struct ieee_aci_aifsn
{
	u8 							aifsn:4; 	/* AIFSN: 2~15 */
	u8 							acm:1; 		/* ACM: 0,1 */
	u8 							aci:2;		/* ACI: 0~3 */
	u8 							reserved:1;	/* Reserved */
} __attribute__((packed)) ieee_aci_aifsn_t;

// AC_BE, AC_BK, AC_VI, and AC_VO Parameter Record fields
typedef struct ieee_edca_record
{
	ieee_aci_aifsn_t 			aci_aifsn;
	u16 						CWmin;
	u16 						CWmax;
	u8 							TXOPLimit;
} __attribute__((packed)) ieee_edca_record_t;

// QoS Info
typedef struct ieee_qos_info
{
	u8 							vo_flag:1; 	/* AC_VO U-APSD Flag */
	u8 							vi_flag:1; 	/* AC_VI U-APSD Flag */
	u8 							bk_flag:1; 	/* AC_BK U-APSD Flag */
	u8 							be_flag:1; 	/* AC_BE U-APSD Flag */
	u8 							q_ack:1;		/* Q-Ack */
	u8 							max_splen:2; /* Max SP Length */
	u8 							data_ack:1;	/* More Data Ack */
} __attribute__((packed)) ieee_qos_info_t;

// EDCA Parameter Set
typedef struct ieee_edca_parameter
{
	ieee_qos_info_t				qos_info;
	u8 							reserved;
	ieee_edca_record_t			ac_be;	/* AC_BE Parameter Record Best effort */
	ieee_edca_record_t			ac_bk;	/* AC_BK Parameter Record Background */
	ieee_edca_record_t			ac_vi;	/* AC_VI Parameter Record Video */
	ieee_edca_record_t			ac_vo;	/* AC_VO Parameter Record Voice */
} __attribute__((packed)) ieee_edca_parameter_t;

typedef struct ieee_mlmex_vsa_req
{
	u8 							dst_mac[ETH_ALEN];
	u8 							mgmt_id;
	ieee_vendor_specific_t 		vendor_spec;	
	u8 							repeat;	
	ieee_channel_identifier_t 	ch_identifier;
	ieee_channel_interval_e 	channel_type; /* Only support CCH only */
} __attribute__((packed)) ieee_mlmex_vsa_req_t;

typedef struct ieee_mlmex_schstart_req
{
	ieee_channel_identifier_t 	ch_identifier;
	ieee_oper_rate_set_t 		rate_set;  
	
	ieee_edca_parameter_t 		edca_params;
	bool 						immediate_access;
	u8							extended_access;
} __attribute__((packed)) ieee_mlmex_schstart_req_t;

typedef struct ieee_mlmex_schend_req
{
	ieee_channel_identifier_t 	ch_identifier;
} __attribute__((packed)) ieee_mlmex_schend_req_t;

typedef struct ieee_mlmex_regtxprofile_req
{
	ieee_channel_identifier_t 	ch_identifier;
	bool 						adaptable;
	u8 							txpwr_level; /* 1-8 */
	u8 							data_rate; /* 1-8 */
} __attribute__((packed)) ieee_mlmex_regtxprofile_req_t;

typedef struct ieee_mlmex_deltxprofile_req
{
	ieee_channel_identifier_t 	ch_identifier;
} __attribute__((packed)) ieee_mlmex_deltxprofile_req_t;

typedef struct ieee_mlmex_canceltx_req
{
	ieee_channel_identifier_t 	ch_identifier;
	u8 							aci;
} __attribute__((packed)) ieee_mlmex_canceltx_req_t;

typedef struct ieee_mlmex_setutctime_req
{
	u8 							value[IEEE16094_TIME_VALUE_SIZE];
	u8 							error[IEEE16094_TIME_ERROR_SIZE];
} __attribute__((packed)) ieee_mlmex_setutctime_req_t;

typedef struct ieee_mlmex_sndprimitive_req
{
	ieee_channel_identifier_t 	ch_identifier;
	ieee_channel_interval_e 	channel_type;
	u8 							primitive_content;
} __attribute__((packed)) ieee_mlmex_sndprimitive_req_t;


// MIB ChannelSetTable
typedef struct ieee_mib_channelsettable
{
	u8					ch_number;			/* Channel Number INTEGER (0..200) */
	u8					index;				/* Index INTEGER (1..32) */
	u8					rate;				/* DataRate INTEGER (2..127) */
	u8					pw_level;			/* PowerLevel INTEGER (1..8) */
	bool				power_rate;			/* AdaptablePowerAndRate TruthValue */
	u32					operating_class;	/* Operating Class INTEGER */
} __attribute__((packed)) ieee_mib_channelsettable_t;

/* Confirmation Data Structure */
typedef enum ieee_result_code
{	
	// VSA confirm
	IEEE_VSA_SUCCESS = 0, // 0
	IEEE_VSA_NO_SYNC, // 1
	IEEE_VSA_INVALID_PARAMETERS, // 2
	IEEE_VSA_UNSPECIFIED_FAILURE, // 3
	
	// VSAEND confirm
	IEEE_VSAEND_SUCCESS, // 4
	IEEE_VSAEND_UNSPECIFIED_FAILURE, // 5
	
	// SCHSTART confirm
	IEEE_SCHSTART_SUCCESS, // 6
	IEEE_SCHSTART_SUCCESS_NO_EXT, /*PARTIAL SUCCESS No Extended Access   7*/
	IEEE_SCHSTART_SUCCESS_NO_IMD, /* PARTIAL SUCCESS No Immediate Access  8*/
	IEEE_SCHSTART_SUCCESS_NO_EXT_OR_IMD, /* PARTIAL SUCCESS No Extended or Immediate Access  9 */
	IEEE_SCHSTART_NO_SYNC, // 10
	IEEE_SCHSTART_INVALID_PARAMETERS,  // 11
	IEEE_SCHSTART_UNSPECIFIED_FAILURE, // 12
	
	// SCHEND confirm
	IEEE_SCHEND_SUCCESS, // 13
	IEEE_SCHEND_INVALID_PARAMETERS, // 14
	
	// MLMEX-REGISTERTXPROFILE.confirm
	IEEE_REGISTER_SUCCESS, // 15
	IEEE_REGISTER_INVALID_PARAMETERS, // 16
	
	// MLMEX-DELETETXPROFILE.confirm
	IEEE_DELETE_SUCCESS, // 17
	IEEE_DELETE_INVALID_PARAMETERS, // 18
	
	// MLMEX-CANCELTX.confirm
	IEEE_CANCELTX_SUCCESS, // 19
	IEEE_CANCELTX_INVALID_PARAMETERS, // 20
	IEEE_CANCELTX_UNSPECIFIED_FAILURE, // 21
	
	// MLMEX-SETUTCTIME.confirm
	IEEE_SETUTCTIME_SUCCESS, // 22
	IEEE_SETUTCTIME_INVALID_PARAMETERS, // 23
	IEEE_SETUTCTIME_UNSPECIFIED_FAILURE, // 24
	
	// MLMEX-AddressChange.confirm
	IEEE_ADDRESSCHANGE_ACCEPTED, // 25
	IEEE_ADDRESSCHANGE_REJECTED, // 26
	
	// MLMEX-SendPrimitive.confirm
	IEEE_SENDP_SUCCESS, // 27
	IEEE_SENDP_NO_SYNC, // 28
	IEEE_SENDP_NOT_SUPPORTED, // 29
	IEEE_SENDP_CHANNEL_UNAVAILABLE, // 30
	IEEE_SENDP_INVALID_PARAMETERS, // 31
	IEEE_SENDP_UNSPECIFIED_FAILURE, // 32	
	
	// MIB ChannelSetTable
	IEEE_SET_MIB_SUCCESS, // 33
	IEEE_SET_MIB_FAILED,//34
	
	//Test-command confirm
	IEEE_TEST_CMD_SUCCESS, // 35
	IEEE_TEST_CMD_INVALID_PARAMETERS, // 36
	IEEE_TEST_CMD_UNSPECIFIED_FAILURE, // 37

	//DBG-command confirm
	IEEE_DBG_CMD_SUCCESS, // 38
	IEEE_DBG_CMD_INVALID_PARAMETERS, // 39
	IEEE_DBG_CMD_UNSPECIFIED_FAILURE, // 40

	// MLMEX-SETUTCTIME.confirm
	IEEE_GETUTCTIME_SUCCESS, // 41
	IEEE_GETUTCTIME_INVALID_PARAMETERS, // 42
	IEEE_GETUTCTIME_UNSPECIFIED_FAILURE // 43 
	
} ieee_result_code_e;

/* IOCTL Response Data Structure */
typedef enum private_result_code
{
	// NL80211_ATTR_WAVE_SET_CTRLMODE
	SET_FIRMWAVE_SUCCESS = 0,
	SET_FIRMWAVE_FAILED, // 1
	
	// NL80211_ATTR_WAVE_SET_TX_POWER
	SET_TX_POWER_SUCCESS, // 2
	SET_TX_POWER_OUTOFRANGE, // 3
	
	// NL80211_ATTR_WAVE_SET_EDCA
	SET_EDCA_SUCCESS, // 4
	SET_EDCA_OUTOFRANGE, // 5
	SET_EDCA_INVALID // 6
} private_result_code_e;

typedef struct ieee_utc_sync
{
	u32 						utc_status; /* 0: Nosync, 1: Sync */
} __attribute__((packed)) ieee_utc_sync_t;

typedef struct ks_private_response
{
	private_result_code_e		result;
} __attribute__((packed)) ks_private_response_t;

typedef struct ieee_primitive_conf
{
	ieee_result_code_e         result;
} __attribute__((packed)) ieee_primitive_conf_t;

typedef struct ieee_get_utc_confirm
{
	u8		utc_time[8];				/* UTC time            */
	u8		time_value[8];				/* TSF time value      */
	u8		time_error[8];				/* UTC time error      */
} __attribute__((packed)) ieee_get_utc_confirm_t;

typedef struct ieee_mlmex_ta_ind
{
	u8							time_stamp[8];
	u8 							src_mac[ETH_ALEN];
	u8 							rcpi;
	u64 						local_time;
	ieee_ta_content_t 			ta_content;	
	ieee_channel_identifier_t 	ch_identifier;	
} __attribute__((packed)) ieee_mlmex_ta_ind_t;

typedef struct ieee_mlmex_vsa_ind
{
	u8 							src_mac[ETH_ALEN];
	u8 							mgmt_id;		/* Identifier of the originator of the data: 0¨C15 */
	ieee_vendor_specific_t 		vendor_spec;
	ieee_channel_identifier_t 	ch_identifier;
	u8 							rcpi;
	u8							category;	/* vendor-specific category */
} __attribute__((packed)) ieee_mlmex_vsa_ind_t;

typedef enum ieee_schend_reason
{
	IEEE16094_LOSS_OF_SYNC = 0,
	IEEE16094_UNSPECIFIED_REASON
} ieee_schend_reason_e;

typedef struct ieee_mlmex_schend_ind
{	
	ieee_schend_reason_e 		reason;
} __attribute__((packed))ieee_mlmex_schend_ind_t;

typedef struct ieee_utc_sync_ind
{	
	u32 		utc_status; /* 0: Nosync, 1: Sync */
} __attribute__((packed))ieee_utc_sync_ind_t;

#if 0
typedef struct iwreq_control {
	u32	control;				/* HostCPU Driver/Firmware control setting */
} __attribute__((packed)) IWREQ_CONTROL;

typedef struct iwreq_txpower {
	u8		txpower;				/* TX power for PbP */
} __attribute__((packed)) IWREQ_TXPOWER;

typedef struct iwreq_edcaparam {
	u32	CWmin[IFACPRIO];		/* Set up the minimum size of CW. */
	u32	CWmax[IFACPRIO];		/* Set up the maximum size of CW. */
	u32	AIFSN[IFACPRIO];		/* Set up the number of slots after SIFS. */
	u32	TXOPLimit[IFACPRIO];	/* 	This parameter is un-supporting. */
} __attribute__((packed)) IWREQ_EDCAPARAM;

typedef struct iwreq_softver {
	u8		fw_ver[IFVERSIZE];			/* Get Firmware verion. */
	u8		drv_ver[IFVERSIZE];			/* Get HostCPU driver verion. */
} __attribute__((packed)) IWREQ_SWVER;
#endif

typedef struct iwreq_mac_addr {
	u8 mac_addr[IFMACSIZ]; /* MAC address */
} __attribute__((packed)) IWREQ_MAC_ADDR;
typedef struct iwreq_ch {
	unsigned short channel; /* default channel( frequency ) */
} __attribute__((packed)) IWREQ_CH;

typedef struct iwreq_bitrate {
	unsigned long bitrate; /* default bit rate */
} __attribute__((packed)) IWREQ_BITRATE;

typedef struct iwreq_ifname {
	unsigned char ifname[IFNAMSIZ]; /* protocol name */
} __attribute__((packed)) IWREQ_IFNAME;

typedef struct iwreq_rts {
	unsigned short rts_threshold; /* RTS/CTS threshold */
} __attribute__((packed)) IWREQ_RTS;

#ifdef UNIT_TEST
typedef struct iwreq_test {
	unsigned short unit_test; 
} __attribute__((packed)) IWREQ_TEST;
#endif

typedef struct iwreq_phyinfo {
	unsigned long rcv_num_frame;	
	unsigned long rcv_num_byte;	
	unsigned long rcv_num_phyerr;	
	unsigned long rcv_num_crcerr;	
	unsigned long trn_num_rts_ok;	
	unsigned long trn_num_rts_ng;	
	unsigned long rcv_num_dup_err;
	unsigned long trn_num_ack_err;
	unsigned long trn_num_retry;	
	unsigned long trn_num_mu_retry;
	unsigned long trn_num_frame;	
	unsigned long trn_num_byte;	
	unsigned long trn_num_phyerr;
	unsigned long trn_num_failed;
	unsigned long trn_num_drv_pk_err;		
	unsigned long trn_num_qu_err;
} __attribute__((packed)) IWREQ_PHYINFO;

typedef struct iwreq_tptx {
	unsigned char tp_tx[IFTPTXSIZ]; 
} __attribute__((packed)) IWREQ_TPTX;

struct iw_event_tprx {
	unsigned char tp_rx[IFTPRXSIZ];
} __attribute__((packed));

typedef struct iwreq_dbgtx {
	unsigned char dbg_tx[IFTPTXSIZ]; 
} __attribute__((packed)) IWREQ_DBGTX;
struct iw_event_dbgrx {
	unsigned char dbg_rx[IFTPRXSIZ];
} __attribute__((packed));

typedef struct iwreq_frag{
	unsigned int frag_threshold; /* fragmentation threshold */
} __attribute__((packed)) IWREQ_FRAG;

typedef struct iwreq_control_event{
	unsigned long control_event; 
} __attribute__((packed)) IWREQ_CTRLEVENT;

typedef struct iwreq_set_mac_addr_ex {
	unsigned short	type;
	unsigned char	cch_mac_addr[IFMACSIZ];
	unsigned char	sch_mac_addr[IFMACSIZ];
} __attribute__((packed)) IWREQ_SET_MAC_ADDR_EX;

typedef struct iwreq_get_mac_addr_ex {
	unsigned char	cch_mac_addr[IFMACSIZ];
	unsigned char	sch_mac_addr[IFMACSIZ];
} __attribute__((packed)) IWREQ_GET_MAC_ADDR_EX;

typedef struct iwreq_sretry {
	unsigned long	short_retry_limit;		/* Short Retry Limit for Unicast */
} __attribute__((packed)) IWREQ_SRETRY;

typedef struct iwreq_lretry {
	unsigned long	long_retry_limit;		/* Long Retry Limit for Unicast */
} __attribute__((packed)) IWREQ_LRETRY;

typedef struct iwreq_retryrate {
	unsigned long	retry_rate_mode;		/* Retry Rate Mode ( fallback control ) */
} __attribute__((packed)) IWREQ_RETRYRATE;

#endif /* _KS_IEEE16094_API_H_ */
