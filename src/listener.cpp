#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <fcntl.h>
#include <easy_tcp/listener.h>

using namespace std;

namespace easy_tcp {
    bool Listener::start(int port) {
        struct sockaddr_in m_serverAddress;
        file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
        if (file_descriptor == -1) { //socket failed
            cerr << "Listener: socket creation failed" << endl;
            return false;
        }
        int option = 1;
        setsockopt(file_descriptor, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        memset(&m_serverAddress, 0, sizeof(m_serverAddress));
        m_serverAddress.sin_family = AF_INET;
        m_serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        m_serverAddress.sin_port = htons(port);
        auto bindSuccess = bind(file_descriptor, (struct sockaddr *) &m_serverAddress, sizeof(m_serverAddress));
        int listenSuccess = listen(file_descriptor, queue_size);
        if (listenSuccess == -1) { // listen failed
            cerr << "Listener: listening failed (or bind failed silently)" << endl;
            return false;
        }
        return true;
    }

#pragma GCC push_options
#pragma GCC optimize("O0")

    int Listener::wait_for_client(int time_out) {
        if (time_out>0) {
            timeval tv;
            fd_set m_fds;
            tv.tv_sec = time_out;
            tv.tv_usec = 0;
            FD_ZERO(&m_fds);
            FD_SET(file_descriptor, &m_fds);
            int selectRet = select(file_descriptor + 1, &m_fds, NULL, NULL, &tv);
            if (selectRet == -1) { // select failed
                cerr << strerror(errno) << endl;
                return -1;
            } else if (selectRet == 0) { // timeout
                return -1;
            } else if (!FD_ISSET(file_descriptor, &m_fds)) { //no new client
                return -1;
            }
        }
        sockaddr_in m_clientAddress;
        socklen_t so_size = sizeof(m_clientAddress);
        int client_file_descriptor = accept(file_descriptor, (struct sockaddr *) &m_clientAddress, &so_size);
        if (client_file_descriptor == -1) { // accept failed
            return -1;
        }
        //client_ip_address = inet_ntoa(m_clientAddress.sin_addr);
        return client_file_descriptor;
    }

#pragma GCC pop_options

    void Listener::stop() {
        shutdown(file_descriptor,SHUT_RDWR);
        close(file_descriptor);
    }

    Listener::~Listener() {
        stop();
    }
}
