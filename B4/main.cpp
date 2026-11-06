#include "extension.hpp"
#include "../B1/test_cases.hpp"
int main(){using namespace hospital;originalTests();
 std::unique_ptr<Bill> b=std::make_unique<TelemedicineBill>("Kumar",30,40);
 std::unique_ptr<Payer> p=std::make_unique<CorporatePayer>();
 assert(b->total()==1200&&p->amountCovered(*b)==1080);std::cout<<"TELE total=1200 corporate=1080 PASS\n";
 bool caught=false;try{GovernmentScheme gov;(void)gov.amountCovered(*b);}catch(const UnknownPackage& e){caught=true;std::cout<<"recovered "<<e.what()<<'\n';}
 assert(caught && b->total()==1200);std::cout<<"extension and original regression PASS\n";}
