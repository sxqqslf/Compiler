#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include "util.h"
#include "btdata.h"
#include "bencode.h"
#include "assert.h"
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h>
#include <stdlib.h> 
#include "sha1.h"
//#define MAXLINE 4096 
// pthread数据
// 打印进度
void* print_file();
// 定义peer列表头
peer_list* peer_head = NULL;
void init()
{
  	g_done = 0;
  	g_tracker_response = NULL;
}
announce_list* url_list = NULL;      // announce url列表头指针

int public_main(int argc, char **argv);
int private_main(int argc, char **argv);
void close_socket(int sig) 
{
	printf("send or recv fail\n");
}
int main(int argc, char **argv) 
{
	/*
  	int sockfd = -1;
  	char rcvline[MAXLINE];
  	char tmp[MAXLINE];
  	FILE* f;
  	int rc;
  	int i;
 	*/
	// 注意: 你可以根据自己的需要修改这个程序的使用方法	
	if(argc < 5)
	{
    		printf("Usage: SimpleTorrent 1.torrent 2.ip 3.port 4.seeder 5.public\n");
		/*
    		printf("\t isseed is optional, 1 indicates this is a seed and won't contact other clients\n");
    		printf("\t 0 indicates a downloading client and will connect to other clients and receive connections\n");
    		*/exit(-1);
  	}
	
	if(argv[5][0] == '0')
		private_main(argc, argv);
	else if(argv[5][0] == '1')
		public_main(argc, argv);	
  	
}
// 打印进度
void* print_file()
{
	int pre_downloaded = 0;
	int pre_uploaded = 0;
	while(1)
	{
		int i;
		int get_pieces = 0;
		for(i = 0; i < g_num_pieces; i++)
		{
			if(p_mark[i] == 2)
				get_pieces++;
		}
		printf("分片:[%d,%d] (下载,总共) ", get_pieces, g_num_pieces);
		printf("下载: %.2f MB, 上传: %.2f MB ", g_downloaded*1.0 / 1048576, g_uploaded*1.0 / 1048576);
		int sub_down = g_downloaded - pre_downloaded;
		pre_downloaded = g_downloaded;
		int sub_up = g_uploaded - pre_uploaded;
		pre_uploaded = g_uploaded;
		printf("速度: %.2f K/s, %.2f K/s\n", sub_down*1.0/2/1024, sub_up*1.0/2/1024);
		sleep(2);
	}
}

char file_name[100];
char piece_hash[10000];
int public_main(int argc, char **argv) 
{
  	int sockfd = -1;
  	char rcvline[MAXLINE];
  	char tmp[MAXLINE];
  	FILE* f;
  	int rc;
  	int i;
 
	// 注意: 你可以根据自己的需要修改这个程序的使用方法	
	if(argc < 5)
	{
    		printf("Usage: SimpleTorrent 1.torrent 2.ip 3.port 4.seeder 5.public\n");
		/*
    		printf("\t isseed is optional, 1 indicates this is a seed and won't contact other clients\n");
    		printf("\t 0 indicates a downloading client and will connect to other clients and receive connections\n");
    		*/exit(-1);
  	}
		
  	init();
  	srand(time(NULL));

  	int val[5];
  	for(i=0; i<5; i++)
  	{
    		val[i] = rand();
  	}
  	memcpy(g_my_id,(char*)val,20);
  	strncpy(g_my_ip,argv[2],strlen(argv[2]));
  	g_my_ip[strlen(argv[2])+1] = '\0';
	
	// 记录下BT监听的端口
	g_peerport = atoi(argv[3]);

	// 记录seeder标记
		seeder = atoi(argv[4]);
  	g_torrentmeta = public_parsetorrentfile(argv[1]);
  	memcpy(g_infohash,g_torrentmeta->info_hash,20);

  	g_filelen = g_torrentmeta->length;
  	g_num_pieces = g_torrentmeta->num_pieces;
  	g_filedata = (char*)malloc(g_filelen*sizeof(char));
	printf("filename: %s\n", g_torrentmeta->name);
	memset(file_name, 0, strlen(g_torrentmeta->name)+1);
	strcpy(file_name, g_torrentmeta->name);
	memcpy(piece_hash, g_torrentmeta->pieces, g_num_pieces*20);
	// 将所有分片的mark设置为0
	
	// get tracker's URL for public torrent
	// 如果该客户端为seeder, 将文件读出
        if(seeder == 1)
        {
                FILE *f;
                f = fopen(g_torrentmeta->name,"r");
                assert(f!=NULL);
                fread(g_filedata,g_filelen,1,f);
                fclose(f);
                g_uploaded = 0;
                g_left = 0;
                g_downloaded = g_filelen;
        }
        else
        {
                g_uploaded = 0;
                g_left = g_filelen;
                g_downloaded = 0;
        }
	// 打印所有tracker的URL以及种子其他信息
	for(int i = 0; i <= tracker_count -1; i++)
	{
	
		// 修正一些错误
                char* str_ann = strstr(tracker_list[i].announce, "/anno");
                str_ann[5] = 'u';
                str_ann[6] = 'n';
                str_ann[7] = 'c';
                str_ann[8] = 'e';
                str_ann[9] = '\0';
		if(i == 0)
			printf("Announce: %s\n", tracker_list[i].announce);
		else if(i == 1)
		{
			printf("Alternates:\n");
			printf("%d: %s\n",i, tracker_list[i].announce);
		}
		else
			printf("%d: %s\n",i, tracker_list[i].announce);
		
	}
	printf("File length: %d\n", g_filelen);
  	printf("Piece length: %d\n", g_torrentmeta->piece_len);  // 每一个分片的字节数
	piece_len = g_torrentmeta->piece_len;
  	printf("Number of pieces: %d\n", g_torrentmeta->num_pieces); // 分片数量(为方便起见)
	//exit(0);	
	char* other;
	// 提取所有tracker的IP和端口
	for(i = 0; i <= tracker_count-1; i++)
	{
		announce_list* u_list = (announce_list*)malloc(sizeof(announce_list));
		memset(u_list, 0, sizeof(announce_list));
		u_list->next = NULL;
		// 如果没有端口号, 且是udp的URL就不解析
		char* pivot = strstr(tracker_list[i].announce, "announce");
		assert(pivot != NULL);
		pivot--;
		pivot--;
		assert(pivot != NULL);
		if(pivot == NULL || !(*pivot >= '0' && *pivot <= '9'))
		{
			continue;
		}
		pivot = NULL;
		pivot = strstr(tracker_list[i].announce, "http://");
		if(pivot == NULL)
		{
			continue;
		}
		// 分离主机名和端口号
		u_list->info = parse_announce_url(tracker_list[i].announce);	
		// 根据主机名得到IP地址
		struct hostent *record;
        	record = gethostbyname(u_list->info->hostname);
        	if (record==NULL)
        	{ 
                	printf("gethostbyname failed");
                	exit(1);
        	}
        	struct in_addr* address;
        	address =(struct in_addr * )record->h_addr_list[0];
		u_list->info->hostname = inet_ntoa(* address);
		printf("%d: URL IP: %s, port: %d\n", i, u_list->info->hostname, u_list->info->port);
		// 得到IP地址替换掉主机名
		// 将该URL信息加入到URL列表中
		if(url_list == NULL)
			url_list = u_list;
		else
		{
			announce_list* q = url_list;
			while(q->next != NULL)
				q = q->next;
			q->next = u_list;
		}
		char* pre = u_list->info->hostname;
		u_list->info->hostname = (char*)malloc(strlen(pre) + 1);
		memset(u_list->info->hostname, 0, strlen(pre) + 1);
		strcpy(u_list->info->hostname, pre);
			
	}
	announce_list* p_list = url_list;
	while(p_list != NULL)
	{
		printf("before: IP: %s, port: %d\n", p_list->info->hostname, p_list->info->port);
		p_list = p_list->next;
	}
	memset(g_tracker_ip, 0, 16);
	strncpy(g_tracker_ip, url_list->info->hostname,strlen(url_list->info->hostname));
	g_tracker_port = url_list->info->port;
	//初始化互斥锁
	if(pthread_mutex_init(&send_recv_mutex, NULL) != 0){
		printf("Fail to create the mutex\n");
		exit(-1);
	}

  	// 初始化
  	// 设置信号句柄
  	signal(SIGINT,client_shutdown);
	// 防止套接字关闭导致程序直接退出
	signal(SIGPIPE, close_socket);
//	exit(0);
  	// 设置监听peer的线程

  	// 定期联系Tracker服务器
  	int firsttime = 1;
  	int mlen;
  	char* MESG;
  	MESG = make_tracker_request(BT_STARTED,&mlen);
	printf("STARTED:\n");
       	for(i=0; i<mlen; i++)
        	printf("%c",MESG[i]);
      	printf("\n");
  	while(!g_done)
  	{
    
		// 置peer列表头指针为空
                peer_head = NULL;
    
    		if(!firsttime)
    		{
      			free(MESG);
      			// -1 指定不发送event参数
      			MESG = make_tracker_request(-1,&mlen);
      			printf("MESG: ");
      			for(i=0; i<mlen; i++)
        			printf("%c",MESG[i]);
      			printf("\n");
    		}
		announce_list* p_list = url_list;
		// 和所有tracker都联系
		while(p_list == url_list)
		{/*
			if(p_list == NULL || p_list->info == NULL || p_list->info->hostname == NULL)
			{
				if(p_list != NULL)
				{
					p_list = p_list->next;
			 		continue;
				}
				else
					break;
			}
	*/
	//		printf("before: IP: %s, port: %d\n", p_list->info->hostname, p_list->info->port);
			// 如果有重复的就不发送报文了
	/*		int flag = 0;
			while(flag == 0 && p_list != NULL)
			{
				announce_list* q = url_list;
				while(q != p_list)
				{
					if(strcmp(q->info->hostname, p_list->info->hostname) == 0 && q->info->port == p_list->info->port)
					{
						flag = 1;
						break;
					}
					q = q->next;
				}
				if(flag == 0)
					break;
				else
				{
					p_list = p_list->next;
					flag = 0;
				}
			}
			if(p_list == NULL)
				break;
	*/		if(sockfd <= 0)
                	{
                        	//创建套接字发送报文给Tracker
                        	printf("Creating socket to tracker...\n");
                        //	sockfd = connect_to_host(g_tracker_ip, g_tracker_port);
//				printf("IP: %s, port: %d\n", p_list->info->hostname, p_list->info->port);
//				if(strcmp(p_list->info->hostname, "222.186.13.71") == 0 && p_list->info->port == 7070)
//				if(strcmp(p_list->info->hostname, "74.125.128.121") == 0 && p_list->info->port == 8000)
					sockfd = connect_to_host(p_list->info->hostname, p_list->info->port);
				if(sockfd < 0)
				{
					p_list = p_list->next;
					continue;
				}
                	}
			
                	printf("Sending request to tracker...\n");
    			send(sockfd, MESG, mlen, 0);
    
    			memset(rcvline,0x0,MAXLINE);
    			memset(tmp,0x0,MAXLINE);
    
    			// 读取并处理来自Tracker的响应
    			tracker_response* tr;
    			tr = public_preprocess_tracker_response(sockfd); 
   
    			// 关闭套接字, 以便再次使用
    			shutdown(sockfd,SHUT_RDWR);
    			close(sockfd);
    			sockfd = 0;

    			printf("Decoding response...\n");
    			char* tmp2 = (char*)malloc(tr->size*sizeof(char));
    			memcpy(tmp2,tr->data,tr->size*sizeof(char));

    			printf("Parsing tracker data\n");
    			g_tracker_response = get_tracker_data(tmp2,tr->size);
    
    			if(tmp)
    			{
      				free(tmp2);
      				tmp2 = NULL;
    			}

    			printf("Num Peers: %d\n",g_tracker_response->numpeers);

			// 更新peer列表
//			peer_head = NULL;
    			for(i=0; i<g_tracker_response->numpeers; i++)
    			{
      				int idl;

				// peer_list中不能保存自己的peer
				if(strcmp(g_my_ip, g_tracker_response->peers[i].ip) == 0 && g_peerport == g_tracker_response->peers[i].port)
					continue;
				// 加入一个新的peer节点到peer列表中
				if(peer_head == NULL)
				{
					peer_head = (peer_list*)malloc(sizeof(peer_list));
					memset(peer_head, 0, sizeof(peer_list));
					for(int k = 0; k < 20; k++)
						peer_head->this_peer.id[k] = g_tracker_response->peers[i].id[k];
					peer_head->this_peer.port = g_tracker_response->peers[i].port;
					peer_head->this_peer.ip = (char*)malloc(strlen(g_tracker_response->peers[i].ip)+1);
					memset(peer_head->this_peer.ip, 0, strlen(g_tracker_response->peers[i].ip)+1);
					strcpy(peer_head->this_peer.ip, g_tracker_response->peers[i].ip);
					peer_head->next = NULL;
				}
				else
				{
					peer_list* p = (peer_list*)malloc(sizeof(peer_list));
                                	memset(p, 0, sizeof(peer_list));
                                	for(int k = 0; k < 20; k++)
                                	        p->this_peer.id[k] = g_tracker_response->peers[i].id[k];
                                	p->this_peer.port = g_tracker_response->peers[i].port;
					p->this_peer.ip = (char*)malloc(strlen(g_tracker_response->peers[i].ip)+1);
                                	memset(p->this_peer.ip, 0, strlen(g_tracker_response->peers[i].ip)+1);
                                	strcpy(p->this_peer.ip, g_tracker_response->peers[i].ip);
                                	p->next = peer_head;
					peer_head = p;
				}
    			}
			break;
			if(p_list == NULL)
				break;	
			p_list = p_list->next;	
		}	
		
		// 打印peer列表
		peer_list* p = peer_head;
/*		while(p != NULL)
		{
			printf("id: %s\n", p->this_peer.id);
			printf("ip:%s\n", p->this_peer.ip);
			printf("port:%d\n", p->this_peer.port);
			p = p->next;
		}
*/		printf("睡眠%d秒...\n", g_tracker_response->interval);  

		// 启动线程与其他peer进行交互
		if(firsttime == 1)		
		{
			p_mark = (char*)malloc(sizeof(char)*g_num_pieces);
			memset(p_mark, 0, sizeof(char)*g_num_pieces);
			p_num = (int*)malloc(sizeof(int)*g_num_pieces);
			for(int i = 0; i < g_num_pieces; i++)
				p_num[i] = 0;
			
			if(seeder == 1)
			{
				memset(p_mark, 2, sizeof(char)*g_torrentmeta->num_pieces);
			}// 如果不是seeder, 那么查看是否存在.bt文件，如果存在，说明需要进行断点续传
			else
			{
				printf("len: %d, num: %d\n", g_filelen, g_num_pieces);
				FILE *bf;
				char* bf_name = (char*)malloc(strlen(file_name)+4);
				memset(bf_name, 0, strlen(file_name)+4);
				strncpy(bf_name, file_name, strlen(file_name));
				strcpy(&bf_name[strlen(file_name)], ".bt");		
				bf = fopen(bf_name, "r");
				if(bf != NULL)
				{
					fread(g_filedata, g_filelen, 1, bf);
					fclose(bf);
					free(bf_name);
					// 计算每个分片的sha1值
					int bt_count = 0;
					for(int i = 1; i <= g_num_pieces; i++)
					{
						int this_hash[5];
						// 计算上面字符串的SHA1哈希值以获取info_hash
						SHA1Context sha;
						SHA1Reset(&sha);
						// 如果不是最后一段分片，直接算SHA1
						if(i < g_num_pieces)
							SHA1Input(&sha,(const unsigned char*)&g_filedata[(i-1)*piece_len],piece_len);
						// 如果是最后一段分片，取得最后一个分片真实长度，再算SHA1
						else
						{
							SHA1Input(&sha,(const unsigned char*)&g_filedata[(i-1)*piece_len],                                                                                 g_filelen-piece_len*(g_num_pieces-1));
						}
				
						if(!SHA1Result(&sha))
						{
							printf("FAILURE\n");
						}
						memcpy(this_hash,sha.Message_Digest,20);	
						// 比较.bt的SHA1和真实的SHA1，如果分片SHA1相同，代表已接收完毕，将p_mark置为2
						for(int k = 0; k < 5; k++)
						 	this_hash[k] = htonl(this_hash[k]);
						if(strncmp((char*)this_hash, &piece_hash[(i-1)*20], 20) == 0)
						{			
				                        bt_count++;
				                        p_mark[i-1] = 2;
						}		
					}
			
					printf("pieces: %d\n", bt_count);
				//	free(this_hash);
				}
				
			}	
			// 设置缓冲区接口
			Buffer = g_filedata;

			for(int i = 0; i < MAX_PEERS; i++)
                        	peer_pool[i] = NULL;
			// 开启4个线程用于交互
		//	printf("start 4 thread\n");
			pthread_t thread[1000];
			peer_list* p = peer_head;
			int pthread_count = 1;
			pthread_create(&thread[600],NULL, print_file, NULL);
			while(p != NULL)
			{
				if(count < MAX_PEERS)
					pthread_create(&thread[pthread_count++], NULL, set_connection, (void*)p);
				else
					break;
				p = p->next;
			}
			
		//	pthread_create(&thread[0], NULL, set_connection, (void *)peer_head);	
			pthread_create(&thread[pthread_count++], NULL, listen_connection, NULL);
			
			pthread_create(&thread[pthread_count++], NULL, judgeTimeOut, NULL);
			pthread_create(&thread[pthread_count++], NULL, set_request,NULL);
			firsttime = 0;
		}
    		// 必须等待td->interval秒, 然后再发出下一个GET请求
    		sleep(g_tracker_response->interval);
//		sleep(60*30);
//		sleep(10);
  	}
 
  	// 睡眠以等待其他线程关闭它们的套接字, 只有在用户按下ctrl-c时才会到达这里
  	sleep(2);

  	exit(0);
}


int private_main(int argc, char **argv) 
{
  	int sockfd = -1;
        char rcvline[MAXLINE];
        char tmp[MAXLINE];
        FILE* f;
        int rc;
        int i;	

  	init();
  	srand(time(NULL));

  	int val[5];
  	for(i=0; i<5; i++)
  	{
    		val[i] = rand();
  	}
  	memcpy(g_my_id,(char*)val,20);
  	strncpy(g_my_ip,argv[2],strlen(argv[2]));
  	g_my_ip[strlen(argv[2])+1] = '\0';
	
	// 记录下BT监听的端口
	g_peerport = atoi(argv[3]);

	// 记录seeder标记
		seeder = atoi(argv[4]);

  	g_torrentmeta = private_parsetorrentfile(argv[1]);
  	memcpy(g_infohash,g_torrentmeta->info_hash,20);

  	g_filelen = g_torrentmeta->length;
  	g_num_pieces = g_torrentmeta->num_pieces;
	piece_len = g_torrentmeta->piece_len;
  	g_filedata = (char*)malloc(g_filelen*sizeof(char));
	memset(g_filedata, 0, g_filelen*sizeof(char));
	printf("filename: %s\n", g_torrentmeta->name);
	memset(file_name, 0, strlen(g_torrentmeta->name)+1);
        strcpy(file_name, g_torrentmeta->name);
	memcpy(piece_hash, g_torrentmeta->pieces, g_num_pieces*20);
	// 将所有分片的mark设置为0
     	p_mark = (char*)malloc(sizeof(char)*g_torrentmeta->num_pieces);
  	memset(p_mark, 0, sizeof(char)*g_torrentmeta->num_pieces);

	// 如果该客户端为seeder, 将文件读出
	if(seeder == 1)
	{
		// 将所有分片置为2
		memset(p_mark, 2, sizeof(char)*g_torrentmeta->num_pieces);
		FILE *f;
        	f = fopen(g_torrentmeta->name,"r");
        	assert(f!=NULL);
        	fread(g_filedata,g_filelen,1,f);
        	fclose(f);	
		g_uploaded = 0;
                g_left = 0;
                g_downloaded = g_filelen;
	}	
	// 如果不是seeder, 那么查看是否存在.bt文件，如果存在，说明需要进行断点续传
	else
	{
		/*
		char*p = &g_torrentmeta->pieces[5*20];
                                printf("\nhis: \n");
                                for(int i = 0; i < 20; i++)
                                {
                                        printf("%02x", *p);
                                        p++;
					if((i+1)%4 == 0)
						printf(" ");
                                }*/
		FILE *bf;
		char* bf_name = (char*)malloc(strlen(file_name)+4);
		memset(bf_name, 0, strlen(file_name)+4);
		strncpy(bf_name, file_name, strlen(file_name));
		strcpy(&bf_name[strlen(file_name)], ".bt");		
		bf = fopen(bf_name, "r");
//		bf = fopen(g_torrentmeta->name, "r");
		if(bf != NULL)
		{
			fread(g_filedata, g_filelen, 1, bf);
			fclose(bf);
			free(bf_name);
			// 计算每个分片的sha1值
			int bt_count = 0;
			for(int i = 1; i <= g_num_pieces; i++)
			{
				int this_hash[5];
				// 计算上面字符串的SHA1哈希值以获取info_hash
        			SHA1Context sha;
        			SHA1Reset(&sha);
				// 如果不是最后一段分片，直接算SHA1
				if(i < g_torrentmeta->num_pieces)
        				SHA1Input(&sha,(const unsigned char*)&g_filedata[(i-1)*piece_len],piece_len);
        			// 如果是最后一段分片，取得最后一个分片真实长度，再算SHA1
				else
				{
					SHA1Input(&sha,(const unsigned char*)&g_filedata[(i-1)*piece_len],                                                                                 g_torrentmeta->length-piece_len*(g_torrentmeta->num_pieces-1));
				}
				
				if(!SHA1Result(&sha))
        			{
                			printf("FAILURE\n");
        			}
        			memcpy(this_hash,sha.Message_Digest,20);	
				// 比较.bt的SHA1和真实的SHA1，如果分片SHA1相同，代表已接收完毕，将p_mark置为2
				for(int k = 0; k < 5; k++)
				 	this_hash[k] = htonl(this_hash[k]);
				char* p = (char*)this_hash;
		/*		printf("my: \n");
				for(int i = 0; i < 20; i++)
				{
					printf("%02x", *p);
					p++;
				}
				p = &piece_hash[(i-1)*20];
				printf("\nhis: \n");
				for(int i = 0; i < 20; i++)
                                {
                                        printf("%02x", *p);
                                        p++;
                                }
		*///		if(strncmp((char*)this_hash, &g_torrentmeta->pieces[(i-1)*20], 20) == 0)
				if(strncmp((char*)this_hash, &piece_hash[(i-1)*20], 20) == 0)
				{			
                                        bt_count++;
                                        p_mark[i-1] = 2;
				}		
			}
			
			printf("pieces: %d\n", bt_count);
		//	free(this_hash);
		}
		g_uploaded = 0;
                g_left = g_filelen;
                g_downloaded = 0;
	}
  	announce_url_t* announce_info;
  	announce_info = parse_announce_url(g_torrentmeta->announce);
  	// 提取tracker url中的IP地址
  	printf("HOSTNAME: %s\n",announce_info->hostname);
	/*  hostent *record;
  	record = gethostbyname(announce_info->hostname);
  	if (record==NULL)
  	{ 
    		printf("gethostbyname failed");
    		exit(1);
  	}
  	struct in_addr* address;
	address =(struct in_addr * )record->h_addr_list[0];
						count++;
  	printf("Tracker IP Address: %s\n", inet_ntoa(* address));
  	strcpy(g_tracker_ip,inet_ntoa(*address));
  	g_tracker_port = announce_info->port;
	*/	
  	printf("Tracker IP Address: %s\n", announce_info->hostname);
	strcpy(g_tracker_ip,announce_info->hostname);
  	g_tracker_port = announce_info->port;
  	free(announce_info);
  	announce_info = NULL;

	//初始化互斥锁
	if(pthread_mutex_init(&send_recv_mutex, NULL) != 0){
		printf("Fail to create send_recv_mutex\n");
		exit(-1);
	}

	if(pthread_mutex_init(&peer_mutex, NULL) != 0)
	{
		printf("Fail to create peer_mutex\n");
		exit(-1);
	}

  	// 初始化
  	// 设置信号句柄
  	signal(SIGINT,client_shutdown);
	// 防止套接字关闭导致程序直接退出
        signal(SIGPIPE, close_socket);
  	// 设置监听peer的线程

  	// 定期联系Tracker服务器
  	int firsttime = 1;
  	int mlen;
  	char* MESG;
  	MESG = make_tracker_request(BT_STARTED,&mlen);
//	MESG = make_tracker_request(-1,&mlen);
	printf("STARTED:\n");
       	for(i=0; i<mlen; i++)
        	printf("%c",MESG[i]);
      	printf("\n");
  	while(!g_done)
  	{
    		if(sockfd <= 0)
    		{
      			//创建套接字发送报文给Tracker
      			printf("Creating socket to tracker...\n");
      			sockfd = connect_to_host(g_tracker_ip, g_tracker_port);
    		}
    
    		printf("Sending request to tracker...\n");
    
    		if(!firsttime)
    		{
      			free(MESG);
      			// -1 指定不发送event参数
      			MESG = make_tracker_request(-1,&mlen);
      			printf("MESG: ");
      			for(i=0; i<mlen; i++)
        			printf("%c",MESG[i]);
      			printf("\n");
    		}
    		send(sockfd, MESG, mlen, 0);
    
    		memset(rcvline,0x0,MAXLINE);
    		memset(tmp,0x0,MAXLINE);
    
    		// 读取并处理来自Tracker的响应
    		tracker_response* tr;
    		tr = private_preprocess_tracker_response(sockfd); 
   
    		// 关闭套接字, 以便再次使用
    		shutdown(sockfd,SHUT_RDWR);
    		close(sockfd);
    		sockfd = 0;

    		printf("Decoding response...\n");
    		char* tmp2 = (char*)malloc(tr->size*sizeof(char));
    		memcpy(tmp2,tr->data,tr->size*sizeof(char));

    		printf("Parsing tracker data\n");
    		g_tracker_response = get_tracker_data(tmp2,tr->size);
    
    		if(tmp)
    		{
      			free(tmp2);
      			tmp2 = NULL;
    		}

    		printf("Num Peers: %d\n",g_tracker_response->numpeers);

		// 更新peer列表
		// 置peer列表头指针为空
		peer_head = NULL;
    		for(i=0; i<g_tracker_response->numpeers; i++)
    		{
      			//printf("Peer id: %d\n",g_tracker_response->peers[i].id);
      	//		printf("Peer id: ");
      			int idl;
	//		printf("%s\n", g_tracker_response->peers[i].id);
      			/*for(idl=0; idl<20; idl++)
        		printf("%d ",(unsigned char)g_tracker_response->peers[i].id[idl]);*/
      	//		printf("\n");
      	//		printf("Peer ip: %s\n",g_tracker_response->peers[i].ip);
      	//		printf("Peer port: %d\n",g_tracker_response->peers[i].port);

			// peer_list中不能保存自己的peer
			if(strcmp(g_my_ip, g_tracker_response->peers[i].ip) == 0 && g_peerport == g_tracker_response->peers[i].port)
				continue;
			// 加入一个新的peer节点到peer列表中
			if(peer_head == NULL)
			{
				peer_head = (peer_list*)malloc(sizeof(peer_list));
				memset(peer_head, 0, sizeof(peer_list));
				for(int k = 0; k < 20; k++)
					peer_head->this_peer.id[k] = g_tracker_response->peers[i].id[k];
				peer_head->this_peer.port = g_tracker_response->peers[i].port;
				peer_head->this_peer.ip = (char*)malloc(strlen(g_tracker_response->peers[i].ip)+1);
				memset(peer_head->this_peer.ip, 0, strlen(g_tracker_response->peers[i].ip)+1);
				strcpy(peer_head->this_peer.ip, g_tracker_response->peers[i].ip);
				peer_head->next = NULL;
			}
			else
			{
				peer_list* p = (peer_list*)malloc(sizeof(peer_list));
                                memset(p, 0, sizeof(peer_list));
                                for(int k = 0; k < 20; k++)
                                        p->this_peer.id[k] = g_tracker_response->peers[i].id[k];
                                p->this_peer.port = g_tracker_response->peers[i].port;
				p->this_peer.ip = (char*)malloc(strlen(g_tracker_response->peers[i].ip)+1);
                                memset(p->this_peer.ip, 0, strlen(g_tracker_response->peers[i].ip)+1);
                                strcpy(p->this_peer.ip, g_tracker_response->peers[i].ip);
                                p->next = peer_head;
				peer_head = p;
			}
    		}
	
		// 打印peer列表
		peer_list* p = peer_head;
		while(p != NULL)
		{
			printf("id: %s\n", p->this_peer.id);
			printf("ip:%s\n", p->this_peer.ip);
			printf("port:%d\n", p->this_peer.port);
			p = p->next;
		}
		// 更新peer列表
		printf("睡眠%d秒...\n", g_tracker_response->interval);  

		// 启动线程与其他peer进行交互
		if(firsttime == 1)		
		{
			p_num = (int*)malloc(sizeof(int)*g_num_pieces);
                        for(int i = 0; i < g_num_pieces; i++)
                                p_num[i] = 0;
			// 设置缓冲区接口
			Buffer = g_filedata;

			for(int i = 0; i < MAX_PEERS; i++)
                        	peer_pool[i] = NULL;
			// 开启4个线程用于交互
			pthread_t thread[1000];
			peer_list* p = peer_head;
			int pthread_count = 1;
			pthread_create(&thread[600],NULL, print_file, NULL);
			while(p != NULL)
			{
				if(count < MAX_PEERS)
				{
//					printf("start thread\n");
					pthread_create(&thread[pthread_count++], NULL, set_connection, (void*)p);
				}	
				else
					break;
				p = p->next;
			}
//			printf("start 4 thread\n");
			//pthread_t thread[4];
			//pthread_create(&thread[0], NULL, set_connection, (void *)peer_head);	
			printf("g_num_pieces: %d\n", g_num_pieces);
			pthread_create(&thread[pthread_count++], NULL, listen_connection, NULL);
			pthread_create(&thread[pthread_count++], NULL, reset_pmark, NULL);
			
			pthread_create(&thread[pthread_count++], NULL, judgeTimeOut, NULL);
			pthread_create(&thread[pthread_count++], NULL, set_request,NULL);
			firsttime = 0;
		}
    		// 必须等待td->interval秒, 然后再发出下一个GET请求
    		sleep(g_tracker_response->interval);
//		sleep(60*30);
//		sleep(10);
  	}
 
  	// 睡眠以等待其他线程关闭它们的套接字, 只有在用户按下ctrl-c时才会到达这里
  	sleep(2);

  	exit(0);
}

