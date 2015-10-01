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

class Pointer {
private:
    Allocator *allocator;
    int index;
    size_t size;
public:
    Pointer(){}
    Pointer(char *address, size_t size): address(address), size(size) {};
    Pointer(void *address): address((char*)address) {};
    char *get() const { 
        return allocator->pointers[index]; 
    } 
    char *getEnd() const { return get() + size; }
    size_t getSize() const { return size; }
    void setSize(size_t new_size) { size = new_size; }

    bool operator==(Pointer p) const {
        return address == p.address;
    }

    bool operator!=(Pointer p) const {
        return address != p.address;
    }
};

class Allocator {
private:
    Pointer base;
    std::vector<Pointer> pointers;
public:
    Allocator(void *base, size_t size);
    
    Pointer alloc(size_t N);
    void realloc(Pointer &p, size_t N);
    void free(Pointer &p);

    void defrag();
    std::string dump();
};

