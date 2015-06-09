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
#include "sha1.h"
int partition(int low, int high, int* p_sort, int* local){
	int pivot = p_sort[low];
	while(low < high){
		while(low < high && p_sort[high] > pivot)
			high--;
		if(low < high){
			int temp_local = local[low];
			local[low] = local[high];
			local[high] = temp_local;
			p_sort[low++] = p_sort[high];
		}
		while(low < high && p_sort[low] < pivot)
			low++;
		if(low < high){
			int temp_local = local[high];
			local[high] = local[low];
			local[low] = temp_local;
			p_sort[high--] = p_sort[low];
		}
	}
	p_sort[low] = pivot;

	return low;
}

void quick_sort(int low, int high, int* p_sort, int* local){
	int pivot;
	if(low < high){
		pivot = partition(low, high, p_sort, local);
		quick_sort(low, pivot-1, p_sort, local);
		quick_sort(pivot+1, high, p_sort, local);
	}
}

int *rarest_first(const int* const p_num, int *local){
	if(local == NULL){
	//	printf("local == NULL ? %d\n", local == NULL);
		local = (int*)malloc(sizeof(int)*g_num_pieces);
		for(int i=0; i<g_num_pieces; i++)
			local[i] = i;
	}
	struct timeval current;
	gettimeofday(&current, NULL);			
	if(current.tv_sec - rf_time.tv_sec < RAREST_FIRST_TIME)
		return local;
	int* temp = (int*)malloc(sizeof(int) * g_num_pieces);
	for(int i=0; i<g_num_pieces; i++){
		temp[i] = p_num[i];
		local[i] = i;
	}
	quick_sort(0, g_num_pieces-1, temp, local);
	printf("��ǰ����Ƭ������С����");
	for(int i=0; i<g_num_pieces; i++){
		printf("piece(%d): number(%d) ", local[i], temp[i]);
		if(i%4 == 0)
			printf("\n");
	}
	printf("\n");
	gettimeofday(&rf_time, NULL);			//���ø�ʱ��
	free(temp);
	return local;
}
//����REQUEST���ú���ֻ����������ÿһ����Ƭ��һ���ӷ�Ƭ����
//���Ǹ÷�Ƭ�ĺ����ӷ�Ƭ������handler�������д���
void* set_request(){			//ͬset_connection,listen_connectionһ�𴴽�
	//printf("enter set_request\n");
	int *local = NULL;
	gettimeofday(&rf_time, NULL);
//	sleep(1);
	while(1){
		pthread_mutex_lock(&send_recv_mutex);		//rarest_first���ܼ��㣬��Ҫ����
		local = rarest_first(p_num, local);
		assert(local != NULL);
		pthread_mutex_unlock(&send_recv_mutex);		//������ɣ�����

		int i;
		//�ж��յ����Ƿ��ǵ����ڶ�����Ƭ
		g_index = -1;
		int recv_pieces = 0, j = 0;
		for(i = 0; i < g_num_pieces; i++)
			if(p_mark[i] == 2)
				++recv_pieces;		//��¼�Ѿ������˶��ٷ�Ƭ
			else if(p_mark[i] == 0)
				g_index = i;		//��¼���δ���صķ�Ƭ�±�

		printf("recv_pieces: %d\n", recv_pieces);

		//�������һ����Ƭ������������������Դ��peer����request����
		if(recv_pieces == g_num_pieces - 1)
		{
			printf("�����׶Σ�Ҫ���յ�pieceΪ��%d\n", g_index);
			//����request����
			peer_request x;
			assert(g_filelen - g_index*piece_len >= 0);		//������С��0��������ָ�������
			unsigned int length = g_filelen - g_index*piece_len;		//�ҳ��÷�Ƭ�Ĵ�С
			length = (length < REQUEST_LEN)?length:REQUEST_LEN;	//�ӷ�Ƭ��Сѡ�����С��16KB��ѡ�䳤��
			unsigned int head_length = htonl(13);
			unsigned char head_id= 6;
			x.index = htonl(g_index);
			x.begin = htonl(0);
			x.length = htonl(length);
			char *sendline = (char *)malloc(sizeof(char) * (sizeof(unsigned int) + sizeof(unsigned char) + sizeof(x)));
			memcpy(sendline, &head_length, sizeof(head_length));
			memcpy(sendline+4, &head_id, sizeof(head_id));
			memcpy(sendline+5, &x, sizeof(x));
			//�������и÷�Ƭ��peer����request����
			for(i = 0; i < MAX_PEERS; i++)
			{
				if(peer_pool[i] != NULL)
				{
					if(peer_pool[i]->peer_mark[g_index] == 2)		//�����peer��������Ҫ�ķ�Ƭ
					{
						if(peer_pool[i]->choked != 1 && peer_pool[i]->have_interest == 1)
						{
							if(p2p_send(sendline, peer_pool[i]->sockfd, ntohl(head_length) + sizeof(unsigned int)) == -1)
							{
								printf("fail to send last request to sockfd: %d\n", peer_pool[i]->sockfd);
								continue;	//���Ͳ��ɹ�����������и÷�Ƭ��
							}
							else
								printf("��sockfdΪ��%d����peer��Ҫ����Ƭ\n", peer_pool[i]->sockfd);
						}
						peer_pool[i]->download_count++;
					}
				}
			}
			free(sendline);
		//	break;
		}
		else
			g_index = -1;		//��ʱ��g_index�����������ĺ���

		sleep(1);
		//usleep(500000);

		if(g_downloaded == g_filelen || seeder == 1)		//������ɻ��������������������
			break;
		for(i=0; i<g_num_pieces; i++){
			assert(p_mark != NULL);
		//	printf("!!!!!!!!!!!!!!!!(%d)\n", local[i]);
		//	printf("!!!!!!!!!!!!!!!!%d\n", p_mark[local[i]]);
			if(p_mark[local[i]] == 0){			//local�ǵ�ǰ���ٷ�Ƭ,��������
				int j;
				recv_pieces = 0;
				for(j = 0; j < g_num_pieces; j++)
				{
					if(p_mark[j] == 2)
						++recv_pieces;		//��¼�Ѿ������˶��ٷ�Ƭ
					else if(p_mark[j] == 0)
						g_index = i;		//��¼���δ���صķ�Ƭ�±�
				}
			//	printf("!!!!recv_pieces:%d\n", recv_pieces);
				if(recv_pieces == g_num_pieces - 1)
					break;
				else
					g_index = -1;

				for(j=0; j<MAX_PEERS; j++){
					pthread_mutex_lock(&send_recv_mutex);	//����
					if(peer_pool[j] == NULL){
						pthread_mutex_unlock(&send_recv_mutex);	//����
						continue;
					}
					pthread_mutex_unlock(&send_recv_mutex);		//����
					//printf("peer_pool[%d]\n", j);
					assert(peer_pool[j]->peer_mark != NULL);
//					printf("##set request: peer_pool[%d]: peer_mark[%d]: %02x\n", j,i, peer_pool[j]->peer_mark[i]);
	//				printf("chocked(%d), have_interest(%d), peer_mark(%d), download_count(%d)\n",                        peer_pool[j]->choked, peer_pool[j]->have_interest, peer_pool[j]->peer_mark[local[i]], peer_pool[j]->download_count);
					if(peer_pool[j]->choked != 1 && peer_pool[j]->have_interest == 1 && peer_pool[j]->peer_mark[local[i]] == 2 && peer_pool[j]->download_count < DOWNLOAD_LIMIT){
					//	printf("ENTER REQUEST \n");					

						peer_request x;
						assert(g_filelen - local[i]*piece_len >= 0);			//������С��0��������ָ�������
						unsigned int length = g_filelen - local[i]*piece_len;		//�ҳ��÷�Ƭ�Ĵ�С
						length = (length < REQUEST_LEN)?length:REQUEST_LEN;	//�ӷ�Ƭ��Сѡ�����С��16KB��ѡ�䳤��
						//printf("In set_request:: The len is %d\n", length);
						unsigned int head_length = htonl(13);
						unsigned char head_id= 6;
						x.index = htonl(local[i]);
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
							continue;							//���Ͳ��ɹ�����������и÷�Ƭ��
						}

						peer_pool[j]->download_count++;
						
						free(sendline);							
						break;									//���ͳɹ���������һ����Ҫ���صķ�Ƭ
					}

				}
			//	pthread_mutex_unlock(&send_recv_mutex);			//����
			//	sleep(REQUEST_TIME);							//�ݶ���������
				//if(peer_poll[j] != NULL)						//����˵�����У���һ��û�÷�Ƭ�������
				//	break;										//ÿ��sleepǰֻ��һ��Request
			}
		}
//		sleep(REQUEST_TIME);
	}
}
//�����������
void* recv_handler(void* pee){

	//printf("enter recv_handler\n");
	peer_t* peer = (peer_t*)pee;
//	int piece_len = g_torrentmeta->piece_len;
	int conn;
	conn = peer->sockfd;
	while(1){
	//	pthread_mutex_lock(&send_recv_mutex);	//����
		unsigned int len;
		unsigned int* len_t = (unsigned int*)malloc(sizeof(unsigned int));
		memset(len_t, 0, sizeof(unsigned int));
		//if(p2p_recv((char *)&len, conn, sizeof(len)) == -1){
		if(p2p_recv((char *)len_t, conn, sizeof(len)) == -1){
		//	printf("::recv_handler: Fail to recv from (%d)\n", conn);
		//	continue;
		//	pthread_mutex_unlock(&send_recv_mutex);
			break;
	//		return NULL;
		}
		
		len = ntohl(*len_t);
		free(len_t);
	//	printf("##len:%d\n", len);
		struct timeval tv;
		gettimeofday(&tv, NULL);
		peer->send_time = tv.tv_sec;
//		printf("!!!!peer->send_time: %d\n ", tv.tv_sec);
		if(len != 0){
			assert(len > 0);
			assert(len <= REQUEST_LEN + sizeof(unsigned char) + sizeof(peer_piece_h));
			//printf("::recv_handler: len!=0 The len is %d\n", len);
			char *buffer = (char *)malloc(sizeof(char) * len);
			if(p2p_recv(buffer, conn, len) == -1){
				//printf("::recv_handler: Fail to recv len(%d) from (%d)\n", len, conn);
			//	continue;
			//	pthread_mutex_unlock(&send_recv_mutex);
				break;
			}
		//	pthread_mutex_lock(&send_recv_mutex);			//����	!!˾ѩ��������λ����Ҫ�޸ģ���piece������
			switch(buffer[0]){				//buffer[0]��id
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
			//		printf("::recv_handler: recv interested\n");
					peer_wire_h head;
					head.length = len;
					head.id = buffer[0];
				//	free(buffer);
					peer->interested = 1;
					peer->choking = 0;
					// ���Է�����һ��unchoke��
					char* h2 = (char*)malloc(5);
					memset(h2, 0, 5);
					int* x = (int*)h2;
					*x = htonl(1);
					h2[4] = 1;
					if(p2p_send((char*)h2, conn, 5) < 0)
						;//printf("fail to send unchoke\n");
					else
						;//printf("succeed to send unchoke\n");
					free(h2);
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

					p_num[i]++;			//��������
					if(p_mark[i] == 0){
						unsigned int head_length = htonl(1);
						unsigned char head_id = 2;
						char* sendline = (char*)malloc(sizeof(unsigned char) + sizeof(unsigned int));
						memcpy(sendline, &head_length, sizeof(unsigned int));
						memcpy(sendline+4, &head_id, sizeof(char));
						//printf("!!Send_intereseted: %02x %02x %02x %02x %02x\n", sendline[0], sendline[1], sendline[2], sendline[3], sendline[4]);
						
						peer->have_interest = 1;
						
						p2p_send(sendline, conn, 5);
						free(sendline);
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
						if(peer->peer_mark[i] == 2){			//��������
							p_num[i]++;							//��������
						}										//��������
						//printf("peer->peer_mark[i] is %d\n", peer->peer_mark[i]);
					}
					int j;
					for(j=0; j<g_num_pieces; j++){
						if(peer->peer_mark[j] == 2 && p_mark[j] == 0){		//���������Ƭ������interested
							unsigned int head_length = htonl(1);
							unsigned char head_id = 2;
							char* sendline = (char*)malloc(sizeof(unsigned char) + sizeof(unsigned int));
							memcpy(sendline, &head_length, sizeof(unsigned int));
							memcpy(sendline+4, &head_id, sizeof(char));
							//printf("Send interested\n");
							peer->have_interest = 1;

							p2p_send(sendline, conn, 5);
							free(sendline);
							break;
						}
					}
					if(j == g_num_pieces){			//�������Ƭ������not interested
						unsigned int head_length = htonl(1);
						unsigned char head_id = 3;
						char* sendline = (char*)malloc(sizeof(unsigned char) + sizeof(unsigned int));
						memcpy(sendline, &head_length, sizeof(unsigned int));
						memcpy(sendline+4, &head_id, sizeof(char));
						//printf("Send not interested\n");
						peer->have_interest = 0;

						p2p_send(sendline, conn, 5);
						free(sendline);
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
					int data_length = r_head.length;
	//				printf("The r_head.length is %d\n", r_head.length);
					int index = r_head.index;

					if(p_mark[index] == 2 && peer->choking == 0){
						unsigned int head_length;
						unsigned char head_id;
						peer_piece_h p_head;
						char *sendline;
						char *block;

						head_length = htonl(sizeof(p_head) + sizeof(unsigned char) + r_head.length);
						//printf("::recv_request:: send piece length is %d\n", ntohl(head_length));
						head_id = 7;
						p_head.index = htonl(index);				
						p_head.begin = htonl(r_head.begin);
						

						//printf("::recv_handler: The p_head_index is %d\n", ntohl(p_head.index));

						block = (char *)malloc(sizeof(char) * r_head.length);
						memcpy(block, &Buffer[piece_len * index + r_head.begin], r_head.length);			
						
						sendline = (char *)malloc(sizeof(char) * (r_head.length + sizeof(p_head) + sizeof(unsigned int) + sizeof(unsigned char)));
						memcpy(sendline, &head_length, sizeof(unsigned int));
						memcpy(sendline+4, &head_id, sizeof(char));
						memcpy(sendline+5, &p_head, sizeof(p_head));
						memcpy(&sendline[5+sizeof(p_head)], block, r_head.length);
						//printf("::recv_handler: the length of send_piece is %d\n", sizeof(unsigned int)+sizeof(unsigned char)+sizeof(p_head)+r_head.length);

						p2p_send(sendline, conn, sizeof(unsigned int) + sizeof(unsigned char) + sizeof(p_head) + r_head.length);
						free(block);
						free(sendline);
						g_uploaded = g_uploaded + data_length;
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

					if(g_index != -1 && p_head.index == g_index && p_head.begin == 0)	//����÷�Ƭ��������Ҫ�����һ����Ƭ
					{
						int j;
						
						pthread_mutex_lock(&peer_mutex);

						if(peer->cancel == 1)
						{
							pthread_mutex_unlock(&peer_mutex);
							break;
						}

						printf("��sockfdΪ��%d����peer��Ҫ�������һ����Ƭ\n", conn);
						for(j = 0; j < MAX_PEERS; j++)
						{
							if(peer_pool[j] != NULL && peer_pool[j]->sockfd != conn && peer_pool[j]->peer_mark[g_index] == 2)
								//��cancelΪ1
								peer_pool[j]->cancel = 1;
							peer->cancel = 0;
						}
						pthread_mutex_unlock(&peer_mutex);

						//��ǵ�ǰpeer������peer���ͷ���cancel���ģ�����have_interestΪ0
						for(j = 0; j < MAX_PEERS; j++)
						{
							if(peer_pool[j] != NULL && peer_pool[j]->sockfd != conn && peer_pool[j]->peer_mark[g_index] == 2)
							{
								printf("��sockfd ��%d�� ����cancel����\n", peer_pool[j]->sockfd);
								//����cancel����(id = 8)
								peer_cancel x;
								assert(g_filelen - g_index*piece_len >= 0);	//������С��0��������ָ�������
								unsigned int length = g_filelen - g_index*piece_len;	//�ҳ��÷�Ƭ�Ĵ�С
								length = (length < REQUEST_LEN)?length:REQUEST_LEN;	//�ӷ�Ƭ��Сѡ�����С��16KB��ѡ�䳤��
								unsigned int head_length = htonl(13);
								unsigned char head_id= 8;
								x.index = htonl(g_index);
								x.begin = htonl(0);
								x.length = htonl(length);
								char *sendline = (char *)malloc(sizeof(char) * (sizeof(unsigned int) + sizeof(unsigned char) + sizeof(x)));
								memcpy(sendline, &head_length, sizeof(head_length));
								memcpy(sendline+4, &head_id, sizeof(head_id));
								memcpy(sendline+5, &x, sizeof(x));
								
								if(p2p_send(sendline, peer_pool[j]->sockfd, ntohl(head_length) + sizeof(unsigned int)) == -1)
								{
									printf("fail to send cancel to sockfd: %d\n", peer_pool[j]->sockfd);
									continue;
								}
							}
						}
					}

					data_len = len - sizeof(p_head) - sizeof(char);
					g_downloaded = g_downloaded + data_len;
					//printf("The datalen is %d\n", data_len);
					pthread_mutex_lock(&send_recv_mutex);		
					p_mark[p_head.index] = 1;

					char *block = (char *)malloc(sizeof(char) * data_len);
					memcpy(block, &buffer[9], data_len);
					
					unsigned int index = p_head.index * piece_len + p_head.begin;
					memcpy(&Buffer[index], block, data_len);			//���ݴ��뻺����
					pthread_mutex_unlock(&send_recv_mutex);
					free(block);
					
					if(p_head.begin + data_len == piece_len){		//��ɸ÷�Ƭ
					//	printf(":::::The piece (%d) was completed\n", p_head.index);
	/*					SHA1Context sha1;
						SHA1Reset(&sha1);
						printf("piece_len : %d\n", piece_len);
						
						SHA1Input(&sha1, (const unsigned char*)&Buffer[p_head.index*piece_len], piece_len);
						if(!SHA1Result(&sha1))
                                		{
                                        	printf("FAILURE\n");
						exit(0);
                                		}
						int* this_hash = malloc(sizeof(int)*5);
						memcpy(this_hash, sha1.Message_Digest, 20);
						for(int k=0; k<5; k++)
							this_hash[k] = htonl(this_hash[k]);
						char* p = (char*)this_hash;
						printf("my: \n");
                                for(int i = 0; i < 20; i++)
                                {
                                        printf("%02x", *p);
					if((i+1) %4 == 0)
						printf(" ");
                                        p++;
                                }
                                p = &piece_hash[p_head.index*20];
                                printf("\nhis: \n");
                                for(int i = 0; i < 20; i++)
                                {
                                        printf("%02x", *p);
					if((i+1) %4 == 0)
                                                printf(" ");
                                        p++;
                                }
				printf("\n");	
						if(strncmp((char*)this_hash, &piece_hash[p_head.index*20], 20) != 0){
							p_mark[p_head.index] = 0;
							printf("PIECE (%d) SHA1 ERROR\n", p_head.index);
							break;
						}
	*/					p_mark[p_head.index] = 2;			
						peer->download_count--;
						assert(peer->download_count >= 0);
						//����have��Ƭ
						peer_have h_head;
						h_head.head.length = htonl(5);
						h_head.head.id = 4;
						h_head.piece_index = htonl(p_head.index);

						send_have(&h_head);

						// �ж��ļ��Ƿ�������
						int i;
						for(i=0; i<g_num_pieces; i++){
							if(p_mark[i] != 2)
								break;
						}
						if(i == g_num_pieces){
							//printf("The file was completed\n");
							seeder = 1;
							 // �����պõ�����д��recv.txt�ļ���ȥ
                                                        FILE *f;
                                                    //    char filename[6] = "recv_";
                                                     //   strcat(filename, g_torrentmeta->name);
                                                        f = fopen(file_name,"w");
                                                        assert(f!=NULL);
                                                        fwrite(Buffer,g_filelen,1,f);
                                                        fclose(f);
							
							// ��tracker����completed����
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
							// �������.bt�ļ�����ʱ��Ҫɾ��
							FILE *bf;
                					char* bf_name = (char*)malloc(strlen(file_name)+4);
                					memset(bf_name, 0, strlen(file_name)+4);
                					strncpy(bf_name, file_name, strlen(file_name));
                					strcpy(&bf_name[strlen(file_name)], ".bt");
                					bf = fopen(bf_name, "r");
                					if(bf != NULL)
                					{
								fclose(bf);
								remove(bf_name);	
							}
						}
						break;
					}
					if(index + data_len == g_filelen){				//���β����Ƭ����Ϊ����Ƭ����С��piece_length
	//					printf(":::::The piece (%d) was completed\n", p_head.index);
		/*				SHA1Context sha1;
						SHA1Reset(&sha1);
						SHA1Input(&sha1, &Buffer[p_head.index*piece_len], g_filelen - p_head.index*piece_len);
						int this_hash[5];
						memcpy(this_hash, sha1.Message_Digest, 20);
						for(int k=0; k<5; k++)
							this_hash[k] = htonl(this_hash[k]);
						if(strncmp((char*)this_hash, &piece_hash[p_head.index*20], 20) != 0){
							p_mark[p_head.index] = 0;
							printf("PIECE (%d) SHA1 ERROR\n", p_head.index);
							break;
						}
		*/				p_mark[p_head.index] = 2;	
						peer->download_count--;
						assert(peer->download_count >= 0);
						//����have��Ƭ
						peer_have h_head;
						h_head.head.length = htonl(5);
						h_head.head.id = 4;
						h_head.piece_index = htonl(p_head.index);

						send_have(&h_head);
						
						// �ж��ļ��Ƿ�������
                                                int i;
                                                for(i=0; i<g_num_pieces; i++){
                                                        if(p_mark[i] != 2)
                                                                break;
                                                }
                                                if(i == g_num_pieces){
                                                        printf("The file was completed\n");
                                                        seeder = 1;
							 // �����պõ�����д��recv.txt�ļ���ȥ
                                                	 // �����պõ�����д��recv.txt�ļ���ȥ
                                                	FILE *f;
							//char filename[6] = "recv_";
							//strcat(filename, g_torrentmeta->name);
                                                	f = fopen(file_name,"w");
                                                	assert(f!=NULL);
							fwrite(Buffer,g_filelen,1,f);	
                                                	fclose(f);
							
							// ��tracker����completed����
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
							// �������.bt�ļ�����ʱ��Ҫɾ��
                                                        FILE *bf;
                                                        char* bf_name = (char*)malloc(strlen(file_name)+4);
                                                        memset(bf_name, 0, strlen(file_name)+4);
                                                        strncpy(bf_name, file_name, strlen(file_name));
                                                        strcpy(&bf_name[strlen(file_name)], ".bt");
                                                        bf = fopen(bf_name, "r");
                                                        if(bf != NULL)
                                                        {       
                                                                fclose(bf);
                                                                remove(bf_name);
                                                        }
                                                }
						break;
					}
					//��δ��ɸ÷�Ƭ������request��Ҫ��һ���ӷ�Ƭ
					peer_request r_head;
					assert(g_filelen - piece_len*p_head.index - data_len - p_head.begin >= 0);		//���������ϵ��
					unsigned int length = g_filelen - piece_len * p_head.index - data_len - p_head.begin;
					length = (length>REQUEST_LEN)?REQUEST_LEN:length;
					unsigned int head_length = htonl(13);
					unsigned char head_id = 6;
					r_head.index = htonl(p_head.index);
					r_head.begin = htonl(p_head.begin + data_len);
					r_head.length = htonl((length>REQUEST_LEN)?REQUEST_LEN:length);
					//printf("::recv_handler the next_request length is %d\n", sizeof(unsigned int)+sizeof(unsigned char)+sizeof(r_head));
					//printf("r_head.length: %d\n",ntohl(r_head.length));
					char* sendline  = (char*)malloc(sizeof(unsigned int) + sizeof(unsigned char) + sizeof(r_head));
					memcpy(sendline, &head_length, sizeof(unsigned int));
					memcpy(sendline+4, &head_id, sizeof(unsigned char));
					memcpy(sendline+5, &r_head, sizeof(r_head));
					//printf("::::::send the request, begin is %d\n", p_head.begin + data_len);
					p2p_send(sendline, conn, sizeof(unsigned int) + sizeof(unsigned char) + sizeof(r_head));

					break;
				}
				case 8:{			//cancel

					//�յ�cancel���ģ��ر��׽���
					close(peer->sockfd);
					peer->sockfd = -1;
					for(int i=0; i<g_num_pieces; i++){
						if(peer->peer_mark[i] == 2)
							p_num[i]--;
					}
					free(peer->peer_mark);
					for(int i=0; i<MAX_PEERS; i++){
						if(peer_pool[i] != NULL){
							if(peer == peer_pool[i])	
								peer_pool[i] = NULL;
						}
					}
					free(peer);
					count--;

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
		//	pthread_mutex_unlock(&send_recv_mutex);			//˾ѩ��������λ����Ҫ�޸ģ���piece�н���

			free(buffer);
		}
	//	pthread_mutex_unlock(&send_recv_mutex);
	}
	
	
	if(peer->sockfd >= 0)
	{
		close(peer->sockfd);
		peer->sockfd = -1;
		for(int i=0; i<g_num_pieces; i++){
			if(peer->peer_mark[i] == 2)
				p_num[i]--;
		}
		free(peer->peer_mark);
		for(int i=0; i<MAX_PEERS; i++){
			if(peer_pool[i] != NULL){
				if(peer == peer_pool[i])	
					peer_pool[i] = NULL;
			}
		}
		free(peer);
		count--;
		
	}
}


//Peer ֮�䷢������
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

//Peer ֮���������
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
//	printf("::send_have the length sendline is %d\n", sizeof(unsigned int)*2 + sizeof(unsigned char));
//	printf("::send_have %02x %02x %02x %02x %02x %02x %02x %02x %02x\n", sendline[0], sendline[1], sendline[2], sendline[3], sendline[4], sendline[5], sendline[6], sendline[7], sendline[8]);
	for(i=0; i<MAX_PEERS; i++){
		if(peer_pool[i] != NULL)
		
			p2p_send(sendline, peer_pool[i]->sockfd, sizeof(unsigned int)*2 + sizeof(unsigned char) );
	}
	free(sendline);
}

void* reset_pmark(){
	
	while(1){
		sleep(30);
		for(int i=0; i<g_num_pieces; i++){
			if(p_mark[i] == 1){
				printf("!!!!!!!!!!!!!!!hahahhahaa\n");
				p_mark[i] = 0;
			}
		}
	}
}

