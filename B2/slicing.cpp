#include <iostream>
#include <stdexcept>
struct ClaimError:std::runtime_error{using std::runtime_error::runtime_error;};
struct Detailed:ClaimError{using ClaimError::ClaimError;};
void f(bool copy){try{throw Detailed("detail");}catch(const ClaimError& e){if(copy)throw e;throw;}}
int main(){for(bool c:{false,true})try{f(c);}catch(const Detailed&){std::cout<<"throw; preserves Detailed\n";}catch(const ClaimError&){std::cout<<"throw e; slices to ClaimError\n";}}
