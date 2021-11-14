#pragma once
#include <easy_tcp/connection.h>
#include <easy_tcp/listener.h>
#include <easy_tcp/service.h>
#include <vector>
#include <iostream>

namespace easy_tcp{
    template <class T>
    struct Server{
        static_assert(std::is_base_of<Service, T>::value, "T must inherit from Service");

        Server(){
            listening = false;
        }

        bool start(int port){
            if (listening) return false;
            if (!listener.start(port)) return false;
            incoming_connections_thread = std::thread([this] () {
                listening = true;
                int i = 0;
                while (listening){
                    auto incoming_connection = listener.wait_for_client(1);
                    if (incoming_connection >= 0){
                        auto new_service = new T();
                        clients.push_back(new_service);
                        new_service->start(incoming_connection);
                    }
                }

                });
            while (!listening);
            return true;
        }

        void stop(){
            if (!listening) return;
            listening = false;
            incoming_connections_thread.join();
            listener.stop();
            for(auto client:clients) {
                client->stop();
                delete (client);
            }
            clients.clear();
        }

        ~Server(){
            stop();
        }
        Listener listener;
    private:
        std::atomic<bool> listening;
        std::thread incoming_connections_thread;
        std::vector<Service *> clients;
    };
}