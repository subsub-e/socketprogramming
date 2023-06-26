//  Created by 이승섭 on 2023/04/12.
// This is a C program for a simple web server that can handle GET requests.
// The server listens for incoming connections and accepts them.
// It then reads the incoming request and extracts the requested file name from it.
// If the file exists, it reads the file and sends it as a response to the client along with an HTTP header.



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <dirent.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define BACKLOG 10
 
// Function to handle HTTP requests
void http_handler(int, char[3][100]);
// Function to find the MIME type of a file
int find_mime(char*, char*);

int main(int argc, char *argv[]) {
    
    int server_socket, new_fd, sock_read;
    /*
     *listen on sock_fd,
     *new connection on new_fd
    */
    struct sockaddr_in my_addr; /* my address*/
    struct sockaddr_in their_addr; /* connector addr */
    socklen_t sin_size;
    char buf[4096];
    
    if(argc != 2){
        printf("입력이 잘못 되었습니다.");
        exit(1);
    }
    // Creating a Server Socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0){
        perror("socket");
        exit(1);
    }
    
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(atoi(argv[1])); /* short, network byte order */
    /* INADDR_ANY allows clients to connect to any one of
       the host’s IP address */
    my_addr.sin_addr.s_addr = INADDR_ANY;
    // Bind socket and server address information
    if (bind(server_socket, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1)
    {
        perror("bind");
        exit(1);
    }
    // Wait to receive a request from a client
    if (listen(server_socket, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }
    
    sin_size = sizeof(their_addr);
    // Loop that receives connection requests from clients and processes requests
    while(1)
    { /* main accept() loop */
        if ((new_fd = accept(server_socket, (struct sockaddr*)&their_addr,  &sin_size)) == -1)
        {
            perror("accept");
            exit(1);
        }
        else{
            printf("server: got connection from %s\n", inet_ntoa(their_addr.sin_addr));
        }
        
        memset(buf, 0, 4096);
        sock_read = read(new_fd, buf, 4095);
        if(sock_read < 0){
            perror("reading error");
            exit(1);
        }
        printf("%s\n", buf);
        
        // Extract the file name from the request
        char ptr[3][100]={};
        int i = 0, j = 0, k = 0, l = 0;
        while(buf[i++] != '/');
        while(buf[i] != ' '){
            ptr[2][l++] = buf[i];
            if(buf[i] == '.'){
                i++; j++; k = 0;
                continue;
            }
            ptr[j][k++] = buf[i++];
        }
        
        // Handle the HTTP request
        http_handler(new_fd, ptr);
        
        close(new_fd);
    }
    // close the socket
    close(server_socket);
    
    return 0;
}

// This function finds the MIME type of the file requested in the URI and prepares the HTTP response message.
int find_mime(char *ans, char *uri) {
    if(!strcmp(ans,"html")){ // if MIME type is html
        strcat(uri,"text/html\n\n");
        return 0; // return 0 to indicate MIME type was found
    }
    else{
        if(!strcmp(ans,"jpeg")){ // if MIME type is jpeg
            strcat(uri,"image/jpeg\n\n");
        }
        else if(!strcmp(ans,"gif")){ // if MIME type is gif
            strcat(uri,"image/gif\n\n");
        }
        else if(!strcmp(ans,"pdf")){ // if MIME type is pdf
            strcat(uri,"application/pdf\n\n");
        }
        else if(!strcmp(ans,"mp3")){ // if MIME type is mp3
            strcat(uri,"mp3\n\n");
        }
        return 1; // return 1 to indicate MIME type was not found
    }
}

void http_handler(int sock_fd, char a[3][100]) {
    char msg[200000]={}; // initialize message buffer
    char* reply;
    int size;
    // append header information to message buffer
    strcat(msg,"HTTP/1.1 200 OK\nAccept_Ranges: bytes\nContent-Type: ");
    // find MIME type of file requested in URI
    int flag = find_mime(a[1],msg);
    if(flag){
        // open file for reading in binary mode
        FILE *file = fopen(a[2],"rb");
        fseek(file, 0, SEEK_END);
        size = ftell(file);
        fseek(file, 0, SEEK_SET);
        reply = (char*)malloc(strlen(msg)+size);
        // append header information to response message
        strcat(reply, msg);
        
        while(1){
            char buff[size+1];
            memset(buff,0,size+1);
            // read file contents into buffer
            int now_read = fread(buff,1,size,file);
            // append file contents to response message
            if(now_read > 0) memcpy(reply+strlen(msg), buff, size);
            // if end of file is reached, break out of loop
            if(now_read <= size) break;
        }
        
        fclose(file);
        // send response message to client
        write(sock_fd, reply, strlen(msg)+size);
    }
    else{
        FILE *file = fopen(a[2], "r");
        
        if (file) {
            char s[4096];
            // read file contents into buffer
            
            while ((fgets(s,4096,file)) != NULL){
                // append file contents to response message
                strcat(msg, s);
            }
            
            // close file
            fclose(file);
            write(sock_fd,msg,strlen(msg));
        }
    }
    printf("%s\n",msg);
    printf("response success!\n\n");
 }
