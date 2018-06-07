#include<netinet/in.h>  // sockaddr_in 
#include<sys/types.h>  // socket 
#include<sys/socket.h>  // socket 
#include<stdio.h>    // printf 
#include<stdlib.h>    // exit 
#include<string.h>    // bzero 
  
#define SERVER_PORT 8000 
#define BUFFER_SIZE 1024 
#define FILE_NAME_MAX_SIZE 512 
  
int main() 
{ 
  // 声明并初始化一个客户端的socket地址结构 
  struct sockaddr_in client_addr;   //client's address information存储在套接字结构体里
  bzero(&client_addr, sizeof(client_addr)); // 自定义缓冲区
  client_addr.sin_family = AF_INET; //创建套接字时，用该字段指定地址家族，对于TCP/IP协议的，必须设置为AF_INET
  client_addr.sin_addr.s_addr = htons(INADDR_ANY); //自动填上所运行的机器的 IP 地址
  client_addr.sin_port = htons(0); //client地址的端口号
  
  // 创建socket，若成功，返回socket描述符 
  int client_socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
  if(client_socket_fd < 0) 
  { 
    perror("Create Socket Failed:"); 
    exit(1); 
  } 
   //socket函数,int socket(int domain, int type, int protocol)
   //socket在错误的时候返回-1
  
  // 绑定客户端的socket和客户端的socket地址结构 非必需 
  if(-1 == (bind(client_socket_fd, (struct sockaddr*)&client_addr, sizeof(client_addr)))) 
  { 
    perror("Client Bind Failed:"); 
    exit(1); 
  } 
  
  //该函数将本地主机地址以及端口号与所创建的套接字绑定起来
   //bind函数：int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
   //sockfd 是调用 socket 返回的文件描述符。my_addr 是指向数据结构 struct sockaddr的指针，它保存地址(即端口和 IP 地址) 信息。
   // addrlen 设置为 sizeof(struct sockaddr)。
   //bind() 在错误的时候返回-1 


  // 声明一个服务器端的socket地址结构，并用服务器那边的IP地址及端口对其进行初始化，用于后面的连接 
  struct sockaddr_in server_addr; //server's address information存储在套接字结构体里 
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET; //创建套接字时，用该字段指定地址家族，对于TCP/IP协议的，必须设置为AF_INET
  if(inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) == 0) 
  { 
    perror("Server IP Address Error:"); 
    exit(1); 
  } 
   //int inet_pton(int family,const char *strptr,void *addrptr);
   //该函数完成两个功能：1.字符串->二进制数值  2.主机字节序->网络字节序（所以调用此函数后不需htonl了)
   //第二个参数是ip地址字符串的指针
   //用法：inet_pton(AF_INET,argv[1],&servaddr.sin_addr);
   //第三个参数使用&servaddr.sin_addr.s_addr也可以通过
   server_addr.sin_port = htons(SERVER_PORT); //server地址的端口号,（本机到网络字节顺序的转换：host to network short）
  socklen_t server_addr_length = sizeof(server_addr); 
  
  // 向服务器发起连接，连接成功后client_socket_fd代表了客户端和服务器的一个socket连接 
  if(connect(client_socket_fd, (struct sockaddr*)&server_addr, server_addr_length) < 0) 
  { 
    perror("Can Not Connect To Server IP:"); 
    exit(0); 
  } 
   //int connect(int sockfd, struct sockaddr *serv_addr, int addrlen); 	
   //sockfd 是系统调用 socket() 返回的套接字文件描述符。serv_addr 是 保存着目的地端口和 IP 地址的数据结构 struct sockaddr。addrlen 设置 为 server_addr_length。 
   //connect 用于建立连接，sever_addr是保存着服务器IP地址和端口号的数据结构struct sockaddr.
   //connect() 的返回值--它在错误的时候返回-1


   // 输入文件名 并放到缓冲区buffer中等待发送 
  char file_name[FILE_NAME_MAX_SIZE+1]; 
  bzero(file_name, FILE_NAME_MAX_SIZE+1); 
  printf("Please Input File Name On Server:\t"); 
  scanf("%s", file_name); 
  //从buffer中拷贝出filename
  char buffer[BUFFER_SIZE]; 
  bzero(buffer, BUFFER_SIZE); 
  strncpy(buffer, file_name, strlen(file_name)>BUFFER_SIZE?BUFFER_SIZE:strlen(file_name)); 
  
  // 向服务器发送buffer中的数据 
  if(send(client_socket_fd, buffer, BUFFER_SIZE, 0) < 0) 
  { 
    perror("Send File Name Failed:"); 
    exit(1); 
  } 
  
  // 打开文件，准备写入 
  FILE *fp = fopen(file_name, "w"); 
  if(NULL == fp) 
  { 
    printf("File:\t%s Can Not Open To Write\n", file_name); 
    exit(1); 
  } 
  
  // 从服务器接收数据到buffer中 
  // 每接收一段数据，便将其写入文件中，循环直到文件接收完并写完为止 
  bzero(buffer, BUFFER_SIZE); 
  int length = 0; 
  while((length = recv(client_socket_fd, buffer, BUFFER_SIZE, 0)) > 0) 
  { 
    if(fwrite(buffer, sizeof(char), length, fp) < length) 
    { 
      printf("File:\t%s Write Failed\n", file_name); 
      break; 
    } 
    bzero(buffer, BUFFER_SIZE); 
  } 
  
  // 接收成功后，关闭文件，关闭socket 
  printf("Receive File:\t%s From Server IP Successful!\n", file_name); 
  close(fp); 
  close(client_socket_fd); 
  return 0; 
} 
