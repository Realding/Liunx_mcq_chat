# Liunx_mcq_chat
liunx下的聊天程序
编写使用环境Ubuntu16.4

1.	项目结构：
（1）	登录Shell脚本：login.sh
（2）	服务端：mcq_server.c
（3）	客户端：mcq_client.c
（4）	Makefile文件
（5）	账户存储文档：mypasswd.txt
（6）	聊天记录文档：memo.txt
2.	客户端：
（1）	主程序进行信息的输入与发送消息至服务端，使用一个另外的线程负责读取服务器返回的信息，并显示出来。
（2）	客户端使用login.sh进行登录，通过shell传入参数昵称，主线程中读取系统时间并显示给用户。可以使用用户名，密码，也可以匿名
（3）	通过输入控制命令完成请求操作进入聊天室后可以通过输入控制命令完成请求操作，使用 ‘\t’ 表示向服务端查询在线用户，’\s nickname’表示向用户发起私聊，’\f  filename’表示向正在私聊的用户发送文件
（4）	使用’Q’或’q’来退出聊天程序。
（5）	文件传输功能，主程序收到’\f’输入打开相应文件并发送给服务端，服务端接受到后转发至接收对象的客户端，子线程读到文件接受命令新建文件并存入文件信息完成文件的传输。
3.	服务端：
（1）	使用链表来存储所有已连接的客户端对应的fd文件描述符，id，用户昵称等信息，建立一个全局变量链表使得每个线程都能访问到。
（2）	使用多线程来对每个客户端连接进行管理。
（3）	服务端建立TCP套接字Socket来绑定端口，然后开始监听，等待客户端连接；
（4）	每当一个新的客户端发来连接请求，使用接收函数获得对应accpet_fd,建立一个新的客户端链表节点保存信息，读入客户端发来的昵称信息，也存入节点内；新建立一个线程开始运行，处理该客户端任务；主程序继续监听。
（5）	每个线程负责读入客户端发来的消息，判断是否为请求命令（查询在线用户，私聊请求，文件发送请求），若是分别进行处理，返回响应信息；否则判定为聊天信息，访问客户端链表除自己外的所有连接，发送该消息，并将消息存入聊天记录文件memo.txt。
4.	登录shell脚本：
（1）	首先输出欢迎及提示信息，可以进行登录，注册，游客登录，退出等操作；
（2）	读取用户选择并进入到相应处理分支，若输入非法，重新读取输入；
（3）	注册时打开账户信息文档mypasswd.txt判断注册用户名是否重复，输入密码昵称后将注册信息存入账户文档。
（4）	用户名登录将用户输入用户名，密码与mypasswd.txt的账户信息对比验证，成功后调用客户端程序client进入客户端，游客登录则只需昵称便可登录。



