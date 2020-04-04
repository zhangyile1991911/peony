//
// Created by 张亦乐 on 2018/7/5.
//

#ifndef SINGLETON_HPP
#define SINGLETON_HPP

template <typename T>
class Singleton
{
private:
    static T* volatile _instance;
protected:
    static void Destroy()
    {
        delete  _instance;
        _instance = nullptr;
    }
    Singleton() = default;
    virtual ~Singleton() = default;
public:
    static T* GetInstance()
    {
        if(nullptr == _instance)
        {
            _instance = new T();
            atexit(Destroy);
        }
    }
};
template<typename T>
T* volatile Singleton<T>::_instance = nullptr;
#endif //SINGLETON_HPP
