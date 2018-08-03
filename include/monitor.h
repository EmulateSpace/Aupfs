#ifndef _PARSE_H
#define _PARSE_H

#define MSG_HEAD       "$%"
#define MSG_CMD        "~^"
#define MSG_TAIL       "#@"

#define CMD_WRITE      "w"
#define CMD_READ       "r"
#define CMD_SET        "s"
#define CMD_DUMP       "d"

#define BUFFER_BOUND    32
#define CMD_BUF_LEN     BUFFER_BOUND

extern int parse_message(const char *message, int nr);

#endif
