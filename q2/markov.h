#pragma once
#include "common.h"
#include "variable.h"
#include "factor.h"

class Markov{
public:
  // Streams
  ifstream fin;
  ofstream fout;

  // Markov specific
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
    cout<<"Creating Markov Network ..."<<endl;
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
    cout<<"Learning Factors ..."<<endl;
    fin.open(dat);
    if(!fin.good()){
      error("Unable to open file!");
    }
    for(auto &factor:factors)
      factor->initialize();
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
      learn_factors(sample, 1);
    }
    fin.close();
    normalize_factors(1);
    gradient_descent();
  }
  void learn_factors(const map<Variable*, int>& sample, int type){
    for(auto& factor:factors){
      factor->learn(sample, type);
    }
  }
  void normalize_factors(int type){
    for(auto& factor:factors){
     factor->normalize_markov(type);
    }
  }
  void clear_factors(){
    for(auto &factor:factors){
      factor->clear();
    }
  }
  void gd_sampling(){
    clear_factors();
    vector<int> x(nvars);
    map<Variable*, int> m;
    for(int i=0;i<nvars;i++){
      m[vars[i]] = rand()%(vars[i]->nvals);
      x[i] = m[vars[i]];
    }
    int burn_out = 2000;
    int itr=1, i=0;
    while(itr<=6000){
      x[i] = sample(i, x);
      m[vars[i]] = x[i];
      if(itr>burn_out){
        learn_factors(m, 2);
      }
      itr = itr+1;
      i = (i+1)%x.size();
    }
    normalize_factors(2);
  }
  void gd_factors(double &alpha){
    static double c=0;
    static double last_delta = 1e9;
    double delta=0, sz=0;
    for(auto &factor:factors){
      delta += factor->gd(alpha,c);
      sz += factor->sz;
    }
    delta /= sz;
    if(delta>last_delta)
      alpha/=2;
    last_delta = delta;
    cout<<"Delta: "<<delta<<"; Alpha: "<<alpha<<endl;
  }
  void gradient_descent(){
    static double alpha=1;
    while(alpha>(1e-3)){
      gd_sampling();
      gd_factors(alpha);
    }
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
      // for(int i=0;i<x.size();i++){
      //   for(int j=0;j<marginal[i].size();j++){
      //     cout<<marginal[i][j]<<" ";
      //   }
      //   cout<<endl;
      // }
    }
    void print(){
      // cout<<acc<<" "<<ll<<endl;
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
    while(itr<=6000){
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
  void emit(string bif){
    fin.open(bif);
    fout.open(bif+".out.mn");
    string s;
    getline(fin, s);fout<<s<<"\n";
    getline(fin, s);fout<<s<<"\n";
    for(int i=0;i<3*nvars;i++){
      getline(fin, s);fout<<s<<"\n";
    }
    for(int i=0;i<nvars;i++){
      // Name
      s = "probability ( ";
      for(auto &var:factors[i]->x){
        s.append(var->name + ", ");
      }
      s.pop_back();s.pop_back();s.append(" ) {");
      fout<<s<<"\n";
      // Table
      for(int j=0;j<factors[i]->sz;j++){
        s = "  (";
        auto v = factors[i]->get_strings(j);
        for(auto &name:v){
          s.append(name + ", ");
        }
        s.pop_back();s.pop_back();s.append(") ");
        fout<<s<<factors[i]->table[j]<<";\n";
      }
      fout<<"}\n";
    }
    fout.close();
    fin.close();
  }
  void emit_query(string bif){
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