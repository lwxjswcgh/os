#include "type.h"
#include "config.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

/*****************************************************************************
 *                                task_log
 *****************************************************************************/
/**
 * <Ring 1> The main loop of TASK LOG.
 * 
 *****************************************************************************/
PUBLIC void task_log()
{	
	while (1) {
		char* message;
		send_recv(RECEIVE, ANY, &log_msg);
		if((log_executing==0)&&(log_msg.type!=STARTLOG)){
			continue;
		}
		while(log_executing==-1){}
		int src = log_msg.source;
		int reply = 1;

		int msgtype = log_msg.type;

		switch (msgtype) {
		case STARTLOG:
			start_log();
			reply = 0;
			break;
		case WRITELOG:
			log_msg.RETVAL = log_generate(&log_msg);
			reply = 0;
			break;
		case READLOG:
			log_msg.RETVAL = log_read(&message);
			reply = 1;
			break;
		case ENDLOG:
			log_msg.RETVAL = end_log();
			reply = 0;
			break;
		default:
			dump_msg("MM::unknown msg", &log_msg);
			assert(0);
			break;
		}
		if (reply) {
			log_msg.type = READLOG;
			//printl("sending to%d",src);
			send_recv(SEND, src, &log_msg);
		}
	}
}