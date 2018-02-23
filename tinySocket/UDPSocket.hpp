#pragma once
#include "Socket.hpp"

namespace tinySocket {

    template<int _AF>
    class UDPSocket : public Socket<_AF, SOCK_DGRAM, IPPROTO_UDP> {
    private:

        template<int>
        inline void Bind(const TCHAR* LocalAddress, u_short LocalPort) const throw(DWORD);

        template<>
        inline void Bind<AF_INET>(const TCHAR* LocalAddress, u_short LocalPort) const throw(DWORD) {
            int status;
            sockaddr_in bind_info = { };
            bind_info.sin_family = AF_INET;
            bind_info.sin_port = htons(LocalPort);
            status = InetPton(AF_INET, LocalAddress, &bind_info.sin_addr);
            if (status == 0)
                throw WSAEADDRNOTAVAIL;
            else if (status == -1)
                throw WSAGetLastError();

            status = ::bind(_descriptor, reinterpret_cast<sockaddr*>(&bind_info), sizeof(sockaddr_in));
            if (status != 0)
                throw WSAGetLastError();
        }

        template<>
        inline void Bind<AF_INET6>(const TCHAR* LocalAddress, u_short LocalPort) const throw(DWORD) {
            int status;
            sockaddr_in6 bind_info = { };
            bind_info.sin6_family = AF_INET6;
            bind_info.sin6_port = htons(LocalPort);
            status = InetPton(AF_INET6, LocalAddress, &bind_info.sin6_addr);
            if (status == 0)
                throw WSAEADDRNOTAVAIL;
            else if (status == -1)
                throw WSAGetLastError();

            status = ::bind(_descriptor, reinterpret_cast<sockaddr*>(&bind_info), sizeof(sockaddr_in6));
            if (status != 0)
                throw WSAGetLastError();
        }

    protected:

        UDPSocket(SOCKET new_descriptor) : Socket(new_descriptor) { }

    public:

        struct ReceiveInfo {
            int length;
            SocketAddr<_AF> from;
        };

        UDPSocket() : Socket() { }

        UDPSocket(const UDPSocket<_AF>&) = delete;

        UDPSocket(UDPSocket<_AF>&& other) : 
            Socket(static_cast<Socket&&>(other)) { }

        UDPSocket<_AF>& operator=(const UDPSocket<_AF>&) = delete;

        UDPSocket<_AF>& operator=(UDPSocket<_AF>&& other) {
            Socket<_AF, SOCK_DGRAM, IPPROTO_UDP>::operator=(static_cast<Socket<_AF, SOCK_DGRAM, IPPROTO_UDP>&&>(other));
            return *this;
        }

        void SetBroadcast(bool enable) const throw(DWORD) {
            DWORD status = enable ? TRUE : FALSE;
            if (::setsockopt(_descriptor,
                             SOL_SOCKET, SO_BROADCAST,
                             reinterpret_cast<const char*>(&status), sizeof(status)) != 0)
                throw WSAGetLastError();
        }

        bool GetBroadcast() const throw(DWORD) {
            DWORD status;
            if (::getsockopt(_descriptor,
                             SOL_SOCKET, SO_BROADCAST,
                             reinterpret_cast<const char*>(&status), sizeof(status)) != 0)
                throw WSAGetLastError();
            return status == 0 ? false : true;
        }

        void Bind(const TCHAR* LocalAddress, u_short LocalPort) const throw(DWORD) {
            Bind<_AF>(LocalAddress, LocalPort);
        }

        void Connect(const TCHAR* HostName, u_short Port) const throw(DWORD) {
            ADDRINFOT hints = { };
            hints.ai_family = _AF;
            hints.ai_socktype = SOCK_DGRAM;
            hints.ai_protocol = IPPROTO_UDP;

            ADDRINFOT* result = nullptr;

            int status = GetAddrInfo(HostName, nullptr, &hints, &result);
            if (status != 0)
                throw status;

            if (_AF == AF_INET) {
                reinterpret_cast<sockaddr_in*>(result->ai_addr)->sin_port = htons(Port);
            } else if (_AF == AF_INET6) {
                reinterpret_cast<sockaddr_in6*>(result->ai_addr)->sin6_port = htons(Port);
            }

            status = ::connect(_descriptor, result->ai_addr, result->ai_addrlen);
            FreeAddrInfo(result);
            if (status != 0)
                throw WSAGetLastError();
        }

        void Connect(const TCHAR* HostName, const TCHAR* ServiceName) const throw(DWORD) {
            ADDRINFOT hints = { };
            hints.ai_family = _AF;
            hints.ai_socktype = SOCK_DGRAM;
            hints.ai_protocol = IPPROTO_UDP;

            ADDRINFOT* result = nullptr;

            int status = GetAddrInfo(HostName, ServiceName, &hints, &result);
            if (status != 0)
                throw status;

            if (_AF == AF_INET) {
                reinterpret_cast<sockaddr_in*>(result->ai_addr)->sin_port = htons(Port);
            } else if (_AF == AF_INET6) {
                reinterpret_cast<sockaddr_in6*>(result->ai_addr)->sin6_port = htons(Port);
            }

            status = ::connect(_descriptor, result->ai_addr, result->ai_addrlen);
            FreeAddrInfo(result);
            if (status != 0)
                throw WSAGetLastError();
        }

        int Send(const void* buffer, int length, int flag = 0) const throw(DWORD) {
            int sent_length = ::send(_descriptor, reinterpret_cast<const char*>(buffer), length, flag);
            if (sent_length == SOCKET_ERROR)
                throw WSAGetLastError();
            return sent_length;
        }

        int SendTo(const void* buffer, int length, const SocketAddr<_AF>& to, int flag = 0) const throw(DWORD) {
            int sent_length;
            sent_length = ::sendto(_descriptor,
                                   reinterpret_cast<const char*>(buffer), length,
                                   flag,
                                   reinterpret_cast<const sockaddr*>(&to), sizeof(to));
            if (sent_length == SOCKET_ERROR)
                throw WSAGetLastError();
            return sent_length;
        }

        int SendTo(const void* buffer, int length, const TCHAR* hostname, u_short port, int flag /* = 0) const throw(DWORD */) {
            ADDRINFOT hints = { };
            hints.ai_family = _AF;
            hints.ai_socktype = SOCK_DGRAM;
            hints.ai_protocol = IPPROTO_UDP;

            ADDRINFOT* result = nullptr;

            int status = GetAddrInfo(hostname, nullptr, &hints, &result);
            if (status != 0)
                throw status;

            if (_AF == AF_INET) {
                reinterpret_cast<sockaddr_in*>(result->ai_addr)->sin_port = htons(Port);
            } else if (_AF == AF_INET6) {
                reinterpret_cast<sockaddr_in6*>(result->ai_addr)->sin6_port = htons(Port);
            }

            status = ::sendto(_descriptor, buffer, length, flag, result->ai_addr, result->ai_addrlen);
            FreeAddrInfo(result);

            if (status != SOCKET_ERROR)
                throw WSAGetLastError();
            return status;
        }

        int Receive(void* buffer, int length, int flag = 0) const throw(DWORD) {
            int received_length = ::recv(_descriptor, reinterpret_cast<char*>(buffer), length, flag);
            if (received_length == SOCKET_ERROR)
                throw WSAGetLastError();
            return received_length;
        }

        ReceiveInfo ReceiveFrom(void* buffer, int length, int flag = 0) const throw(DWORD) {
            ReceiveInfo ret = { };
            ret.length = ::recvfrom(_descriptor,
                                    reinterpret_cast<char*>(buffer), length,
                                    flag,
                                    reinterpret_cast<sockaddr*>(&ret.from), sizeof(ret.from));
            if (ret.length == SOCKET_ERROR)
                throw WSAGetLastError();
            return ret;
        }

    };

}