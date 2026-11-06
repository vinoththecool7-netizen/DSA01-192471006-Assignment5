#pragma once
#include "../include/billing.hpp"
namespace hospital {
class TelemedicineBill final:public Bill {
 double value;
public:TelemedicineBill(std::string p,double minutes,double rate):Bill(std::move(p)),value(charge(charge(minutes)*charge(rate))){}
 double total()const noexcept override{return value;}
 const char* type()const noexcept override{return "TELE";}
};
class CorporatePayer final:public Payer {
public:double amountCovered(const Bill& b)const noexcept override{return b.total()*0.9;}
};
}
