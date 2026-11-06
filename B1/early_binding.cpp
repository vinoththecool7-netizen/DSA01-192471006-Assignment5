#include <iostream>
struct Bill { virtual ~Bill() = default; double total() const { return 0; } };
struct OP : Bill { double total() const { return 1700; } };
struct IP : Bill { double total() const { return 47500; } };
int main() { OP op; IP ip; Bill* bills[]={&op,&ip}; double sum=0;
 for(auto b:bills) { std::cout<<b->total()<<' '; sum+=b->total(); }
 std::cout<<"Total "<<sum<<'\n'; }
