#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 2048

const char* hot_topics[] = {
    "习近平的文化足迹",
    "香港恒生银行抢劫案嫌犯在深圳落网",
    "韩国总统李在明宣誓就职",
    "李在明当选韩国总统 任期正式开始",
    "马斯克怒批特朗普税改法案：令人作呕",
    "亚朵酒店被曝出现医院枕套",
    "追火箭上火星 科技旅游火出圈",
    "得知陶喆比外婆还大1岁 小孩哥震惊",
    "卫健委通报女游客遭毒蛇咬伤身亡",
    "赶考出行这些事项要留意"
};

void send_response(int client_socket, const char* content) {
    char response[BUFFER_SIZE];
    snprintf(response, BUFFER_SIZE,
        "HTTP/1.1 200 OK\r\n"
        "Access-Control-Allow-Origin: http://localhost:3000\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Content-Type: application/json\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s", content);
    write(client_socket, response, strlen(response));
}

void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    read(client_socket, buffer, BUFFER_SIZE);

    // 检查请求路径
    if (strstr(buffer, "GET /api/hot-topics")) {
        char content[BUFFER_SIZE] = "[";

        // 调整循环起始和tag生成逻辑
        for (int i = 0; i < 9; i++) {  // 只生成9个条目
            char item[512];
            snprintf(item, sizeof(item), 
                "%s{\"id\": %d, \"title\": \"%s\", \"tag\": \"%s\"}",
                (i > 0) ? "," : "",
                i+2,  // ID从2开始
                hot_topics[i+1],  // 从数组第二个元素开始
                (i % 3 == 0) ? "new" : (i % 3 == 1) ? "hot" : "");

            strncat(content, item, BUFFER_SIZE - strlen(content) - 1);
        }
        strcat(content, "]");
        send_response(client_socket, content);
    } else {
        // 返回404错误
        char *response = 
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: application/json\r\n"
            "\r\n"
            "{\"error\":\"Endpoint not found\"}";
        write(client_socket, response, strlen(response));
    }
    close(client_socket);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // 修改这里：只使用SO_REUSEADDR选项
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

   
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on port %d...\n", PORT);
    
    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }
        
        handle_request(client_socket);
        close(client_socket);
    }
    
    return 0;
}