#!/bin/bash

./gama-local demo01-a.gkf --xml demo01-a.xml --text demo01-a.txt
./gama-local demo01-b.gkf --xml demo01-b.xml --text demo01-b.txt

./gama-local-deformation demo01-a.xml demo01-b.xml
