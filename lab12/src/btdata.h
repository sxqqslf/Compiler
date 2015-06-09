
#include <pthread.h>
#include "bencode.h"

#ifndef BTDATA_H
#define BTDATA_H

/**************************************
 * 一些常量定义
**************************************/

#define HANDSHAKE_LEN 68  // peer握手消息的长度, 以字节为单位
/*#define BT_PROTOCOL "BitTorrent protocol"
#define INFOHASH_LEN 20
#define PEER_ID_LEN 20*/
#define LISTEN_Q 8
#define MAX_PEERS 10
#define KEEP_ALIVE_INTERVAL 120
#define WAIT_TIME 180		//判断是否超时间隔，以秒为单位
#define REQUEST_TIME 5
#define REQUEST_LEN 16384
#define DOWNLOAD_LIMIT 5
#define RECV_SLEEP 200000
#define RAREST_FIRST_TIME 3		//每3秒查询一次

#define BT_STARTED 0
#define BT_STOPPED 1
#define BT_COMPLETED 2

/**************************************
 * 数据结构
**************************************/
// 公网得到的tracker服务器的URL列表结点结构体
extern char piece_hash[10000];
typedef struct tracker_node
{
	char announce[100];			// 此结点代表的服务器的URL
	struct tracker_node* next;	// 指向下一个服务器URL节点
}tracker_node;

extern tracker_node tracker_list[100];
extern int tracker_count;
extern char file_name[100];
// 公网服务器URL节点列表

// Tracker HTTP响应的数据部分
typedef struct _tracker_response {
  int size;       // B编码字符串的字节数
  char* data;     // B编码的字符串
} tracker_response;


// 元信息文件中包含的数据
typedef struct _torrentmetadata {
  int info_hash[5]; // torrent的info_hash值(info键对应值的SHA1哈希值)
  char* announce; // tracker的URL
  int length;     // 文件长度, 以字节为单位
  char* name;     // 文件名
  int piece_len;  // 每一个分片的字节数
  int num_pieces; // 分片数量(为方便起见)
  char* pieces;   // 针对所有分片的20字节长的SHA1哈希值连接而成的字符串
} torrentmetadata_t;

// 包含在announce url中的数据(例如, 主机名和端口号)
typedef struct _announce_url_t {
  char* hostname;
  int port;
} announce_url_t;

// 包含在announce url中的数据(例如, 主机名和端口号)的列表节点
typedef struct announce_list
{
	announce_url_t* info;
	struct announce_list* next;
}announce_list;

extern announce_list* url_list;      // announce url列表头指针，定义在simpletorrent.c中

// 由tracker返回的响应中peers键的内容
typedef struct _peerdata {
  char id[21]; // 20用于null终止符
  int port;
  char* ip; // Null终止
} peerdata;

// 由tracker返回的peer列表
typedef struct peer_list
{
	peerdata this_peer;
	struct peer_list* next;
}peer_list;

// 由tracker返回的peer列表头指针
extern peer_list* peer_head;

// 包含在tracker响应中的数据
typedef struct _tracker_data {
  int interval;
  int numpeers;
  peerdata* peers;
} tracker_data;

typedef struct _tracker_request {
  int info_hash[5];
  char peer_id[20];
  int port;
  int uploaded;
  int downloaded;
  int left;
  char ip[16]; // 自己的IP地址, 格式为XXX.XXX.XXX.XXX, 最后以'\0'结尾
} tracker_request;

// 针对到一个peer的已建立连接, 维护相关数据
typedef struct _peer_t {
  int sockfd;
  int choking;        // 作为上传者, 阻塞远端peer
  int interested;     // 远端peer对我们的分片有兴趣
  int choked;         // 作为下载者, 我们被远端peer阻塞
  int have_interest;  // 作为下载者, 对远端peer的分片有兴趣
	int cancel;	// 0无影响，1不接收piece
  char name[20]; 
    unsigned char *peer_mark;
    int send_time;	
	int download_count;		//同一时间从单个peer下载的分片数
} peer_t;
extern int count;
//握手报文格式
typedef struct _handshake
{
	unsigned char len;
	char name[19];
	char reserved[8];
	int info_hash[5];
	char peer_id[20];
} handshake;

//peer communication Header
typedef struct peer_wire_head{
	unsigned int length;
	unsigned char id;
} peer_wire_h;

typedef struct peer_wire_have{
	peer_wire_h head;
	unsigned int piece_index;
} peer_have;

typedef struct peer_wire_reqeust{
	//peer_wire_h head;
	unsigned int index;
	unsigned int begin;
	unsigned int length;
} peer_request;

typedef struct peer_wire_piece_head{
	//peer_wire_h head;
	unsigned int index;
	unsigned int begin;
} peer_piece_h;

typedef struct peer_wire_cancel{
	//peer_wire_h head;
	unsigned int index;
	unsigned int begin;
	unsigned int length;
} peer_cancel;

/**************************************
 * 全局变量 
**************************************/
peer_t *peer_pool[MAX_PEERS];

char g_my_ip[16]; // 格式为XXX.XXX.XXX.XXX, null终止
int g_peerport; // peer监听的端口号
int g_infohash[5]; // 要共享或要下载的文件的SHA1哈希值, 每个客户端同时只能处理一个文件
char g_my_id[20];
int piece_len;
int num_pieces;
int g_index;		//用来记录最后一个要下载的分片的index
pthread_mutex_t peer_mutex;		//用来对修改peer_pool里面的内容进行加锁
pthread_mutex_t send_recv_mutex;		//互斥锁

int g_done; // 表明程序是否应该终止

torrentmetadata_t* g_torrentmeta;
char* g_filedata;      // 文件的实际数据
int g_filelen;
int g_num_pieces;

char g_tracker_ip[16]; // tracker的IP地址, 格式为XXX.XXX.XXX.XXX(null终止)
int g_tracker_port;
tracker_data *g_tracker_response;

// 这些变量用在函数make_tracker_request中, 它们需要在客户端执行过程中不断更新.
int g_uploaded;
int g_downloaded;
int g_left;

// 存储接收到的数据
char* Buffer;
// 标记每个分片的下载情况的头指针
char* p_mark;
// 存储当前环境每个分片的数目
int* p_num;
// 执行rarest_first的时间
struct timeval rf_time;

// 标记本客户端是否为seeder, 为1是seeder, 为0不是seeder
int seeder;
/***************************************
 * 相关函数声明
***************************************/

//数据传输
int p2p_send(char *buffer, int conn, int len);			//Peer 之间发送数据。-1: Fail 1: Succeed
int p2p_recv(char *buffer, int conn, int len);			//Peer 之间接收数据。-1: Fail 1: Succeed
void* recv_handler(void *peer);			//处理接收数据
void* set_request();		//发送REQUEST，该函数只是用于启动每一个分片第一个子分片下载
int send_have(peer_have *h_head);			//Inner function
void* reset_pmark();

//握手时用到的相关函数
int compare_info_hash(int info_hash1[], int info_hash2[]);	//比较info_hash
int find_peer_id(char peer_id[]);				//寻找peer id是否在该群组里
int hand_shake(int sockfd);			//进行握手，成功返回1，失败返回0
void* listen_connection();					//接受并监听其他peer的连接
void* set_connection(void *peer_list);			//和其他peer进行握手
void* keep_alive(void* sockfd);					//每隔定时发送一个keep alive报文
void* judgeTimeOut();						//判断该连接是否超时
void send_bitfield(int conn);
#endif

