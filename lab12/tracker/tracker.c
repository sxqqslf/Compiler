#include "stdio.h"
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "assert.h"
// ��tracker��c�ļ�
void* process_req(void* q);
//����̣߳�����ظ�peer�Ƿ�����intervalʱ��ظ�����
void* monitor_alive(void* p);


char my_ip[16]; // ��ʽΪXXX.XXX.XXX.XXX, null��ֹ
int my_port; // peer�����Ķ˿ں�
int infohash[5]; // Ҫ�����Ҫ���ص��ļ���SHA1��ϣֵ, ÿ���ͻ���ͬʱֻ�ܴ���һ���ļ�

typedef struct tcp_msg
{
	struct sockaddr* msg;
	int sockfd;
}tcp_msg;
int main(int argc, char **argv)
{

	int listenfd, connfd;
        int clilen, n, mark;
        struct sockaddr_in cliaddr,servaddr;
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
                perror("Problem in creating the socket");
		exit(0);
        }
	
	if(argc != 5)
	{
		printf("arguments: -i <tracker_ip> -p <tracker_port>\n");
		exit(0);
	}
	strncpy(my_ip,argv[2],strlen(argv[2]));
        my_ip[strlen(argv[2])+1] = '\0';

        my_port = atoi(argv[4]);
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr(my_ip);
        servaddr.sin_port = htons(my_port);

        bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

        listen(listenfd, 100000);
	pthread_t thread[100000];
	int t_count = 0;
	while(1)
	{
		struct sockaddr* cliaddr = (struct sockaddr*)malloc(sizeof(struct sockaddr));
		memset(cliaddr, 0, sizeof(struct sockaddr));
		clilen = sizeof(struct sockaddr_in);
                connfd = accept(listenfd, cliaddr, &clilen);
		tcp_msg* m = (tcp_msg*)malloc(sizeof(tcp_msg));
		memset(m, 0, sizeof(tcp_msg));
		m->msg = cliaddr;
		m->sockfd = connfd;
		pthread_create(&thread[t_count++], NULL, process_req, (void*)m);
	}

}

#define BT_STARTED 0
#define BT_COMPLETED 1
#define BT_STOPPED 2

typedef struct peer_node
{
	char* ip;
	int port;
	int current_time;	// �ϴθ�peer���ͱ��ĵ�ʱ��
	int seeder_flag;	// ���Ϊ1�������peerΪseeder, ���򣬴����peer��Ϊseeder
}peer_node;

typedef struct peer
{
	peer_node* this_peer;
	struct peer* next;
	struct peer* prev;
}peer;

#define TABLE_SIZE 1024
peer* peer_table[TABLE_SIZE];


// hash����
unsigned int hash_function(char* name)
{
        unsigned int val = 0, i;
        for(; *name; ++name)
        {
                val = (val << 2) + *name;
                if(i = val & ~0x03ff)
                        val = (val ^ (i >> 8)) & 0x03ff;
        }
        return val;
}

void* process_req(void* q)
{
	printf("enter here\n");
	
	tcp_msg* m = (tcp_msg*)q;
	struct sockaddr_in sin;
        memcpy(&sin, m->msg, sizeof(sin));

	// ��ȡip,�˿�,�׽���
        char ip[16];
        memset(ip, 0, 16);
	int port;
        // ȡ��ip�Ͷ˿ں�
        memcpy(ip, inet_ntoa(sin.sin_addr), strlen(inet_ntoa(sin.sin_addr)));
	int sockfd = m->sockfd;

	char buffer[1000];
	memset(buffer, 0, 1000);
	int n;
	int count = 0;
	while((n = recv(sockfd, &buffer[count++], 1, 0)) > 0)	
	{
	//	printf("%c", buffer[count-1]);
		if(count>= 4 && buffer[count-4] == '\r' && buffer[count-3] == '\n'&& buffer[count-2] == '\r' && buffer[count-1] == '\n')
			break;
		/*
		if(count >= 4)
		{
			if(buffer[count-4] == '\\' && buffer[count-3] == 'r' && buffer[count-2] == '\\'                                                            && buffer[count-1] == 'n')
				break;
		}*/
	}
	printf("RECV: \n%s\n", buffer);
	//��õ�ǰʱ�䣬��д�뵽���Ͷ��ʱ�䵥λΪ΢��
        struct timeval tv;
        gettimeofday (&tv , NULL);
	int current_time = tv.tv_sec;
	// �ҵ�info_hash
	char* p = strstr(buffer, "info_hash");
	int info_hash[5];
	int i;
	char* pure_hash = (char*)malloc(50);
	memset(pure_hash, 0, 50);
	count = 0;
 	p = p + 10;
	while(*p != '&')
	{
		if(*p != '%')
		{
			pure_hash[count++] = *p;
		}
		p++;
	}
	printf("pure_hash: %s\n", pure_hash);
/*	if(strcmp(pure_hash, "lE19Bl87pFBBD1BF741C6811B052C8CFB7D5F") != 0)
		return NULL;
*/	
	// ��ȡport
	p = strstr(buffer, "&port=");
	memset(pure_hash, 0, 50);
	p = p + 6;
	count = 0;
	while(*p >= '0' && *p <= '9')
	{
		pure_hash[count++] = *p;
		p++;
	}
	port = atoi(pure_hash);
	/*
	// ��ȡip
	p = strstr(buffer, "&ip=");
	p = p + 4;
	count = 0;
	while(*p != '&')
	{
		ip[count++] = *p;
		p++;
	}*/
	// ��ȡleft
	int left = 0;
	p = strstr(buffer, "&left=");
        memset(pure_hash, 0, 50);
        p = p + 6;
        count = 0;
        while(*p >= '0' && *p <= '9')
        {
                pure_hash[count++] = *p;
                p++;
        }
        left = atoi(pure_hash);	

	// ��ȡevent
	int event = -1;
	p = strstr(buffer, "&event=");
	if(p != NULL)
	{
		memset(pure_hash, 0, 50);
        	p = p + 7;
        	count = 0;
        	while(*p != ' ')
        	{
                	pure_hash[count++] = *p;
                	p++;
        	}
		if(strcmp(pure_hash, "started") == 0)
			event = BT_STARTED;
		else if(strcmp(pure_hash, "completed") == 0)
			event = BT_COMPLETED;
		else if(strcmp(pure_hash, "stopped") == 0)
			event = BT_STOPPED;
	}
	printf("ip: %s, port: %d, left: %d, event: %d\n", ip, port, left, event);
	// ����event��ִ����Ӧ����
		
	// ���ΪSTARTED����
	if(event == BT_STARTED)
	{
		// ����ip��port����peer����
		// �����peer��leftΪ0����Ǹ�peerΪseeder
		i = hash_function(ip);
                assert(i >= 0 && i < TABLE_SIZE);
                // ȡ�ø�ip��Ӧ���ڵ�hash��λ�õ�ָ��
                peer* t = peer_table[i];	
		int flag = 0;
		while(t != NULL)
		{
			if(strcmp(t->this_peer->ip, ip) == 0 && t->this_peer->port == port)
			{
				flag = 1;
				break;
			}
			t = t->next;
		}
		// ���ip��portû��������������뵽hash����
		if(flag == 0)
		{
			peer* this_p = (peer*)malloc(sizeof(peer));
			memset(this_p, 0, sizeof(peer));
			this_p->this_peer = (peer_node*)malloc(sizeof(peer_node));
			memset(this_p->this_peer, 0, sizeof(peer_node));
			this_p->next = NULL;
			this_p->prev = NULL;
			this_p->this_peer->ip = (char*)malloc(strlen(ip) + 1);
			memset(this_p->this_peer->ip, 0, strlen(ip) + 1);
			memcpy(this_p->this_peer->ip, ip, strlen(ip));
			this_p->this_peer->port = port;
			this_p->this_peer->current_time = current_time;
			if(left == 0)
				this_p->this_peer->seeder_flag = 1;
			// ������뵽hash����
			if(peer_table[i] == NULL)
				peer_table[i] = this_p;
			else
			{
				this_p->next = peer_table[i];
				peer_table[i] = this_p;
			}
			// ����һ���̣߳�����ظ�peer�Ƿ�����intervalʱ��ظ�����
                	pthread_t thread;
                	pthread_create(&thread, NULL, monitor_alive,(void*)this_p);
		}
	}
	// �����STOPPED����, ɾ��ӵ�и�ip��port��peer��Ϣ
	else if(event == BT_STOPPED)
	{
		// ȥhash��
                i = hash_function(ip);
                assert(i >= 0 && i < TABLE_SIZE);
                // ȡ�ø�ip��Ӧ���ڵ�hash��λ�õ�ָ��
                int flag = 0;
                if(peer_table[i] != NULL)
                {
                        if(strcmp(peer_table[i]->this_peer->ip, ip) == 0 &&                                                                                      peer_table[i]->this_peer->port == port)
                                peer_table[i] = peer_table[i]->next;
                        else
                        {
                                peer* pre = peer_table[i];
                                peer* t = peer_table[i]->next;
                                while(t != NULL)
                                {
                                        if(strcmp(t->this_peer->ip, ip) == 0 &&                                                                                                 t->this_peer->port == port)
                                        {
                                                pre->next = t->next;
                                                break;
                                        }
                                        pre = t;
                                        t = t->next;
                                }
                        }
                }
	}
	// �����COMPLETED����,�޸ĸ�peerΪseeder
        else if(event == BT_COMPLETED)
        {
		i = hash_function(ip);
                assert(i >= 0 && i < TABLE_SIZE);
                // ȡ�ø�ip��Ӧ���ڵ�hash��λ�õ�ָ��
                peer* t = peer_table[i];
                int flag = 0;
                while(t != NULL)
                {
                        if(strcmp(t->this_peer->ip, ip) == 0 && t->this_peer->port == port)
                        {
				// ����peer���Ϊseeder
				t->this_peer->seeder_flag = 1;
                                break;
                        }
                        t = t->next;
                }
	}
	// ���Ϊ-1�¼����޸ĸ�peer��keep aliveʱ��
	else if(event == -1)
	{
		i = hash_function(ip);
                assert(i >= 0 && i < TABLE_SIZE);
                // ȡ�ø�ip��Ӧ���ڵ�hash��λ�õ�ָ��
                peer* t = peer_table[i];
                int flag = 0;
                while(t != NULL)
                {
                        if(strcmp(t->this_peer->ip, ip) == 0 && t->this_peer->port == port)
                        {
                                // �޸�ʱ��
                                t->this_peer->current_time = current_time;
                                break;
                        }
                        t = t->next;
                }
	}

	// ���peer���͵���started��keep alive���ģ���Ҫ����һ������
	if(event == -1 || event == BT_STARTED)
	{	
		char msg[1000];
		memset(msg, 0, 600);
		
		count = 0;
		memcpy(msg, "d8:completei", 12);	
		count = count + strlen("d8:completei");

		char peert[400];
		count = 0;
		memset(peert, 0, 400);
		int complete = 0;
		int incomplete = 0;
		int peers = 0;
		peer* t2;
		for(i = 0; i < TABLE_SIZE; i++)
		{
			t2 = peer_table[i];
			while(t2 != NULL)
			{
				// ������Լ����ͼ��뵽peerlist�� // ����seeder��peerΪseeder����Ϣ
				if(strcmp(t2->this_peer->ip, ip) == 0 && t2->this_peer->port == port)
				{
					struct in_addr* p = (struct in_addr*)&peert[count];
                                        inet_aton(t2->this_peer->ip, p);       
                                        count = count + 4;      
                                        uint16_t* q = (uint16_t*)&peert[count];
                                        *q = htons(t2->this_peer->port);
                                        count = count + 2;
					peers++;
				}
				else if(!(t2->this_peer->seeder_flag == 1 && left == 0))
				{
					struct in_addr* p = (struct in_addr*)&peert[count];
					inet_aton(t2->this_peer->ip, p);		
					count = count + 4;
					uint16_t* q = (uint16_t*)&peert[count];
					*q = htons(t2->this_peer->port);
					count = count + 2;
					peers++;
				}
				if(t2->this_peer->seeder_flag == 1)
					complete++;
				else
					incomplete++;
				t2 = t2->next;
			}
			
		}

		//
		int content_count = 0;
                memcpy(msg, "d8:completei", 12);
                content_count = content_count + strlen("d8:completei");
		char integer[10];
		memset(integer, 0, 10);
		sprintf(integer, "%d", complete);
			
		memcpy(&msg[content_count], integer, strlen(integer));
		content_count = content_count + strlen(integer);
		memcpy(&msg[content_count], "e10:downloadedi0e", 17);
		content_count = content_count + 17;
		memcpy(&msg[content_count], "10:incompletei", 14);
		content_count = content_count + 14;
	
		memset(integer, 0, 10);
                sprintf(integer, "%d", incomplete);	
		memcpy(&msg[content_count], integer, strlen(integer));
                content_count = content_count + strlen(integer);
		memcpy(&msg[content_count], "e8:intervali1862e12:min intervali931e5:peers",                                                                    strlen("e8:intervali1862e12:min intervali931e5:peers"));
		content_count = content_count + strlen("e8:intervali1862e12:min intervali931e5:peers");

		memset(integer, 0, 10);
                sprintf(integer, "%d", peers*6);
		memcpy(&msg[content_count], integer, strlen(integer));
                content_count = content_count + strlen(integer);
		msg[content_count++] = ':';
		memcpy(&msg[content_count], peert, count);	
		printf("PEER: \n");
		for(int k = 0; k < count/6; k++)
		{
			char* ip_10 = inet_ntoa(*(struct in_addr*)&peert[k]);
			printf("ip: %s ", ip_10);
			int po = ntohs(*(uint16_t*)&peert[k+4]);
			printf("port: %d", po);
		}
		printf("\n");	
		content_count = content_count + count;

		count = 0;
			
		char head[1000];
		memset(head, 0, 1000);
		int h_count = 0;
		memcpy(&head[h_count], "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: ",                                                      strlen("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "));
		h_count = h_count +  strlen("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: ");
		memset(integer, 0, 10);
                sprintf(integer, "%d", content_count+1);
		memcpy(&head[h_count], integer, strlen(integer));
		h_count = h_count + strlen(integer);
		memcpy(&head[h_count], "\r\n\r\n",strlen("\r\n\r\n"));
		h_count = h_count + strlen("\r\n\r\n");
		memcpy(&head[h_count], msg, content_count);
		h_count = h_count + content_count;
		head[h_count] = 'e';
		h_count++;
		printf("SEND:\n%s\n", head);
		// ���ر���
		int len = send(sockfd, head, h_count, 0);
		assert(len == h_count);
	}

}
int interval = 1683;
//����̣߳�����ظ�peer�Ƿ�����intervalʱ��ظ�����
void* monitor_alive(void* p)
{
	peer* this_p = (peer*)p;
	char* ip = this_p->this_peer->ip;
	while(1)
	{
		// ˯��intervalʱ��
		sleep(interval);
		// ����peer��keep aliveʱ���Ƿ���£����û�и��£��ӱ���ȥ����peer��Ϣ
		//��õ�ǰʱ�䣬��д�뵽���Ͷ��ʱ�䵥λΪ΢��
        	struct timeval tv;
        	gettimeofday (&tv , NULL);
        	int current_time = tv.tv_sec;
		if(current_time - this_p->this_peer->current_time > interval)
		{
			// ȥhash��
			int i = hash_function(ip);
                	assert(i >= 0 && i < TABLE_SIZE);
                	// ȡ�ø�ip��Ӧ���ڵ�hash��λ�õ�ָ��
                	int flag = 0;
			if(peer_table[i] != NULL)
			{
				if(strcmp(peer_table[i]->this_peer->ip, this_p->this_peer->ip) == 0 &&                                                                     peer_table[i]->this_peer->port == this_p->this_peer->port)
					peer_table[i] = peer_table[i]->next;
				else
				{
					peer* pre = peer_table[i];
					peer* t = peer_table[i]->next; 
                			while(t != NULL)
                			{
                        			if(strcmp(t->this_peer->ip, this_p->this_peer->ip) == 0 &&                                                                                  t->this_peer->port == this_p->this_peer->port)
                        			{
							pre->next = t->next;
                                			break;
                        			}
						pre = t;
                        			t = t->next;
                			}
				}
			}

			free(this_p);
			return NULL;
		}	
	}	
}
