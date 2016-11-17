#pragma once
#include "common.h"
#include "variable.h"

#define ALPHA (1e-2)

class Factor{
public:
	int id, n, sz;
	vector<Variable*> x;
  vector<int> base;
  vector<double> table;
  vector<double> table_sampled_data;
  vector<double> table_original_data;
	Factor(int _id=0):id(_id){
	}
	void parse(ifstream &fin, map<string, Variable*> &vars_factory){
    string dummy;
    getline(fin, dummy, '(');
    fin>>dummy;
    x.push_back(vars_factory[dummy]);
    fin>>dummy;
    if(dummy=="|"){
    	while(1){
	  		fin>>dummy;
	 	 		if(dummy.back()!=','){
	 	 			x.push_back(vars_factory[dummy]);
	 	 			break;
	 	 		}
 		   	dummy.pop_back();
	    	x.push_back(vars_factory[dummy]);
    	}
    }
    getline(fin, dummy,'}');
    init();
  }
  void init(){
    n = x.size();
    base.assign(n, 1);
    for(int i=1;i<n;i++){
      base[i] = (x[i-1]->nvals)*base[i-1];
    }
    sz = base[n-1]*x[n-1]->nvals;
    table.assign(sz, 0);
    table_original_data = table;
    table_sampled_data = table;
  }
  bool contain(Variable* v){
    for(auto var:x)
      if(var==v)return true;
    return false;
  }
  int key(const vector<int>& v){
    int ret=0;
    for(int i=0;i<n;i++){
      ret += v[i]*base[i];
    }
    return ret;
  }
  void learn(const map<Variable*, int>& sample, int type=0){
    vector<int> v(n);
    for(int i=0;i<n;i++){
      v[i] = sample.at(x[i]);
    }
    if(type==0)
      table[key(v)]+=1;
    else if(type==1)
      table_original_data[key(v)]+=1;
    else if(type==2)
      table_sampled_data[key(v)]+=1;
    else
      error("Unknown type!");
  }
  double prob(const vector<int>& sample){
    vector<int> v(n);
    for(int i=0;i<n;i++){
      v[i] = sample[(x[i]->id)];
    }
    return table[key(v)];
  }
  void normalize(){
    for(int i=0;i<sz;i+=x[0]->nvals){
      double sum=0;
      for(int j=0;j<x[0]->nvals;j++){
        sum += table[i+j];
      }
      for(int j=0;j<x[0]->nvals;j++){
        table[i+j] = (table[i+j]+1)/(sum + x[0]->nvals);
      }
    }
  }

  // Markov specific
  void normalize_markov(int type=0){
    double sum=0;
    for(int i=0;i<sz;i++){
      if(type==0) sum += table[i];
      else if(type==1) sum += table_original_data[i];
      else if(type==2) sum += table_sampled_data[i];
    }
    for(int i=0;i<sz;i++){
      if(type==0) table[i] /= sum;
      else if(type==1) table_original_data[i] /= sum;
      else if(type==2) table_sampled_data[i] /= sum;
    }
  }
  void initialize(){
    table.assign(sz, 1);
    table_original_data=table;
    table_sampled_data=table;
  }
  void clear(){
    table_sampled_data.assign(sz, 1);
  }
  double gd(double t, double c){
    double delta=0;
    for(int i=0;i<sz;i++){
      table[i] = table[i] + (table_original_data[i] - table_sampled_data[i] + c*table[i])*t;
      delta += abs(table_original_data[i]-table_sampled_data[i]);
    }
    return delta;
  }
  vector<string> get_strings(int key){
    vector<string> ret;
    for(int i=0;i<x.size();i++){
      int id = key%(x[i]->nvals);
      ret.push_back(x[i]->vals[id]);
      key /= x[i]->nvals;
    }
    return ret;
  }
};