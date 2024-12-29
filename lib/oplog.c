#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

/*log*/
PUBLIC void log_open()
{
	MESSAGE msg;
	msg.type	= STARTLOG;
	send_recv(SEND, TASK_LOG, &msg);
}

PUBLIC void reading_log()
{   
    
	MESSAGE msg;
    msg.type    = READLOG;
    //printf("                %d\n",TASK_LOG);
    send_recv(BOTH, TASK_LOG, &msg);
    return;
}

PUBLIC void log_end()
{
	MESSAGE msg;
	msg.type	= ENDLOG;
	send_recv(SEND, TASK_LOG, &msg);
}
