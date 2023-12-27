//
// Kapua UDPNetwork class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#include "UDPNetwork.hpp"

#include "Protocol.hpp"

namespace Kapua {

UDPNetwork::UDPNetwork(Logger* logger, Core* core) {
  _logger = new ScopedLogger("UDPNetwork", logger);
  _core = core;
  _running = false;
}

UDPNetwork::~UDPNetwork() {
  if (_running) stop();
  delete _logger;
}

bool UDPNetwork::start(int port) {
  _logger->debug("Starting...");
  if (_running) {
    _logger->warn("start called, but thread already running");
    return false;
  }

  _main_thread = new std::thread(&UDPNetwork::_main_loop, this);
  return true;
}

bool UDPNetwork::stop() {
  if (!_running) {
    _logger->warn("stop called, but thread not running");
    return false;
  }
  _running = false;

  _main_thread->join();

  delete _main_thread;
  _main_thread = nullptr;

  return true;
}

bool UDPNetwork::_listen(int port) {
#ifdef _WIN32
  // Initialize Windows Socket API (Winsock)
  if (WSAStartup(MAKEWORD(2, 2), &_wsaData) != 0) {
    _logger->error("WIN32: Failed to initialize Winsock");
    _shutdown();
    return false;
  }
#endif

  // Create a server socket
  _server_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (_server_socket_fd == -1) {
    _logger->error("Failed creating server socket");
    return false;
  }

  // Set up server address
  _server_addr.sin_family = AF_INET;
  _server_addr.sin_port = htons(port);
  _server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // Bind the server socket
  if (bind(_server_socket_fd, (struct sockaddr*)&_server_addr, sizeof(_server_addr)) == -1) {
    _logger->error("Failed binding server socket");
    _shutdown();
    return false;
  }

  // Create a sending socket
  _client_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (_client_socket_fd == -1) {
    _logger->error("Failed creating send socket");
    return false;
  }

  // Set broadcast enabled on sending socket
  int broadcast_enable = 1;
  if (setsockopt(_client_socket_fd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) == -1) {
    _logger->error("Failed setting send socket options (SO_BROADCAST)");
    _shutdown();
    return false;
  }

  return true;
}

void UDPNetwork::_main_loop() {
  char buffer[65536];
  sockaddr_in from_addr;

  // Set state _running true
  _running = true;

  // Set up listening on the server port
  if (!_listen(KAPUA_PORT)) {
    _logger->error("Listen failed");
    _shutdown();
    return;
  }

  // Set this in the past so we immediately do a broadcast
  auto last_broadcast_time = std::chrono::steady_clock::now() - std::chrono::hours(24);

  while (_running) {
    // Receive if any
    ssize_t len = _receive((char*)(&buffer), 65536, from_addr);
    if (len < 0) {
      _logger->error("Server receive error: " + std::string(strerror(errno)));
    } else if (len > 0) {
      std::string from_addr_str = std::string(inet_ntoa(from_addr.sin_addr)) + ":" + std::to_string(ntohs(from_addr.sin_port));
      _logger->debug("Packet From " + from_addr_str + ", " + std::to_string(len) + " bytes.");

      // Verify the packet
      if (len < 32) {
        // Length First
        _logger->debug("Non-Kapua packet received (too short)");
      } else {
        // Has magic Number
        Packet* pk = (Packet*)buffer;
        if (pk->magic != KAPUA_MAGIC_NUMBER) {
          _logger->debug("Non-Kapua packet received (bad magic number)");
          // Isn't from us
        } else if (pk->from_id == _core->get_my_id()) {
          _logger->debug("Packet received from own id");
        } else {
          // TODO: Check if registered, register new ID
          std::string from_id_hex = [&] {
            std::ostringstream oss;
            oss << "0x" << std::setfill('0') << std::setw(16) << std::hex << pk->from_id;
            return oss.str();
          }();
          _logger->info("New node detected, ID: " + from_id_hex + " (" + from_addr_str + ")");
        }
      }
    }

    // Get timing
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - last_broadcast_time);

    if (duration.count() >= 5) {
      // Do the discovery broadcast
      _broadcast();

      // Reset the last_broadcast_time variable
      last_broadcast_time = now;
    }
  }

  _logger->debug("Stopping...");
  _shutdown();

  _logger->debug("Stopped");
}

void UDPNetwork::_broadcast() {
  Packet* pkt = static_cast<Packet*>(calloc(512, 1));

  pkt->magic = KAPUA_MAGIC_NUMBER;
  // TODO: Replace with our unique ID
  pkt->from_id = _core->get_my_id();
  pkt->to_id = KAPUA_ID_BROADCAST;
  pkt->length = 0;

  // The broadcast address
  struct sockaddr_in broadcast_addr;
  std::memset(&broadcast_addr, 0, sizeof(broadcast_addr));
  broadcast_addr.sin_family = AF_INET;
  broadcast_addr.sin_port = htons(KAPUA_PORT);
  broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

  // Do the send
  _logger->debug("Discovery broadcast");
  ssize_t res = _send((char*)pkt, sizeof(Packet), broadcast_addr);
  if (res == -1) {
    _logger->error("Discovery broadcast error: " + std::string(strerror(errno)));
  }

  free(pkt);
}

ssize_t UDPNetwork::_receive(char* buffer, size_t buffer_size, sockaddr_in& client_addr) {
  // Do a select to see if there is a packet waiting
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;
  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(_server_socket_fd, &rfds);
  int recVal = select(_server_socket_fd + 1, &rfds, NULL, NULL, &tv);

  // If there is, get the packet and return
  if (recVal > 0) {
    socklen_t client_len = sizeof(client_addr);
    return recvfrom(_server_socket_fd, buffer, buffer_size, 0, (struct sockaddr*)&client_addr, &client_len);
  } else {
    return 0;
  }
}

ssize_t UDPNetwork::_send(const char* buffer, size_t len, const sockaddr_in& client_addr) {
  return sendto(_client_socket_fd, buffer, len, 0, (const struct sockaddr*)&client_addr, sizeof(client_addr));
}

bool UDPNetwork::_shutdown() {
  // Close the server socket
  if (_server_socket_fd != -1) {
#ifdef _WIN32
    closesocket(_server_socket_fd);
#else
    close(_server_socket_fd);
#endif
    _server_socket_fd = -1;
  }

  // Close the client socket
  if (_client_socket_fd != -1) {
#ifdef _WIN32
    closesocket(_client_socket_fd);
#else
    close(_client_socket_fd);
#endif
    _client_socket_fd = -1;
  }

#ifdef _WIN32
  // Cleanup if Windows
  WSACleanup();
#endif

  return true;
}

}  // namespace Kapua
