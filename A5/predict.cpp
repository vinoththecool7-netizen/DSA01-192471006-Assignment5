#include <iostream> 
#include <string> 
struct Guard { 
 std::string n; 
 explicit Guard(const std::string& s) : n(s) {} 
 ~Guard() { std::cout << '~' << n << ' '; } 
}; 
void f() 
{ 
 Guard g("f"); 
 throw 42; 
}
int main() 
{ 
 try { 
 Guard m("main"); 
 f(); 
 std::cout << "not printed "; 
 } catch (int x) { 
 std::cout << "caught " << x; 
 } catch (...) { 
 std::cout << "other"; 
 } 
 std::cout << '\n'; 
}
