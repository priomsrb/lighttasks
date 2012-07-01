#ifndef SINGLETON_H
#define SINGLETON_H

// Adapated from:Brian van der Beek

template <typename T>
class Singleton
{
public:
    static T* getInstance() {
        if (instance == 0)
            instance = new T;
        return instance;
    }

    static void destroyInstance() {
        delete instance;
        instance = 0;
    }

private:
    static T* instance;
};

template <typename T> T* Singleton<T>::instance = 0;

#endif // SINGLETON_H
