#include <vector>
#include <list>
#include <malloc.h>
template <typename T>
class Allocator
{
public:
    using value_type = T;
    Allocator() noexcept
    {
    }

    ~Allocator() = default;

    template <class U>
    Allocator(const Allocator<U>&) noexcept
    {
    }

    T* allocate(const std::size_t n) const
    {
        return static_cast<T*>(malloc(n*sizeof(T)));
    }

    void deallocate(T* const p, std::size_t) const noexcept
    {
        return free((void*)p);
    }

    template <class U>
    bool operator==(const Allocator<U>&) const noexcept
    {
        return true;
    }
    template <class U>
    bool operator!=(const Allocator<U>&) const noexcept
    {
        return false;
    }
};
class Test
{
    std::vector<unsigned char> test;
public:
    Test(){}
    ~Test(){};
};

int main(int argc,char* argv[])
{
    std::list<Test,Allocator<Test> > test;
    Test a;
    test.push_back(a);
    return 0;
}