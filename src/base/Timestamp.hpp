//
// Created by zhangyile on 18-7-13.
//

#ifndef PEONY_TIMESTAMP_HPP
#define PEONY_TIMESTAMP_HPP

#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <cstdio>
const int64_t PerMiscroSeconds = 1000*1000;
class Timestamp
{
private:
    int64_t _microsecondsSinceEpoch;//从1970到现在经过微妙
public:
    Timestamp(int64_t microseconds):_microsecondsSinceEpoch(microseconds){}
    Timestamp():_microsecondsSinceEpoch(0){}
    ~Timestamp() = default;

    int64_t Epoch()const{return _microsecondsSinceEpoch;}
public:
    //当前微妙级时间戳
    static uint64_t Nowms()
    {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        uint64_t nowMs = tv.tv_sec*1000 + tv.tv_usec/1000;
        return nowMs;
    }

    static Timestamp Now()
    {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        int64_t nowMs = tv.tv_sec*1000 * 1000 + tv.tv_usec;
        return Timestamp(nowMs);
    }

    static int64_t NowTimeDiff(const Timestamp& t1)
    {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        int64_t nowMicrosecond = tv.tv_sec*1000 * 1000 + tv.tv_usec;
        int64_t diff = t1._microsecondsSinceEpoch - nowMicrosecond;
        if(diff < 0)
        {
            printf("now %" PRId64 "diff %" PRId64"\n", nowMicrosecond,t1._microsecondsSinceEpoch);
            //printf("now = %ld diff = %ld\n",nowMicrosecond,t1._microsecondsSinceEpoch);
        }
        return diff;
    }
    static int64_t NowTimeDiff(int64_t microseconds)
    {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        int64_t nowMs = tv.tv_sec*1000 * 1000 + tv.tv_usec;
        return microseconds - nowMs;
    }

    static Timestamp AddCurrentTime(int64_t microseconds)
    {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        int64_t nowMicrosecond = tv.tv_sec*1000 * 1000 + tv.tv_usec;
        return Timestamp((nowMicrosecond + microseconds));
    }

    static Timestamp addTime(const Timestamp& timestamp,int64_t milliseconds)
    {
        return  Timestamp(timestamp._microsecondsSinceEpoch + milliseconds);
    }
};

bool operator < (const Timestamp& lts,const Timestamp& rts);
bool operator > (const Timestamp& lts,const Timestamp& rts);
bool operator >= (const Timestamp& lts,const Timestamp& rts);
bool operator == (const Timestamp& lts,const Timestamp& rts);

#endif //PEONY_TIMESTAMP_HPP
