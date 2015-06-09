
#define REQUEST_TIME 5
#define MAX_PEERS 5
#define REQUEST_LEN 16384

//peer communication Header
typedef struct peer_wire_head{
	unsigned int length;
	unsigned char id;
}peer_wire_h;

typedef struct peer_wire_have{
	peer_wire_h head;
	unsigned int piece_index;
}peer_have;

typedef struct peer_wire_reqeust{
	peer_wire_h head;
	unsigned int index;
	unsigned int begin;
	unsigned int length;
}peer_request;

typedef struct peer_wire_piece_head{
	peer_wire_h head;
	unsigned int index;
	unsigned int begin;
}peer_piece_h;

typedef struct peer_wire_cancel{
	peer_wire_h head;
	unsigned int index;
	unsigned int begin;
	unsigned int length;
}peer_cancel;

//Send&Receive Data
int p2p_send(char *buffer, int conn, int len);			//-1: Fail 1: Succeed
int p2p_recv(char *buffer, int conn, int len);			//-1: Fail 1: Succeed

//Thread
void set_connection();
void listen_connection();
void recv_handler();
void set_request();
void keep_alive();

//Inner function
int send_have(peer_have *h_head, int conn);
