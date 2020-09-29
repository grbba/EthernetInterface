#ifndef Singelton_H
#define Singelton_H
#include "Arduino.h"


template <class T> class Singleton
{
public:
    static  T* get();
    static void kill();

protected:
    static T* m_i;

private:
    T& operator= (const T&){}
};

template <class T> T* Singleton<T>::m_i=0;

template <class T>  T* Singleton<T>::get()
{
        if(m_i==0)
        {
                m_i=new T();
        }
        return m_i;
}

template <class T> void Singleton<T>::kill()
{
        delete m_i;
        m_i=0;
}


#endif