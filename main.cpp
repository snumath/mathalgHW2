#include <algorithm>
#include <cstring>
#include <forward_list>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>
#include <utility>
#include "gmpxx.h"
#include "gmp.h"

using namespace std;

class Mono{
    public:
        mpz_t coeff;        // coefficient can be rational number.
        forward_list<pair<string, int>> univariate;
        // Note that every deg elements are natural number.
        // Store name of variable. For instance, x^3 y^2 z^5 will be 'z' -> 'y' -> 'x'
    
        Mono(string LaTeX);
        Mono(mpz_t coeff, string vari, int deg);
        Mono(mpz_t coeff, forward_list<pair<string, int>> univariate_input);
        Mono(const Mono &X);
        ~Mono();
        void print();
        void println();
        bool isConstant();
        bool operator == (Mono &);
        Mono product(Mono multiplicand);
};

class Poly{
    public:
    forward_list<Mono> mono;

    void insert(Mono mono);
    bool isZero();
    void println();
    void sort();

    Mono lt();
    Mono lm();
    //mpz_t lc();

    Poly operator = (Poly &);
    bool operator == (Poly &);
    Poly operator + (Poly &);
    Poly operator - (Poly &);
    Poly operator * (Poly &);
    Poly operator / (Poly &);
    Poly operator % (Poly &);

    Poly();
    Poly(string LaTeX);
    Poly(forward_list<Mono> poly);
    Poly(const Poly &);
    ~Poly();

    private:
    void delete_zero();
};


bool delete_zero_Help(Mono univariate);
Poly S_poly(Poly g, Poly h);


//**************************************************************
//**************************************************************
//
//                  Implementing Monomial 
//
//**************************************************************
//**************************************************************

Mono::Mono(string LaTeX){

    //************************************************************************************
    //************************************************************************************
    //                                  Parsing Problem                                 //
    //                                                                                  //
    //  1. Assume every monomial has form '5 x^3 y^2 z^1' (Solved)                      //
    //  2. Next, deal with '5 x^3 y^2 z'                                                //
    //  3. Deal with constant such as '10'                (Solved)                      //
    //  4. If we can, (but I think I don't have enough time) make '7x^{13}y^{2}z'       //
    //  5. Deal with x_1^7 x_2^3...                       (Solved)                      //
    //************************************************************************************
    //************************************************************************************

    // @TODO
    // Solve 2. It is crucial.
    // To solve 5, think what '_' would be

    mpz_init(coeff);

    stringstream    ss(LaTeX);
    string          token;

    ss >> token;
    
    if (token.empty()){
        univariate.push_front(make_pair(" ", 0));
        return;
    }

    mpz_set_si(coeff, stoi(token));

    while (ss >> token){
        int index = token.find('^');
        univariate.push_front(make_pair(token.substr(0,index), stoi(token.substr(index+1))));
    }

    if (univariate.begin() == univariate.end()){
        univariate.push_front(make_pair(" ", 0));
    }

    univariate.reverse();


}


Mono::Mono(mpz_t coeff_input, string vari, int deg){
    mpz_init(coeff);
    mpz_set(coeff, coeff_input);
    univariate.push_front(make_pair(vari, deg));
}

Mono::Mono(mpz_t coeff_input, forward_list<pair<string, int>> univariate_input){
    mpz_init(coeff);

    mpz_set(coeff, coeff_input);

    for (auto iter = univariate_input.begin(); iter != univariate_input.end(); ++iter){
        univariate.push_front(make_pair(iter->first, iter->second));
    }
    univariate.reverse();
}

Mono::Mono(const Mono &X){

    univariate.clear();
    mpz_init(coeff);
    mpz_set(coeff, X.coeff);
    
    forward_list<pair<string, int>> univ = X.univariate;

    forward_list<pair<string, int>>::iterator iter;

    for (iter = univ.begin(); iter != univ.end(); ++iter){
        univariate.push_front(make_pair(iter->first, iter->second));
    }

    univariate.reverse();
}

//Destructor
Mono::~Mono(){
    univariate.clear();
}

int lexicographic_order(Mono lhs, Mono rhs){
    forward_list<pair<string, int>>::iterator liter = lhs.univariate.begin();
    forward_list<pair<string, int>>::iterator riter = rhs.univariate.begin();

    string a;
    string b;

    while((liter != lhs.univariate.end()) && (riter != rhs.univariate.end())){
        a = liter->first;
        b = riter->first;

        //if (liter->first.compare(riter->first) != 0){
        if (a.compare(b) != 0){
            if (a.compare(b) < 0)
                return 1;
            if (a.compare(b) > 0)
                return -1;
            else{
                cout << "ERROR" << endl;
                return 0;
            }
        }
        else{
            if (liter->second > riter->second)
                return 1;
            else if (liter->second < riter->second)
                return -1;
            // Pass if liter == riter
        }
        ++liter;
        ++riter;
    }

    if (riter == rhs.univariate.end()){
        if (liter == lhs.univariate.end())
            return 0;
        return 1;
    }
    else{
        return -1;
    }
}

int deg_order(Mono lhs, Mono rhs){

    forward_list<pair<string, int>>::iterator liter;
    forward_list<pair<string, int>>::iterator riter;

    int ldeg, rdeg = 0;

    for (liter = lhs.univariate.begin(); liter != lhs.univariate.end(); ++liter)
        ldeg += liter->second;

    for (riter = rhs.univariate.begin(); riter != rhs.univariate.end(); ++riter)
        rdeg += riter->second;

    
    if (ldeg > rdeg)
        return 1;
    else if (ldeg == rdeg)
        return 0;
    else
        return -1;
        
}

// Print monomial in LaTeX-form.
void Mono::print(){
    if (!mpz_cmp_si(coeff,0)){
        cout << "0";
        return;
    }

    forward_list<pair<string, int>>::iterator iter;

    gmp_printf("%Zd", coeff);

    if (isConstant() == 1){
        return;
    }

    for (iter = univariate.begin(); iter !=univariate.end(); ++iter){
        cout << iter->first << "^" << iter->second;
    }
}

void Mono::println(){
    if (!mpz_cmp_si(coeff,0)){
        cout << "0" << endl;
        return;
    }

    gmp_printf("%Zd", coeff);

    if (isConstant() == 1){
        cout << endl;
        return;
    }

    forward_list<pair<string, int>>::iterator iter;

    for (iter = univariate.begin(); iter != univariate.end(); ++iter){
        cout << iter->first << "^" << iter->second;
    }

    cout << endl;
}

bool Mono::isConstant(){
    if (univariate.begin() == univariate.end())
        return true;
    if (univariate.begin()->second == 0)
        return true;
    return false;
}

Mono Mono::product(Mono multiplicand){
    mpz_t newCoeff;
    mpz_init(newCoeff);

    mpz_mul(newCoeff, coeff, multiplicand.coeff);

    if (isConstant() == 1){
        Mono token(newCoeff, multiplicand.univariate);
        return token;
    }

    if (multiplicand.isConstant() == 1){
        Mono token(newCoeff, univariate);
        return token;
    }

    forward_list<pair<string, int>> Yuniv;
    int flag = 0;

    forward_list<pair<string, int>>::iterator iter = univariate.begin();
    forward_list<pair<string, int>>::iterator Xiter = multiplicand.univariate.begin();

    forward_list<pair<string, int>>::iterator iter_end = univariate.end();
    forward_list<pair<string, int>>::iterator Xiter_end = multiplicand.univariate.end();


    while(flag == 0){
        flag = iter->first.compare(Xiter->first);

        if (flag > 0){
            Yuniv.push_front(make_pair(Xiter->first, Xiter->second));
            ++Xiter;
        } else if (flag < 0){
            Yuniv.push_front(make_pair(iter->first, iter->second));
            ++iter;
        } else{
            Yuniv.push_front(make_pair(iter->first, iter->second + Xiter->second));
            ++iter;
            ++Xiter;
        }

        flag = 0;
        if (Xiter == Xiter_end)
            flag += 1000;
        if (iter == iter_end)
            flag += 2000;
    }

    if (flag == 2000){
        while(Xiter != Xiter_end){
            Yuniv.push_front(make_pair(Xiter->first, Xiter->second));
            ++Xiter;
        }
    }
    else if(flag == 1000){
        while(iter != iter_end){
            Yuniv.push_front(make_pair(iter->first, iter->second));
            ++iter;
        }
    }
    else if (flag == 3000){
        // No instruction
    }else{
        // Cannot be occur
        cout << "ERROR" << endl;
    }

    Yuniv.reverse();
    Mono Y(newCoeff, Yuniv);
    return Y;
}

bool Mono::operator == (Mono &X){
    forward_list<pair<string,int>>::iterator iter = univariate.begin();
    forward_list<pair<string,int>>::iterator Xiter = X.univariate.begin();

    while (true){
        if (iter->first != Xiter->first){
            return false;
        }
        
        if (iter->second != Xiter->second){
            return false;
        }

        ++iter;
        ++Xiter;

        if (iter == univariate.end()){
            if (Xiter == X.univariate.end())
                return true;
            return false;
        }

        if (Xiter == X.univariate.end())
            return false;
    }
}

//**************************************************************
//**************************************************************
//
//                  Implementing Polynomial 
//
//**************************************************************
//**************************************************************

Poly::Poly(){
    mono.clear();
}

Poly::Poly(string LaTeX){

    // To prevent meaningless function call, I use push_front and reverse
    // Remark that mono.push_front(token_mono) can be converted to insert(token_mono)
    // Also remark that insert(token_mono) is too slow!

    size_t start = 0, end = 0;
    if (LaTeX.at(0) == '-')
        start++;

    if ((end = LaTeX.find_first_of("+-", start)) != string::npos) {
        Mono token_mono(LaTeX.substr(start, end));
        if (start == 1)
            mpz_neg(token_mono.coeff, token_mono.coeff);
        mono.push_front(token_mono);
        start = end + 1;
    }
    else{
        Mono token_mono(LaTeX);
        mono.push_front(token_mono);
        return;
    }

    while ((end = LaTeX.find_first_of("+-", start)) != string::npos) {
        Mono token_mono(LaTeX.substr(start, end - start));
        if (LaTeX.at(start - 1) == 45)
            mpz_neg(token_mono.coeff, token_mono.coeff);
        mono.push_front(token_mono);
        start = end + 1;
    }

    Mono token_mono(LaTeX.substr(start));
    if (LaTeX.at(start - 1) == 45)
        mpz_neg(token_mono.coeff, token_mono.coeff);
    mono.push_front(token_mono);

    mono.reverse();

    // @TODO
    // print " - " when coeff is negative.
}

Poly::Poly(forward_list<Mono> poly){
    
    forward_list<Mono>::iterator iter;
    mono.clear();

    for (iter = poly.begin(); iter != poly.end(); ++iter){
        Mono token(iter->coeff, iter->univariate);
        mono.push_front(token);
    }

    mono.reverse();
}

Poly::Poly(const Poly &X){
    // Be care not to shallow copy
    // In general, we want to use h = f + g, then use f, g, and h independently.

    mono.clear();
    
    forward_list<Mono> poly = X.mono;

    forward_list<Mono>::iterator iter;
    for (iter = poly.begin(); iter != poly.end(); ++iter){
        Mono token(iter->coeff, iter->univariate);
        mono.push_front(token);
    }

    mono.reverse();
}

Poly::~Poly(){
    mono.clear();
}
 
void Poly::insert(Mono token_mono){
    // @TODO
    // Make insert algorithm
    // Note: the terms in polynomial are sorted in lexicographical order (Of course, we can modify the order)
    // Find where to be inserted
    // I think while loop is suitable for it.

    forward_list<Mono>::iterator iter = mono.begin();

    if (lexicographic_order(*iter, token_mono) == -1){
        mono.push_front(token_mono);
    }

    forward_list<Mono>::iterator iter_temp = mono.begin();
    ++iter;
    
    while((lexicographic_order(*iter, token_mono) > 0) && iter != mono.end()){
        ++iter_temp;
        ++iter;
    }

    if (lexicographic_order(*iter, token_mono))
        mono.insert_after(iter_temp,token_mono); // Since there is no insert_before :(
    else
        mpz_add(iter->coeff, iter->coeff, token_mono.coeff);
}

bool Poly::isZero(){
    if (mono.begin() == mono.end())
        return true;
    return (mpz_cmp_si(mono.begin()->coeff,0));
    // If leading term is zero, then this polynomial should be zero
}

void Poly::println(){

    if (mono.begin() == mono.end()){
        cout << "0" << endl;
        return;
    }

    if (!mpz_cmp_si(mono.begin()->coeff,0)){
        cout << "0" << endl;
        return;
    }

    forward_list<Mono>::iterator iter;
    forward_list<Mono>::iterator iter_end = mono.end();

    for (iter = mono.begin(); distance(iter, iter_end) > 1; ++iter){
        iter->print();
        cout << " + ";
    }
    iter->print();
    cout << endl;
}

void Poly::delete_zero(){

    mono.remove_if(delete_zero_Help);

    /*
    forward_list<Mono>::iterator iter;
    for (iter = mono.begin(); iter != mono.end(); ++iter){
        if (!mpz_cmp_si(iter->coeff, 0))
            mono.remove(*iter);
    }
    */
}

void Poly::sort(){
    // If we want to use another sorting method, then change return condition
    mono.sort( [](const Mono &a, const Mono &b ) {
        return (lexicographic_order(a,b) >= 0);
        } );

    forward_list<Mono>::iterator rear = mono.begin();
    forward_list<Mono>::iterator front = mono.begin();

    ++rear;
    
    while(rear != mono.end()){
        if (*rear ==*front){
            mpz_add(front->coeff, front->coeff, rear->coeff);
            mpz_init(rear->coeff);
            rear->univariate.clear();
            ++rear;
        }
        else{
            ++rear;
            ++front;
        }
    }

    delete_zero();
}

Mono Poly::lt(){

    if (mono.empty() == 1){
        mpz_t a;
        mpz_init(a);
        mpz_set_si(a, 0);
        forward_list<pair<string, int>> b;
        b.push_front(make_pair(" ", 0));
        Mono token(a,b);
        return token;
    }


    mpz_t a;
    mpz_init(a);
    mpz_set_si(a, 1);

    Mono token(a, mono.begin()->univariate);
    return token;
}

Mono Poly::lm(){
    if (mono.empty() == 1){
        mpz_t a;
        mpz_init(a);
        mpz_set_si(a, 0);
        forward_list<pair<string, int>> b;
        b.push_front(make_pair(" ", 0));
        Mono token(a,b);
        return token;
    }

    Mono token(mono.begin()->coeff, mono.begin()->univariate);

    return token;
}

/*
It is a function returning array, hence it is invalid...
mpz_t Poly::lc(){
    if (mono.empty() == 1){
        return 0;
    }

    mpz_t a;
    mpz_init(a);
    mpz_set(a, b);
    return a;
}
*/


// Before operator, assume that we have sorted in order
// For this, I'll use lexicographical order
// I think using Karatsuba algorithm is not good for this :(
// For division with remainder, see pg 599 in Textbook.

// Unfortunately, = operator does not work, due to 'cannot bind lvalue problem'
// I think there are some operator overloading problems.
Poly Poly::operator = (Poly & X){
    // Be care not to shallow copy
    // In general, we want to use h = f + g, then use f, g, and h independently.

    /*
    mono.clear();
    
    //forward_list<Mono> poly = X.mono;

    //forward_list<Mono>::iterator iter;
    for (auto iter = X.mono.begin(); iter != X.mono.end(); ++iter){
        Mono token(iter->coeff, iter->univariate);
        //Mono token(*iter);
        mono.push_front(token);
    }
    mono.reverse();
    return *this;
    */


   Poly Y(X);
   return Y;
}

bool Poly::operator == (Poly & X){
    forward_list<Mono>::iterator iter = mono.begin();
    forward_list<Mono>::iterator Xiter = X.mono.begin();

    
    while (true){

        if (~(*iter == *Xiter))
            return false;

        ++iter;
        ++Xiter;

        if (iter == mono.end()){
            if (Xiter == X.mono.end())
                return true;
            return false;
        }

        if (Xiter == X.mono.end())
            return false;
    }
}

Poly Poly::operator + (Poly & X){

    forward_list<Mono> Ymono;
    int flag = 0;

    forward_list<Mono>::iterator iter = mono.begin();
    forward_list<Mono>::iterator Xiter = X.mono.begin();

    forward_list<Mono>::iterator iter_end = mono.end();
    forward_list<Mono>::iterator Xiter_end = X.mono.end();

    while(flag == 0){
        flag = lexicographic_order(*Xiter, *iter);

        if (flag == 1){
            Mono token(*Xiter);
            Ymono.push_front(token);
            ++Xiter;
        } else if (flag == -1){
            Mono token(*iter);
            Ymono.push_front(token);
            ++iter;
        } else{
            Mono token(*iter);
            mpz_add(token.coeff, token.coeff, Xiter->coeff);
            Ymono.push_front(token);
            ++iter;
            ++Xiter;
        }

        flag = 0;
        if (Xiter == Xiter_end)
            flag += 1000;
        if (iter == iter_end)
            flag += 2000;
    }

    if (flag == 2000){
        while(Xiter != Xiter_end){
            Mono token(*Xiter);
            Ymono.push_front(token);
            ++Xiter;
        }
    }
    else if(flag == 1000){
        while(iter != iter_end){
            Mono token(*iter);
            Ymono.push_front(token);
            ++iter;
        }
    }
    else if (flag == 3000){
        // No instruction
        cout << "flag = 3000" << endl;
    }else{
        // Cannot be occur
        cout << "ERROR" << endl;
    }

    Ymono.reverse();
    Poly Y(Ymono);
    Y.delete_zero();

    return Y;
}


Poly Poly::operator - (Poly & X){

    forward_list<Mono> Ymono;
    int flag = 0;

    forward_list<Mono>::iterator Xiter = X.mono.begin();
    forward_list<Mono>::iterator iter = mono.begin();

    forward_list<Mono>::iterator Xiter_end = X.mono.end();
    forward_list<Mono>::iterator iter_end = mono.end();

	while(!flag){

        flag = lexicographic_order(*Xiter, *iter);

        if (flag == 1){
            Mono token(*Xiter);
            mpz_ui_sub(token.coeff, 0, token.coeff);
            Ymono.push_front(token);
            ++Xiter;
        } else if (flag == -1){
            Mono token(*iter);
            Ymono.push_front(token);
            ++iter;
        } else{
            Mono token(*iter);
            mpz_sub(token.coeff, token.coeff, Xiter->coeff);
            Ymono.push_front(token);
            ++Xiter;
            ++iter;
        }

        flag = 0;
        if (Xiter == Xiter_end)
            flag += 1000;
        if (iter == iter_end)
            flag += 2000;
	}

    if (flag == 2000){
        while(Xiter != Xiter_end){
            Mono token(*Xiter);
            Ymono.push_front(token);
            ++Xiter;
        }
    }
    else if(flag == 1000){
        while(iter != iter_end){
            Mono token(*iter);
            Ymono.push_front(token);
            ++iter;
        }
    }
    /*
    else if (flag == 3000){
        cout << "flag = 3000" << endl;
    }else{
        cout << "ERROR" << endl;
    }
    */

    Ymono.reverse();
    Poly Y(Ymono);
    Y.delete_zero();

    return Y;
}

Poly Poly::operator * (Poly & X){
    if (isZero() == 0){
        Poly Y(" ");
        return Y;
    }

    if (X.isZero() == 0){
        Poly Y(" ");
        return Y;
    }

    forward_list<Mono> Ymono;

    forward_list<Mono>::iterator iter;
    forward_list<Mono>::iterator Xiter;

    forward_list<Mono>::iterator iter_end = mono.end();
    forward_list<Mono>::iterator Xiter_end = X.mono.end();


    for (iter = mono.begin(); iter != iter_end; ++iter){
        for (Xiter = X.mono.begin(); Xiter != Xiter_end; ++Xiter){

            // This code seems to very ugly, but we have to do it since there is 4 linked lists
            Mono token(iter->product(*Xiter));
            Ymono.push_front(token);
        }
    }

    Ymono.reverse();
    Poly Y(Ymono);
    Y.sort();
    Y.delete_zero();
    Y.sort();

    // Now, Both this and X are not zero polynomial.
    return Y;
}

// Assume that the polynomial is divisible by X
// Of cousre, it is not a division algorithm
// We should modify
Poly Poly::operator / (Poly & X){
    return X;
}

Poly Poly::operator % (Poly & X){
    return X;
}


//**************************************************************
//**************************************************************
//
//                  Other helpful functions 
//
//**************************************************************
//**************************************************************

bool delete_zero_Help(Mono univariate){
    return (!mpz_cmp_si(univariate.coeff,0));
}

Poly S_poly(Poly g, Poly h){

/*
    forward_list<pair<string, int>> gamma;

    forward_list<Mono>::iterator giter = g.begin();
    forward_list<Mono>::iterator hiter = h.begin();

    for(giter = g.begin(); giter != g.end(); ++giter){
        for(hiter = h.begin(); hiter != h.end(); ++hiter){

        }
    }
    */

    return g;
}

//**************************************************************
//**************************************************************
//
//                      Main Function 
//
//**************************************************************
//**************************************************************

int main(){

    cout << "###   Monomial Test Case   ###" << endl;

    Mono m1("5 x^2 y^2 z^1");
    Mono m2("3 x^3 y^1");
    Mono m3(" ");
    Mono m4("4 x_1^3 x_2^3 y_2^3");

    m1.println();
    m2.println();
    m3.println();
    m4.println();

    Mono m5(m1.product(m2));
    m5.println();

    cout << "###   Polynomial Test Case   ###" << endl;

    Poly p1("5 x^2 y^3 - 7 x^1 y^4 z^5 + 19 z^1 + 20");
    Poly p2("7 x^3 y^2 z^2 - 4 x^2 z^1 + 30 x^1 y^3");
    Poly p3(" ");

    p1.println();
    p2.println();
    p3.println();

    Poly p4(p1+p2);
    p4.println();

    Poly p5("-7 x^3 y^2 z^2");
    p5.println();
    Poly p6(p4-p5);
    p6.println();
    Poly p7("-7 x^3 y^2 z^2");
    p7.println();

    Poly p8(p5-p7);
    p8.println();

    Poly p9(p1*p2);
    p9.println();

    Poly f("3 x^2 y^1 + 5 x^1 y^2");

    Poly g("4 x^3 y^2 + 5 x^2 y^3");

    Poly fg(f*g);

    fg.println();
    

    cout << "NO SEGMENTATION FAULT" << endl;
    return 0;
}
