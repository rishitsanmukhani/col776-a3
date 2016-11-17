#include "bayes.h"
#include "markov.h"

void bayes(char** argv){
  Bayes bayes;
  bayes.init(string(argv[1]));
  bayes.parse_dat_file(string(argv[2]));
  bayes.parse_test_file(string(argv[3]));
  bayes.parse_true_file(string(argv[4]));
  bayes.emit(string(argv[1]));
  bayes.emit_query(string(argv[1]));
  cout<<"Final Results:"<<endl;
  cout<<"Ch-Acc: "<<bayes.acc<<endl;
  cout<<"LL    : "<<bayes.ll<<endl;
  cout<<endl;
}
void markov(char** argv){
  Markov markov;
  markov.init(string(argv[1]));
  markov.parse_dat_file(string(argv[2]));
  markov.parse_test_file(string(argv[3]));
  markov.parse_true_file(string(argv[4]));
  markov.emit(string(argv[1]));
  markov.emit_query(string(argv[1]));
  cout<<"Final Results:"<<endl;
  cout<<"Ch-Acc: "<<markov.acc<<endl;
  cout<<"LL    :"<<markov.ll<<endl;
  cout<<endl;
}
int main(int argc, char** argv){
  srand(time(NULL));
  if(argc!=5){
    error("./a.out <bif-file> <dat-file> <query-file> <true-file>");
  }
  // bayes(argv);
  markov(argv);
  return 0;
}