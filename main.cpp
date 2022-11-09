#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "socket_manager.h"
#include "solver.h"

class client {
    std::unique_ptr<socket_manager> socket;
    std::chrono::duration<double> process_timeout_s;
    solver your_solver;

   public:
    client(std::unique_ptr<socket_manager> conn, int process_timeout_ms, std::string team_name, std::string hash)
        : socket(std::move(conn)), process_timeout_s(process_timeout_ms / 1000.0) {
        if (!socket->is_valid()) {
            std::cerr << "[main] Failed to establish connection to the server" << std::endl;
            return;
        }
        socket->send_messages({ "LOGIN " + team_name + " " + hash + " 2"});
    }

   public:
    void run() {
        bool firstTime = true;
        while (socket->is_valid()) {
            auto measure_start = std::chrono::steady_clock::now();

            std::vector<std::string> tmp = socket->receive_message();

            std::chrono::duration<double> read_seconds = std::chrono::steady_clock::now() - measure_start;
            if (read_seconds > process_timeout_s * 2) {
                std::cerr << "[main] Read took: " << read_seconds.count() << " seconds (>" << (process_timeout_s * 2).count() << "s)" << std::endl;
            }

            if (tmp.empty()) {
                continue;
            }

            std::clock_t measure_clock_start = std::clock();
            measure_start = std::chrono::steady_clock::now();

            if (firstTime) {
                your_solver.startMessage(tmp);
                tmp.clear();
                firstTime = false;
            } else
                tmp = your_solver.processTick(tmp);

            std::chrono::duration<double> process_seconds = std::chrono::steady_clock::now() - measure_start;
            if (process_seconds > process_timeout_s) {
                std::cerr << "[main] Process took: " << process_seconds.count() << " seconds (>" << process_timeout_s.count() << "s)\n";
                std::cerr << "[main] CPU time used: " << static_cast<double>(std::clock() - measure_clock_start) / CLOCKS_PER_SEC << std::endl;
            }

            if (!socket->is_valid() || tmp.empty()) {
                continue;
            }

            socket->send_messages(tmp);

            std::chrono::duration<double> process_with_send_seconds = std::chrono::steady_clock::now() - measure_start;
            if (process_seconds > process_timeout_s) {
                std::cerr << "[main] Process with send took: " << process_with_send_seconds.count() << " seconds (>" << process_timeout_s.count() << "s)\n";
                std::cerr << "[main] CPU time used: " << static_cast<double>(std::clock() - measure_clock_start) / CLOCKS_PER_SEC << " sec" << std::endl;
            }
        }
        std::cerr << "[main] Game over" << std::endl;
    }
};

int main(int argc, char **argv) {
    if (argc == 1 || argc > 1 && 0 == std::strcmp("help", argv[1])) {
        // A hostname-et és port-ot le kell tudni kezelni a gites futtatásokhoz!
        std::cerr << "Usage:\n"
                  << argv[0] << " help                                 - Print this message\n"
                  << argv[0] << " <team name> <hash> [hostname [port]] - Connect to the server and start the game" << std::endl;
        return 0;
    } else if (argc == 2) {
		std::cerr << "Invalid arguments" << std::endl;
	}

	const std::string team_name = argv[1];
	const std::string hash = argv[2];

    /* config area */
    const std::string host_name = argc < 4 ? "localhost" : argv[3];
    const unsigned short port = argc < 5 ? 63155 : std::atoi(argv[4]);
    try {
        client(std::make_unique<socket_manager>(host_name.c_str(), port), 2000, team_name, hash).run();
    } catch (std::exception &e) {
        std::cerr << "[main] Exception thrown. what(): " << e.what() << std::endl;
    }
}
