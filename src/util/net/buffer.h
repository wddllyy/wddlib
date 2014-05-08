#ifndef __UTIL_BUFFER_H
#define __UTIL_BUFFER_H

#include <vector>

class Buffer
{
public:
    Buffer(size_t initSize = 1024);
    ~Buffer();
public:
    size_t ReadableBytes() const;
    size_t WritableBytes() const;
    const char* Peek() const { return Begin() + m_iReadIndex; }
    size_t Retrieve(size_t len);
    void Append(const char* data, size_t len);

    void EnsureWritableBytes(size_t len);
    char* BeginWrite() { return Begin() + m_iWriteIndex; }
    const char* BeginWrite() const { return Begin() + m_iWriteIndex; }

    int Writed(size_t len);

    size_t AllCapacity(){ return m_buffer.capacity(); }
    size_t AllSize(){ return m_buffer.size(); }

    void Shrink(size_t reserve);
protected:
    char* Begin() { return &*m_buffer.begin(); }
    const char* Begin() const{ return &*m_buffer.begin(); }
    void MakeSpace(size_t len);
    
private:
    std::vector<char> m_buffer;
    size_t m_iReadIndex;
    size_t m_iWriteIndex;
};
#endif
