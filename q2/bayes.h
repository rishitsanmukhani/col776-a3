#pragma once
#include "common.h"
#include "variable.h"
#include "factor.h"

class Bayes{
public:
  // Streams
  ifstream fin;
  ofstream fout;

  // Bayes specific
  int nvars, nfactors;
  vector<Variable*> vars;
  vector<Factor*> factors;
  map<string, Variable*> vars_factory;
  void init(string bif){
    fin.open(bif);
    if(!fin.good()){
      error("Unable to open file!");
    }
    nvars=0;
    nfactors=0;
    parse_bif_file();
    fin.close();
  }
  void parse_bif_file(){
    cout<<"Creating Bayesian Network ..."<<endl;
    string line;
    getline(fin, line);
    getline(fin, line);
    while(fin>>line){
      if(line=="variable"){
        vars.push_back(new Variable(nvars++));
        vars.back()->parse(fin);
        vars_factory[vars.back()->name] = vars.back();
      }
      else{
        factors.push_back(new Factor(nfactors++));
        factors.back()->parse(fin, vars_factory);
      }
    }
  }
  void parse_dat_file(string dat){
    cout<<"Learning CPTs ..."<<endl;
    fin.open(dat);
    if(!fin.good()){
      error("Unable to open file!");
    }
    string s;
    vector<Variable*> v(nvars);
    for(int i=0;i<nvars;i++){
      fin>>s;
      v[i] = vars_factory[s];
    }
    map<Variable*, int> sample;
    while(fin>>s){
      sample[v[0]] = v[0]->get_val(s);
      for(int i=1;i<nvars;i++){
        fin>>s;
        sample[v[i]] = v[i]->get_val(s);
      }
      learn_factors(sample);
    }
    normalize_factors();
    fin.close();
  }
  void learn_factors(const map<Variable*, int>& sample){
    for(auto& factor:factors){
      factor->learn(sample);
    }
  }
  void normalize_factors(){
    for(auto& factor:factors){
      factor->normalize();
    }
  }
  void emit(string bif){
    cout<<"Emitting Bayesian Network ..."<<endl;
    fin.open(bif);
    fout.open(bif+".out.bn");
    string s;
    getline(fin, s);fout<<s<<"\n";
    getline(fin, s);fout<<s<<"\n";
    for(int i=0;i<3*nvars;i++){
      getline(fin, s);fout<<s<<"\n";
    }
    for(int i=0;i<nvars;i++){
      getline(fin, s);fout<<s<<"\n";
      auto &p = factors[i]->table;
      for(int j=0;j<p.size();j++){
        getline(fin, s,'?');fout<<s<<p[j];
      }
      getline(fin, s);fout<<s<<"\n";
      getline(fin, s);fout<<s<<"\n";
    }
    fout.close();
    fin.close();
  }
  // Query
  struct Query{
    vector<vector<double> > marginal;   // marginal distribution
    vector<int> x;                      // true values
    vector<int> px;                     // predicted values
    vector<int> unknown;                // unknown values
    double ll, acc;
    void init(int n=0){
      x.resize(n);
      px.resize(n);
      marginal.resize(n);
    }
    void calculate_accuracy(){
      acc=0, ll=0;
      for(auto i:unknown)
        if(x[i]==px[i])
          acc+=1;
      acc /= unknown.size();
      for(int i=0;i<unknown.size();i++){
        int j = unknown[i];
        ll += log(marginal[j][x[j]]);
      }
    }
    void calcuate_marginal(){
      for(int i=0;i<x.size();i++){
        double sum=0;
        px[i] = 0;
        for(int j=0;j<marginal[i].size();j++){
          sum += marginal[i][j];
          if(marginal[i][px[i]] < marginal[i][j]){
            px[i] = j;
          }
        }
        for(auto &cnt:marginal[i]){
          cnt = (1+cnt)/(marginal[i].size()+sum);
        }
      }
    }
    void print(){
      // cout<<"Ch-Acc: "<<acc<<endl;
      // cout<<"LL    : "<<ll<<endl;
      // cout<<endl;
    }
  };
  vector<Query> queries;
  double acc, ll;
  void parse_test_sample(map<Variable*, int>& sample){
    Query query;
    query.init(nvars);
    for(int i=0;i<nvars;i++){
      query.px[i] = sample[vars[i]];
      if(query.px[i]==-1){
        query.px[i] = rand()%(vars[i]->nvals);
        query.unknown.push_back(i);
      }
    }
    queries.push_back(query);
  }
  void parse_test_file(string file){
    cout<<"Parsing query file ..."<<endl;
    fin.open(file);
    if(!fin.good()){
      error("Unable to open file!");
    }
    string s;
    vector<Variable*> v;
    for(int i=0;i<nvars;i++){
      fin>>s;
      v.push_back(vars_factory[s]);
    }
    map<Variable*, int> sample;
    while(fin>>s){
      if(s=="?")sample[v[0]]=-1;
      else sample[v[0]] = v[0]->get_val(s);
      for(int i=1;i<nvars;i++){
        fin>>s;
        if(s=="?")sample[v[i]]=-1;
        else sample[v[i]] = v[i]->get_val(s);
      }
      parse_test_sample(sample);
    }
    fin.close();
  }
  void parse_true_sample(Query& query, map<Variable*, int>& sample){
    for(int i=0;i<nvars;i++)
      query.x[i] = sample[vars[i]];
  }
  void parse_true_file(string file){
    cout<<"Parsing true values ..."<<endl;
    fin.open(file);
    if(!fin.good()){
      error("Unable to open file!");
    }
    string s;
    vector<Variable*> v;
    for(int i=0;i<nvars;i++){
      fin>>s;
      v.push_back(vars_factory[s]);
    }
    map<Variable*, int> sample;
    for(int q=0;q<queries.size();q++){
      for(int i=0;i<nvars;i++){
        fin>>s;
        sample[v[i]] = v[i]->get_val(s);
      }
      parse_true_sample(queries[q], sample);
    }
    fin.close();
    query();
  }
  void query(){
    cout<<"Inference ..."<<endl;
    acc=0, ll=0;
    for(auto& query:queries){
      sampler(query.marginal, query.px, query.unknown);
      query.calcuate_marginal();
      query.calculate_accuracy();
      query.print();
      acc += query.acc;
      ll += query.ll;
    }
    acc /= queries.size();
    ll /= queries.size();
  }
  double prob(int k, const vector<int>& x){
    double ret=0;
    for(auto& factor:factors){
      if(factor->contain(vars[k]))
        ret += log(factor->prob(x));
    }
    return exp(ret);
  }
  int sample(int k, vector<int>& x){
    vector<double> marginal(vars[k]->nvals, 0);
    double sum=0;
    for(int i=0;i<marginal.size();i++){
      x[k]=i;
      marginal[i] = prob(k, x);
      sum += marginal[i];
    }
    for(int i=0;i<marginal.size();i++)
      marginal[i] /= sum;
    for(int i=1;i<marginal.size();i++)
      marginal[i] += marginal[i-1];
    double toss_value = double(rand())/INT_MAX;
    for(int i=marginal.size()-1;i>=0;i--){
      if(toss_value>=marginal[i])
        return i+1;
    }
    return 0;
  }
  void sampler(vector<vector<double> >& p, vector<int>& x, const vector<int>& v){
    for(int i=0;i<nvars;i++)
      p[i].assign(vars[i]->nvals, 0);

    int burn_out = 2000;
    int itr=1, i=0, j;
    while(itr<=10000){
      j = v[i];
      x[j] = sample(j, x);
      if(itr>burn_out){
        for(int k=0;k<nvars;k++){
          p[k][x[k]]+=1;
        }
      }
      itr = itr+1;
      i = (i+1)%v.size();
    }
  }
  void emit_query(string bif){
    cout<<"Emitting query results"<<endl;
    fout.open(bif+".bn.out");
    for(auto &query:queries){
      for(auto i:query.unknown){
        Variable* var = vars[i];
        fout<<var->name<<" ";
        for(int j=0;j<var->nvals;j++){
          fout<<var->vals[j]<<":"<<query.marginal[i][j]<<" ";
        }
        fout<<endl;
      }
      fout<<endl;
    }
    fout.close();
  }
};