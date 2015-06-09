
#include <pthread.h>
#include "bencode.h"

#ifndef BTDATA_H
#define BTDATA_H

/**************************************
 * һЩ��������
**************************************/

#define HANDSHAKE_LEN 68  // peer������Ϣ�ĳ���, ���ֽ�Ϊ��λ
/*#define BT_PROTOCOL "BitTorrent protocol"
#define INFOHASH_LEN 20
#define PEER_ID_LEN 20*/
#define LISTEN_Q 8
#define MAX_PEERS 10
#define KEEP_ALIVE_INTERVAL 120
#define WAIT_TIME 180		//�ж��Ƿ�ʱ���������Ϊ��λ
#define REQUEST_TIME 5
#define REQUEST_LEN 16384
#define DOWNLOAD_LIMIT 5
#define RECV_SLEEP 200000
#define RAREST_FIRST_TIME 3		//ÿ3���ѯһ��

#define BT_STARTED 0
#define BT_STOPPED 1
#define BT_COMPLETED 2

/**************************************
 * ���ݽṹ
**************************************/
// �����õ���tracker��������URL�б���ṹ��
extern char piece_hash[10000];
typedef struct tracker_node
{
	char announce[100];			// �˽�����ķ�������URL
	struct tracker_node* next;	// ָ����һ��������URL�ڵ�
}tracker_node;

extern tracker_node tracker_list[100];
extern int tracker_count;
extern char file_name[100];
// ����������URL�ڵ��б�

// Tracker HTTP��Ӧ�����ݲ���
typedef struct _tracker_response {
  int size;       // B�����ַ������ֽ���
  char* data;     // B������ַ���
} tracker_response;


// Ԫ��Ϣ�ļ��а���������
typedef struct _torrentmetadata {
  int info_hash[5]; // torrent��info_hashֵ(info����Ӧֵ��SHA1��ϣֵ)
  char* announce; // tracker��URL
  int length;     // �ļ�����, ���ֽ�Ϊ��λ
  char* name;     // �ļ���
  int piece_len;  // ÿһ����Ƭ���ֽ���
  int num_pieces; // ��Ƭ����(Ϊ�������)
  char* pieces;   // ������з�Ƭ��20�ֽڳ���SHA1��ϣֵ���Ӷ��ɵ��ַ���
} torrentmetadata_t;

// ������announce url�е�����(����, �������Ͷ˿ں�)
typedef struct _announce_url_t {
  char* hostname;
  int port;
} announce_url_t;

// ������announce url�е�����(����, �������Ͷ˿ں�)���б�ڵ�
typedef struct announce_list
{
	announce_url_t* info;
	struct announce_list* next;
}announce_list;

extern announce_list* url_list;      // announce url�б�ͷָ�룬������simpletorrent.c��

// ��tracker���ص���Ӧ��peers��������
typedef struct _peerdata {
  char id[21]; // 20����null��ֹ��
  int port;
  char* ip; // Null��ֹ
} peerdata;

// ��tracker���ص�peer�б�
typedef struct peer_list
{
	peerdata this_peer;
	struct peer_list* next;
}peer_list;

// ��tracker���ص�peer�б�ͷָ��
extern peer_list* peer_head;

// ������tracker��Ӧ�е�����
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
  char ip[16]; // �Լ���IP��ַ, ��ʽΪXXX.XXX.XXX.XXX, �����'\0'��β
} tracker_request;

// ��Ե�һ��peer���ѽ�������, ά���������
typedef struct _peer_t {
  int sockfd;
  int choking;        // ��Ϊ�ϴ���, ����Զ��peer
  int interested;     // Զ��peer�����ǵķ�Ƭ����Ȥ
  int choked;         // ��Ϊ������, ���Ǳ�Զ��peer����
  int have_interest;  // ��Ϊ������, ��Զ��peer�ķ�Ƭ����Ȥ
	int cancel;	// 0��Ӱ�죬1������piece
  char name[20]; 
    unsigned char *peer_mark;
    int send_time;	
	int download_count;		//ͬһʱ��ӵ���peer���صķ�Ƭ��
} peer_t;
extern int count;
//���ֱ��ĸ�ʽ
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
 * ȫ�ֱ��� 
**************************************/
peer_t *peer_pool[MAX_PEERS];

char g_my_ip[16]; // ��ʽΪXXX.XXX.XXX.XXX, null��ֹ
int g_peerport; // peer�����Ķ˿ں�
int g_infohash[5]; // Ҫ�����Ҫ���ص��ļ���SHA1��ϣֵ, ÿ���ͻ���ͬʱֻ�ܴ���һ���ļ�
char g_my_id[20];
int piece_len;
int num_pieces;
int g_index;		//������¼���һ��Ҫ���صķ�Ƭ��index
pthread_mutex_t peer_mutex;		//�������޸�peer_pool��������ݽ��м���
pthread_mutex_t send_recv_mutex;		//������

int g_done; // ���������Ƿ�Ӧ����ֹ

torrentmetadata_t* g_torrentmeta;
char* g_filedata;      // �ļ���ʵ������
int g_filelen;
int g_num_pieces;

char g_tracker_ip[16]; // tracker��IP��ַ, ��ʽΪXXX.XXX.XXX.XXX(null��ֹ)
int g_tracker_port;
tracker_data *g_tracker_response;

// ��Щ�������ں���make_tracker_request��, ������Ҫ�ڿͻ���ִ�й����в��ϸ���.
int g_uploaded;
int g_downloaded;
int g_left;

// �洢���յ�������
char* Buffer;
// ���ÿ����Ƭ�����������ͷָ��
char* p_mark;
// �洢��ǰ����ÿ����Ƭ����Ŀ
int* p_num;
// ִ��rarest_first��ʱ��
struct timeval rf_time;

// ��Ǳ��ͻ����Ƿ�Ϊseeder, Ϊ1��seeder, Ϊ0����seeder
int seeder;
/***************************************
 * ��غ�������
***************************************/

//���ݴ���
int p2p_send(char *buffer, int conn, int len);			//Peer ֮�䷢�����ݡ�-1: Fail 1: Succeed
int p2p_recv(char *buffer, int conn, int len);			//Peer ֮��������ݡ�-1: Fail 1: Succeed
void* recv_handler(void *peer);			//�����������
void* set_request();		//����REQUEST���ú���ֻ����������ÿһ����Ƭ��һ���ӷ�Ƭ����
int send_have(peer_have *h_head);			//Inner function
void* reset_pmark();

//����ʱ�õ�����غ���
int compare_info_hash(int info_hash1[], int info_hash2[]);	//�Ƚ�info_hash
int find_peer_id(char peer_id[]);				//Ѱ��peer id�Ƿ��ڸ�Ⱥ����
int hand_shake(int sockfd);			//�������֣��ɹ�����1��ʧ�ܷ���0
void* listen_connection();					//���ܲ���������peer������
void* set_connection(void *peer_list);			//������peer��������
void* keep_alive(void* sockfd);					//ÿ����ʱ����һ��keep alive����
void* judgeTimeOut();						//�жϸ������Ƿ�ʱ
void send_bitfield(int conn);
#endif

