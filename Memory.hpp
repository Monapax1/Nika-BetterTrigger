#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstdio>
#include <cstring>
#include <sys/uio.h>
#include <unistd.h>

#pragma once

namespace mem {
    constexpr int MAX_BUFFER_SIZE = 512;

    pid_t m_pid = 0;

    pid_t GetPID() {
        if (m_pid > 0)
            return m_pid;

        char buf[MAX_BUFFER_SIZE];
        FILE* cmd_pipe = popen("pidof -s r5apex.exe", "r");
        if (!cmd_pipe)
            throw std::runtime_error("Failed to open command pipe.");

        if (!fgets(buf, MAX_BUFFER_SIZE, cmd_pipe))
            throw std::runtime_error("Failed to read PID.");

        pid_t pid = strtoul(buf, nullptr, 10);
        pclose(cmd_pipe);
        m_pid = pid;
        return pid;
    }

    bool IsValidPointer(long Pointer) {
        return Pointer > 0x00010000 && Pointer < 0x7FFFFFFEFFFF;
    }

    bool Read(long address, void* pBuff, size_t size) {
        if (size == 0)
            return false;

        void* pAddress = reinterpret_cast<void*>(address);
        pid_t pid = GetPID();
        struct iovec iovLocalAddressSpace[1]{{0}};
        struct iovec iovRemoteAddressSpace[1]{{0}};

        iovLocalAddressSpace[0].iov_base = pBuff;
        iovLocalAddressSpace[0].iov_len = size;
        iovRemoteAddressSpace[0].iov_base = pAddress;
        iovRemoteAddressSpace[0].iov_len = size;

        ssize_t sSize = process_vm_readv(
            pid, iovLocalAddressSpace, 1, iovRemoteAddressSpace, 1, 0);

        return sSize == static_cast<ssize_t>(size);
    }

    bool Write(long address, void* pBuff, size_t size) {
        if (size == 0)
            return false;

        void* pAddress = reinterpret_cast<void*>(address);
        pid_t pid = GetPID();
        struct iovec iovLocalAddressSpace[1]{{0}};
        struct iovec iovRemoteAddressSpace[1]{{0}};

        iovLocalAddressSpace[0].iov_base = pBuff;
        iovLocalAddressSpace[0].iov_len = size;
        iovRemoteAddressSpace[0].iov_base = pAddress;
        iovRemoteAddressSpace[0].iov_len = size;

        ssize_t sSize = process_vm_writev(
            pid, iovLocalAddressSpace, 1, iovRemoteAddressSpace, 1, 0);

        return sSize == static_cast<ssize_t>(size);
    }

    std::string ConvertPointerToHexString(long pointer) {
        std::stringstream stream;
        stream << "0x" << std::hex << pointer;
        return stream.str();
    }

    template <class T>
    void Write(long address, T value) {
        if (!Write(address, &value, sizeof(T))) {
            m_pid = 0;
            throw std::runtime_error(
                "Failed to write memory " + std::to_string(sizeof(T)) +
                " at: " + ConvertPointerToHexString(address));
        }
    }

    template <class T>
    T Read(long address, std::string stringRead) {
        T buffer;
        if (!Read(address, &buffer, sizeof(T))) {
            m_pid = 0;
            throw std::runtime_error(
                "Failed to read memory [" + stringRead + "] at address : " +
                ConvertPointerToHexString(address));
        }
        return buffer;
    }

    void ReadByteArray(long address, char* buffer, int size) {
        for (int i = 0; i < size; i++) {
            if (!Read(address + i, &(buffer[i]), sizeof(char)))
                throw std::runtime_error(
                    "Failed to read byte at address: " +
                    ConvertPointerToHexString(address));
        }
    }

    std::string ReadString(long address, int size, std::string whatAreYouReading) {
        char buffer[MAX_BUFFER_SIZE] = {0};
        if (!Read(address, &buffer, size)) {
            m_pid = 0;
            throw std::runtime_error(
                "Failed to read memory string [" + whatAreYouReading +
                "] at address : " + ConvertPointerToHexString(address));
        }
        return std::string(buffer);
    }
}
