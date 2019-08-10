#include <cctype>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include <cmath>
#include <vector>
#include <sstream>
using namespace std;

#include "ubigint.h"
#include "debug.h"

//used to check length of vectors
const int THIS = 0;
const int THAT = 1;
const int EQLN = 2;

ubigint::ubigint (unsigned long that) {
   stringstream ss;
   string that_;
   ss << that;
   ss >> that_;

   for (char digit: that_) {
      ubigvalue_t::iterator it = ubig_value.begin();
      ubig_value.insert(it, digit);
      if (not isdigit(digit)) {
         throw invalid_argument ("ubigint::ubigint(" + that_ + ")");
      }
   }
}

ubigint::ubigint (const string& that) {
   for (char digit: that) {
      ubigvalue_t::iterator it = ubig_value.begin();
      ubig_value.insert(it, digit);
      if (not isdigit (digit)) {
         throw invalid_argument ("ubigint::ubigint(" + that + ")");
      }
 
  }
}

ubigint ubigint::operator+ (const ubigint& that) const {
   ubigint ubig_int;
   int loop_len;
   int carry = 0;
   int rem;
   int longer_num_len; 
   int longer_num = EQLN; // set flag to equal length
   int thisnum, thatnum;
   int sum;
   unsigned char char_rem;
   int i;

   if (this->ubig_value.size() < that.ubig_value.size()) {
      longer_num_len = that.ubig_value.size();
      loop_len = this->ubig_value.size(); // endLoop&bringxtrNumsDown
      longer_num = THAT;
   }
   else if (this->ubig_value.size() > that.ubig_value.size()) {
      longer_num_len = this->ubig_value.size();
      loop_len = that.ubig_value.size();
      longer_num = THIS;
   }
   else {
      longer_num_len = this->ubig_value.size();
      loop_len = this->ubig_value.size();
      longer_num = EQLN;
   }

   for (i = 0; i < loop_len; ++i) {
      thisnum = this->ubig_value[i] - '0'; 
      thatnum = that.ubig_value[i]  - '0';
      sum = thisnum + thatnum + carry;
      rem = sum % 10; // get single digit
      char_rem = rem + '0';
      if (sum < 10) {
         carry = 0;
         ubig_int.ubig_value.push_back(char_rem);
      }
      else {
         carry = 1;
         ubig_int.ubig_value.push_back(char_rem);
      }   
   }

   if (longer_num == THIS) {
      for (i = loop_len; i < longer_num_len; ++i) {
         thisnum = this->ubig_value[i] - '0';
         sum = thisnum + carry;
         rem = sum % 10;
         char_rem = rem + '0';
         if (sum < 10) {
            carry = 0;
            ubig_int.ubig_value.push_back(char_rem);
         }
         else {
            carry = 1;
            ubig_int.ubig_value.push_back(char_rem);
         }
      }
   }
   else if (longer_num == THAT) {
      for (i = loop_len; i < longer_num_len; ++i) {
         thisnum = that.ubig_value[i] - '0';
         sum = thisnum + carry;
         rem = sum % 10;
         char_rem = rem + '0';
         if (sum < 10) {
            carry = 0;
            ubig_int.ubig_value.push_back(char_rem);
         }
         else {
            carry = 1;
            ubig_int.ubig_value.push_back(char_rem);
         }
      }
   }
   // else longer_num == EQLN   

   if (carry == 1) {
      unsigned char char_carry = carry + '0';
      ubig_int.ubig_value.push_back(char_carry);
   }

   return ubig_int;;
}

ubigint ubigint::operator- (const ubigint& that) const {
   if (*this < that) throw domain_error ("ubigint::operator-(a<b)");

   ubigint ubig_int;
   ubigvalue_t this_vec = this->ubig_value;
   ubigvalue_t that_vec = that.ubig_value;   
   int thisnum, thatnum;
   int diff;
   int carry = 0;
   unsigned char char_diff;
   unsigned char lead_zero = '0';
   int i;
   if (this->ubig_value.size() < that.ubig_value.size()) {
      for (i = this->ubig_value.size(); 
            i < that.ubig_value.size(); ++i) {
         this_vec.push_back(lead_zero);
      }
   }
   else if (this->ubig_value.size() > that.ubig_value.size()) {
      for (i = that.ubig_value.size(); 
            i < this->ubig_value.size(); ++i) {
         that_vec.push_back(lead_zero); 
      }
   }

   for (i = 0; i < this_vec.size(); ++i) {
      thisnum = this_vec[i] - '0';
      thatnum = that_vec[i] - '0';

      diff = thisnum - thatnum + carry;
      if (diff < 0) {
         diff += 10;
         carry = -1;
      }
      else {
         carry = 0;
      }
      char_diff = diff + '0';
      ubig_int.ubig_value.push_back(char_diff);
   }

   for (i = ubig_int.ubig_value.size() - 1; i >= 0; --i) {
      if (ubig_int.ubig_value.size() == 1 ||
          ubig_int.ubig_value[i-1] == '\0') {
         break;
      }
      else if (ubig_int.ubig_value[i] == '0' 
                && ubig_int.ubig_value[i-1] != '0') {
         ubig_int.ubig_value.pop_back();
      }
      else if (ubig_int.ubig_value[i] == '0' 
                && ubig_int.ubig_value[i-1] == '0') {
         ubig_int.ubig_value.pop_back();
         break;
      }
      else break;
   } 

   return ubig_int;
}

ubigint ubigint::operator* (const ubigint& that) const {
   ubigint ubig_int;   

   vector<unsigned char> product(this->ubig_value.size() 
                            + that.ubig_value.size(), '0');
   
   int c, d;
   int i, j;
   for (i = 0; i < this->ubig_value.size(); ++i) {
      c = 0;
      for (j = 0; j < that.ubig_value.size(); ++j) {
         d = (product[i + j] - '0') + ((this->ubig_value[i] - '0')
             * (that.ubig_value[j] - '0')) + c;
         product[i + j] = ((d % 10) + '0');
         c = floor(d / 10);
      }
      product[i + that.ubig_value.size()] = c + '0';
   }
   
   ubig_int.ubig_value = product;

   for (i = ubig_int.ubig_value.size() - 1; i >= 0; --i) {
      if (ubig_int.ubig_value.size() == 1 ||
          ubig_int.ubig_value[i - 1] == '\0') {
         break;
      }
      else if (ubig_int.ubig_value[i] == '0'
                && ubig_int.ubig_value[i - 1] != '0') {
         ubig_int.ubig_value.pop_back();
      }
      else if (ubig_int.ubig_value[i] == '0'
                && ubig_int.ubig_value[i - 1] == '0') {
         ubig_int.ubig_value.pop_back();
         break;
      }
      else break;
   }
 
   return ubig_int;
}

void ubigint::multiply_by_2() {
   ubigint ubig_int;
   vector<unsigned char> product(this->ubig_value.size() + 1, '0');
   vector<unsigned char> that;
   that.push_back('2');

   int c, d;
   int i, j;
   for (i = 0; i < this->ubig_value.size(); ++i) {
      c = 0;
      for (j = 0; j < that.size(); ++j) {
         d = (product[i + j] - '0') + ((this->ubig_value[i] - '0') 
             * (that[j] - '0')) + c;
         product[i + j] = ((d % 10) + '0');
         c = floor(d / 10);
      }
      product[i + that.size()] = c + '0';
   }
   
   ubig_int.ubig_value = product;   

   for (i = ubig_int.ubig_value.size() - 1; i >= 0; --i) {
      if (ubig_int.ubig_value.size() == 1 ||
          ubig_int.ubig_value[i-1] == '\0') {
         break;
      }
      else if (ubig_int.ubig_value[i] == '0'
                && ubig_int.ubig_value[i - 1] != '0') {
         ubig_int.ubig_value.pop_back();
      }
      else if (ubig_int.ubig_value[i] == '0'
                && ubig_int.ubig_value[i-1] == '0') {
         ubig_int.ubig_value.pop_back();
         break;
      }
      else break;
   }
   this->ubig_value.clear();
   this->ubig_value = ubig_int.ubig_value;
}

void ubigint::divide_by_2() {
   
   vector<unsigned char> answer;
   int calc;
   int sub;
   int dividend = this->ubig_value.back() - '0';
   int i;

   for (i = this->ubig_value.size() - 1; i >= 0; --i) {
      if (dividend < 2) {
         answer.push_back('0');
         if (dividend == 0) {
            dividend = (this->ubig_value[i - 1] - '0');
         }
         else {
            dividend = (this->ubig_value[i - 1] - '0') 
                        + (dividend * 10);
         }
      }
      else {
         calc = dividend / 2;
         answer.push_back(calc + '0');
         if(i == 0) break;
         else {
            sub = dividend - (2 * calc);
            dividend = (sub * 10) 
                        + (this->ubig_value[i - 1] - '0');
         }
      }    
   }
   this->ubig_value.clear();
   for (i = answer.size() - 1; i >= 0; --i) {
      this->ubig_value.push_back(answer[i]);
   }
   for (i = this->ubig_value.size() - 1; i >= 0; --i) {
      if(this->ubig_value[i] != '0') break;
      else this->ubig_value.pop_back();
   } 
}

struct quo_rem { ubigint quotient; ubigint remainder; };
quo_rem udivide (const ubigint& dividend, ubigint divisor) {
   // Note: divisor is modified so pass by value (copy).
   ubigint zero {0};
   if (divisor == zero) throw domain_error ("udivide by zero");
   ubigint power_of_2 {1};
   ubigint quotient {0};
   ubigint remainder {dividend}; // left operand, dividend
   while (divisor < remainder) {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }
   while (power_of_2 > zero) {
      if (divisor <= remainder) {
         remainder = remainder - divisor;
         quotient = quotient + power_of_2;
      }
      divisor.divide_by_2();
      power_of_2.divide_by_2();
   }
   return {.quotient = quotient, .remainder = remainder};
}

ubigint ubigint::operator/ (const ubigint& that) const {
   return udivide (*this, that).quotient;
}

ubigint ubigint::operator% (const ubigint& that) const {
   return udivide (*this, that).remainder;
}

bool ubigint::operator== (const ubigint& that) const {

   int this_size = this->ubig_value.size();
   int that_size = that.ubig_value.size();
   bool is_eq = false;
   bool eq_len = false;
   int i;

   eq_len = (this_size == that_size);
   if (eq_len) {
      is_eq = true;
      for (i = 0; i < this_size; ++i) {
         if(this->ubig_value[i] != that.ubig_value[i]) {
            is_eq = false;
            return is_eq;
         }
      }
   }
   return is_eq;
}

bool ubigint::operator< (const ubigint& that) const {
   
   int this_size = this->ubig_value.size();
   int that_size = that.ubig_value.size();
   int i;   

   if (this_size > that_size) return false;
   else if (this_size < that_size) return true;
   else {
      for (i = this_size - 1; i >= 0; --i) {
         if ((this->ubig_value[i] != that.ubig_value[i])
             && (this->ubig_value[i] < that.ubig_value[i])) {
            return true;
         }
         else if ((this->ubig_value[i] != that.ubig_value[i])
             && (this->ubig_value[i] > that.ubig_value[i])) {
            return false;
         }
      }
   }
   return false;
}

ostream& operator<< (ostream& out, const ubigint& that) { 
   int that_size = that.ubig_value.size();
   int i;
   for (i = that_size - 1; i >= 0; --i) {
      out << that.ubig_value[i];
      if (i%69 == 0 && i!=0) {
         out << "\\\n";
      }
   }
   //out << endl;
   return out;
}

