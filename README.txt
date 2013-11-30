MPCS 52015 Autumn 2013
UCEE Project
Mike Borosh (mborosh@uchicago.edu)
Marcelo Alvisio (alvisio@uchicago.edu)
Dan Sullivan (dansully@uchicago.edu)

The most recent version of this source tree can be pulled down (with authentication):
git clone git@bitbucket.org:mpcs51025gamma/ucee.git

STARTING THE SIMULATION ENVIRONMENT:

1) make clean; make; ./database.sh
2) ./connmgr
3) ./tradeBot/md_receiver -a 239.192.07.07 -p 1234
4) ./matcheng
5) ./tradepub
6) ./bookpub
7) ./rpteng
8) ./simulate.sh

USING THE REPORTING ENVIRONMENT:

To run reports after the environment is shut down, you can use the following:
1) Trader Report:       ./rpt -r 1
2) Instrument Report:   ./rpt -r 2
3) Summary Report:      ./rpt -r 3
