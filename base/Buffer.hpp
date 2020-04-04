//
// Created by 张亦乐 on 2018/6/22.
//

#ifndef ATELIER_BUFFER_HPP
#define ATELIER_BUFFER_HPP

#include <unistd.h>
#include <vector>
#include <string>
#include <cstddef>

const int kCheapPrepend = 8;
const int kInitialSize = 65536;

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

class Buffer
{
private:
    std::vector<char>    _data;
    std::size_t        _readIndex;
    std::size_t        _writeIndex;
private:
    void  hasRead(std::size_t len){_readIndex += len;}
    void  hasWrite(std::size_t len){_writeIndex += len;}
    void  resetBegin();
    void  makeMoreSpace(std::size_t len);
    void  debug();
public:
    //只是提供给ZeroCopyInput/OutputStream使用
    void  backRead(std::size_t len){_readIndex -= len;}
    void  backWrite(std::size_t len){_writeIndex -= len;}
public:
    Buffer():_readIndex(kCheapPrepend),_writeIndex(kCheapPrepend)
    {
        _data.resize(kInitialSize);
    }
    ~Buffer(){}

    std::size_t ReadableBytes(){return _writeIndex - _readIndex;}

    std::size_t WriteableBytes(){return _data.capacity() - _writeIndex;}

    bool Empty(){return ReadableBytes() == 0;}

    void    Append(const char* buff,std::size_t len);
    ssize_t appendFD(int fd,int& saveErrno);

    std::string Peek(std::size_t len);

    std::string Read(std::size_t len);
    std::string ReadAll();
    void        HasRead(std::size_t len);
    void        HasWrite(std::size_t len);
    char*       ReadPos(){return &_data[_readIndex];}

    bool        ensureWriteSpace(std::size_t len)
                {
                    return WriteableBytes() >= len;
                }
    char*       writePos(){return &_data[_writeIndex];}
};

#endif //ATELIER_BUFFER_HPP
