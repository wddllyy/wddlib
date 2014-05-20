#include "util/net/buffer.h"

Buffer::Buffer(size_t initSize)
    :m_buffer(initSize),
    m_iReadIndex(0),
    m_iWriteIndex(0)
{

}

Buffer::~Buffer()
{

}

size_t Buffer::ReadableBytes() const
{
    return m_iWriteIndex - m_iReadIndex;
}

size_t Buffer::WritableBytes() const
{
    return m_buffer.size() - m_iWriteIndex;
}

void Buffer::MakeSpace( size_t len )
{
    size_t iReadable = ReadableBytes();
    std::copy(Begin()+m_iReadIndex, Begin()+m_iWriteIndex, Begin());
    m_iReadIndex = 0;
    m_iWriteIndex = iReadable;

    if (WritableBytes() < len )
    {
        m_buffer.resize(m_iWriteIndex+len);
    }
}

size_t Buffer::Retrieve( size_t len )
{
    size_t readable = ReadableBytes();
    if (len < readable)
    {
        m_iReadIndex += len;
        return len;
    }
    else
    {
        m_iReadIndex = 0;
        m_iWriteIndex = 0;
        return readable;
    }
}

void Buffer::Append( const char* data, size_t len )
{
    EnsureWritableBytes(len);
    std::copy(data, data+len, BeginWrite());
    m_iWriteIndex += len;
}

void Buffer::EnsureWritableBytes( size_t len )
{
    if (WritableBytes() < len)
    {
        MakeSpace(len);
    }
}

void Buffer::Shrink( size_t reserve )
{
    size_t iReadable = ReadableBytes();
    std::copy(Begin()+m_iReadIndex, Begin()+m_iWriteIndex, Begin());
    m_iReadIndex = 0;
    m_iWriteIndex = iReadable;

    m_buffer.resize(iReadable + reserve);
}

int Buffer::Writed( size_t len )
{
    size_t writable = WritableBytes();
    if (writable < len)
    {
        MakeSpace(len - writable);
    }
    m_iWriteIndex += len;
    return 0;
}
