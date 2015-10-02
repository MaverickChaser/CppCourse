#include "allocator.h"
#include <iostream>
#include <vector>
#include <cassert>

char* SmartPointer::get() {
    return (this->index != -1 ? this->allocator->pointers[index].addr : NULL);
}

void Allocator::dump() {
    printf("----ALLOC DEBUG----\n");
    printf("base=%d size=%d\n", this->pointers[0].addr, this->pointers.back().addr - this->pointers[0].addr);
    printf("%d pointers\n", this->pointers.size());
    for (int i = 1; i < this->pointers.size() - 1; i++) {
        printf("%d\n", this->pointers[i].addr);
    }
    printf("------\n");
}

Allocator::Allocator(void *base, size_t size) {
    this->pointers.push_back(MyPar((char*)base, 0));
    this->pointers.push_back(MyPar((char*)base + size, 0));
}

bool canAlloc(char *free_block, size_t size, char *occupied_block) {
    return free_block + size <= occupied_block;
}

Pointer Allocator::alloc(size_t size) {
    for (int i = 0; i < this->pointers.size() - 1; i++) {
        char *end_addr = this->pointers[i].addr + this->pointers[i].size;
        if (canAlloc(end_addr, size, this->pointers[i + 1].addr)) {
            SmartPointer *new_pointer = new SmartPointer(this, i + 1);
            for (auto iter = this->pointers.begin() + i + 1; iter != this->pointers.end() - 1; iter++) {
                iter->p->inc();
            } 
            this->pointers.insert(this->pointers.begin() + i + 1, MyPar(end_addr, size, new_pointer));
            return Pointer(new_pointer);
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
    for (auto iter = it; iter != this->pointers.end() - 1; iter++) {
        iter->p->dec();
    } 
    p.setNull();
}

void Allocator::realloc(Pointer &p, size_t size) {
    auto it = std::find(this->pointers.begin() + 1, this->pointers.end() - 1, p);
    if (it == this->pointers.end() - 1) {
        try {
            p = this->alloc(size);
        }
        catch (...) {
            throw AllocError(AllocErrorType::NoMemory, "Realloc Failed. Pointer was not found.");
        }
        return;
    }
    if (canAlloc(p.get(), size, (it + 1)->addr)) {
        it->setSize(size);
    } else {
        this->free(p);
        
        try {
            p = this->alloc(size);
        } 
        catch (...) {
            throw AllocError(AllocErrorType::NoMemory, "Realloc Failed. No Memory.");
        }
    }
}

void Allocator::defrag() {
    char *base = this->pointers[0].addr;
    for (int i = 1; i < this->pointers.size() - 1; i++) {
        char *old_address = this->pointers[i].addr;
        char *new_address = base;
        this->pointers[i].addr = base;
        int size = this->pointers[i].size;
        for (int sz = 0; sz < size; sz++) {
            *new_address++ = *old_address++;
        }
        base += size;
    }
}
