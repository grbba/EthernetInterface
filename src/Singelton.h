/*
 *  © 2020, Gregor Baues. All rights reserved.
 *  
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  It is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CommandStation.  If not, see <https://www.gnu.org/licenses/>.
 */

// Singelton pattern but not thread nor copy safe
// shouldn't be an issue for the thread but copy 
// may have to be looked into
// pattern shall apply for the Wifi/Ethernet connection only one of each at
// any time allocated dynamically @runtime

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