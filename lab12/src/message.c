#include "btdata.h"
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
int count = 0;      //��������Ѵ�Ľṹ
int compare_info_hash(int info_hash1[], int info_hash2[])
{
	int i = 0;
	for(; i < 5; i++)
		if(info_hash1[i] != info_hash2[i])
			return 0;
	return 1;
}

int find_peer_id(char peer_id[])
{
	peer_list *head = peer_head;
	while(head->next != NULL)
		if(strncmp(peer_id, head->this_peer.id, 20) == 0)
			return 1;
	return 0;
}

//�������֣��ɹ�����1��ʧ�ܷ���-1
int hand_shake(int sockfd)
{
	handshake* msg = (handshake*)malloc(sizeof(handshake));
	memset(msg, 0, sizeof(handshake));
	msg->len = 19;
	strcpy(msg->name, "BitTorrent protocol");
	memset(msg->reserved, 0, 8);
//	memcpy(msg->info_hash, g_infohash, 20);
	int j;
      	for(j = 0; j <= 4; j++)
               	msg->info_hash[j] = htonl(g_infohash[j]);
	memcpy(msg->peer_id, g_my_id, 20);
	return send(sockfd, (char*)msg, sizeof(handshake), 0);
}

//�������ӣ�������Ϣ
void* listen_connection()
{
	//printf("enter listen_connection\n");
	int piece_len = g_torrentmeta->piece_len;
	int listenfd, connfd;
	int clilen, n, mark;
	struct sockaddr_in cliaddr,servaddr;
	char buf[HANDSHAKE_LEN];
	int info_hash[5];
	char peer_id[20];
	pthread_t thread[MAX_PEERS];
	int i;

	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Problem in creating the socket");
	//	exit(2);
		return NULL;
	}
	
	servaddr.sin_family = AF_INET;
//	servaddr.sin_addr.s_addr = inet_addr(INADDR_ANY);
	servaddr.sin_addr.s_addr = inet_addr(g_my_ip);
	servaddr.sin_port = htons(g_peerport);

	bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	listen(listenfd, LISTEN_Q);

	while(1)
	{
		mark = 0;		//mark������־�����ĸ�ѭ��
		clilen = sizeof(cliaddr);
		//printf("Before accept!\n");
		connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
//		printf("#######cliaddr.sin_addr.s_addr = %x\n", cliaddr.sin_addr.s_addr);
		//printf("After accept!\n");
		
		if((n = recv(connfd, buf, HANDSHAKE_LEN, 0)) > 0)
		{
			if(buf[0] == 19)	//���յ�������Ϣ
			{
				if(count < MAX_PEERS)
				{
					//printf("whole handshake data:\n");
					/*
					for(int j = 0; j < HANDSHAKE_LEN; j++)
						printf("%02x", buf[j]);
					printf("\n");*/
					//�ж�info hash��peer id
					//�ж�info hash��peer id
					int j;
                                        for(j = 0; j <= 4; j++)
                                                info_hash[j] = ntohl(*(int*)(buf+j*4+28));
					//memcpy(info_hash, buf+28, 20);
					memcpy(peer_id, buf+48, 20);
					/*
					printf("my hash:\n");
					for(int j = 0; j < 20; j++)
					{
						char* p = (char*)g_infohash;
						printf("%02x", p[j]);
					}
					printf("\nhis hash:\n");
                                        for(int j = 0; j < 20; j++)
                                        {
                                                char* p = (char*)info_hash;
                                                printf("%02x", p[j]);
                                        }
					printf("\n");*/
					if(compare_info_hash(info_hash, g_infohash) == 0)	//info_hash����ȷ
					{
					//	printf("wrong info hash\n");
						close(connfd);
						mark = 1;
					}
			/*		else if(find_peer_id(peer_id) == 0)	//peer_id����ȷ 
					{
						printf("wrong peer id\n");
						close(connfd);
						mark = 1;
					}*/
					else		//��Ӧ������Ϣ�������뱾�ر���
					{
						for(i = 0; i < MAX_PEERS; i++)
						{
							if(peer_pool[i] == NULL)	//���뵽�����б�
							{
								mark = 1;
								memcpy(buf+48, g_my_id, 20);
								send(connfd, buf, HANDSHAKE_LEN, 0);	//��Ӧ������Ϣ
								pthread_mutex_lock(&send_recv_mutex);		//lock
								peer_pool[i] = (peer_t*)malloc(sizeof(peer_t));
								peer_pool[i]->sockfd = connfd;
								peer_pool[i]->peer_mark = (char *)malloc(sizeof(char) * g_num_pieces);

								memset(peer_pool[i]->peer_mark, 0 ,sizeof(char) * g_num_pieces);
								//��õ�ǰʱ�䣬��д�뵽���Ͷ��ʱ�䵥λΪ΢��
								struct timeval tv;
								gettimeofday (&tv , NULL);
//								peer_pool[i]->send_time = tv.tv_sec * 1000000 + tv.tv_usec;
								peer_pool[i]->send_time = tv.tv_sec;
								peer_pool[i]->choking = 1;
								peer_pool[i]->interested = 0;
								peer_pool[i]->choked = 1;
								peer_pool[i]->have_interest = 0;
								peer_pool[i]->download_count = 0;
								pthread_mutex_unlock(&send_recv_mutex);		//unlock

								//���̷߳���keep alive����
								//����bitfield
								/*
                                                                if(seeder == 1){
                                                                        send_bitfield(connfd);
                                                                }*/
								send_bitfield(connfd);
                                                                int* connfd_cpy = (int *)malloc(sizeof(int));
                                                                *connfd_cpy = connfd;
                                                                //���̷߳���keep alive����
                                                                pthread_create(&thread[i], NULL, keep_alive, (void *)connfd_cpy);

								//���̴߳�������
								pthread_create(&thread[i], NULL, recv_handler, (void *)peer_pool[i]);
								count++;
								break;
								//recv_handler(peer_pool[i], piece_len);
							}
						}
						if(mark == 0)
							close(connfd);
					}
				}
			}
		}
		/*
		if(n < 0 )
			printf("receive end\n");*/
	}
}

//�������ӣ���peer list��ĳ�Ա�ֱ���handshake
void* set_connection(void *peer)	//��Ҫ�õ�info hash��peer list������ɴ���tracker�˵ĺ�������
{
//	printf("enter set_connection\n");
	peer_list* peerlist = (peer_list*)peer;
	int piece_len = g_torrentmeta->piece_len;
	int sockfd, i;
	struct sockaddr_in servaddr;
	char buf[HANDSHAKE_LEN];
	pthread_t thread[MAX_PEERS];

	//��peer list��ѡ��peer��������
	peer_list *head = peerlist;
	int flag = 0;
	while(g_downloaded < g_filelen)	//���peer list������β�������������Ѿ�������������ƣ���ֹͣ
	{
		if(head == NULL)
			head = peerlist;
		if(count >= MAX_PEERS)
		{
			printf("FULL\n");
		//	sleep(5*60);
			sleep(20);
			while(count >= MAX_PEERS)
				;
			continue;	
		}
		if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
		//	perror("Problem in creating the socket");
		//	exit(2);
		//	sleep(5*60);
			sleep(20);
                        while(count >= MAX_PEERS)
                                ;
                        continue;
		}
		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = inet_addr(head->this_peer.ip);
		servaddr.sin_port = htons(head->this_peer.port);

	//	printf("set_connection: ip: %s, port: %d\n", head->this_peer.ip, head->this_peer.port);

		if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
		{
		//	printf("Prolem in connecting to the peer\n");
			head = head->next;
			close(sockfd);
		//	sleep(5*60);
			sleep(20);
                        while(count >= MAX_PEERS)
                                ;
                        continue;
			
		}
		if(count >= MAX_PEERS)
                {
			close(sockfd);
		//	sleep(5*60);
			sleep(20);
                        while(count >= MAX_PEERS)
                                ;
                        continue;
                }
		sleep(1);
		if(hand_shake(sockfd) == -1)	//����handshake��Ϣ
		{
			close(sockfd);
		//	sleep(5*60);
			sleep(20);
                        while(count >= MAX_PEERS)
                                ;
                        continue;
			//printf("fail to send handshake message\n");
		//	head = head->next;
//			break;
		}
		else		//�������ӳɹ����ȴ��Է���Ӧ������Ϣ
		{
			//��������
		//	printf("before handshake recv\n");
//			while(recv(sockfd, buf, HANDSHAKE_LEN, 0) > 0)
			if(recv(sockfd, buf, HANDSHAKE_LEN, 0) <= 0)
			{
			//	printf("no handshake back\n");
				close(sockfd);
			//	sleep(5*60);
				sleep(20);
                        	while(count >= MAX_PEERS)
                                	;
                        	continue;
			}			
			else
			{
				if(buf[0] == 19)	//���յ��Է���Ӧ��������Ϣ
				{
					//��ʱ��û���ٽ����ж�������Ϣ�Ƿ���ȷ������ֱ�Ӽ��ڱ����б���
					if(count < MAX_PEERS)
					{
						for(i = 0; i < MAX_PEERS; i++)
						{
							if(peer_pool[i] == NULL)
							{
								pthread_mutex_lock(&send_recv_mutex);		//lock
								peer_pool[i] = (peer_t*)malloc(sizeof(peer_t));
								peer_pool[i]->sockfd = sockfd;
								peer_pool[i]->peer_mark = (char *)malloc(sizeof(char) * g_num_pieces);
								
								memset(peer_pool[i]->peer_mark, 0 ,sizeof(char) * g_num_pieces);
							//	printf("##listen: create peer_pool[%d]", i);
								//��õ�ǰʱ�䣬��д�뵽���Ͷ��ʱ�䵥λΪ΢��
								struct timeval tv;
								gettimeofday (&tv , NULL);
//								peer_pool[i]->send_time = tv.tv_sec * 1000000 + tv.tv_usec;
								peer_pool[i]->send_time = tv.tv_sec;
								peer_pool[i]->choking = 1;
								peer_pool[i]->interested = 0;
								peer_pool[i]->choked = 1;
								peer_pool[i]->have_interest = 0;
								peer_pool[i]->download_count = 0;
								peer_pool[i]->cancel = 0;
								pthread_mutex_unlock(&send_recv_mutex);		//unlock
								//printf("prepare to create keep_alive and recvhandler\n");
								//���̷߳���keep alive����
								//����bitfield
								/*
                                                                if(seeder == 1){
                                                                        send_bitfield(sockfd);
                                                                }*/
								send_bitfield(sockfd);
                                                                //���̷߳���keep alive����
                                                                int *sockfd_cpy = (int*)malloc(sizeof(int));
                                                                *sockfd_cpy = sockfd;

								//printf("before open keep_alive\n");
                                                                pthread_create(&thread[i], NULL, keep_alive, (void *)sockfd_cpy);
								
							
								//printf("after open keep_alive\n");
								
								
								//���̴߳�������
								//printf("before open recv_handler\n");
								pthread_create(&thread[i], NULL, recv_handler, (void *)peer_pool[i]);
								//printf("after open recv_handler\n");

					
								//printf("open keep_alive and recvhandler for %s, sockfd: %d\n", head->this_peer.ip, sockfd);
								count++;
							//			recv_handler(peer_pool[i], piece_len);
								break;
							}
						}
						while(i < MAX_PEERS)
						{
						//	sleep(5*60);
							sleep(20);
							int j;
							for(j = 0; j < MAX_PEERS; j++)
							{
								if(peer_pool[j] != NULL && peer_pool[j]->sockfd == sockfd)
									break;
							}	
							if(j == MAX_PEERS)
								break;
						}
						while(count >= MAX_PEERS)
                                                        ;
                                                continue;
					}
					else		//������������
					{
						//printf("full connections\n");
						close(sockfd);
                                	//	sleep(5*60);
						sleep(20);
                                		while(count >= MAX_PEERS)
                                        		;
                                		continue;
					//	break;
					//	break;
					}
				}
			}
		}
	//		return NULL;
//		break;
	}

//	exit(0);
}

//ÿ����ʱ����һ��keep alive����
void* keep_alive(void* sock)
{
	//printf("enter keep_alive\n");
	int sockfd = *(int*)sock;
	int i = 0;
	unsigned int len = htonl(0);
	int flag = 0;
	while(flag == 0)
	{
//		printf("!!!!!flag: %d\n", flag);
		for(i = 0; i < MAX_PEERS; i++)
		{
//			if(peer_pool[i] != NULL && peer_pool[i]->sockfd != sockfd)	//��������Ա����keep alive����
			if(peer_pool[i] != NULL && peer_pool[i]->sockfd == sockfd)      //��������Ա����keep alive����
			{
				if(send(peer_pool[i]->sockfd, (void*)&len, 4, 0) <= 0)
				{
					if(peer_pool[i]->sockfd >= 0)
					{
                                                close(peer_pool[i]->sockfd);
						peer_pool[i]->sockfd = -1;
					}
					peer_pool[i] = NULL;
					count--;
					//printf("disconnect from sockfd: %d\n", sockfd);
					flag = 1;			
				}
				else
					//printf("!!!!keep alive from sockfd: %d\n", sockfd);
	
				break;
			}
		}
		if(flag == 0)
			usleep(KEEP_ALIVE_INTERVAL * 1000000);		//�ȴ�KEEP_ALIVE_INTERVALʱ��
	}
}

//�жϸ������Ƿ�ʱ
void* judgeTimeOut()
{
	//��õ�ǰʱ�䣬��д�뵽���Ͷ��ʱ�䵥λΪ΢��
	struct timeval tv;
	gettimeofday (&tv , NULL);
//	int current_time = tv.tv_sec * 1000000 + tv.tv_usec;
	int current_time;
        while(!g_done)
        {
		int i = 0;
		for(i = 0; i < MAX_PEERS; i++)
		{
			if(peer_pool[i] != NULL)
			{
				current_time = tv.tv_sec;
//				printf("!!!!!current_time: %d\n", current_time);
//				printf("!!!!g_done : %d\n", g_done);
//				if(current_time - peer_pool[i]->send_time >= WAIT_TIME * 1000000)	//��ʱ
//				printf("!!!!time difference : %d ,and peer num : %d\n", current_time-peer_pool[i]->send_time, i);
//				printf("!!!!wait time:%d\n", WAIT_TIME/2);
//				printf("!!!value:%d\n", current_time - peer_pool[i]->send_time >= WAIT_TIME/2);
				if(current_time - peer_pool[i]->send_time >= WAIT_TIME)
				{
					if(peer_pool[i]->sockfd >= 0)
					{
						close(peer_pool[i]->sockfd);
						peer_pool[i]->sockfd = -1;
					}
					count--;
					//printf("time out(sockfd: %d)\n", peer_pool[i]->sockfd);
					peer_pool[i] = NULL;
				}
			}
		}
		usleep(WAIT_TIME * 1000000);

	}
}

void send_bitfield(int conn){
		unsigned int bit_length = g_num_pieces/8;
		
		if(g_num_pieces%8 != 0){
			bit_length++;
		}

		unsigned int head_length = htonl(1 + bit_length);
        unsigned char head_id = 5;

        char* sendline = (char*)malloc(sizeof(char) * (sizeof(unsigned int) + ntohl(head_length)));
		memcpy(sendline, &head_length, sizeof(unsigned int));
		memcpy(sendline+4, &head_id, sizeof(unsigned char));
        int i;
        for(i=0; i<g_num_pieces; i++){
			sendline[i/8+5] = sendline[i/8+5] | ((p_mark[i] == 2)?(0x80 >> i%8):0x00);
        }
	//printf("biffield length:%d pre:%d\n", ntohl(head_length), head_length);
        p2p_send(sendline, conn, ntohl(head_length) + sizeof(head_length));

	//printf("send biffield\n");
        free(sendline);
}


