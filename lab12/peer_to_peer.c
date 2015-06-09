#include "btdata.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "assert.h"
#include "util.h"

#define MAX_LINE 262144			//256KB
//发送REQUEST，该函数只是用于启动每一个分片第一个子分片下载
//但是该分片的后续子分片下载由handler函数进行处理
void* set_request(){			//同set_connection,listen_connection一起创建
//	printf("enter set_request\n");
//	int piece_len = g_torrentmeta->piece_len;
//	sleep(5);
	while(1){
//		sleep(5);
		int i;
		for(i=0; i<g_num_pieces; i++){
//			usleep(2000*1000);
			if(p_mark[i] == 0){
//				sleep(5);
				int j;
				for(j=0; j<MAX_PEERS; j++){
					if(peer_pool[j] == NULL){
						continue;
					}
					//printf("peer_pool[%d]\n", j);
					assert(peer_pool[j]->peer_mark != NULL);
//					printf("##set request: peer_pool[%d]: peer_mark[%d]: %02x\n", j,i, peer_pool[j]->peer_mark[i]);
					if(peer_pool[j]->choked != 1 && peer_pool[j]->have_interest == 1 && peer_pool[j]->peer_mark[i] == 2 && peer_pool[j]->download_count < DOWNLOAD_LIMIT){
//						pthread_mutex_lock(&send_recv_mutex);		//上锁
						peer_request x;
						assert(g_filelen - i*piece_len >= 0);			//不可能小于0，如果出现告诉土豪
						unsigned int length = g_filelen - i*piece_len;		//找出该分片的大小
						length = (length < REQUEST_LEN)?length:REQUEST_LEN;	//子分片大小选择，如果小于16KB则选其长度
					//	printf("In set_request:: The len is %d\n", length);
						unsigned int head_length = htonl(13);
						unsigned char head_id= 6;
						x.index = htonl(i);
						x.begin = htonl(0);
						x.length = htonl(length);				
	//					printf("::::set request: The index is %d\n", ntohl(x.index));
						char *sendline = (char *)malloc(sizeof(char) * (sizeof(unsigned int) + sizeof(unsigned char) + sizeof(x)));
						//printf("The sizeof(x) %d\n ________________\n", sizeof(x));
						memcpy(sendline, &head_length, sizeof(head_length));
						memcpy(sendline+4, &head_id, sizeof(head_id));
						memcpy(sendline+5, &x, sizeof(x));
	//					for(int i=0; i<17; i++){
	//						printf("  %02x  ", sendline[i]);
	//					}
						//printf("\n__________________\n");
	//					printf("::::set request: the p2p_send size is %d\n", ntohl(head_length) + sizeof(unsigned int));
						if(p2p_send(sendline, peer_pool[j]->sockfd, ntohl(head_length) + sizeof(unsigned int)) == -1){
							free(sendline);
							continue;							//发送不成功，另外查找有该分片的
						}

						peer_pool[j]->download_count++;
						
	//					unsigned int x2 = htonl(13);
	//					char* buf = (char*)&x2;
	//					printf("::The request_length needed is %d\n", ntohl(x.length));
	//					printf("::The sendline about x.length %02x %02x %02x %02x\n", sendline[13], sendline[14], sendline[15], sendline[16]);
	//					printf("::After request:: len%d pre: %d\n", ntohl(head_length), head_length);
	//					printf("::buf: %02x %02x %02x %02x \n", buf[0], buf[1], buf[2], buf[3]);
						free(sendline);							
						
						// 最后阶段
			//			assert(g_filelen - g_downloaded >= 136866*20);
/*						if(g_filelen - g_downloaded >= 136866*20)
						break;*/									//发送成功，查找下一个需要下载的分片
//					pthread_mutex_unlock(&send_recv_mutex);                 //解锁
					break;
					}

				}
//				pthread_mutex_unlock(&send_recv_mutex);			//解锁
			//	sleep(REQUEST_TIME);							//暂定这样操作
				//if(peer_poll[j] != NULL)						//下面说法不行，万一都没该分片则出错了
				//	break;										//每次sleep前只发一个Request
			}
		}
//		sleep(REQUEST_TIME);
	}
}
//处理接收数据
void* recv_handler(void* pee){

	//printf("enter recv_handler\n");
	peer_t* peer = (peer_t*)pee;
//	int piece_len = g_torrentmeta->piece_len;
	int conn;
	conn = peer->sockfd;
/*	if(conn > MAX_Conn)
		MAX_Conn = conn;
*/	char buffer[MAX_LINE];
	char sendline[MAX_LINE];
	while(1){
		memset(buffer, 0, MAX_LINE);
		memset(sendline, 0, MAX_LINE);
	//	pthread_mutex_lock(&send_recv_mutex);	//上锁
		unsigned int len;
		if(p2p_recv((char *)&len, conn, sizeof(len)) == -1){
			//printf("::recv_handler: Fail to recv from (%d)\n", conn);
		//	continue;
		//	pthread_mutex_unlock(&send_recv_mutex);
			break;
		}
		
		len = ntohl(len);
	//	printf("##len:%d\n", len);
		struct timeval tv;
		gettimeofday(&tv, NULL);
		peer->send_time = tv.tv_sec;
//		printf("!!!!peer->send_time: %d\n ", tv.tv_sec);
		if(len != 0){
			assert(len > 0);
			assert(len <= REQUEST_LEN + sizeof(unsigned char) + sizeof(peer_piece_h));
			//printf("::recv_handler: len!=0 The len is %d\n", len);
	//		char *buffer = (char *)malloc(sizeof(char) * len);
			if(p2p_recv(buffer, conn, len) == -1){
				//printf("::recv_handler: Fail to recv len(%d) from (%d)\n", len, conn);
			//	continue;
			//	pthread_mutex_unlock(&send_recv_mutex);
				break;
			}
//			pthread_mutex_lock(&send_recv_mutex);			//上锁
			switch(buffer[0]){				//buffer[0]是id
				case 0:{					//choke
					//printf("::recv_handler: recv choke\n");
					peer_wire_h head;
					head.length = len;
					head.id = buffer[0];
				//	free(buffer);
					peer->choked = 1;
					break;
				}
				case 1:{					//unchoke
					//printf("::recv_handler: recv unchoke\n");
					peer_wire_h head;
					head.length = len;
					head.id = buffer[0];
				//	free(buffer);
					peer->choked = 0;
					break;
				}
				case 2:{					//interested
					//printf("::recv_handler: recv interested\n");
					peer_wire_h head;
					head.length = len;
					head.id = buffer[0];
				//	free(buffer);
					peer->interested = 1;
					peer->choking = 0;
					// 给对方发送一个unchoke包
					char* h2 = (char*)malloc(5);
					memset(h2, 0, 5);
					int* x = (int*)h2;
					*x = htonl(1);
					h2[4] = 1;
					if(p2p_send((char*)h2, conn, 5) < 0)
						;//printf("fail to send unchoke\n");
					else
						;//printf("succeed to send unchoke\n");
					break;
				}
				case 3:{					//not interested
					//printf("::recv_handler: recv not interested\n");
					peer_wire_h head;
					head.length = len;
					head.id = buffer[0];
				//	free(buffer);
					peer->interested = 0;
					break;
				}
				case 4:{					//have
					//printf("::recv_handler: recv have\n");
					peer_have h_head;
					h_head.head.length = len;
					h_head.head.id = buffer[0];

					memcpy(&h_head.piece_index, &buffer[1], sizeof(h_head.piece_index));
					h_head.piece_index = ntohl(h_head.piece_index);
					unsigned int i = h_head.piece_index;
					peer->peer_mark[i] = 2;
					if(p_mark[i] == 0){
						unsigned int head_length = htonl(1);
						unsigned char head_id = 2;
					//	char* sendline = (char*)malloc(sizeof(unsigned char) + sizeof(unsigned int));
						memcpy(sendline, &head_length, sizeof(unsigned int));
						memcpy(sendline+4, &head_id, sizeof(char));
						//printf("!!Send_intereseted: %02x %02x %02x %02x %02x\n", sendline[0], sendline[1], sendline[2], sendline[3], sendline[4]);
						
						peer->have_interest = 1;
						
						p2p_send(sendline, conn, 5);
				//		free(sendline);
					}
					break;
				}
				case 5:{					//bitfield
					//printf("::recv_handler: recv bitfield\n");
					peer_wire_h head;
					head.length = len;
					head.id = buffer[0];
					unsigned char *temp = &buffer[1];
					int i;
					for(i=0; i<g_num_pieces; i++){
						peer->peer_mark[i] = ((temp[i/8] & (0x80>>(i%8)))==(0x80>>(i%8)))?2:0;
						//printf("peer->peer_mark[i] is %d\n", peer->peer_mark[i]);
					}
					int j;
					for(j=0; j<g_num_pieces; j++){
						if(peer->peer_mark[j] == 2 && p_mark[j] == 0){		//存在所需分片，发送interested
							unsigned int head_length = htonl(1);
							unsigned char head_id = 2;
					//		char* sendline = (char*)malloc(sizeof(unsigned char) + sizeof(unsigned int));
							memcpy(sendline, &head_length, sizeof(unsigned int));
							memcpy(sendline+4, &head_id, sizeof(char));
							//printf("Send interested\n");
							peer->have_interest = 1;

							p2p_send(sendline, conn, 5);
			//				free(sendline);
							break;
						}
					}
					if(j == g_num_pieces){			//无所需分片，发送not interested
						unsigned int head_length = htonl(1);
						unsigned char head_id = 3;
			//			char* sendline = (char*)malloc(sizeof(unsigned char) + sizeof(unsigned int));
						memcpy(sendline, &head_length, sizeof(unsigned int));
						memcpy(sendline+4, &head_id, sizeof(char));
						//printf("Send not interested\n");
						peer->have_interest = 0;

						p2p_send(sendline, conn, 5);
			//			free(sendline);
						break;
					}
					break;
				}
				case 6:{					//request
					//printf("::recv_handler: recv request\n");
					peer_wire_h head;
					peer_request r_head;
					head.length = len;
					head.id = buffer[0];
	//				printf("::recv_request:: head.length %d\n", head.length);
	//				printf("::Buffer length in recv_request: %02x %02x %02x %02x", buffer[9], buffer[10], buffer[11], buffer[12]);
					memcpy(&r_head.index, &buffer[1], sizeof(r_head.index));
					memcpy(&r_head.begin, &buffer[5], sizeof(r_head.begin));
					memcpy(&r_head.length, &buffer[9], sizeof(r_head.length));
					r_head.index = ntohl(r_head.index);
					r_head.begin = ntohl(r_head.begin);
					r_head.length = ntohl(r_head.length);
					g_uploaded = g_uploaded + r_head.length;
	//				printf("The r_head.length is %d\n", r_head.length);
					int index = r_head.index;

					if(p_mark[index] == 2 && peer->choking == 0){
						unsigned int head_length;
						unsigned char head_id;
						peer_piece_h p_head;
		//				char *sendline;
						char *block;

						head_length = htonl(sizeof(p_head) + sizeof(unsigned char) + r_head.length);
						//printf("::recv_request:: send piece length is %d\n", ntohl(head_length));
						head_id = 7;
						p_head.index = htonl(index);				
						p_head.begin = htonl(r_head.begin);
						

						//printf("::recv_handler: The p_head_index is %d\n", ntohl(p_head.index));

						block = (char *)malloc(sizeof(char) * r_head.length);
						memcpy(block, &Buffer[piece_len * index + r_head.begin], r_head.length);			
						
		//				sendline = (char *)malloc(sizeof(char) * (r_head.length + sizeof(p_head) + sizeof(unsigned int) + sizeof(unsigned char)));
						memcpy(sendline, &head_length, sizeof(unsigned int));
						memcpy(sendline+4, &head_id, sizeof(char));
						memcpy(sendline+5, &p_head, sizeof(p_head));
						memcpy(&sendline[5+sizeof(p_head)], block, r_head.length);
						//printf("::recv_handler: the length of send_piece is %d\n", sizeof(unsigned int)+sizeof(unsigned char)+sizeof(p_head)+r_head.length);

						p2p_send(sendline, conn, sizeof(unsigned int) + sizeof(unsigned char) + sizeof(p_head) + r_head.length);
		//				free(block);
		//				free(sendline);
					}
					break;
				}
				case 7:{					//piece
					//printf("::recv_handler: recv piece\n");
					peer_wire_h head;
					peer_piece_h p_head;
					unsigned int data_len;
					head.length = len;
				//	printf("::recv_piece::head.length %d\n", head.length);
					head.id = buffer[0];
					memcpy(&p_head.index, &buffer[1], sizeof(p_head.index));
					memcpy(&p_head.begin, &buffer[5], sizeof(p_head.begin));
					p_head.index = ntohl(p_head.index);
					p_head.begin = ntohl(p_head.begin);
					//printf(":::::The piece_index is %d\n", p_head.index);
					//printf(":::::The subpiece_index is %d\n", p_head.begin);
					data_len = len - sizeof(p_head) - sizeof(char);
					g_downloaded = g_downloaded + data_len;
					//printf("The datalen is %d\n", data_len);

					p_mark[p_head.index] = 1;

		//			char *block = (char *)malloc(sizeof(char) * data_len);
		//			memcpy(block, &buffer[9], data_len);
					
					unsigned int index = p_head.index * piece_len + p_head.begin;
					memcpy(&Buffer[index], &buffer[9], data_len);			//数据存入缓冲区

		//			free(block);
					
					if(p_head.begin + data_len == piece_len){		//完成该分片
						//printf(":::::The piece (%d) was completed\n", p_head.index);
						p_mark[p_head.index] = 2;			
						peer->download_count--;
						assert(peer->download_count >= 0);
						//发送have分片
						peer_have h_head;
						h_head.head.length = htonl(5);
						h_head.head.id = 4;
						h_head.piece_index = htonl(p_head.index);

						send_have(&h_head);

						// 判断文件是否接受完毕
						int i;
						for(i=0; i<g_num_pieces; i++){
							if(p_mark[i] != 2)
								break;
						}
						if(i == g_num_pieces){
							//printf("The file was completed\n");
							seeder = 1;
							 // 将接收好的数据写入recv.txt文件中去
                                                        FILE *f;
                                                        char filename[6] = "recv_";
                                                        strcat(filename, g_torrentmeta->name);
                                                        f = fopen(filename,"w");
                                                        assert(f!=NULL);
                                                        fwrite(Buffer,g_filelen,1,f);
                                                        fclose(f);
							
							// 向tracker发送completed报文
                                                        int mlen;
        						char* MESG = make_tracker_request(BT_COMPLETED,&mlen);
        						printf("##COMPLETED:\n");
        						for(int i=0; i<mlen; i++)
                						printf("%c",MESG[i]);
        						int sockfd = connect_to_host(g_tracker_ip, g_tracker_port);
        						if(sockfd < 0)
        						{
                				//		printf("fail to connect to tracker\n");
        						}
        						if(send(sockfd, MESG, mlen, 0) <= 0)
                						printf("fail to send completed data to tracker\n");
        						else
                						printf("succeed to send completed data to tracker\n");
						}
						break;
					}
					if(index + data_len == g_filelen){				//完成尾部分片，且为部分片长度小于piece_length
					//	printf(":::::The piece (%d) was completed\n", p_head.index);
						p_mark[p_head.index] = 2;	
						peer->download_count--;
						assert(peer->download_count >= 0);
						//发送have分片
						peer_have h_head;
						h_head.head.length = htonl(5);
						h_head.head.id = 4;
						h_head.piece_index = htonl(p_head.index);

						send_have(&h_head);
						
						// 判断文件是否接受完毕
                                                int i;
                                                for(i=0; i<g_num_pieces; i++){
                                                        if(p_mark[i] != 2)
                                                                break;
                                                }
                                                if(i == g_num_pieces){
                                                        //printf("The file was completed\n");
                                                        seeder = 1;
							 // 将接收好的数据写入recv.txt文件中去
                                                	FILE *f;
							char filename[6] = "recv_";
							strcat(filename, g_torrentmeta->name);
                                                	f = fopen(filename,"w");
                                                	assert(f!=NULL);
							fwrite(Buffer,g_filelen,1,f);	
                                                	fclose(f);
							
							// 向tracker发送completed报文
							int mlen;
							char* MESG = make_tracker_request(BT_COMPLETED,&mlen);
                                                        printf("##COMPLETED:\n");
                                                        for(int i=0; i<mlen; i++)
                                                                printf("%c",MESG[i]);
                                                        int sockfd = connect_to_host(g_tracker_ip, g_tracker_port);
                                                        if(sockfd < 0)
                                                        {
                                                                printf("fail to connect to tracker\n");
                                                        }
                                                        if(send(sockfd, MESG, mlen, 0) <= 0)
                                                                printf("fail to send completed data to tracker\n");
                                                        else    
                                                                printf("succeed to send completed data to tracker\n");
                                                }
						break;
					}
					//尚未完成该分片，发送request索要下一个子分片
					peer_request r_head;
					assert(g_filelen - piece_len*p_head.index - data_len - p_head.begin >= 0);		//如果报错联系我
					unsigned int length = g_filelen - piece_len * p_head.index - data_len - p_head.begin;
					length = (length>REQUEST_LEN)?REQUEST_LEN:length;
					unsigned int head_length = htonl(13);
					unsigned char head_id = 6;
					r_head.index = htonl(p_head.index);
					r_head.begin = htonl(p_head.begin + data_len);
					r_head.length = htonl((length>REQUEST_LEN)?REQUEST_LEN:length);
					//printf("::recv_handler the next_request length is %d\n", sizeof(unsigned int)+sizeof(unsigned char)+sizeof(r_head));
					//printf("r_head.length: %d\n",ntohl(r_head.length));
			//		char* sendline  = (char*)malloc(sizeof(unsigned int) + sizeof(unsigned char) + sizeof(r_head));
					memcpy(sendline, &head_length, sizeof(unsigned int));
					memcpy(sendline+4, &head_id, sizeof(unsigned char));
					memcpy(sendline+5, &r_head, sizeof(r_head));
					//printf("::::::send the request, begin is %d\n", p_head.begin + data_len);
					p2p_send(sendline, conn, sizeof(unsigned int) + sizeof(unsigned char) + sizeof(r_head));

					break;
				}
				case 8:{			//cancel
					//printf("recv cancel\n");
		/*			peer_cancel c_head;
					c_head.head.length = len;
					c_head.head.id = buffer[0];
					memcpy(&c_head.index, &buffer[1], sizeof(c_head.index));
					memcpy(&c_head.begin, &buffer[5], sizeof(c_head.begin));
					memcpy(&c_head.length, &buffer[9], sizeof(c_head.length));
					c_head.index = ntohl(c_head.index);
					c_head.begin = ntohl(c_head.begin);
					c_head.length = ntohl(c_head.length);
		*/

					break;
				}
			}
//			pthread_mutex_unlock(&send_recv_mutex);

	//		free(buffer);
		}
	//	pthread_mutex_unlock(&send_recv_mutex);
	}
	//printf("Ready to close the sockfd\n");
	if(peer->sockfd >= 0)
	{
		//printf("Close the sockfd\n");
		close(peer->sockfd);
		peer->sockfd = -1;
	}	
}


//Peer 之间发送数据
int p2p_send(char *buffer, int conn, int len){
//	char *buf = (char *)malloc(sizeof(char) * len);
//	memcpy(buf, buffer, len);
	int n;
	int left_len = len;
	int send_len = 0;
	while(send_len < len){
		if((n=send(conn, buffer+send_len, left_len, 0)) <= 0){
			//printf("Fail to Send to (%d)\n", conn);
		//	free(buf);
			return -1;
		}
		left_len -= n;
		send_len += n;
	}
	assert(send_len == len);
//	free(buf);
	return 1;
}

//Peer 之间接收数据
int p2p_recv(char *buffer, int conn, int len){
	//printf("len: %d\n", len);
//	char *buf = (char *)malloc(sizeof(char) * len);
	//sleep(2);
/*	fd_set fDR;
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 10000;
*/	int recv_len = 0;
	int left_len = len;
	int n;

	//printf("!!!!Before while\n");
	while(recv_len < len){
		if((n = recv(conn, buffer+recv_len, left_len, 0)) <=0 ){
			//printf("Fail to Recv from (%d)\n", conn);
//			free(buf);
			return -1;
		}
		recv_len += n;
		left_len -= n;
		//printf("the left_len is %d and n is %d\n", left_len, n);
		assert(left_len >= 0);
	}
	//printf("After while\n");
	
	//usleep(RECV_SLEEP);
/*	for( ; ;){
		FD_ZERO(&fDR);
		FD_SET(conn, &fDR);
		int ret = select(MAX_Conn+1, &fDR, NULL, NULL, &timeout);
		if(ret <= 0)
			continue;
		else{
			if(FD_ISSET(conn, &fDR)){
				if((n=recv(conn, buffer, len, 0)) <= 0){
					printf("Fail to Recv from (%d)\n", conn);
					free(buf);
					return -1;
				}
				break;
			}
		}
	}*/
	assert(recv_len == len);
//	printf("buf: %02x %02x %02x %02x\n", buffer[0],buffer[1],buffer[2],buffer[3]);
//	memcpy(buffer, buf, len);
//	free(buf);
	return 1;
}

int send_have(peer_have *h_head){
	int i;
	char* sendline = (char*)malloc(9);
	memcpy(sendline, &h_head->head.length, sizeof(unsigned int));
	sendline[4] = h_head->head.id;
	memcpy(sendline+5, &h_head->piece_index, sizeof(unsigned int));
	//printf("::send_have the length sendline is %d\n", sizeof(unsigned int)*2 + sizeof(unsigned char));
	//printf("::send_have %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", sendline[0], sendline[1], sendline[2], sendline[3], sendline[4], sendline[5], sendline[6], sendline[7], sendline[8]);
	for(i=0; i<MAX_PEERS; i++){
		if(peer_pool[i] != NULL)
			p2p_send(sendline, peer_pool[i]->sockfd, sizeof(unsigned int)*2 + sizeof(unsigned char) );
	}
	free(sendline);
}
