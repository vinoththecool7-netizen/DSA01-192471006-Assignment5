#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <variant>
#include <vector>
#if defined(__clang__) || defined(__GNUC__)
#define NOINLINE __attribute__((noinline))
#else
#define NOINLINE
#endif
using Clock=std::chrono::steady_clock;
volatile double sink=0;
struct Base {virtual ~Base()=default;virtual double total()const noexcept=0;};
struct OP:Base{double a,b;OP(double x,double y):a(x),b(y){}double total()const noexcept override{return a+b;}};
struct IP:Base{double a,b,c;IP(double x,double y,double z):a(x),b(y),c(z){}double total()const noexcept override{return a*b+c;}};
struct DC:Base{double a;explicit DC(double x):a(x){}double total()const noexcept override{return a;}};
struct ER:Base{double a,b,c;ER(double x,double y,double z):a(x),b(y),c(z){}double total()const noexcept override{return a+b*c;}};
struct Data{int tag;double a,b,c;};
// Value-only alternatives avoid carrying a vptr in the variant representation.
struct VOP{double a,b;double total()const{return a+b;}};
struct VIP{double a,b,c;double total()const{return a*b+c;}};
struct VDC{double a;double total()const{return a;}};
struct VER{double a,b,c;double total()const{return a+b*c;}};
using Var=std::variant<VOP,VIP,VDC,VER>;
Data input(std::size_t i){double x=double(i%97);return {int(i%4),100+x,2,50};}
NOINLINE double byswitch(const Data& d){switch(d.tag){case 0:return d.a+d.b;case 1:return d.a*d.b+d.c;case 2:return d.a;default:return d.a+d.b*d.c;}}
NOINLINE double virt(const Base* b){return b->total();}
NOINLINE double byvariant(const Var& v){return std::visit([](const auto& x){return x.total();},v);}
std::unique_ptr<Base> make(const Data& d){switch(d.tag){case 0:return std::make_unique<OP>(d.a,d.b);case 1:return std::make_unique<IP>(d.a,d.b,d.c);case 2:return std::make_unique<DC>(d.a);default:return std::make_unique<ER>(d.a,d.b,d.c);}}
Var variantOf(const Data& d){switch(d.tag){case 0:return VOP{d.a,d.b};case 1:return VIP{d.a,d.b,d.c};case 2:return VDC{d.a};default:return VER{d.a,d.b,d.c};}}
template<class F> void measure(const char* name,std::size_t n,F fn,double expected){
 for(int r=0;r<3;++r){auto t=Clock::now();double sum=fn();auto end=Clock::now();assert(sum==expected);sink=sum;
 std::cout<<name<<','<<n<<','<<r<<','<<std::chrono::duration<double,std::milli>(end-t).count()<<','<<sum<<'\n';}}
// Same noinline boundary for all validators; runtime input comes from prebuilt data.
NOINLINE int status(double x){return x<0?1:0;}
NOINLINE double throwing(double x){if(x<0)throw std::invalid_argument("negative");return x;}
NOINLINE std::optional<double> optional(double x){if(x<0)return std::nullopt;return x;}
int main(int argc, char**){std::cout<<std::fixed<<std::setprecision(6);
 std::cout<<"method,n,repeat,ms,checksum\n";
 const double hand[]={102,252,102,203,106,260,106,207,110,268};double handSum=0;
 for(std::size_t i=0;i<10;++i){auto d=input(i);auto b=make(d);auto v=variantOf(d);assert(virt(b.get())==hand[i]&&byswitch(d)==hand[i]&&byvariant(v)==hand[i]);handSum+=hand[i];}
 assert(handSum==1716);std::cerr<<"10 hand totals agree; total=1716\n";
 std::cerr<<"sizeof Base="<<sizeof(Base)<<" OP="<<sizeof(OP)<<" IP="<<sizeof(IP)<<" DC="<<sizeof(DC)<<" ER="<<sizeof(ER)<<" pointer="<<sizeof(Base*)<<" switch="<<sizeof(Data)<<" variant="<<sizeof(Var)<<" optional<double>="<<sizeof(std::optional<double>)<<" status="<<sizeof(int)<<'\n';
 if(argc>1)return 0;
 for(std::size_t n:{1000UL,1000000UL,10000000UL}){
 double expected=0;for(std::size_t i=0;i<n;++i){auto d=input(i);switch(i%4){case 0:expected+=d.a+2;break;case 1:expected+=2*d.a+50;break;case 2:expected+=d.a;break;case 3:expected+=d.a+100;}}
 {std::vector<std::unique_ptr<Base>> a;a.reserve(n);for(std::size_t i=0;i<n;++i)a.push_back(make(input(i)));
 measure("virtual",n,[&]{double sum=0;for(auto& b:a)sum+=virt(b.get());return sum;},expected);}
 {std::vector<Data> a;a.reserve(n);for(std::size_t i=0;i<n;++i)a.push_back(input(i));
 measure("switch",n,[&]{double sum=0;for(auto& b:a)sum+=byswitch(b);return sum;},expected);}
 {std::vector<Var> a;a.reserve(n);for(std::size_t i=0;i<n;++i)a.push_back(variantOf(input(i)));
 measure("variant",n,[&]{double sum=0;for(auto& b:a)sum+=byvariant(b);return sum;},expected);}
 }
 const std::size_t n=1000000;
 for(int percent:{0,1,50}){std::vector<double>a(n);std::size_t invalid=0;
 // Permuted residue gives exactly the target rate, interspersed through the loop.
 for(std::size_t i=0;i<n;++i){bool bad=(i*37%100)<std::size_t(percent);a[i]=bad?-1:100;invalid+=bad;}
 const double expected=double(n-invalid)*100;
 std::string suffix=std::to_string(percent);
 measure(("status_"+suffix).c_str(),n,[&]{double sum=0;std::size_t bad=0;for(double x:a){if(status(x))++bad;else sum+=x;}assert(bad==invalid);return sum;},expected);
 measure(("exception_"+suffix).c_str(),n,[&]{double sum=0;std::size_t bad=0;for(double x:a){try{sum+=throwing(x);}catch(const std::invalid_argument&){++bad;}}assert(bad==invalid);return sum;},expected);
 measure(("optional_"+suffix).c_str(),n,[&]{double sum=0;std::size_t bad=0;for(double x:a){auto v=optional(x);if(v)sum+=*v;else ++bad;}assert(bad==invalid);return sum;},expected);
 }
}
