#include <stdexcept>
#include <string>
#include <vector>

const int MAXN = 100;

enum class AllocErrorType {
    InvalidFree,
    NoMemory,
};

class AllocError: std::runtime_error {
private:
    AllocErrorType type;

public:
    AllocError(AllocErrorType _type, std::string message):
            runtime_error(message),
            type(_type)
    {}

    AllocErrorType getType() const { return type; }
};

class Allocator;

struct SmartPointer {
    Allocator *allocator;
    int index;
    SmartPointer(Allocator *allocator, int index): allocator(allocator), index(index) {}
    void dec() { index--; }
    void inc() { index++; }
    char *get();
};

class Pointer {
private:
    SmartPointer *smart_ptr;
public:
    Pointer(){ smart_ptr = NULL;}
    Pointer(SmartPointer *smart_ptr): smart_ptr(smart_ptr) {}
    char *get() { return smart_ptr ? smart_ptr->get() : NULL; }
    void setNull() { smart_ptr = NULL; }
};

struct MyPar {
    char *addr;
    int size;
    SmartPointer *p;

    MyPar(){}
    MyPar(char *addr, int size, SmartPointer *p = NULL): addr(addr), size(size), p(p) {}

    char *getEnd() const { return addr + size; }
    size_t getSize() const { return size; }
    void setSize(size_t new_size) { size = new_size; }


    bool operator==(Pointer p) const {
        return addr == p.get();
    }

    bool operator==(MyPar p) const {
        return addr == p.addr;
    }
};

class Allocator {

public:
    std::vector<MyPar> pointers;
    Allocator(void *base, size_t size);
    
    Pointer alloc(size_t N);
    void realloc(Pointer &p, size_t N);
    void free(Pointer &p);

    void defrag();
    void dump();
};


