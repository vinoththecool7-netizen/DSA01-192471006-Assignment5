#include "../include/billing.hpp"
#include <cassert>
#include <limits>
using namespace hospital;
struct Guard { static int live; const char* name; explicit Guard(const char* n):name(n){++live;} ~Guard() noexcept{--live;std::cout<<'~'<<name<<' ';} };int Guard::live=0;
struct TrackedBill final:Bill {
 static int live;
 TrackedBill():Bill("Tracked"){++live;}
 ~TrackedBill() noexcept override {--live;std::cout<<"~tracked-bill ";}
 double total()const noexcept override{return 0;}
 const char* type()const noexcept override{return "TRACKED";}
};int TrackedBill::live=0;
double audit(const Payer& p,const Bill& b){try{return p.amountCovered(b);}catch(const ClaimError& e){std::cout<<"audit "<<e.what()<<' ';throw;}}
void inject(int mode){
 Guard g("scope");auto bills=examples();bills.push_back(std::make_unique<TrackedBill>());auto owned=std::make_unique<int[]>(32);
 switch(mode){
 case 0:{OutPatientBill bad("R",-1,1);break;}
 case 1:{Insurance p(10,0);audit(p,*bills[0]);break;}
 case 2:{class Unknown:public Bill{public:Unknown():Bill("X"){}double total()const noexcept override{return 2;}const char* type()const noexcept override{return "UNKNOWN";}} b;GovernmentScheme p;audit(p,b);break;}
 case 3:(void)std::stod("not-a-number");break;
 case 4:{OutPatientBill b("",1,2);break;}
 case 5:throw 42;
 case 6:{OutPatientBill b("N",std::numeric_limits<double>::quiet_NaN(),0);break;}
 case 7:{Insurance p(1,101);break;}
 case 8:{std::size_t pos=0;(void)std::stod("1e9999",&pos);break;}
 } }
int main(){
 int caught=0;for(int i=0;i<9;++i){try{inject(i);assert(false);}catch(const std::exception& e){++caught;std::cout<<"case "<<i<<" caught "<<e.what()<<'\n';}catch(...){++caught;std::cout<<"case "<<i<<" caught unknown; continuing\n";}assert(Guard::live==0 && TrackedBill::live==0);}
 assert(caught==9);std::cout<<"9 errors recovered; live guards=0; live tracked bills=0\n";
}
