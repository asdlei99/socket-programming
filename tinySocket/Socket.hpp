#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "WS2_32.lib")

namespace tinySocket {

    template<int _AF>
    struct SocketAddr;

    template<>
    struct SocketAddr<AF_INET> : public sockaddr_in { 
    
        SocketAddr() { }

        SocketAddr(const TCHAR* Address, u_short Port) {
            sin_family = AF_INET;
            sin_port = htons(Port);
            int status = InetPton(AF_INET, Address, &sin_addr);
            if (status == 0)
                throw WSAEADDRNOTAVAIL;
            else if (status == -1)
                throw WSAGetLastError();
        }

    };

    template<>
    struct SocketAddr<AF_INET6> : public sockaddr_in6 {

        SocketAddr() { }

        SocketAddr(const TCHAR* Address, u_short Port) {
            sin6_family = AF_INET6;
            sin6_port = htons(Port);
            int status = InetPton(AF_INET6, Address, &sin6_addr);
            if (status == 0)
                throw WSAEADDRNOTAVAIL;
            else if (status == -1)
                throw WSAGetLastError();
        }

    };

    template<int _AF, int _SocketType, int _Protocol>
    class Socket {
    protected:
        SOCKET _descriptor;

        Socket(SOCKET new_descriptor) throw(DWORD) : _descriptor(new_descriptor)  {
            if (_descriptor == INVALID_SOCKET)
                throw WSAENOTSOCK;
        }

    public:

        Socket() throw(DWORD) {
            _descriptor = ::socket(_AF, _SocketType, _Protocol);
            if (_descriptor == INVALID_SOCKET)
                throw WSAGetLastError();
        }

        Socket(const Socket<_AF, _SocketType, _Protocol>&) = delete;

        Socket(Socket<_AF, _SocketType, _Protocol>&& other) : _descriptor(other._descriptor) {
            other._descriptor = INVALID_SOCKET;sockaddr
        }

        ~Socket() throw(DWORD) {
            if (_descriptor == INVALID_SOCKET)
                return;
            if (closesocket(_descriptor) != 0)
                throw WSAGetLastError();
        }

        Socket<_AF, _SocketType, _Protocol>& 
            operator=(const Socket<_AF, _SocketType, _Protocol>&) = delete;

        Socket<_AF, _SocketType, _Protocol>&
            operator=(Socket<_AF, _SocketType, _Protocol>&& other) {
            _descriptor = other._descriptor;
            other._descriptor = INVALID_SOCKET;
            return *this;
        }

        SOCKET GetDescriptor() const {
            return _descriptor;
        }

        //----------------------------
        //  SO_TYPE
        //----------------------------

        DWORD GetSocketType() const throw(DWORD) {
            DWORD SocketType;
            if (::getsockopt(_descriptor, SOL_SOCKET, SO_TYPE, 
                             reinterpret_cast<char*>(&SocketType), sizeof(SocketType)) != 0)
                throw WSAGetLastError();
            return SocketType;
        }

        //----------------------------
        //  SO_ERROR
        //----------------------------

        DWORD GetLastError() const throw(DWORD) {
            DWORD SocketError;
            if (::getsockopt(_descriptor, SOL_SOCKET, SO_ERROR, 
                             reinterpret_cast<char*>(&SocketError), sizeof(SocketError)) != 0)
                throw WSAGetLastError();
            return SocketError;
        }

        //----------------------------
        //  SO_REUSEADDR
        //----------------------------

        void SetReuseAddress(bool enable) const throw(DWORD) {
            DWORD Status = enable ? TRUE : FALSE;
            if (::setsockopt(_descriptor, SOL_SOCKET, SO_REUSEADDR,
                             reinterpret_cast<char*>(&Status), sizeof(Status)) != 0)
                throw WSAGetLastError();
        }

        bool GetReuseAddressStatus() const throw(DWORD) {
            DWORD Status;
            if (::getsockopt(_descriptor, SOL_SOCKET, SO_REUSEADDR,
                             reinterpret_cast<char*>(&Status), sizeof(Status)) != 0)
                throw WSAGetLastError();
            return Status == 0 ? false : true;
        }

        void GetSocketOption(int Level, int OptionName, 
                             void* OptionValue, int OptionLength) const throw(DWORD) {
            if (::getsockopt(_descriptor,
                             Level, OptionName,
                             reinterpret_cast<char*>(OptionValue), OptionLength) != 0)
                throw WSAGetLastError();
        }

        void SetSocketOption(int Level, int OptionName,
                             const void* OptionValue, int OptionLength) const throw(DWORD) {
            if (::setsockopt(_descriptor,
                             Level, OptionName,
                             reinterpret_cast<const char*>(OptionValue), OptionLength) != 0)
                throw WSAGetLastError();
        }

        SocketAddr<_AF> GetSocketName() const throw(DWORD) {
            SocketAddr<_AF> ret = { };
            int ret_size = sizeof(ret);
            if (::getsockname(_descriptor, reinterpret_cast<sockaddr*>(&ret), sizeof(ret)) != 0)
                throw WSAGetLastError();

            return ret;
        }

        SocketAddr<_AF> GetPeerName() const throw(DWORD) {
            SocketAddr<_AF> ret = { };
            int ret_size = sizeof(ret);
            if (::getpeername(_descriptor, reinterpret_cast<sockaddr*>(&ret), sizeof(ret)) != 0)
                throw WSAGetLastError();

            return ret;
        }

        void Close() {
            if (closesocket(_descriptor) != 0)
                throw WSAGetLastError();
            _descriptor = INVALID_SOCKET;
        }

    };

}