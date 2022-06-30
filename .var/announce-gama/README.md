# gnulib

* cd gama.work.ac
* git clone git://git.sv.gnu.org/gnulib.git
* ln -s gnulib/build-aux/announce-gen        # create a symlink or copy ...

# new release

* copy or update file NEWS from the new release
* copy new release tar ball
* run ./gama-announce.sh current_ver previous_ver > announce-version.txt
** for example: ./gama-announce.sh  2.20 2.19 > announce-2.20.txt
* edit announce-version.txt

# when done

* git add announce-version.txt
* git commit
