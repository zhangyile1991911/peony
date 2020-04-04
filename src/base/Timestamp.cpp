//
// Created by zhangyile on 18-7-13.
//

#include "Timestamp.hpp"

bool operator < (const Timestamp& lts,const Timestamp& rts)
{
    return lts.Epoch() < rts.Epoch();
}

bool operator > (const Timestamp& lts,const Timestamp& rts)
{
    return lts.Epoch() > rts.Epoch();
}

bool operator >= (const Timestamp& lts,const Timestamp& rts)
{
    return lts.Epoch() >= rts.Epoch();
}

bool operator == (const Timestamp& lts,const Timestamp& rts)
{
    return lts.Epoch() == rts.Epoch();
}