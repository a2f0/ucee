MPCS 52015 Autumn 2013
UCEE Project
Mike Borosh (mborosh@uchicago.edu)
Marcelo Alvisio (alvisio@uchicago.edu)
Dan Sullivan (dansully@uchicago.edu)

The most recent version of this source tree can be pulled down (with authentication):
git clone git@bitbucket.org:mpcs51025gamma/ucee.git

STARTING THE SIMULATION ENVIRONMENT:
* ./database.sh needs to be run only the first time the environment is set up to ensure the tables and trigger are set up. If you shut the environment down and bring it up again, database.sh should not be run a second time as it will clear the tables.

1) make clean; make
2) ./connmgr
2a)*
3) ./matcheng
4) ./tradepub
5) ./bookpub
6) ./rpteng
7) ./simulate.sh
8) ./rpt
