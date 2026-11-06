#include <iostream>
#include <new>
#include <memory>
#include <cassert>
// Deterministic allocation-failure injection in a class-specific allocation function.
struct FailAllocation { static void* operator new(std::size_t) { throw std::bad_alloc(); } };
struct Guard {~Guard(){std::cout<<"~allocation-guard ";}};
int main(){bool caught=false;try {Guard g;std::unique_ptr<FailAllocation> p(new FailAllocation);}
 catch(const std::bad_alloc&){caught=true;std::cout<<"caught std::bad_alloc\n";}assert(caught);}
