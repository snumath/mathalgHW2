#include <iostream>
#include <forward_list>
#include <cstring>
#include <sstream>
#include <iterator>
#include <vector>
#include <utility>
#include "gmpxx.h"
#include "gmp.h"
using namespace std;

// @TODO
// Split into header. It's too mess!

class Mono{
    public:
        mpz_t coeff;        // coefficient can be rational number.
        forward_list<pair<string, int>> univariate;
        // Note that every deg elements are natural number.
        // Store name of variable. For instance, x^3 y^2 z^5 will be 'z' -> 'y' -> 'x'
    
        Mono(string LaTeX);
        Mono(mpz_t coeff, string vari, int deg);
        Mono(mpz_t coeff, forward_list<pair<string, int>> univariate_input);
        void print();
        void println();
};

class Poly{
    public:
    forward_list<Mono> mono;

    void insert(Mono mono);
    bool isZero();
    void println();

    Poly operator = (Poly &);
    Poly operator + (Poly &);
    Poly operator - (Poly &);
    Poly operator * (Poly &);
    Poly operator / (Poly &);
    Poly operator % (Poly &);

    Poly(string LaTeX);
    Poly(forward_list<Mono> poly);

    private:
    void delete_zero();
};


bool delete_zero_Help(Mono univariate);


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
        univariate.push_front(make_pair(token,0));
        return;
    }

    mpz_set_si(coeff, stoi(token));

    while (ss >> token){
        int index = token.find('^');
        univariate.push_front(make_pair(token.substr(0,index), stoi(token.substr(index+1))));
    }
    univariate.reverse();
}


Mono::Mono(mpz_t coeff, string vari, int deg){
    mpz_set(this->coeff, coeff);
    univariate.push_front(make_pair(vari, deg));
}

Mono::Mono(mpz_t coeff, forward_list<pair<string, int>> univariate_input){
    mpz_set(this->coeff, coeff);
    forward_list<pair<string, int>>::iterator iter;

    for (iter = univariate_input.begin(); iter != univariate_input.end(); ++iter){
        univariate.push_front(make_pair(iter->first, iter->second));
    }
    univariate.reverse();
}

int lexicographic_order(Mono lhs, Mono rhs){
    forward_list<pair<string, int>>::iterator liter;
    forward_list<pair<string, int>>::iterator riter;

    while((liter != lhs.univariate.end()) && (riter != rhs.univariate.end())){
        if (liter->first.compare(riter->first)){
            return (liter->first.compare(riter->first));
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
    // I think it is not a lexicographic_order, or I just misunderstood the problem
    // @TODO
    // Implement degree order.

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
        cout << "0" << endl;
        return;
    }

    cout << coeff;
    forward_list<pair<string, int>>::iterator iter;

    for (iter = univariate.begin(); iter !=univariate.end(); ++iter){
        cout << iter->first << "^" << iter->second;
    }
}


void Mono::println(){
    if (!mpz_cmp_si(coeff,0)){
        cout << "0" << endl;
        return;
    }

    cout << coeff;
    forward_list<pair<string, int>>::iterator iter;

    for (iter = univariate.begin(); iter !=univariate.end(); ++iter){
        cout << iter->first << "^" << iter->second;
    }

    cout << endl;
}


//**************************************************************
//**************************************************************
//
//                  Implementing Polynomial 
//
//**************************************************************
//**************************************************************

Poly::Poly(string LaTeX){

    // To prevent meaningless function call, I use push_front and reverse
    // Remark that mono.push_front(token_mono) can be converted to insert(token_mono)
    // Also remark that insert(token_mono) is too slow!

    size_t start = 0, end = 0;

    if ((end = LaTeX.find_first_of("+-", start)) != string::npos) {
        Mono token_mono(LaTeX.substr(start, end));
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
    insert(LaTeX.substr(start));

    mono.reverse();

    // @TODO
    // print " - " when coeff is negative.
}

Poly::Poly(forward_list<Mono> poly){
    
    forward_list<Mono>::iterator iter;

    for (iter = poly.begin(); iter != poly.end(); ++iter){
        Mono token(iter->coeff, iter->univariate);
        mono.push_front(token);
    }

    mono.reverse();
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
    return (!mpz_cmp_si(mono.begin()->coeff,0));
    // If leading term is zero, then this polynomial should be zero
}

void Poly::println(){

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


// Before operator, assume that we have sorted in order
// For this, I'll use lexicographical order
// I think using Karatsuba algorithm is not good for this :(
// For division with remainder, see pg 599 in Textbook.

Poly Poly::operator = (Poly & X){
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
    return *this;
}

/*
Following this:

void add(Node *p, Node *q)
{
	//A1
	p = p->link;

	Node *q1 = new Node;

	q1->coeff = q->coeff;
	q1->exponent = q->exponent;
	q1->sign = q->sign;
	q1->link = q->link;

	q = q->link;

	while(1){
		//A2
		while (p->exponent*p->sign < q->exponent*q->sign){
			q1 = q;
			q = q->link;
		}	
		
		//A3: add coefficient
		if (p->exponent*p->sign == q->exponent*q->sign){
			if (p->sign < 0)
				return;
			q->coeff += p->coeff;
			if (q->coeff == 0){
				//A4: delete zero term
				Node *q2 = new Node;
				q2 = q;
				q1->link = q;
				q = q->link;
				q2->link = NULL;
				p = p->link;
			}
			p = p->link;
			q1 = q;
			q = q->link;
		} else { 
			//A5: insert new term
			Node *q2 = new Node;

			q2->coeff = p->coeff;
			q2->exponent = p->exponent;
			q2->sign = p->sign;
			q2->link = q;
			q1->link = q2;
			q1 = q2;
			p = p->link;
		}
		
	}

}



*/

Poly Poly::operator + (Poly & X){

    Poly Y(mono);

	//A1
    forward_list<Mono>::iterator Xiter = X.mono.begin();;
    forward_list<Mono>::iterator Yiter = Y.mono.begin();;
	p = p->link;

	Node *q1 = new Node;

	q1->coeff = q->coeff;
	q1->exponent = q->exponent;
	q1->sign = q->sign;
	q1->link = q->link;

	q = q->link;

	while(1){
		//A2
		while (lexicographic_order(Xiter, Yiter) == -1){
            ++Yiter;
		}	
		
		//A3: add coefficient
		if (lexicographic_order(Xiter, Yiter) == 0){
			if (p->sign < 0)
				return;

			q->coeff += p->coeff;
			if (q->coeff == 0){
				//A4: delete zero term
				Node *q2 = new Node;
				q2 = q;
				q1->link = q;
				q = q->link;
				q2->link = NULL;
				p = p->link;
			}
			p = p->link;
			q1 = q;
			q = q->link;
		} else { 
			//A5: insert new term
			Node *q2 = new Node;

			q2->coeff = p->coeff;
			q2->exponent = p->exponent;
			q2->sign = p->sign;
			q2->link = q;
			q1->link = q2;
			q1 = q2;
			p = p->link;
		}
		
	}




    return Y;
}

Poly Poly::operator - (Poly & X){
    return X;
}

Poly Poly::operator * (Poly & X){
    return X;
}

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


//**************************************************************
//**************************************************************
//
//                      Main Function 
//
//**************************************************************
//**************************************************************

int main(){

    Mono m1("5 x^2 y^2");
    Mono m2("3 x^3 y^1");
    Mono m3(" ");
    Mono m4("4 x_1^3 x_2^3 y_2^3");

    m1.println();
    m2.println();
    m3.println();
    m4.println();


    Poly p1("5 x^2 y^3 - 7 x^1 y^4 z^5 + 19 z^1 + 20");
    Poly p2("7 x^3 y^2 z^2");
    Poly p3(" ");

    p1.println();
    p2.println();
    p3.println();

    return 0;
}
