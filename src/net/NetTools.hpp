//
// Created by 张亦乐 on 2018/6/22.
//

#ifndef NETTOOLS_HPP
#define NETTOOLS_HPP

int set_noblock_fd(int fd);
int set_nodelay_fd(int fd);
int create_socket();
int create_epoll_fd();
void set_reuse_addr(int sockfd);
void set_reuse_port(int sockfd);
#endif //NETTOOLS_HPP
