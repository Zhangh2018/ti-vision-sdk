/*
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

#ifndef _PACKET_
#define _PACKET_

// C++ STL headers
#include <vector>
#include <iostream>
#include <random>

// Legacy C headers
#include <cstdint>
#include <cstddef>

namespace test
{
    class MACAddress;
    class IDataSource;

    class Packet
    {
        public:
            //
            static const size_t ETHERNET_HEADER_SIZE = 14u;
            static const size_t ETHERNET_WITH_VLAN_HEADER_SIZE = 18u;
            static const size_t IP_HEADER_SIZE = 20u;
            static const size_t UDP_HEADER_SIZE = 8u;
            static const size_t TCP_HEADER_SIZE = 20u;

            Packet(size_t size);
            virtual ~Packet();
            size_t size() const;
            const uint8_t* data() const;

        protected:
            size_t _size;
            std::vector<uint8_t> _data;
    };

    class CommandPacket : public Packet
    {
        public:
            CommandPacket();
            CommandPacket(uint32_t cmdId);
            CommandPacket(uint32_t cmdId, uint32_t param);
            void setCommand(uint32_t cmdId);
    };

    class DataPacket : public Packet
    {
        public:
            DataPacket(size_t headerLen, size_t payloadSize, IDataSource& source);
            virtual ~DataPacket();

            virtual bool fill();
            virtual void print(std::ostream& stream) const;
            void randomizePayloadLen(bool flag)
            {
                randomizeLen = flag;
            }
            void setHeaderSize(uint16_t size)
            {
                headerLen = size;
            }
            void setPayloadSize(uint16_t size)
            {
                payloadSize = size;
                randomizeLen = false;
            }
            virtual void reset();

        protected:
            size_t headerLen;
            size_t payloadSize;

            IDataSource* source;
            bool randomizeLen;
            std::uniform_int_distribution<uint16_t> distribution;
            std::default_random_engine engine;
    };

    class RawPacket : public DataPacket
    {
        public:
            static const size_t MAX_PAYLOAD                 = 1500u;
            static const uint16_t ETHER_TYPE_EXPERIMENTAL   = 0x88B5u;
            RawPacket(const MACAddress& srcMac, const MACAddress& dstMac,
                     size_t size, IDataSource& source);
            RawPacket(const RawPacket& packet) = default;

            void setPacketSize(uint16_t packetSize)
            {
                if ((packetSize - HEADER_SIZE) <= MAX_PAYLOAD)
                {
                    setPayloadSize(packetSize - HEADER_SIZE);
                }
            }
            bool fill();
            void reset();

        private:
            void writePayloadLen();
            void writeSeqNum();

            uint32_t seqNum;

            static const size_t HEADER_SIZE                 = 24u;
            static const uint16_t VLAN_TPID                 = 0x8100u;
            static const uint16_t VLAN_DEFAULT_ID           = 1024u;
            static const uint8_t VLAN_DEFAULT_PRIORITY      = 5u;
    };

    class AVBTPStreamPacket : public DataPacket
    {
        public:
            static const size_t MAX_PAYLOAD = 1472u;
            AVBTPStreamPacket(const MACAddress& srcMac, const MACAddress& dstMac,
                          uint16_t id, IDataSource& source );
            AVBTPStreamPacket(const AVBTPStreamPacket& packet) = default;

            void setEthertype(uint16_t etherType);
            void setVLAN(uint16_t vlanId, uint8_t vlanPriority);
            void setSubtype(uint8_t subtype);
            
            bool fill();
            void reset();

        private:
            void writePayloadLen();
            void writeSeqNum();

            uint8_t seqNum;

            static const size_t HEADER_SIZE                 = 42u;
            static const uint16_t VLAN_TPID                 = 0x8100u;
            static const uint16_t VLAN_DEFAULT_ID           = 1024u;
            static const uint8_t VLAN_DEFAULT_PRIORITY      = 5u;

            static const uint16_t ETHER_TYPE                = 0x22F0u;
            static const uint16_t ETHER_TYPE_EXPERIMENTAL   = 0x88B5u;

            static const uint8_t SUBTYPE_RESERVED           = 0x02u;
            static const uint8_t SUBTYPE_61883              = 0x00;
            static const uint8_t SUBTYPE_EXPERIMENTAL       = 0x7Fu;
    };
}

std::ostream& operator<<(std::ostream& os, const test::DataPacket& packet);

#endif  // _PACKET_
