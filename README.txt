MPCS 52015 Autumn 2013
UCEE Project
Mike Borosh (mborosh@uchicago.edu)
Marcelo Alvisio (alvisio@uchicago.edu)
Dan Sullivan (dansully@uchicago.edu)

The most recent version of this source tree can be pulled down (with authentication):
git clone git@bitbucket.org:mpcs51025gamma/ucee.git

A. ONE-TIME SETUP OF THE DATABASE ENVIRONMENT:

Before starting the environment for the first time, please run ./database.sh in order to create the tables and set up the trigger.  If you shut the environment down and bring it up again, database.sh should not be run a second time as it will drop and re-create the tables.

B. STARTING THE SIMULATION ENVIRONMENT:

1) make clean; make
2) ./connmgr
3) ./tradeBot/md_receiver -a 239.192.07.07 -p 1234;
4) ./matcheng
5) ./tradepub
6) ./bookpub
7) ./rpteng
8) ./simulate.sh

C. USING THE REPORTING ENVIRONMENT:

To run reports after the environment is shut down, you can use the following:
1) Trader Report:     prompt>./rpt -a 1
2) Instrument Report: prompt>./rpt -a 2
3) Summary Report:    prompt>./rpt -a 3
