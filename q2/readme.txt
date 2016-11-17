####Output Format for parameters of Markov Network  ################

For Markov network, you just need to modify the probability part of bif file:

For e.g
Bayesian Network CPT of insurance.bif is

probability ( SocioEcon | Age ) {
  (Adolescent) ?, ?, ?, ?;
  (Adult) ?, ?, ?, ?;
  (Senior) ?, ?, ?, ?;
}

will change into below format (Save it with .txt extension) :

probability ( SocioEcon, Age ) {
  (Prole, Adolescent) ?;
  (Prole, Adult) ?;
  (Prole, Senior) ?;
  (Middle, Adolescent) ?;
  (Middle, Adult) ?;
  (Middle, Senior) ?;
  (UpperMiddle, Adolescent) ?;
  (UpperMiddle, Adult) ?;
  (UpplerMiddle, Senior) ?;
  (Wealthy, Adolescent) ?;
  (Wealthy, Adult) ?;
  (Wealthy, Senior) ?;
}