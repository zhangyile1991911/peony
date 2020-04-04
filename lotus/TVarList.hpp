//
// Created by 张亦乐 on 2018/6/27.
//

#ifndef TVARLIST_H
#define TVARLIST_H

#include <assert.h>
#include "TVarListInterf.hpp"
// 参数集
class TVarListAlloc
{
public:
    TVarListAlloc() {}
    ~TVarListAlloc() {}

    // 申请内存
    void* Alloc(size_t size) { return new char[size]; }
    // 释放内存
    void Free(void* ptr, size_t size) { delete[](char*)ptr; }
    // 交换
    void Swap(TVarListAlloc& src) {}
};

template<size_t DATA_SIZE, size_t BUFFER_SIZE, typename ALLOC = TVarListAlloc>
class TVarList : public IVarList
{
private:
    typedef TVarList<DATA_SIZE, BUFFER_SIZE, ALLOC> self_type;

    struct var_data_t
    {
        int nType;
        union
        {
            bool boolValue;
            int intValue;
            uint32_t uintValue;
            int64_t int64Value;
            float floatValue;
            double doubleValue;
            size_t stringValue;
        };
    };

public:
    TVarList()
    {
        assert(DATA_SIZE > 0);
        assert(BUFFER_SIZE > 0);

        m_pData = m_DataStack;
        m_nDataSize = DATA_SIZE;
        m_nDataUsed = 0;
        m_pBuffer = m_BufferStack;
        m_nBufferSize = BUFFER_SIZE;
        m_nBufferUsed = 0;
    }

    TVarList(const self_type& src)
    {
        assert(DATA_SIZE > 0);
        assert(BUFFER_SIZE > 0);

        m_pData = m_DataStack;
        m_nDataSize = DATA_SIZE;
        m_nDataUsed = 0;
        m_pBuffer = m_BufferStack;
        m_nBufferSize = BUFFER_SIZE;
        m_nBufferUsed = 0;
        InnerAppend(src, 0, src.GetCount());
    }

    virtual ~TVarList()
    {
        if (m_nDataSize > DATA_SIZE)
        {
            m_Alloc.Free(m_pData, m_nDataSize * sizeof(var_data_t));
        }

        if (m_nBufferSize > BUFFER_SIZE)
        {
            m_Alloc.Free(m_pBuffer, m_nBufferSize);
        }
    }

    void InnerAppend(const IVarList& src, size_t start, size_t end)
    {
        for (size_t i = start; i < end; ++i)
        {
            switch (src.GetType(i))
            {
                case VTYPE_BOOL:
                    AddBool(src.BoolVal(i));
                    break;
                case VTYPE_INT:
                    AddInt(src.IntVal(i));
                    break;
                case VTYPE_UINT:
                    AddUInt(src.UIntVal(i));
                    break;
                case VTYPE_INT64:
                    AddInt64(src.Int64Val(i));
                    break;
                case VTYPE_FLOAT:
                    AddFloat(src.FloatVal(i));
                    break;
                case VTYPE_DOUBLE:
                    AddDouble(src.DoubleVal(i));
                    break;
                case VTYPE_STRING:
                    AddString(src.StringVal(i));
                    break;
                default:
                    assert(0);
                    break;
            }
        }
    }

    self_type& operator=(const self_type& src)
    {
        if (m_nDataSize > DATA_SIZE)
        {
            m_Alloc.Free(m_pData, m_nDataSize * sizeof(var_data_t));
        }

        if (m_nBufferSize > BUFFER_SIZE)
        {
            m_Alloc.Free(m_pBuffer, m_nBufferSize);
        }

        m_pData = m_DataStack;
        m_nDataSize = DATA_SIZE;
        m_nDataUsed = 0;
        m_pBuffer = m_BufferStack;
        m_nBufferSize = BUFFER_SIZE;
        m_nBufferUsed = 0;
        InnerAppend(src, 0, src.GetCount());

        return *this;
    }

    // 合并
    virtual bool Concat(const IVarList& src)
    {
        InnerAppend(src, 0, src.GetCount());

        return true;
    }

    // 添加
    virtual bool Append(const IVarList& src, size_t start, size_t count)
    {
        if (start >= src.GetCount())
        {
            return false;
        }

        size_t end = start + count;

        if (end > src.GetCount())
        {
            return false;
        }

        InnerAppend(src, start, end);

        return true;
    }

    // 清空
    virtual void Clear()
    {
        m_nDataUsed = 0;
        m_nBufferUsed = 0;
    }

    // 是否为空
    virtual bool IsEmpty() const
    {
        return (0 == m_nDataUsed);
    }

    // 数据数量
    virtual size_t GetCount() const
    {
        return m_nDataUsed;
    }

    // 数据类型
    virtual int GetType(size_t index) const
    {
        if (index >= m_nDataUsed)
        {
            return 0;
        }

        return m_pData[index].nType;
    }

    // 添加数据
    virtual bool AddBool(bool value)
    {
        var_data_t* p = AddVarData();

        p->nType = VTYPE_BOOL;
        p->boolValue = value;

        return true;
    }

    virtual bool AddInt(int value)
    {
        var_data_t* p = AddVarData();

        p->nType = VTYPE_INT;
        p->intValue = value;

        return true;
    }

    virtual bool AddUInt(uint32_t value)
    {
        var_data_t* p = AddVarData();

        p->nType = VTYPE_UINT;
        p->uintValue = value;
        return true;
    }

    virtual bool AddInt64(int64_t value)
    {
        var_data_t* p = AddVarData();

        p->nType = VTYPE_INT64;
        p->int64Value = value;

        return true;
    }

    virtual bool AddFloat(float value)
    {
        var_data_t* p = AddVarData();

        p->nType = VTYPE_FLOAT;
        p->floatValue = value;

        return true;
    }

    virtual bool AddDouble(double value)
    {
        var_data_t* p = AddVarData();

        p->nType = VTYPE_DOUBLE;
        p->doubleValue = value;

        return true;
    }

    virtual bool AddString(const char* value)
    {
        assert(value != NULL);

        var_data_t* p = AddVarData();

        p->nType = VTYPE_STRING;
        p->stringValue = m_nBufferUsed;

        const size_t value_size = strlen(value) + 1;
        char* v = AddBuffer(value_size);

        memcpy(v, value, value_size);

        return true;
    }


    // 获得数据
    virtual bool BoolVal(size_t index) const
    {
        if (index >= m_nDataUsed)
        {
            return false;
        }

        switch (m_pData[index].nType)
        {
            case VTYPE_BOOL:
                return m_pData[index].boolValue;
            case VTYPE_INT:
                return (m_pData[index].intValue != 0);
            case VTYPE_INT64:
                return (m_pData[index].int64Value != 0);
            case VTYPE_STRING:
                return (*(m_pBuffer + m_pData[index].stringValue) != 0);
            default:
                break;
        }

        return false;
    }

    virtual int IntVal(size_t index) const
    {
        if (index >= m_nDataUsed)
        {
            return 0;
        }

        switch (m_pData[index].nType)
        {
            case VTYPE_BOOL:
                return int(m_pData[index].boolValue);
            case VTYPE_INT:
                return m_pData[index].intValue;
            case VTYPE_UINT:
                return m_pData[index].uintValue;
            case VTYPE_INT64:
                return int(m_pData[index].int64Value);
            case VTYPE_FLOAT:
                return int(m_pData[index].floatValue);
            case VTYPE_DOUBLE:
                return int(m_pData[index].doubleValue);
            case VTYPE_STRING:
                return atoi(m_pBuffer + m_pData[index].stringValue);
            default:
                break;
        }

        return 0;
    }

    virtual uint32_t UIntVal(size_t index) const
    {
        if (index >= m_nDataUsed)
        {
            return 0;
        }

        switch (m_pData[index].nType)
        {
            case VTYPE_BOOL:
                return int(m_pData[index].boolValue);
            case VTYPE_INT:
                return m_pData[index].intValue;
            case VTYPE_UINT:
                return m_pData[index].uintValue;
            case VTYPE_INT64:
                return int(m_pData[index].int64Value);
            case VTYPE_FLOAT:
                return int(m_pData[index].floatValue);
            case VTYPE_DOUBLE:
                return int(m_pData[index].doubleValue);
            case VTYPE_STRING:
                return atoi(m_pBuffer + m_pData[index].stringValue);
            default:
                break;
        }

        return 0;
    }

    virtual int64_t Int64Val(size_t index) const
    {
        if (index >= m_nDataUsed)
        {
            return 0;
        }

        switch (m_pData[index].nType)
        {
            case VTYPE_BOOL:
                return int64_t(m_pData[index].boolValue);
            case VTYPE_INT:
                return int64_t(m_pData[index].intValue);
            case VTYPE_UINT:
                return int64_t(m_pData[index].uintValue);
            case VTYPE_INT64:
                return m_pData[index].int64Value;
            case VTYPE_FLOAT:
                return int64_t(m_pData[index].floatValue);
            case VTYPE_DOUBLE:
                return int64_t(m_pData[index].doubleValue);
            case VTYPE_STRING:
                return strtol(m_pBuffer + m_pData[index].stringValue, NULL, 10);
            default:
                break;
        }

        return 0;
    }

    virtual float FloatVal(size_t index) const
    {
        if (index >= m_nDataUsed)
        {
            return 0.0f;
        }

        switch (m_pData[index].nType)
        {
            case VTYPE_BOOL:
                return float(m_pData[index].boolValue);
            case VTYPE_INT:
                return float(m_pData[index].intValue);
            case VTYPE_UINT:
                return float(m_pData[index].uintValue);
            case VTYPE_INT64:
                return float(m_pData[index].int64Value);
            case VTYPE_FLOAT:
                return m_pData[index].floatValue;
            case VTYPE_DOUBLE:
                return float(m_pData[index].doubleValue);
            case VTYPE_STRING:
                return float(atof(m_pBuffer + m_pData[index].stringValue));
            default:
                break;
        }

        return 0.0f;
    }

    virtual double DoubleVal(size_t index) const
    {
        if (index >= m_nDataUsed)
        {
            return 0.0;
        }

        switch (m_pData[index].nType)
        {
            case VTYPE_BOOL:
                return double(m_pData[index].boolValue);
            case VTYPE_INT:
                return double(m_pData[index].intValue);
            case VTYPE_UINT:
                return double(m_pData[index].uintValue);
            case VTYPE_INT64:
                return double(m_pData[index].int64Value);
            case VTYPE_FLOAT:
                return double(m_pData[index].floatValue);
            case VTYPE_DOUBLE:
                return m_pData[index].doubleValue;
            case VTYPE_STRING:
                return atof(m_pBuffer + m_pData[index].stringValue);
            default:
                break;
        }

        return 0.0;
    }

    virtual const char* StringVal(size_t index) const
    {
        if (index >= m_nDataUsed)
        {
            return "";
        }

        switch (m_pData[index].nType)
        {
            case VTYPE_STRING:
                return m_pBuffer + m_pData[index].stringValue;
            default:
                break;
        }

        return "";
    }



    //设置数据
    virtual bool SetInt(size_t index, int value)
    {
        if (index >= m_nDataUsed)
            return false;
        if (m_pData[index].nType != VTYPE_INT)
            return false;

        m_pData[index].intValue = value;
        return true;
    }

    virtual bool SetUInt(size_t index, int32_t value)
    {
        if (index >= m_nDataUsed)
            return false;
        if (m_pData[index].nType != VTYPE_UINT)
            return false;

        m_pData[index].uintValue = value;
        return true;
    }

    virtual bool SetInt64(size_t index, int64_t value)
    {
        if (index >= m_nDataUsed)
            return false;
        if (m_pData[index].nType != VTYPE_INT64)
            return false;

        m_pData[index].int64Value = value;
        return true;
    }

    virtual bool SetFloat(size_t index, float value)
    {
        if (index >= m_nDataUsed)
            return false;
        if (m_pData[index].nType != VTYPE_FLOAT)
            return false;

        m_pData[index].floatValue = value;
        return true;
    }

    virtual bool SetDouble(size_t index, double value)
    {
        if (index >= m_nDataUsed)
            return false;
        if (m_pData[index].nType != VTYPE_DOUBLE)
            return false;

        m_pData[index].doubleValue = value;
        return true;
    }

    virtual bool SetString(size_t index, const char* value)
    {
        assert(value != NULL);
        if (index >= m_nDataUsed)
            return false;
        if (m_pData[index].nType != VTYPE_STRING)
            return false;
        char *ps = m_pBuffer + m_pData[index].stringValue;
        const size_t SIZE1 = strlen(value) + 1;
        //小于等于以前的,放回原地
        if (SIZE1 <= strlen(ps) + 1)
        {
            strcpy(ps, value);
            return true;
        }
        m_pData[index].stringValue = m_nBufferUsed;
        const size_t value_size = strlen(value) + 1;
        char* v = AddBuffer(value_size);
        memcpy(v, value, value_size);
        return true;
    }

    // 获得内存占用
    virtual size_t GetMemoryUsage() const
    {
        size_t size = sizeof(self_type);

        if (m_nDataSize > DATA_SIZE)
        {
            size += sizeof(var_data_t) * m_nDataSize;
        }

        if (m_nBufferSize > BUFFER_SIZE)
        {
            size += m_nBufferSize;
        }

        return size;
    }

private:
    var_data_t* AddVarData()
    {
        if (m_nDataUsed >= m_nDataSize)
        {
            size_t new_size = m_nDataSize * 2;
            var_data_t* p = (var_data_t*)m_Alloc.Alloc(
                    new_size * sizeof(var_data_t));

            memcpy(p, m_pData, m_nDataUsed * sizeof(var_data_t));

            if (m_nDataSize > DATA_SIZE)
            {
                m_Alloc.Free(m_pData, m_nDataSize * sizeof(var_data_t));
            }

            m_pData = p;
            m_nDataSize = new_size;
        }

        return m_pData + m_nDataUsed++;
    }

    char* AddBuffer(size_t need_size)
    {
        size_t new_used = m_nBufferUsed + need_size;

        if (new_used > m_nBufferSize)
        {
            size_t new_size = m_nBufferSize * 2;

            if (new_used > new_size)
            {
                new_size = new_used * 2;
            }

            char* p = (char*)m_Alloc.Alloc(new_size);

            memcpy(p, m_pBuffer, m_nBufferUsed);

            if (m_nBufferSize > BUFFER_SIZE)
            {
                m_Alloc.Free(m_pBuffer, m_nBufferSize);
            }

            m_pBuffer = p;
            m_nBufferSize = new_size;
        }

        char* ret = m_pBuffer + m_nBufferUsed;

        m_nBufferUsed = new_used;

        return ret;
    }



private:
    ALLOC m_Alloc;
    var_data_t m_DataStack[DATA_SIZE];
    var_data_t* m_pData;
    size_t m_nDataSize;
    size_t m_nDataUsed;
    char m_BufferStack[BUFFER_SIZE];
    char* m_pBuffer;
    size_t m_nBufferSize;
    size_t m_nBufferUsed;
};

typedef TVarList<8, 128> CVarList;
#endif //TVARLIST_H
