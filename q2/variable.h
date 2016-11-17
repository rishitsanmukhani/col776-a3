#pragma once
#include "common.h"

class Variable{
public:
  int id, nvals;
  string name, val;
  vector<string> vals;
  Variable(int _id):id(_id){
  }
  void parse(ifstream &fin){
    string dummy;
    fin>>name;
    getline(fin, dummy, '[');
    fin>>nvals;
    getline(fin, dummy, '{');
    for(int i=0;i<nvals-1;i++){
      fin>>dummy;
      dummy.pop_back();
      vals.push_back(dummy);
    }
    fin>>dummy;
    vals.push_back(dummy);
    getline(fin, dummy);
    getline(fin, dummy);
  }
  int get_val(string s){
    for(int i=0;i<nvals;i++)
      if(s==vals[i])
        return i;
    error("Value error!");
  }
  void set_val(int i){
    assert(i>=0 && i<nvals);
    val = vals[i];
  }
  void print(){
    cout<<name<<endl;
    for(auto &s:vals)
      cout<<s<<" ";
    cout<<endl;
  }
};