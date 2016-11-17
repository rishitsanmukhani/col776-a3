#pragma once
#include "common.h"
#include "graph.h"

const char c[NUM_CHARS] = {'a', 'd', 'e', 'h', 'i', 'n', 'o', 'r', 's', 't'};

class Word{
public:
	char w1[MAX_WORD_LEN+1], w2[MAX_WORD_LEN+1];
	char pw1[MAX_WORD_LEN+1], pw2[MAX_WORD_LEN+1];
	vector<int> w1_i, w2_i;
	vector<int> pw1_i, pw2_i;
	vector<int> img1, img2;
	Graph graph;
	int n1, n2, n;	// number of characters
	int nc1, nc2, nc;		// number of correct characters
	double ll, T;		// log-likelihood
	Timer t;
	void init(){
		n1 = strlen(w1);img1.resize(n1);w1_i.resize(n1);pw1_i.resize(n1);
		n2 = strlen(w2);img2.resize(n2);w2_i.resize(n2);pw2_i.resize(n1);
		n = n1 + n2;
	}
	void convert_to_int(map<char,int>& char_to_id){
		for(int i=0;i<n1;i++)
			w1_i[i] = char_to_id[w1[i]];
		for(int i=0;i<n2;i++)
			w2_i[i] = char_to_id[w2[i]];
	}
	void convert_to_char(){
		pw1[n1]=0, pw2[n2]=0;
		for(int i=0;i<n1;i++)
			pw1[i] = c[pw1_i[i]];
		for(int i=0;i<n2;i++)
			pw2[i] = c[pw2_i[i]];
	}
	void init_graph(const Model& model, int type){
		graph.init(img1, img2, model, type);
		vector<int> v(n);
		vector<vector<double> > p(n, vector<double>(10, 0));
		t.reset();
		graph.gibbs_sampler(v, p);
		T = t.elapsed();
		for(int i=0;i<n1;i++)pw1_i[i]=v[i];
		for(int i=0;i<n2;i++)pw2_i[i]=v[i+n1];
		convert_to_char();
		init_accuracy();
		init_likelihood(p);
	}
	void init_likelihood(vector<vector<double > > &p){
		ll=0;
		for(int i=0;i<n1;i++)
			ll += log(p[i][w1_i[i]]);
		for(int i=0;i<n2;i++)
			ll += log(p[i+n1][w2_i[i]]);
	}
	void init_accuracy(){
		nc1=0;
		for(int i=0;i<n1;i++)
			if(pw1_i[i]==w1_i[i])
				nc1++;
		nc2=0;
		for(int i=0;i<n2;i++)
			if(pw2_i[i]==w2_i[i])
				nc2++;
		nc = nc1+nc2;
	}
	void print(){
		printf("%s %s\n", w1, w2);
		printf("%s %s\n", pw1, pw2);
		// graph.print();
		// printf("Word-1: %f (%d/%d)\n", double(nc1)/n1, nc1, n1);
		// printf("Word-2: %f (%d/%d)\n", double(nc2)/n2, nc2, n2);
		// printf("LL    : %f\n", ll/2);
		// printf("Time  : %f ms\n", T);
	}
};