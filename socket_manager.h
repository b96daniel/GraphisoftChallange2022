#ifndef SOCKET_CONNECTOR_H_INCLUDED
#define SOCKET_CONNECTOR_H_INCLUDED

#include <cstdio>
#include <iostream>
#include <string>

#include "platform_dep.h"

class socket_manager final {
    platform_dep::enable_socket _;
    platform_dep::tcp_socket socket_handler;
    bool valid = true;
    std::string remained_buffer;

    std::streamsize send(const char *data, std::streamsize size) {
        return ::send(socket_handler.get_handler(), data, size, 0);
    }

    std::streamsize recv(char *buffer, std::streamsize size) {
        return ::recv(socket_handler.get_handler(), buffer, size, 0);
    }

   public:
    socket_manager(const char *host_name, unsigned short port) {
        if (!socket_handler.valid()) {
            invalidate();
            throw std::runtime_error("Error: Cannot open a socket!");
        }

        hostent *host = gethostbyname(host_name);

        if (!host) {
            invalidate();
            throw std::runtime_error("Error: Cannot find host: " + std::string(host_name));
        }

        sockaddr_in socket_address;
        socket_address.sin_family = AF_INET;
        socket_address.sin_port = htons(port);
        socket_address.sin_addr.s_addr = inet_addr(inet_ntoa(*reinterpret_cast<in_addr *>(host->h_addr_list[0])));

        if (connect(socket_handler.get_handler(), reinterpret_cast<struct sockaddr *>(&socket_address), sizeof(socket_address)) != 0) {
            invalidate();
            std::string full_addr = std::string(host_name) + ":" + std::to_string(port);
            throw std::runtime_error("Error: Cannot connect to the server: " + full_addr + " error code: " + std::to_string(socketerrno));
        }

        // please use std::cerr for logging.
        std::cout.rdbuf(nullptr);
        std::cout.exceptions(std::ios_base::failbit);
        std::fclose(stdout);
    }

    virtual ~socket_manager() = default;

    void send_messages(const std::vector<std::string> &messages) {
        std::string message;

        for (std::size_t i = 0; i < messages.size(); ++i) {
            message += messages[i] + '\n';
        }
        message += ".\n";

        auto sent_bytes = this->send(message.c_str(), message.size());

        if (sent_bytes != static_cast<std::streamsize>(message.size())) {
            std::cerr << "[main] Warning: Couldn't send message properly: " << message << "\n";
            std::cerr << "[main] " << sent_bytes << " byte sent from " << message.size() << ". Closing connection." << std::endl;
            this->invalidate();
        }
    }

    std::vector<std::string> receive_message() {
        std::vector<std::string> result;

        std::string curr_buffer;
        std::swap(curr_buffer, remained_buffer);
        while (true) {
            std::string line;
            std::stringstream consumer(curr_buffer);
            while (std::getline(consumer, line)) {
                if (line == ".") {
                    if (!consumer.eof()) {
                        remained_buffer =
                            consumer.str().substr(consumer.tellg());
                    }
                    return result;
                } else if (!line.empty()) {
                    result.push_back(line);
                }
            }

            char array_buffer[512];
            auto received_bytes = this->recv(array_buffer, 511);

            switch (received_bytes) {
                case -1:
                    std::cerr << "[main] Error: recv failed!\n";
                    [[fallthrough]];
                case 0:
                    std::cerr << "[main] Connection closed." << std::endl;
                    this->invalidate();
                    if (!result.empty()) {
                        std::cerr << "[main] Latest message processing ..." << std::endl;
                    }
                    return result;
            }
            array_buffer[received_bytes] = '\0';
            if (!curr_buffer.empty() && curr_buffer.back() != '\n') {
                curr_buffer = result.back();
                result.pop_back();
            } else {
                curr_buffer.clear();
            }
            curr_buffer += array_buffer;
        }
    }

    void invalidate() {
        valid = false;
        socket_handler.invalidate();
    }

    bool is_valid() const { return valid && socket_handler.valid(); }
};

#endif  // SOCKET_CONNECTOR_H_INCLUDED
