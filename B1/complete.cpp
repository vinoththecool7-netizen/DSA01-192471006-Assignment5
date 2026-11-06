#include <iostream> 
#include <string> 
class Bill { 
protected: 
 std::string patient; 
public: 
 explicit Bill(const std::string& p) : patient(p) {} 
 virtual ~Bill() = default;
 virtual double total() const = 0;
 virtual std::string type() const { return "Bill"; }
 void print() const 
 { 
 std::cout << type() << ' ' << patient << ": " << total() << '\n';  } 
}; 
class OutPatient : public Bill { 
 double consultation, tests; 
public: 
 OutPatient(const std::string& p, double c, double t) 
 : Bill(p), consultation(c), tests(t) {} 
 double total() const override { return consultation + tests; }
 std::string type() const override { return "OP"; }
}; 
class InPatient : public Bill { 
 int days; 
 double perDay, surgery; 
public: 
 InPatient(const std::string& p, int d, double rate, double s) 
 : Bill(p), days(d), perDay(rate), surgery(s) {} 
 double total() const override { return days * perDay + surgery; }
 std::string type() const override { return "IP"; }
}; 
int main() 
{ 
 Bill* bills[] = {new OutPatient("Ravi", 500, 1200), 
 new InPatient("Meena", 3, 2500, 40000)}; 
 double sum = 0; 
 for (Bill* b : bills) { 
 b->print(); // runtime polymorphism 
 sum += b->total(); 
 } 
 std::cout << "Total " << sum << '\n'; 
 for (Bill* b : bills) delete b; // virtual destructor runs the right destructor
} 
