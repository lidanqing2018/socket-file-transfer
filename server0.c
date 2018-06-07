#include<netinet/in.h> // sockaddr_in 
#include<sys/types.h>  // socket 
#include<sys/socket.h> // socket 
#include<stdio.h>    // printf 
#include<stdlib.h>   // exit 
#include<string.h>   // bzero 
  
#define SERVER_PORT 8000 
#define LENGTH_OF_LISTEN_QUEUE 20 
#define BUFFER_SIZE 1024 
#define FILE_NAME_MAX_SIZE 512 
  
int main(void) 
{ 
  // 声明并初始化一个服务器端的socket地址结构 
  struct sockaddr_in server_addr; //server's address information存储在套接字结构体里
  bzero(&server_addr, sizeof(server_addr)); 
  server_addr.sin_family = AF_INET; //创建套接字时，用该字段指定地址家族，对于TCP/IP协议的，必须设置为AF_INET
  server_addr.sin_addr.s_addr = htons(INADDR_ANY); //自动填上所运行的机器的 IP 地址
  server_addr.sin_port = htons(SERVER_PORT); //server地址的端口号,（本机到网络字节顺序的转换：host to network short）
  
  // 创建socket，若成功，返回socket描述符 
  int server_socket_fd = socket(PF_INET, SOCK_STREAM, 0); 
  if(server_socket_fd < 0) 
  { 
    perror("Create Socket Failed:"); 
    exit(1); 
  } 
   //socket函数,int socket(int domain, int type, int protocol)
   //socket在错误的时候返回-1
  int opt = 1; 
  setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); 
  
  // 绑定socket和socket地址结构 
  if(-1 == (bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)))) 
  { 
    perror("Server Bind Failed:"); 
    exit(1); 
  } 
   //该函数将本地主机地址以及端口号与所创建的套接字绑定起来
   //bind函数：int bind(int sockfd, struct sockaddr *my_addr, int addrlen);
   //sockfd 是调用 socket 返回的文件描述>符。my_addr 是指向数据结//构 struct sockaddr >的指针，它保存地址(即端口和 IP 地址) 信息。
   // addrlen 设置为 sizeof(struct sockaddr)。
   //bind() 在错误的时候返回-1 

  // socket监听 
  if(-1 == (listen(server_socket_fd, LENGTH_OF_LISTEN_QUEUE))) 
  { 
    perror("Server Listen Failed:"); 
    exit(1); 
  } 
  	//int listen(int sockfd, int backlog); 
	//sockfd 是调用 socket() 返回的套接字文件描述符。backlog 是在进入 队列中允许的连接数目。
	//listen 监听连接
	//发生错误的时候返回-1  

  while(1) 
  { 
    // 定义客户端的socket地址结构 
    struct sockaddr_in client_addr; 
    socklen_t client_addr_length = sizeof(client_addr); 
  
    // 接受连接请求，返回一个新的socket(描述符)，这个新socket用于同连接的客户端通信 
    // accept函数会把连接到的客户端信息写到client_addr中 
    int new_server_socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_addr, &client_addr_length); 
    if(new_server_socket_fd < 0) 
    { 
      perror("Server Accept Failed:"); 
      break; 
    } 
    	//int accept(int sockfd, void *addr, int *addrlen); 
	//sockfd 是和 listen() 中一样的套接字描述符。addr 是个指向局部的数据结构sockaddr_in的指针。这是要求接入的信息所要去的地方（测定那个地址在那个端口呼叫，即客户端口号）。在它的地址传递给 accept 之 前，addrlen 是个局部的整形变量，设置为 sizeof(struct sockaddr_in)。
	//accept函数
	//在错误时返回-1


    // recv函数接收数据到缓冲区buffer中 
    char buffer[BUFFER_SIZE]; 
    bzero(buffer, BUFFER_SIZE); 
    if(recv(new_server_socket_fd, buffer, BUFFER_SIZE, 0) < 0) 
    { 
      perror("Server Recieve Data Failed:"); 
      break; 
    } 
    //int recv(int sockfd, void *buf, int len, unsigned int flags); 
    //sockfd 是要读的套接字描述符。buf 是要读的信息的缓冲。len 是缓冲的最大长度。flags 可以设置为0. recv() 返回实际读入缓冲的数据的字节数。或者在错误的时候返回-1

    // 然后从buffer(缓冲区)拷贝到file_name中 
    char file_name[FILE_NAME_MAX_SIZE+1]; 
    bzero(file_name, FILE_NAME_MAX_SIZE+1); 
    strncpy(file_name, buffer, strlen(buffer)>FILE_NAME_MAX_SIZE?FILE_NAME_MAX_SIZE:strlen(buffer)); 
    printf("%s\n", file_name); 
  
    // 打开文件并读取文件数据 
    FILE *fp = fopen(file_name, "r"); 
    if(NULL == fp) 
    { 
      printf("File:%s Not Found\n", file_name); 
    } 
    else
    { 
      bzero(buffer, BUFFER_SIZE); 
      int length = 0; 
      // 每读取一段数据，便将其发送给客户端，循环直到文件读完为止 
      while((length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0) 
      { 
        if(send(new_server_socket_fd, buffer, length, 0) < 0) 
        { 
          printf("Send File:%s Failed./n", file_name); 
          break; 
        } 
        bzero(buffer, BUFFER_SIZE); 
      } 
      //int send(int sockfd, const void *msg, int len, int flags); 
      //sockfd 是发送数据的套接字描述符(或者是调用 socket() 或者是 accept() 返回的。)msg 是指向你想发送的数据的指针。len 是数据的长度。 把 flags 设置为 0 。它在错误的时候返回-1

      // 关闭文件 
      fclose(fp); 
      printf("File:%s Transfer Successful!\n", file_name); 
    } 
    // 关闭与客户端的连接 
    close(new_server_socket_fd); 
  } 
  // 关闭监听用的socket 
  close(server_socket_fd); 
  return 0; 
} 
