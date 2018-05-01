# Unix shell
Utilized C to build a Unix shell that accepts user commands and execute each command in a separate process

## Course
SFU CMPT 300(Fall 2017): Operating Systems I

## Author
James(Yuhao) He

## Description
The shell accepts user commands and then executes each command in a separate process. The shell provides the user a prompt at which the next command is entered. 

## History feature 
The internal command "history" displays the ten most recent commands executed in the shell. If there are not yet ten commands, display all the commands entered so far. 

<dt>Sample output:</dt>
<dt>/documents/James> history</dy>
<dt>2 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cd home</dt>
<dt>3 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ls</dt>
<dt>4 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ls</dt>
<dt>5 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;history</dt>
<dt>6 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;clear</dt>
<dt>7 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cd home/documents/</dt>
<dt>8 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;hello world</dt>
<dt>9 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cd..</dt>
<dt>10&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cd..</dt>
<dt>11 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ls</dt>

## Customized signal
Customized the signal handler. Instead of terminating processes, user displays a list of history commands(same as "history" command) by entering ctrl-c.
