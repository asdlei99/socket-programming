#pragma once
#include "Socket.hpp"

namespace tinySocket {

    template<int _AF>
    class TCPSocket : public Socket<_AF, SOCK_STREAM, IPPROTO_TCP> {
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

        TCPSocket(SOCKET new_descriptor) : Socket(new_descriptor) { }

    public:

        TCPSocket() : Socket() { }

        TCPSocket(const TCPSocket<_AF>&) = delete;

        TCPSocket(TCPSocket<_AF>&& other) : 
            Socket(static_cast<Socket<_AF, SOCK_STREAM, IPPROTO_TCP>&&>(other)) { }

        TCPSocket<_AF>& operator=(const TCPSocket<_AF>&) = delete;

        TCPSocket<_AF>& operator=(TCPSocket<_AF>&& other) {
            Socket<_AF, SOCK_STREAM, IPPROTO_TCP>::operator=(static_cast<Socket<_AF, SOCK_STREAM, IPPROTO_TCP>&&>(other));
            return *this;
        }

        void Bind(const TCHAR* LocalAddress, u_short LocalPort) const throw(DWORD) {
            Bind<_AF>(LocalAddress, LocalPort);
        }

        void SetKeepAlive(bool open) const throw(DWORD) {
            DWORD Status = open ? TRUE : FALSE;
            if (::setsockopt(_descriptor, 
                             SOL_SOCKET, SO_KEEPALIVE, 
                             reinterpret_cast<char*>(&Status), sizeof(Status)) != 0)
                throw WSAGetLastError();
        }

        bool GetKeepAliveStatus() const throw(DWORD) {
            DWORD Status;
            if (::getsockopt(_descriptor, 
                             SOL_SOCKET, SO_KEEPALIVE, 
                             reinterpret_cast<char*>(&Status), sizeof(Status)) != 0)
                throw WSAGetLastError();
            return Status == 0 ? false : true;
        }

        void SetLinger(bool onoff, u_short time) const throw(DWORD) {
            linger _linger;
            _linger.l_onoff = onoff ? 1 : 0;
            _linger.l_linger = time;
            if (::setsockopt(_descriptor,
                             SOL_SOCKET, SO_LINGER,
                             reinterpret_cast<char*>(&_linger), sizeof(_linger)) != 0)
                throw WSAGetLastError();
        }

        linger GetLingerStatus() const throw(DWORD) {
            linger _linger;
            if (::getsockopt(_descriptor,
                             SOL_SOCKET, SO_LINGER,
                             reinterpret_cast<char*>(&_linger), sizeof(_linger)) != 0)
                throw WSAGetLastError();
            return _linger;
        }

    };

    template<int _AF>
    class TCPCommunicateSocke;

    template<int _AF>
    class TCPListenSocket;

    template<int _AF>
    class TCPCommunicateSocket : public TCPSocket<_AF> {
        friend class TCPListenSocket<_AF>;
    protected:

        TCPCommunicateSocket(SOCKET new_descriptor) : TCPSocket(new_descriptor) { }

    public:

        TCPCommunicateSocket() : TCPSocket() { }

        TCPCommunicateSocket(const TCPCommunicateSocket<_AF>&) = delete;

        TCPCommunicateSocket(TCPCommunicateSocket <_AF>&& other) :
            TCPSocket(static_cast<TCPSocket<_AF>&&>(other)) { }

        TCPCommunicateSocket<_AF>& operator=(const TCPCommunicateSocket<_AF>&) = delete;

        TCPCommunicateSocket<_AF>& operator=(TCPCommunicateSocket<_AF>&& other) {
            TCPSocket<_AF>::operator=(static_cast<TCPSocket<_AF>&&>(other));
            return *this;
        }

        void Connect(const TCHAR* HostName, u_short Port) const throw(DWORD) {
            ADDRINFOT hints = { };
            hints.ai_family = _AF;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            
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
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;

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

        int Receive(void* buffer, int length, int flag = 0) const throw(DWORD) {
            int received_length = ::recv(_descriptor, reinterpret_cast<char*>(buffer), length, flag);
            if (received_length == SOCKET_ERROR)
                throw WSAGetLastError();
            return received_length;
        }

    };

    template<int _AF>
    class TCPListenSocket : public TCPSocket<_AF> {
        friend class TCPCommunicateSocket<_AF>;
    protected:

        TCPListenSocket(SOCKET new_descriptor) : TCPSocket(new_descriptor) { }

    public:

        TCPListenSocket() : TCPSocket() { }

        TCPListenSocket(const TCPListenSocket<_AF>&) = delete;

        TCPListenSocket(TCPListenSocket<_AF>&& other) :
            TCPSocket(static_cast<TCPSocket<_AF>&&>(other)) { }

        TCPListenSocket<_AF>& operator=(const TCPListenSocket<_AF>&) = delete;

        TCPListenSocket<_AF>& operator=(TCPListenSocket<_AF>&& other) {
            TCPSocket<_AF>::operator=(static_cast<TCPSocket<_AF>&&>(other));
            return *this;
        }

        void Listen(int backlog) const throw(DWORD) {
            if (::listen(_descriptor, backlog) != 0)
                throw WSAGetLastError();
        }

        TCPCommunicateSocket<_AF> Accept() const throw(DWORD) {
            SOCKET s = ::accept(_descriptor, nullptr, nullptr);
            if (s == INVALID_SOCKET)
                throw WSAGetLastError();
            return s;
        }

    };

}