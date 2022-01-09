#include <vector>
#include <string>

template<class T>
void appendBufferValue(std::vector<unsigned char>& buffer, T value) {
    buf.insert(buf.end(), (unsigned char*)&value, (unsigned char*)&value + sizeof(T));
}

template<class T>
void appendBufferValue(std::string& buffer, T value) {
    buffer.append((unsigned char*)&value, (unsigned char*)&value + sizeof(T));
}


template<class Container>
void alignment_buffer(Container& buffer) {
    while (buffer.size() % 4 != 0) {
        buffer.push_back(0x00);
    }
}
