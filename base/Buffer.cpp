//
// Created by 张亦乐 on 2018/6/22.
//
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <cerrno>
#include <unistd.h>
#include <cassert>
#include <algorithm>

#include "Buffer.hpp"

using namespace std;
void Buffer::Append(const char *buff, unsigned long len)
{
    if(!ensureWriteSpace(len))
    {
        //TODO 扩大空间
        makeMoreSpace(len - WriteableBytes());
    }

    std::copy(buff,buff+len,writePos());
    hasWrite(len);
}

ssize_t Buffer::appendFD(int fd,int& saveErrno)
{
    //一次性提供大块内存,减少io系统调用
    char extrabuf[65536];
    struct iovec vec[2];
    std::size_t writable = WriteableBytes();
    vec[0].iov_base = writePos();
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const std::size_t iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    const ssize_t num = ::readv(fd, vec, iovcnt);
    if (num < 0)
    {
        saveErrno = errno;
        return -1;
    }
    else if(num == 0)
    {
        //对端关闭
        return 0;
    }
    else if ((std::size_t)num <= writable)
    {
        hasWrite(num);
    }
    else
    {
        hasWrite(writable);
        std::size_t overNum = num - writable;
        Append(extrabuf,overNum);
    }
    return num;
}

string Buffer::Read(std::size_t len)
{
    //debug();
    assert(len <= ReadableBytes());
    string str(ReadPos(),len);
    hasRead(len);
    if(ReadableBytes() == 0)
    {
        resetBegin();
    }
    //return std::move(str);
    return str;
}

void Buffer::HasRead(std::size_t len)
{
    assert(len <= ReadableBytes());
    hasRead(len);
};

void Buffer::HasWrite(std::size_t len)
{
    assert(len <= WriteableBytes());
    hasWrite(len);
}

string Buffer::ReadAll()
{
    //return std::move(Read(ReadableBytes()));
    return Read(ReadableBytes());
}

string Buffer::Peek(std::size_t len)
{
    assert(len <= ReadableBytes());
    string str(ReadPos(),len);
    //return std::move(str);
    return str;
}

void Buffer::makeMoreSpace(std::size_t len)
{
    std::size_t headSize = _readIndex - kCheapPrepend;
    std::size_t tailSize = _data.capacity() - _writeIndex;
    // std::size_t total = headSize + tailSize;
    if(headSize >= tailSize)
    {
        if(Empty())
        {
            printf("重置读写偏移\n");
            resetBegin();
        }
        else
        {
            //将末尾数据迁移到前面
            printf("迁移数据 当前数据量 %ld\n",ReadableBytes());
            char* pos = ReadPos();
            std::size_t num = ReadableBytes();
            copy(pos,pos+num,&_data[kCheapPrepend]);
            _readIndex = kCheapPrepend;
            _writeIndex = kCheapPrepend + num;
        }
    }
    else
    {//申请更大空间
        printf("空间不足 申请更多内存 当前数据量 %ld 当前空间大小 %ld\n",ReadableBytes(),_data.capacity());
        _data.resize(_writeIndex+len);
    }
}

void Buffer::debug()
{
    printf("==============================\n");
    printf("_readIndex = %ld\n",_readIndex);
    printf("_writeIndex = %ld\n",_writeIndex);
    printf("payload = %ld\n",ReadableBytes());
    printf("_data.size() = %ld\n",_data.size());
    printf("_data.capacity() = %ld\n",_data.capacity());
}

void Buffer::resetBegin()
{
    _readIndex = kCheapPrepend;
    _writeIndex = kCheapPrepend;
}
