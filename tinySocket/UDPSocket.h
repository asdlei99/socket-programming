#pragma once
#include "Socket.hpp"

namespace tinySocket {

    template<int _AF>
    class UDPSocket : public Socket<_AF, SOCK_DGRAM, IPPROTO_UDP> {
    protected:

        UDPSocket(SOCKET new_descriptor) : Socket(new_descriptor) { }

    public:

        UDPSocket() : Socket() { }

        UDPSocket(const )

    };

}