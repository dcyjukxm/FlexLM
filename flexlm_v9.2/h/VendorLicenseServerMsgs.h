//****************************************************************************
//	COPYRIGHT (c) 2003 by Macrovision Corporation.	
//	This software has been provided pursuant to a License Agreement	
//	containing restrictions on its use.  This software contains
//	valuable trade secrets and proprietary information of
//	Macrovision Corporation and is protected by law.
//	It may 	not be copied or distributed in any form or medium, disclosed
//	to third parties, reverse engineered or used in any manner not
//	provided for in said License Agreement except with the prior
//	written authorization from Macrovision Corporation.
//*****************************************************************************/
//*	$Id: VendorLicenseServerMsgs.h,v 1.9.2.5 2003/06/30 23:55:19 sluu Exp $   */
//*****************************************************************************/
// Mvsn Event Log Message File
// Message ID Type
// Severity Names
// Facility Names
// Language Support
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define FACILITY_SYSTEM                  0x0
#define FACILITY_STUBS                   0x3
#define FACILITY_RUNTIME                 0x2
#define FACILITY_IO_ERROR_CODE           0x4


//
// Define the severity codes
//
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


//
// MessageId: CAT_INTERNAL_ERROR
//
// MessageText:
//
//  Internal Software Logic
//
#define CAT_INTERNAL_ERROR               ((DWORD)0x00000001L)

//
// MessageId: CAT_SYSTEM
//
// MessageText:
//
//  OS System Configuration
//
#define CAT_SYSTEM                       ((DWORD)0x00000002L)

//
// MessageId: CAT_ENVIRONMENT
//
// MessageText:
//
//  Environmental Configuration
//
#define CAT_ENVIRONMENT                  ((DWORD)0x00000003L)

//
// MessageId: CAT_FLEXLM_LMGRD
//
// MessageText:
//
//  FLEXlm License Manager
//
#define CAT_FLEXLM_LMGRD                 ((DWORD)0x00000004L)

//
// MessageId: CAT_FLEXLM_LICENSE_SERVER
//
// MessageText:
//
//  FLEXlm License Server
//
#define CAT_FLEXLM_LICENSE_SERVER        ((DWORD)0x00000005L)

//
// MessageId: CAT_FLEXLM_LICENSE_FILE
//
// MessageText:
//
//  FLEXlm License File
//
#define CAT_FLEXLM_LICENSE_FILE          ((DWORD)0x00000006L)

//
// MessageId: CAT_FLEXLM_OPTIONS_FILE
//
// MessageText:
//
//  FLEXlm Vendor Options File
//
#define CAT_FLEXLM_OPTIONS_FILE          ((DWORD)0x00000007L)

//
// MessageId: CAT_FLEXLM_LMGRD_PERFORMANCE
//
// MessageText:
//
//  FLEXlm License Manager Performance
//
#define CAT_FLEXLM_LMGRD_PERFORMANCE     ((DWORD)0x00000008L)

//
// MessageId: CAT_FLEXLM_LICENSE_SERVER_PERFORMANCE
//
// MessageText:
//
//  FLEXlm License Server Performance
//
#define CAT_FLEXLM_LICENSE_SERVER_PERFORMANCE ((DWORD)0x00000009L)

//
// MessageId: CAT_FLEXLM_LMGRD_HEALTH
//
// MessageText:
//
//  FLEXlm License Manager Health
//
#define CAT_FLEXLM_LMGRD_HEALTH          ((DWORD)0x0000000AL)

//
// MessageId: CAT_FLEXLM_SERVER_HEALTH
//
// MessageText:
//
//  FLEXlm License Server Health
//
#define CAT_FLEXLM_SERVER_HEALTH         ((DWORD)0x0000000BL)

//
// MessageId: CAT_FLEXLM_NETWORK_COMM
//
// MessageText:
//
//  FLEXlm Network Communications
//
#define CAT_FLEXLM_NETWORK_COMM          ((DWORD)0x0000000CL)

//
// MessageId: CAT_FLEXLM_DEBUGLOG
//
// MessageText:
//
//  FLEXlm Debug Log
//
#define CAT_FLEXLM_DEBUGLOG              ((DWORD)0x0000000DL)

//
// MessageId: CAT_FLEXLM_SERVER_REPORTLOG
//
// MessageText:
//
//  FLEXlm License Server Report Log
//
#define CAT_FLEXLM_SERVER_REPORTLOG      ((DWORD)0x0000000EL)

//
// MessageId: CAT_FLEXLM_LMGRD_EVENT
//
// MessageText:
//
//  FLEXlm License Manager Event
//
#define CAT_FLEXLM_LMGRD_EVENT           ((DWORD)0x0000000FL)

//
// MessageId: CAT_FLEXLM_SERVER_EVENT
//
// MessageText:
//
//  FLEXlm License Server Event
//
#define CAT_FLEXLM_SERVER_EVENT          ((DWORD)0x00000010L)

//
// MessageId: CAT_FLEXLM_EVENT_BROKER
//
// MessageText:
//
//  FLEXlm Event Broker
//
#define CAT_FLEXLM_EVENT_BROKER          ((DWORD)0x00000011L)

//
// MessageId: CAT_FLEXLM_EVENTLOG
//
// MessageText:
//
//  FLEXlm Event Log Engine
//
#define CAT_FLEXLM_EVENTLOG              ((DWORD)0x00000012L)

//
// MessageId: CAT_FLEXLM_AGENT
//
// MessageText:
//
//  FLEXlm Remote Agent
//
#define CAT_FLEXLM_AGENT                 ((DWORD)0x00000013L)

// Message definitions
// -------------------
//
// MessageId: MSG_FLEXLM_LICENSE_EXPIRED
//
// MessageText:
//
//  (%1) This license (%2) has expired.
//
#define MSG_FLEXLM_LICENSE_EXPIRED       ((DWORD)0xC00002CCL)

//
// MessageId: MSG_FLEXLM_NEAR_EXPIRED
//
// MessageText:
//
//  (%1) Feature: %2, expires on %3.
//  This is a two week notice.
//
#define MSG_FLEXLM_NEAR_EXPIRED          ((DWORD)0x800002CEL)

//
// MessageId: MSG_FLEXLM_BAD_AUTHCODE
//
// MessageText:
//
//  (%1) Invalid license key: %2.  Inconsistent authentication code.
//
#define MSG_FLEXLM_BAD_AUTHCODE          ((DWORD)0xC00002D4L)

//
// MessageId: MSG_FLEXLM_NOT_READY
//
// MessageText:
//
//  (%1) Feature: %2 has not been enabled yet.
//  The current date is earlier than the start date for this feature.
//
#define MSG_FLEXLM_NOT_READY             ((DWORD)0x400002D5L)

//
// MessageId: MSG_FLEXLM_NOT_READY_YET
//
// MessageText:
//
//  (%1) Feature: %2 has not been enabled yet; it starts on %3.
//  The current date is earlier than the start date for this feature.
//
#define MSG_FLEXLM_NOT_READY_YET         ((DWORD)0x400002D6L)

//
// MessageId: MSG_FLEXLM_LICENSE_EXPIRED_ONREREAD
//
// MessageText:
//
//  (%1) Feature: %2 (Date: %3), has expired on rereading.
//
#define MSG_FLEXLM_LICENSE_EXPIRED_ONREREAD ((DWORD)0x400002D7L)

//
// MessageId: MSG_FLEXLM_LICENSE_NOINCREMENTLINE_ONUPGRADE
//
// MessageText:
//
//  (%1) Warning: no prior INCREMENT line for UPGRADE.
//  Feature: %2, Version: %3->%4
//
#define MSG_FLEXLM_LICENSE_NOINCREMENTLINE_ONUPGRADE ((DWORD)0x800002D8L)

//
// MessageId: MSG_FLEXLM_DAEMON_INIT_ERROR
//
// MessageText:
//
//  (%1) Daemon initialization error: %2.
//
#define MSG_FLEXLM_DAEMON_INIT_ERROR     ((DWORD)0xC00002D9L)

//
// MessageId: MSG_FLEXLM_VENDOR_FEATUREDB_ERROR
//
// MessageText:
//
//  (%1) Feature database corrupted.
//
#define MSG_FLEXLM_VENDOR_FEATUREDB_ERROR ((DWORD)0xC00002E1L)

//
// MessageId: MSG_FLEXLM_LMGRD_VERSION_ERROR
//
// MessageText:
//
//  (%1) Client/server comm version mismatch (client: %2.%3, server: %4.%5).
//  The client and server programs are running potentially incompatible versions of FLEXlm comm software.  
//
#define MSG_FLEXLM_LMGRD_VERSION_ERROR   ((DWORD)0x800002EDL)

//
// MessageId: MSG_FLEXLM_CLIENT_COMM_VERSION_ERROR
//
// MessageText:
//
//  (%1) Invalid client communications revision: %2 %3 message.
//  COMM revision: %3 (%5)
//  Version: %6 (%7)
//
#define MSG_FLEXLM_CLIENT_COMM_VERSION_ERROR ((DWORD)0x800002EEL)

//
// MessageId: MSG_FLEXLM_VENDOR_OPTIONS_FILE
//
// MessageText:
//
//  (%1) Using options file: %2
//
#define MSG_FLEXLM_VENDOR_OPTIONS_FILE   ((DWORD)0x400002F6L)

//
// MessageId: MSG_FLEXLM_VENDOR_OPTIONS_FILE_OPEN_ERROR
//
// MessageText:
//
//  (%1) Cannot open options file: %2
//
#define MSG_FLEXLM_VENDOR_OPTIONS_FILE_OPEN_ERROR ((DWORD)0x800002F7L)

//
// MessageId: MSG_FLEXLM_VENDOR_REPORT_LOG_STARTED
//
// MessageText:
//
//  (%1) Report logging started, using report log: %2
//
#define MSG_FLEXLM_VENDOR_REPORT_LOG_STARTED ((DWORD)0x400002F8L)

//
// MessageId: MSG_FLEXLM_VENDOR_REPORT_LOG_FAILED
//
// MessageText:
//
//  (%1) Report logging did not start: %2
//
#define MSG_FLEXLM_VENDOR_REPORT_LOG_FAILED ((DWORD)0xC00002F9L)

//
// MessageId: MSG_FLEXLM_VENDOR_DEBUG_LOG_STARTED
//
// MessageText:
//
//  (%1) Debug logging started, using debug log: %2
//
#define MSG_FLEXLM_VENDOR_DEBUG_LOG_STARTED ((DWORD)0x400002FAL)

//
// MessageId: MSG_FLEXLM_VENDOR_REPORT_LOG_OPEN_ERROR
//
// MessageText:
//
//  (%1) Cannot open report log: %2; %3
//
#define MSG_FLEXLM_VENDOR_REPORT_LOG_OPEN_ERROR ((DWORD)0xC00002FBL)

//
// MessageId: MSG_FLEXLM_VENDOR_WRONG_HOSTID
//
// MessageText:
//
//  (%1) Wrong hostid on SERVER line in license file: %2.  HOSTID specified, %3, does not match actual: %4.
//
#define MSG_FLEXLM_VENDOR_WRONG_HOSTID   ((DWORD)0xC00002FCL)

//
// MessageId: MSG_FLEXLM_VENDOR_CLOCK_ALTERED
//
// MessageText:
//
//  (%1) System clock has been altered.
//
#define MSG_FLEXLM_VENDOR_CLOCK_ALTERED  ((DWORD)0xC00002FEL)

//
// MessageId: MSG_FLEXLM_DEBUG_LOG_OPEN_ERROR
//
// MessageText:
//
//  (%1) Cannot open debug log: %2; %3
//
#define MSG_FLEXLM_DEBUG_LOG_OPEN_ERROR  ((DWORD)0xC0000302L)

