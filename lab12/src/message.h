#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define MAX_CON 4	//一个peer所能允许的最大连接数
#define LISTEN_Q 8
#define HANDSHAKE_LEN 68  // peer握手消息的长度, 以字节为单位
#define KEEP_ALIVE_INTERVAL 120		//keep alive轮询间隔，以秒为单位
#define WAIT_TIME 180		//判断是否超时间隔，以秒为单位

int count;	//用来标记已存的结构
peer_t *LocalList[4];

//握手报文格式
typedef struct _handshake
{
　　	unsigned char len;
　　	char name[19];
　　	char reserved[8];
　　	int info_hash[5];
	char peer_id[20];
} handshake;

//其他消息报文格式
typedef struct _peer_wire
{
	unsigned int len;
	char type;
} peer_wire;

//比较info_hash
int compare_info_hash(int info_hash1[], int info_hash2[]);

//寻找peer id是否在该群组里
int find_peer_id(char peer_id[]);

//进行握手，成功返回1，失败返回0
int handshake(int info_hash[], int sockfd)；

//接受并监听其他peer的连接
void listen_connection();

//和其他peer进行握手
void set_connection(peerlist *peer_list)

//每隔定时发送一个keep alive报文
void keepAlive(int sockfd);

//判断该连接是否超时
void judgeTimeOut();
