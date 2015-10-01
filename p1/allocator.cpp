#include "allocator.h"
#include <iostream>
#include <vector>

Allocator::Allocator(void *base, size_t size) {
    this->base = Pointer((char*)base, size);
    this->pointers.push_back(Pointer((char*)base, 0));
    this->pointers.push_back(Pointer((char*)base + size, 0));
}

bool canAlloc(char *free_block, size_t size, char *occupied_block) {
    return free_block + size <= occupied_block;
}

bool canAlloc(Pointer p1, Pointer p2) {
    return p1.getEnd() <= p2.get();
}

Pointer Allocator::alloc(size_t size) {
    for (int i = 0; i < this->pointers.size() - 1; i++) {
        Pointer p = Pointer(this->pointers[i].getEnd(), size);
        if (canAlloc(p, this->pointers[i + 1])) {
            this->pointers.insert(this->pointers.begin() + i + 1, p);
            return p;
        }
    }
    throw AllocError(AllocErrorType::NoMemory, "Alloc failed. No Memory");
}

void Allocator::free(Pointer &p) {
    auto it = std::find(this->pointers.begin() + 1, this->pointers.end() - 1, p);
    if (it == this->pointers.end() - 1) {
        throw AllocError(AllocErrorType::InvalidFree, "Invalid Free");
    }
    this->pointers.erase(it);
    p = NULL;
}

void Allocator::realloc(Pointer &p, size_t size) {
    //std::cout << p.get() << "\n";
    auto it = std::find(this->pointers.begin() + 1, this->pointers.end() - 1, p);
    if (it == this->pointers.end() - 1) {
        std::cout << "not found\n";
        try {
            p = this->alloc(size);
        }
        catch (...) {
            throw AllocError(AllocErrorType::NoMemory, "Realloc Failed. Pointer was not found.");
        }
        return;
    }
    std::cout << "xxx " << (*(it)).get() - (*(it + 1)).get() << " " << this->pointers.size() << "\n";
    for (Pointer p : this->pointers) {
        printf("%d\n", (char*)p.get());
    }
    if (canAlloc(p.get(), size, (*(it + 1)).get())) {
        //std::cout << "here " << p.get() << "\n";
        p.setSize(size);

        (*it).setSize(size);
    } else {
        this->free(p);
        
        try {
            p = this->alloc(size);
            std::cout << "ok: " << p.get() << "\n";
        } 
        catch (...) {
            throw AllocError(AllocErrorType::NoMemory, "Realloc Failed. No Memory.");
        }
    }
}

void Allocator::defrag() {
    for (int i = 1; i < this->pointers.size() - 1; i++) {
        Pointer p = this->pointers[i];
        char *old_address = p.get();
        int size = p.getSize();
        this->free(p);
        p = this->alloc(size);
        int j = 0;
        for (char *address = p.get(); address < p.getEnd(); address++, old_address++) {
            *address = *old_address;
        }
    }

}
