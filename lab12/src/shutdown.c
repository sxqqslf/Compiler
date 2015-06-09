#include "btdata.h"
#include "util.h"
#include "assert.h"
extern int g_done;

// 正确的关闭客户端
void client_shutdown(int sig)
{
  
	// 设置全局停止变量以停止连接到其他peer, 以及允许其他peer的连接.                                                                    Set global stop variable so that we stop trying to connect to peers and
  	// 这控制了其他peer连接的套接字和连接到其他peer的线程.
  	g_done = 1;

	// 向tracker发送stopped数据包
	int mlen;
	char* MESG = make_tracker_request(BT_STOPPED,&mlen);
	printf("##STOP:\n");
	for(int i=0; i<mlen; i++)
        	printf("%c",MESG[i]);
	int sockfd = connect_to_host(g_tracker_ip, g_tracker_port);
	if(sockfd < 0)
	{
		printf("fail to connect to tracker\n");
		return;
	}
	if(send(sockfd, MESG, mlen, 0) <= 0)
		printf("fail to send stopped data to tracker\n");
	else
		printf("succeed to send stopped data to tracker\n");

	
	// 关闭套接字, 以便再次使用
      	shutdown(sockfd,SHUT_RDWR);
   	if(close(sockfd) < 0)
	{
		printf("fail to close sockfd connected to tracker\n");	
	}
	// 关闭与peers交互的套接字
	for(int i = 0; i < MAX_PEERS; i++)
	{
		if(peer_pool[i] != NULL && peer_pool[i]->sockfd >= 0)
		{
			// 关闭与之相连的套接字
			shutdown(peer_pool[i]->sockfd,SHUT_RDWR);
		}
	}

	// 如果文件未传完,就生成.bt文件
	FILE *f;
       	f = fopen(file_name,"r");
	if(f == NULL)
	{
		FILE *bt;
		char* bf_name = (char*)malloc(strlen(file_name)+4);
                memset(bf_name, 0, strlen(file_name)+4);
                strncpy(bf_name, file_name, strlen(file_name));
                strcpy(&bf_name[strlen(file_name)], ".bt");
              	bt = fopen(bf_name,"w");
              	assert(bt!=NULL);
               	fwrite(Buffer,g_filelen,1,bt);
               	fclose(bt);
		printf("write\n");	
	}	
	else
		fclose(f);
}
