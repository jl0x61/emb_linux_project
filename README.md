# emb_linux_project
## 大作业要求
在Linux下实现一个基于C/S模式的即时通讯系统，要求在以下三种情况下均能正常通信1.ClientA在子网，ClientB也在同一个子网；2.ClientA在子网，ClientB在外网；3.ClientA在一个子网内，ClientB在另一个子网内。可采用TCP或UDP模型。扩展功能，1.实现文件互传；2.实现群组通信。

## 协议设计

请求：
```
FROM_ID TO_ID
<empty line>
REQUEST_TYPE
<empty line>
BODY
```
BODY可省略 \
响应：
```
STATUS
<empty line>
BODY
```
STATUS为OK时，BODY为对应响应。 \
STATUS为FAIL时，BODY为失败原因。 
## 登录
客户端请求：\
FROM_ID=0，TO_ID=0。 \
REQUEST_TYPE=LOGIN \
服务端响应： \
一个随机ID 

## 聊天
客户端请求： \
FROM_ID=FROM_ID，TO_ID=TO_ID。 \
REQUEST_TYPE=SEND \
当私聊时，TO_ID是对方ID。 \
当群聊时，TO_ID是群组ID。 

## 创建群组
客户端请求： \
FROM_ID=FROM_ID，TO_ID=0 \
REQUEST_TYPE=CREATE_GROUP \
响应： \
一个随机群组ID，并会将FROM_ID自动加入群。 

## 加入群组
客户端请求： \
FROM_ID=FROM_ID，TO_ID=GROUP_ID \
REQUEST_TYPE=ENTER_GROUP 

