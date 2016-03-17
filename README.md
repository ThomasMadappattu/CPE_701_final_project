CPE_701_final_project
=====================

This is our implementation of the final project. It is nearly 3000 lines of
code.

 Building & Compiling
=====================

The project depends on mhash library to calculate crcs. In ubuntu (debian) based
systems it can installed by typing sudo apt-get install libmhash-dev. This is
by far the most common distro. Refer to distro's documention ( package manger)

 
We used Slackware linux so we installed the mhash libray using the slackware pack
age manger (pkgtool). The entire thing was written in C using the vim editor.

After mhash library is installed type make to build the system


Notes
===================
  * I removed many calls to free(), beware there are memory leaks.  There is a bug somewhere - I just masked it out. 
  * Set the socket options to reuse the socket ( in server code).  Too lazy to do it now, but you figure it out okay

Usage
=====================


To start the program , type ./nfdapp  "node_id" "itc_filename" to start
the program.

Commands Supported
=====================


By default it puts lots of messages on screen , they are done by separate
threads so one should be able type into the shell. (its little inconvinient
though).


(1) start_sevice "max_connections" : starts the service wiht max connections
provided, stats the listening thread. Make sure that this is done as immediately
as possible. (or increase the ROUTE_DELAY contanst) so that routing packets
do not grab the socket before server does a bind.


(2) download "from_node_id" "filename" : downloads filename from node_id



(3)set_garbler "loss" "corruption" : sets the garbler characteristics


(4) route_table: dumps the route table to route_table.txt


(5) link_down "node_id" : updates the routing table of current node ,
so that link is down


(6) link_up <node_id> : updates the routing table of current node
so that link is up


(7) exit : exits the program
  
