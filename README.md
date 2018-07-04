# emb_linux_project
## 大作业要求
在Linux下实现一个基于C/S模式的即时通讯系统，要求在以下三种情况下均能正常通信1.ClientA在子网，ClientB也在同一个子网；2.ClientA在子网，ClientB在外网；3.ClientA在一个子网内，ClientB在另一个子网内。可采用TCP或UDP模型。扩展功能，1.实现文件互传；2.实现群组通信。

## 协议设计

请求：
```
FROM_ID TO_ID
REQUEST_TYPE
BODY
```
BODY可省略 
## 登录
客户端请求：\
FROM_ID=0，TO_ID=0。 \
REQUEST_TYPE=LOGIN \
服务端响应： \
FROM_ID=0，TO_ID=分配给客户端的随机ID。 \
REQUEST_TYPE=MEMBER_ID 

## 聊天
客户端请求： \
FROM_ID=FROM_ID，TO_ID=TO_ID。 \
REQUEST_TYPE=SEND \
当私聊时，TO_ID是对方ID。 \
当群聊时，TO_ID是群组ID。 \ \
客户端会给所有需要接收到消息的客户响应： \
FROM_ID=发送方ID，TO_ID=TO_ID。 \
REQUEST_TYPE=SEND。 

## 创建群组
客户端请求： \
FROM_ID=FROM_ID，TO_ID=0 \
REQUEST_TYPE=CREATE_GROUP \
响应： \
FROM_ID=分配的随机群组ID，TO_ID=请求客户的ID \
REQUEST_TYPE=GROUP_ID 

## 加入群组
客户端请求： \
FROM_ID=FROM_ID，TO_ID=GROUP_ID \
REQUEST_TYPE=ENTER_GROUP 

## 文件传输
客户端请求服务端： \
FROM_ID=FROM_ID，TO_ID=TO_ID \
REQUEST_TYPE=TRANS_FILE \

服务端先给接收端发送请求，让其开一个线程准备接收文件： \
FROM_ID=0，TO_ID=接收方ID \
REQUEST_TYPE=RECV_FILE \

服务端响应TO_ID的ADDR： \
FROM_ID=0，TO_ID=发送请求的服务端ID \
REQUEST_TYPE=SEND_FILE \
BODY="<ip_address>" \

然后发送方新建一个线程向对应的ip发送文件。 \
接收文件的端口是3208，每个客户端启动时自动创建一个线程进行接收。 \

发送方接着也打开一个ip和端口，发送文件。
