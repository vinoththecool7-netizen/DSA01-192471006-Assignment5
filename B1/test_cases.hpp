#pragma once
#include "../include/billing.hpp"
#include <array>
#include <cassert>
inline void originalTests() {
 using namespace hospital; auto bills=examples();
 SelfPay self; Insurance ins(50000,20); GovernmentScheme gov;
 const double expected[4][4]={{1700,1700,1360,1000},{48000,48000,38400,40000},{20000,20000,16000,18000},{2500,2500,2000,2500}};
 Payer* payers[]={&self,&ins,&gov}; double day=0;
 for(std::size_t i=0;i<bills.size();++i){ assert(bills[i]->total()==expected[i][0]); day+=bills[i]->total();
 std::cout<<bills[i]->type()<<" total "<<bills[i]->total();
 for(int j=0;j<3;++j){double x=payers[j]->amountCovered(*bills[i]);assert(x==expected[i][j+1]);std::cout<<" payer"<<j<<'='<<x;}std::cout<<'\n';}
 assert(day==72200);std::cout<<"day=72200; 12 payer checks PASS\n";
 OutPatientBill zero("Zero",0,0); Insurance edge(1360,20),none(0,100);
 assert(zero.total()==0&&edge.amountCovered(*bills[0])==1360&&none.amountCovered(*bills[0])==0);
 std::cout<<"zero, exact cover, 100 percent copay PASS\n";
}
