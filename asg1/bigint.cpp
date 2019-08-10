#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "bigint.h"
#include "debug.h"
#include "relops.h"

bigint::bigint (long that): uvalue (that), is_negative (that < 0) {
   DEBUGF ('~', this << " -> " << uvalue)
}

bigint::bigint (const ubigint& uvalue, bool is_negative):
                uvalue(uvalue), is_negative(is_negative) {
}

bigint::bigint (const string& that) {
   is_negative = that.size() > 0 and that[0] == '_';
   uvalue = ubigint (that.substr (is_negative ? 1 : 0));
}

bigint bigint::operator+ () const {
   return *this;
}

bigint bigint::operator- () const {
   return {uvalue, not is_negative};
}

bigint bigint::operator+ (const bigint& that) const {
   bigint result;
   // + sum + OR - sum -
   // same sign so result.is_negative depends on
   // sign of that.is_negative (or is_negative)
   if (is_negative == that.is_negative) {
      result.uvalue = uvalue + that.uvalue;
      result.is_negative = that.is_negative;
   }
   // - sum +
   else if (is_negative && not that.is_negative) {
      if ((uvalue < that.uvalue) || (uvalue == that.uvalue)) {
         result.uvalue = that.uvalue - uvalue;
         result.is_negative = false;
      }
      else {
         result.uvalue = uvalue - that.uvalue;
         result.is_negative = true;
      }
   }
   // + sum -
   else {
      if (uvalue < that.uvalue) {
         result.uvalue = that.uvalue - uvalue;
         result.is_negative = true;
      }
      else if (uvalue == that.uvalue) {
         result.uvalue = uvalue - that.uvalue;
         result.is_negative = false;
      }
      else {
         result.uvalue = uvalue - that.uvalue;
         result.is_negative = false;
      }
   }
   return result;
}

bigint bigint::operator- (const bigint& that) const {
   bigint result;
   // + sub + 
   if (not is_negative && not that.is_negative) {
      if (uvalue < that.uvalue) {
         result.uvalue = that.uvalue - uvalue;
         result.is_negative = true;
      }
      // handles case where:
      //    uvalue == that.uvalue and
      //    uvalue > that.uvalue
      else {
         result.uvalue = uvalue - that.uvalue;
         result.is_negative = false;
      }
      
   }
   // - sub +
   else if (is_negative && not that.is_negative) {
      if ((uvalue < that.uvalue) || (uvalue == that.uvalue)) {
         result.uvalue = that.uvalue - uvalue;
         result.is_negative = false;
      }
      else {
         result.uvalue = uvalue - that.uvalue;
         result.is_negative = true;
      }
   }
   // - sub - == - sum +
   else if (is_negative && that.is_negative) {
      if ((uvalue < that.uvalue) || (uvalue == that.uvalue)) {
         result.uvalue = that.uvalue - uvalue;
         result.is_negative = false;
      }
      else {
         result.uvalue = uvalue - that.uvalue;
         result.is_negative = true;
      }
   }
   // + sub - == + sum +
   else {
      result.uvalue = uvalue + that.uvalue;
      result.is_negative = false;
   }
 
   return result;
}

bigint bigint::operator* (const bigint& that) const {
   
   bigint result;
   result.uvalue = uvalue * that.uvalue;

   if (is_negative == that.is_negative) {
      result.is_negative = false;
   }
   else {
      result.is_negative = true;
   }
   
   return result;
}

bigint bigint::operator/ (const bigint& that) const {
   
   bigint result;
   result.uvalue = uvalue / that.uvalue;
   static const bigint ZERO (0);   

   if (result == ZERO) {
      result.is_negative = false;
   }
   else if (is_negative == that.is_negative) {
      result.is_negative = false;
   }
   else {
      result.is_negative = true;
   }

   return result;
}

bigint bigint::operator% (const bigint& that) const {
   
   bigint result;
   result.uvalue = uvalue % that.uvalue;
  
   if (is_negative && that.is_negative) {
      result.is_negative = true;
   }
   else if (not is_negative && not that.is_negative) {
      result.is_negative = false;
   }
   else if (is_negative && not that.is_negative) {
      result.is_negative = true;
   }
   else {
      result.is_negative = false;
   }
   return result;
}

bool bigint::operator== (const bigint& that) const {
   return is_negative == that.is_negative and uvalue == that.uvalue;
}

bool bigint::operator< (const bigint& that) const {
   if (is_negative != that.is_negative) return is_negative;
   return is_negative ? uvalue > that.uvalue
                      : uvalue < that.uvalue;
}

ostream& operator<< (ostream& out, const bigint& that) {
   return out << (that.is_negative ? "-" : "")
              << that.uvalue;
}

