#pragma once
#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
namespace hospital {
struct BillingError : std::runtime_error { using std::runtime_error::runtime_error; };
struct InvalidCharge : BillingError { using BillingError::BillingError; };
struct MissingPatient : BillingError { using BillingError::BillingError; };
struct ClaimError : BillingError { using BillingError::BillingError; };
struct UnknownPackage : ClaimError { using ClaimError::ClaimError; };
inline double charge(double x) { if (!std::isfinite(x)||x<0) throw InvalidCharge("invalid charge"); return x; }
class Bill {
 std::string patient_;
protected:
 explicit Bill(std::string p):patient_(std::move(p)) { if(patient_.empty()) throw MissingPatient("missing patient record"); }
public:
 virtual ~Bill() noexcept = default;
 Bill(const Bill&)=delete; Bill& operator=(const Bill&)=delete;
 virtual double total() const noexcept =0;
 virtual const char* type() const noexcept =0;
 const std::string& patient() const noexcept { return patient_; }
 void print(std::ostream& out) const { out<<type()<<' '<<patient_<<": "<<total()<<'\n'; }
};
class OutPatientBill final : public Bill {
 double value;
public:
 OutPatientBill(std::string p,double c,double t):Bill(std::move(p)),value(charge(charge(c)+charge(t))){}
 double total() const noexcept override {return value;}
 const char* type() const noexcept override {return "OP";}
};
class InPatientBill final : public Bill {
 double value;
public:
 InPatientBill(std::string p,int days,double rate,double procedures,double medicines):Bill(std::move(p)),value(0) {
 if(days<0) throw InvalidCharge("negative days");
 value=charge(days*charge(rate)+charge(procedures)+charge(medicines)); }
 double total() const noexcept override {return value;}
 const char* type() const noexcept override {return "IP";}
};
class DayCareBill final : public Bill {
 double value;
public:
 DayCareBill(std::string p,double package):Bill(std::move(p)),value(charge(package)){}
 double total() const noexcept override {return value;}
 const char* type() const noexcept override {return "DC";}
};
class EmergencyBill final : public Bill {
 double value;
public:
 EmergencyBill(std::string p,double base,double hours,double rate):Bill(std::move(p)),value(charge(charge(base)+charge(hours)*charge(rate))){}
 double total() const noexcept override {return value;}
 const char* type() const noexcept override {return "ER";}
};
class Payer {
public:
 virtual ~Payer() noexcept=default;
 virtual double amountCovered(const Bill&) const=0;
};
class SelfPay final:public Payer {
public: double amountCovered(const Bill& b) const noexcept override {return b.total();}
};
class Insurance final:public Payer {
 double limit,copay;
public:
 Insurance(double l,double c):limit(charge(l)),copay(charge(c)) {if(c>100) throw InvalidCharge("copay above 100 percent");}
 double amountCovered(const Bill& b) const override {
 const double claim=b.total()*(1-copay/100);
 if(claim>limit) throw ClaimError("exceeds cover");
 return claim; }
};
class GovernmentScheme final:public Payer {
public:
 double amountCovered(const Bill& b) const override {
 const std::string t=b.type(); double rate;
 if(t=="OP")rate=1000; else if(t=="IP")rate=40000;
 else if(t=="DC")rate=18000; else if(t=="ER")rate=3000;
 else throw UnknownPackage("unknown package");
 return std::min(rate,b.total()); }
};
inline std::vector<std::unique_ptr<Bill>> examples() {
 std::vector<std::unique_ptr<Bill>> b;
 b.push_back(std::make_unique<OutPatientBill>("Ravi",500,1200));
 b.push_back(std::make_unique<InPatientBill>("Meena",3,2500,40000,500));
 b.push_back(std::make_unique<DayCareBill>("Arun",20000));
 b.push_back(std::make_unique<EmergencyBill>("Priya",1000,2,750));return b;
}
}
