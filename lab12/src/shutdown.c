#include "btdata.h"
#include "util.h"
#include "assert.h"
extern int g_done;

// ��ȷ�Ĺرտͻ���
void client_shutdown(int sig)
{
  
	// ����ȫ��ֹͣ������ֹͣ���ӵ�����peer, �Լ���������peer������.                                                                    Set global stop variable so that we stop trying to connect to peers and
  	// �����������peer���ӵ��׽��ֺ����ӵ�����peer���߳�.
  	g_done = 1;

	// ��tracker����stopped���ݰ�
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

	
	// �ر��׽���, �Ա��ٴ�ʹ��
      	shutdown(sockfd,SHUT_RDWR);
   	if(close(sockfd) < 0)
	{
		printf("fail to close sockfd connected to tracker\n");	
	}
	// �ر���peers�������׽���
	for(int i = 0; i < MAX_PEERS; i++)
	{
		if(peer_pool[i] != NULL && peer_pool[i]->sockfd >= 0)
		{
			// �ر���֮�������׽���
			shutdown(peer_pool[i]->sockfd,SHUT_RDWR);
		}
	}

	// ����ļ�δ����,������.bt�ļ�
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
