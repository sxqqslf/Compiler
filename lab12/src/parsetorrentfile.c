
#include "bencode.h"
#include "util.h"
#include "sha1.h"
#include "assert.h"
// 定义的公网服务器URL节点列表
tracker_node tracker_list[100];
int tracker_count = 0;

// 注意: 这个函数只能处理单文件模式torrent
torrentmetadata_t* public_parsetorrentfile(char* filename)
{
  	int i;
  	be_node* ben_res;
  	FILE* f;
  	int flen;
  	char* data;
  	torrentmetadata_t* ret;
  	// 打开文件, 获取数据并解码字符串
  	f = fopen(filename,"r");
  	flen = file_len(f);
  	data = (char*)malloc(flen*sizeof(char));
  	fread((void*)data,sizeof(char),flen,f);
  	fclose(f);
  	ben_res = be_decoden(data,flen);
  	// 遍历节点, 检查文件结构并填充相应的字段.
  	if(ben_res->type != BE_DICT)
  	{
    		perror("Torrent file isn't a dictionary");
    		exit(-13);
  	}

  	ret = (torrentmetadata_t*)malloc(sizeof(torrentmetadata_t));
  	memset(ret, 0, sizeof(torrentmetadata_t));
  	if(ret == NULL)
  	{
    		perror("Could not allocate torrent meta data");
    		exit(-13);
  	}

  	// 计算这个torrent的info_hash值
  	// 注意: SHA1函数返回的哈希值存储在一个整数数组中, 对于小端字节序主机来说,
  	// 在与tracker或其他peer返回的哈希值进行比较时, 需要对本地存储的哈希值
  	// 进行字节序转换. 当你生成发送给tracker的请求时, 也需要对字节序进行转换.
  	char* info_loc, *info_end;
  	info_loc = strstr(data,"infod");  // 查找info键, 它的值是一个字典
  	info_loc += 4; // 将指针指向值开始的地方
  	info_end = data+flen-1;
	int k;
	char* pp;
        //printf("%c%c%c%c\n", data[flen-6],data[flen-5],data[flen-4], data[flen-3],data[flen-2],data[flen-1]);
        for(k = 0; k <= flen - 8; k++)
        {
                pp = (char*)malloc(8);
                strncpy(pp, &data[k], 7);
                pp[7] = '\0';
                if(strcmp(pp, "5:nodes") == 0)
                {
                        //printf("this\n");
                        break;
                }
        }
	if(k != flen - 7)
	{       
        	data[k] = 'e';
		info_end = &data[k];
	}
  	// 去掉结尾的e
  	if(*info_end == 'e')
  	{
    		--info_end;
 	}
  	char* p;
  	int len = 0;
	
//	printf("INFO:\n");
	
  	for(p=info_loc; p<=info_end; p++)
 	{
//		printf("%c", *p);
		len++;
	}
//	printf("\n");
  	// 计算上面字符串的SHA1哈希值以获取info_hash
  	SHA1Context sha;
  	SHA1Reset(&sha);
  	SHA1Input(&sha,(const unsigned char*)info_loc,len);
  	if(!SHA1Result(&sha))
  	{
    		printf("FAILURE\n");
  	}
  
  	memcpy(ret->info_hash,sha.Message_Digest,20);
  	printf("SHA1:\n");
  	for(i=0; i<5; i++)
  	{
    		printf("%08X ",ret->info_hash[i]);
  	}
  	printf("\n");

  	// 检查键并提取对应的信息
  	int filled=0;
	int flag = 0;
  	for(i=0; ben_res->val.d[i].val != NULL; i++)
  	{
    		int j;
		/*
    		if(!strncmp(ben_res->val.d[i].key,"announce",strlen("announce")) && flag == 0)
    		{
			flag = 1;
	  		ret->announce = (char*)malloc(strlen(ben_res->val.d[i].val->val.s)*sizeof(char));
	  		memcpy(ret->announce,ben_res->val.d[i].val->val.s,strlen(ben_res->val.d[i].val->val.s));
      			filled++;
    		}*/
		// 处理announce list
		if(strcmp(ben_res->val.d[i].key,"announce-list") == 0)
		{
			be_node* u_list = ben_res->val.d[i].val;	//得到URL列表
			assert(u_list->type == BE_LIST);
			// 依次得到每个URL表
			int i;
			for(i = 0; u_list->val.l[i] != NULL; i++)
			{
				be_node* this_list = u_list->val.l[i];
				assert(this_list->type == BE_LIST && this_list->val.l[0] != NULL);
				// 得到该URL的内容
				be_node* this_url_str = this_list->val.l[0];
				assert(this_url_str->type == BE_STR);	
				tracker_node* this_node = (tracker_node*)malloc(sizeof(tracker_node));
				memset(this_node, 0, sizeof(tracker_node));
				memset(tracker_list[tracker_count].announce, 0, 100);	
				strcpy(tracker_list[tracker_count].announce, this_url_str->val.s);
				tracker_list[tracker_count].next = NULL;
				tracker_count++;
				// 将其加入到公网服务器的URL列表中
			}			
		}
    		// info是一个字典, 它还有一些其他我们关心的键
    		if(!strncmp(ben_res->val.d[i].key,"info",strlen("info")))
    		{
      			be_dict* idict;
      			if(ben_res->val.d[i].val->type != BE_DICT)
      			{
        			perror("Expected dict, got something else");
        			exit(-3);
      			}
      			idict = ben_res->val.d[i].val->val.d;
      			// 检查这个字典的键
      			for(j=0; idict[j].key != NULL; j++)
      			{ 
        			if(!strncmp(idict[j].key,"length",strlen("length")))
        			{
          				ret->length = idict[j].val->val.i;
          				filled++;
        			}
        			if(!strncmp(idict[j].key,"name",strlen("name")))
        			{
          				ret->name = (char*)malloc((strlen(idict[j].val->val.s)+1)*sizeof(char));
          				memcpy(ret->name,idict[j].val->val.s,strlen(idict[j].val->val.s));
	 				ret->name[strlen(idict[j].val->val.s)] = '\0';
			printf("!!!!!!!!!!!!!!!!!!!!!!the name is %s and the len is %d\n", ret->name, strlen(idict[j].val->val.s)+1);
          				filled++;
        			}
        			if(!strncmp(idict[j].key,"piece length",strlen("piece length")))
        			{
          				ret->piece_len = idict[j].val->val.i;
          				filled++;
        			}
        			if(!strncmp(idict[j].key,"pieces",strlen("pieces")))
        			{
          				int num_pieces = ret->length/ret->piece_len;
          				if(ret->length % ret->piece_len != 0)
            					num_pieces++;
          				ret->pieces = (char*)malloc(num_pieces*20);
          				memcpy(ret->pieces,idict[j].val->val.s,num_pieces*20);
          				ret->num_pieces = num_pieces;
          				filled++;
       	 			}

    	 	 	} // for循环结束
    		} // info键处理结束
  	}
  
  	// 确认已填充了必要的字段
  
  	be_free(ben_res);  
 
  	if(filled < 5)
  	{
    		printf("Did not fill necessary field\n");
    		return NULL;
  	}
  	else
    		return ret;
}


// 注意: 这个函数只能处理单文件模式torrent
torrentmetadata_t* private_parsetorrentfile(char* filename)
{
  int i;
  be_node* ben_res;
  FILE* f;
  int flen;
  char* data;
  torrentmetadata_t* ret;
  // 打开文件, 获取数据并解码字符串
  f = fopen(filename,"r");
  flen = file_len(f);
  data = (char*)malloc(flen*sizeof(char));
  fread((void*)data,sizeof(char),flen,f);
  fclose(f);
  ben_res = be_decoden(data,flen);

  // 遍历节点, 检查文件结构并填充相应的字段.
  if(ben_res->type != BE_DICT)
  {
    perror("Torrent file isn't a dictionary");
    exit(-13);
  }

  ret = (torrentmetadata_t*)malloc(sizeof(torrentmetadata_t));
  memset(ret, 0, sizeof(torrentmetadata_t));
  if(ret == NULL)
  {
    perror("Could not allocate torrent meta data");
    exit(-13);
  }

  // 计算这个torrent的info_hash值
  // 注意: SHA1函数返回的哈希值存储在一个整数数组中, 对于小端字节序主机来说,
  // 在与tracker或其他peer返回的哈希值进行比较时, 需要对本地存储的哈希值
  // 进行字节序转换. 当你生成发送给tracker的请求时, 也需要对字节序进行转换.
  char* info_loc, *info_end;
  info_loc = strstr(data,"infod");  // 查找info键, 它的值是一个字典
  info_loc += 4; // 将指针指向值开始的地方
  info_end = data+flen-1;
  // 去掉结尾的e
  if(*info_end == 'e')
  {
    --info_end;
  }
  
  char* p;
  int len = 0;
  for(p=info_loc; p<=info_end; p++) len++;

  // 计算上面字符串的SHA1哈希值以获取info_hash
  SHA1Context sha;
  SHA1Reset(&sha);
  SHA1Input(&sha,(const unsigned char*)info_loc,len);
  if(!SHA1Result(&sha))
  {
    printf("FAILURE\n");
  }
  
  memcpy(ret->info_hash,sha.Message_Digest,20);
  printf("SHA1:\n");
  for(i=0; i<5; i++)
  {
    printf("%08X ",ret->info_hash[i]);
  }
  printf("\n");

  // 检查键并提取对应的信息
  int filled=0;
  for(i=0; ben_res->val.d[i].val != NULL; i++)
  {
    int j;
    if(!strncmp(ben_res->val.d[i].key,"announce",strlen("announce")))
    {
	  ret->announce = (char*)malloc(strlen(ben_res->val.d[i].val->val.s)*sizeof(char));
	  memcpy(ret->announce,ben_res->val.d[i].val->val.s,strlen(ben_res->val.d[i].val->val.s));
      filled++;
    }
    // info是一个字典, 它还有一些其他我们关心的键
    if(!strncmp(ben_res->val.d[i].key,"info",strlen("info")))
    {
      be_dict* idict;
      if(ben_res->val.d[i].val->type != BE_DICT)
      {
        perror("Expected dict, got something else");
        exit(-3);
      }
      idict = ben_res->val.d[i].val->val.d;
      // 检查这个字典的键
      for(j=0; idict[j].key != NULL; j++)
      { 
        if(!strncmp(idict[j].key,"length",strlen("length")))
        {
          ret->length = idict[j].val->val.i;
          filled++;
        }
        if(!strncmp(idict[j].key,"name",strlen("name")))
        {
          	ret->name = (char*)malloc((strlen(idict[j].val->val.s)+1)*sizeof(char));
          	memcpy(ret->name,idict[j].val->val.s,strlen(idict[j].val->val.s));
			printf("!!!!!!!!!!!!!!!!!!!!!!the name is %s and the len is %d\n", ret->name, strlen(idict[j].val->val.s)+1);
	 	ret->name[strlen(idict[j].val->val.s)] = '\0';
          filled++;
        }
        if(!strncmp(idict[j].key,"piece length",strlen("piece length")))
        {
          ret->piece_len = idict[j].val->val.i;
          filled++;
        }
        if(!strncmp(idict[j].key,"pieces",strlen("pieces")))
        {
          int num_pieces = ret->length/ret->piece_len;
          if(ret->length % ret->piece_len != 0)
            num_pieces++;
          ret->pieces = (char*)malloc(num_pieces*20);
          memcpy(ret->pieces,idict[j].val->val.s,num_pieces*20);
          ret->num_pieces = num_pieces;
          filled++;
        }

      } // for循环结束
    } // info键处理结束
  }
  
  // 确认已填充了必要的字段
  
  be_free(ben_res);  
 
  if(filled < 5)
  {
    printf("Did not fill necessary field\n");
    return NULL;
  }
  else
    return ret;
}
