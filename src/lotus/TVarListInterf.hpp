//
// Created by 张亦乐 on 2018/6/27.
//

#ifndef TVARLISTINTERF_HPP
#define TVARLISTINTERF_HPP
#include <string>
#include <stdint.h>

enum VARIANT_TYPE
{
    VTYPE_UNKNOWN,	// 未知
    VTYPE_BOOL,		// 布尔
    VTYPE_INT,		// 32位整数
    VTYPE_UINT,		// 32位无符号整数
    VTYPE_INT64,	// 64位整数
    VTYPE_FLOAT,	// 单精度浮点数
    VTYPE_DOUBLE,	// 双精度浮点数
    VTYPE_STRING,	// 字符串
    VTYPE_WIDESTR,	// 宽字符串
    VTYPE_OBJECT,	// 对象号
    VTYPE_POINTER,	// 指针
    VTYPE_USERDATA,	// 用户数据
    VTYPE_TABLE,	// 表
    VTYPE_MAX,
};

// 参数集接口
class IVarList
{
public:
    virtual ~IVarList() = 0;
    // 合并
    virtual bool Concat(const IVarList& src) = 0;
    // 部分添加
    virtual bool Append(const IVarList& src, size_t start, size_t count) = 0;
    // 清空
    virtual void Clear() = 0;
    // 是否为空
    virtual bool IsEmpty() const = 0;
    // 数据数量
    virtual size_t GetCount() const = 0;
    // 数据类型
    virtual int GetType(size_t index) const = 0;
    // 添加数据
    virtual bool AddBool(bool value) = 0;
    virtual bool AddInt(int value) = 0;
    virtual bool AddUInt(uint32_t value) = 0;
    virtual bool AddInt64(int64_t value) = 0;
    virtual bool AddFloat(float value) = 0;
    virtual bool AddDouble(double value) = 0;
    virtual bool AddString(const char* value) = 0;
    // 获得数据
    virtual bool BoolVal(size_t index) const = 0;
    virtual int IntVal(size_t index) const = 0;
    virtual uint32_t UIntVal(size_t index)const = 0;
    virtual int64_t Int64Val(size_t index) const = 0;
    virtual float FloatVal(size_t index) const = 0;
    virtual double DoubleVal(size_t index) const = 0;
    virtual const char* StringVal(size_t index) const = 0;
    // 获得内存占用
    virtual size_t GetMemoryUsage() const = 0;

    inline IVarList& operator<<(bool value)
    {
        AddBool(value);
        return *this;
    }
    inline IVarList& operator<<(char value)
    {
        AddInt(value);
        return *this;
    }
    inline IVarList& operator<<(unsigned char value)
    {
        AddInt(value);
        return *this;
    }
    inline IVarList& operator<<(short value)
    {
        AddInt(value);
        return *this;
    }
    inline IVarList& operator<<(unsigned short value)
    {
        AddInt(value);
        return *this;
    }
    inline IVarList& operator<<(int value)
    {
        AddInt(value);
        return *this;
    }
    inline IVarList& operator<<(unsigned int value)
    {
        AddUInt(value);
        return *this;
    }
    inline IVarList& operator<<(long value)
    {
        AddInt64(value);
        return *this;
    }
    inline IVarList& operator<<(unsigned long value)
    {
        AddInt64(value);
        return *this;
    }
    inline IVarList& operator<<(float value)
    {
        AddFloat(value);
        return *this;
    }
    inline IVarList& operator<<(double value)
    {
        AddDouble(value);
        return *this;
    }
    inline IVarList& operator<<(const char* value)
    {
        AddString(value);
        return *this;
    }
    inline IVarList& operator<<(const std::string& value)
    {
        AddString(value.c_str());
        return *this;
    }
    inline IVarList& operator<<(const IVarList& value)
    {
        Concat(value);
        return *this;
    }
};

inline IVarList::~IVarList() {}

#endif //TVARLISTINTERF_HPP
