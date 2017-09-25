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
//*	$Id: MvsnLicenseServerMsgs.h,v 1.7.2.5 2003/06/30 23:55:19 sluu Exp $     */
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

//
// MessageId: MVSN_CAT_EVENTLOG_INTERNAL_ERROR
//
// MessageText:
//
//  General Software Logic
//
#define MVSN_CAT_EVENTLOG_INTERNAL_ERROR ((DWORD)0x00000014L)

//
// MessageId: MVSN_CAT_EVENTLOG_WIN32_NETWORK
//
// MessageText:
//
//  Windows Domain-based Networking
//
#define MVSN_CAT_EVENTLOG_WIN32_NETWORK  ((DWORD)0x00000015L)

//
// MessageId: MVSN_CAT_EVENTLOG_WIN32_REGISTRY
//
// MessageText:
//
//  Windows Registry Logic
//
#define MVSN_CAT_EVENTLOG_WIN32_REGISTRY ((DWORD)0x00000016L)

//
// MessageId: MVSN_CAT_EVENTLOG_WIN32_KERNEL
//
// MessageText:
//
//  Win32 Kernel Logic
//
#define MVSN_CAT_EVENTLOG_WIN32_KERNEL   ((DWORD)0x00000017L)

//
// MessageId: MVSN_CAT_EVENTLOG_GENERAL_INFO
//
// MessageText:
//
//  General Information
//
#define MVSN_CAT_EVENTLOG_GENERAL_INFO   ((DWORD)0x00000018L)

//
// MessageId: MVSN_CAT_EVENTLOG_BASIC_TRACE
//
// MessageText:
//
//  Internal Basic Tracing
//
#define MVSN_CAT_EVENTLOG_BASIC_TRACE    ((DWORD)0x00000019L)

// Message definitions
// -------------------
//
// MessageId: MSG_FLEXLM_READLEN_ERROR
//
// MessageText:
//
//  (%1) An unexpected input error has been detected: %2.
//
#define MSG_FLEXLM_READLEN_ERROR         ((DWORD)0xC00002BEL)

//
// MessageId: MSG_FLEXLM_NO_FORCED_SHUTDOWN
//
// MessageText:
//
//  (%1) No forced shutdown requested.
//
#define MSG_FLEXLM_NO_FORCED_SHUTDOWN    ((DWORD)0x400002C1L)

//
// MessageId: MSG_FLEXLM_EVENT_LOG_ACTIVE
//
// MessageText:
//
//  (%1) FLEXlm License Manager started.
//
#define MSG_FLEXLM_EVENT_LOG_ACTIVE      ((DWORD)0x400002D1L)

//
// MessageId: MSG_FLEXLM_LMGRD_DAEMON_NOT_FOUND
//
// MessageText:
//
//  (%1) LMGRD cannot determine which  vendor daemon to start; lmgrd exiting. 
//  There are no VENDOR (or DAEMON) lines in the license file.
//
#define MSG_FLEXLM_LMGRD_DAEMON_NOT_FOUND ((DWORD)0xC00002D2L)

//
// MessageId: MSG_FLEXLM_LMGRD_WIN32_SERVICE
//
// MessageText:
//
//  (%1) FLEXlm License Server error: %2 %3.
//
#define MSG_FLEXLM_LMGRD_WIN32_SERVICE   ((DWORD)0xC00002DBL)

//
// MessageId: MSG_FLEXLM_LMGRD_WIN32_SERVICE_STOPPED
//
// MessageText:
//
//  (%1) FLEXlm License Server is shutting down.
//
#define MSG_FLEXLM_LMGRD_WIN32_SERVICE_STOPPED ((DWORD)0x400002DCL)

//
// MessageId: MSG_FLEXLM_LMGRD_STARTMSG
//
// MessageText:
//
//  (%1) FLEXlm (v%2.%3%4) started on %5%6 (%7/%8/%9).
//
#define MSG_FLEXLM_LMGRD_STARTMSG        ((DWORD)0x400002DEL)

//
// MessageId: MSG_FLEXLM_LMGRD_CLOSE_ERROR
//
// MessageText:
//
//  (%1) TCP/IP file descriptor: %2.
//  Length: %3
//  Error number: %4
//  Module: %5
//  Line number: %6
//
#define MSG_FLEXLM_LMGRD_CLOSE_ERROR     ((DWORD)0xC00002EAL)

//
// MessageId: MSG_FLEXLM_LMGRD_READ_ERROR
//
// MessageText:
//
//  (%1) TCP/IP read error: %2.
//  TCP/IP file descriptor: %3.
//
#define MSG_FLEXLM_LMGRD_READ_ERROR      ((DWORD)0xC00002EBL)

//
// MessageId: MSG_FLEXLM_LMGRD_BAD_CHECKSUM
//
// MessageText:
//
//  (%1) BAD CHECKSUM.  %2 sending WHAT command.
//  Client comm revision: %3
//  Message size: %4
//
#define MSG_FLEXLM_LMGRD_BAD_CHECKSUM    ((DWORD)0xC00002ECL)

//
// MessageId: MSG_FLEXLM_LMGRD_PORT
//
// MessageText:
//
//  (%1) LMGRD is using TCP-port: %2.
//
#define MSG_FLEXLM_LMGRD_PORT            ((DWORD)0x400002EFL)

//
// MessageId: MSG_FLEXLM_LMGRD_LICENSE_FILE
//
// MessageText:
//
//  (%1) LMGRD is using license file: %2.
//
#define MSG_FLEXLM_LMGRD_LICENSE_FILE    ((DWORD)0x400002F1L)

//
// MessageId: MSG_FLEXLM_LMGRD_SHUTDOWN_REQ
//
// MessageText:
//
//  (%1) LMGRD shutdown request from %2 at node: %3
//
#define MSG_FLEXLM_LMGRD_SHUTDOWN_REQ    ((DWORD)0x400002F2L)

//
// MessageId: MSG_FLEXLM_LMGRD_SHUTDOWN
//
// MessageText:
//
//  (%1) Shutting down vendor daemon:  %2, on node: %3
//
#define MSG_FLEXLM_LMGRD_SHUTDOWN        ((DWORD)0x400002F3L)

//
// MessageId: MSG_FLEXLM_LMGRD_STARTUP_FAILED
//
// MessageText:
//
//  (%1) License server startup failed. LMGRD cannot find vendor daemon:  %2
//
#define MSG_FLEXLM_LMGRD_STARTUP_FAILED  ((DWORD)0xC00002F4L)

//
// MessageId: MSG_FLEXLM_VENDOR_REPORT_LOG_OPEN_ERROR
//
// MessageText:
//
//  (%1) Cannot open report log: %2; %3
//
#define MSG_FLEXLM_VENDOR_REPORT_LOG_OPEN_ERROR ((DWORD)0xC00002FBL)

//
// MessageId: MSG_FLEXLM_VENDOR_BORROW_OUTSTANDING
//
// MessageText:
//
//  (%1) Cannot shutdown when borrowed licenses are outstanding.  Specify "-force" to  override.
//
#define MSG_FLEXLM_VENDOR_BORROW_OUTSTANDING ((DWORD)0x800002FDL)

//
// MessageId: MSG_FLEXLM_LMGRD_PORT_NOT_AVAILABLE
//
// MessageText:
//
//  (%1) The TCP/IP port number, %2,  specified in the license file is already in use.
//
#define MSG_FLEXLM_LMGRD_PORT_NOT_AVAILABLE ((DWORD)0x80000300L)

//
// MessageId: MSG_FLEXLM_LMGRD_PORT_OPEN_FAILED
//
// MessageText:
//
//  (%1) LMGRD failed to open the TCP/IP port number specified in the license file.
//
#define MSG_FLEXLM_LMGRD_PORT_OPEN_FAILED ((DWORD)0xC0000301L)

//
// MessageId: MSG_FLEXLM_DEBUG_LOG_OPEN_ERROR
//
// MessageText:
//
//  (%1) Cannot open debug log: %2; %3
//
#define MSG_FLEXLM_DEBUG_LOG_OPEN_ERROR  ((DWORD)0xC0000302L)

