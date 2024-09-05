


#include <stdio.h>
#include <string>
#include <cstring>

#include <libconfig.h++>

#if 0
class fstring 
{
public:
    fstring (char* ptr, size_t size) : 
        m_dataPtr(ptr), 
        m_size(size) 
    {
        m_dataPtr[0] = '\0';
    }

    fstring (char* ptr, size_t size, const char* str) : 
        m_dataPtr(ptr), 
        m_size(size) 
    {
        if (strlen (str) >= size) {
            strncpy (m_dataPtr, str, size-1);
            m_dataPtr [size-1] = '\0';
        }
        else {
            strncpy (m_dataPtr, str, size);
        }
    }

    void operator= (const std::string str)
    {
        if (str.size() < m_size) {
            strncpy (m_dataPtr, str.c_str(), str.size());
        }
        else {
            strncpy (m_dataPtr, str.c_str(), m_size-1);
            m_dataPtr [m_size-1] = '\0';
        }
    }

    size_t size () const {return m_size;}

    char* c_str() {return m_dataPtr;}

private:

    char* m_dataPtr;
    size_t m_size;
};
#else

template <std::size_t Size>
class fstring 
{
public:
    fstring ()
    {
        m_dataPtr[0] = '\0';
    }

    ~fstring ()
    {
       printf ("Destructor\n");
    }

    fstring (const char* str)
    {
        if (strlen (str) >= Size) {
            strncpy (m_dataPtr, str, Size-1);
            m_dataPtr [Size-1] = '\0';
        }
        else {
            strncpy (m_dataPtr, str, Size);
        }
    }

    void operator= (const std::string str)
    {
        if (str.size() < Size) {
            strncpy (m_dataPtr, str.c_str(), str.size());
        }
        else {
            strncpy (m_dataPtr, str.c_str(), Size-1);
            m_dataPtr [Size-1] = '\0';
        }
    }

    size_t size () const {return Size;}

    char* c_str() {return m_dataPtr;}

private:

    char m_dataPtr[Size];
};
#endif

using namespace libconfig;


int main ()
{


    std::string cppstr = "Prova c++ string";

    fstring<80> str ("Francesco Gritti");
    printf ("string is %s\n", str.c_str());

    printf ("%d\n", sizeof (str));

    str = cppstr;
    printf ("string is %s\n",  str.c_str());


    return 0;
}