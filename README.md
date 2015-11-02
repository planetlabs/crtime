Wow. It's hard to get the file creation time on linux:

http://unix.stackexchange.com/questions/50177/birth-is-empty-on-ext4/50184

But who would need that?

https://lkml.org/lkml/2010/7/22/249

Me.

Here's how to build it and test it:

        sudo apt-get install e2fslibs-dev
        ./configure
        make test

(Use `autoreconf -vfi` if you chance any of the autoconf business).

Wanna build a .deb? Try `./dist.sh`.
