This repository is a fork of
[university-of-york/cs-www-users-fisher](https://github.com/university-of-york/cs-www-users-fisher).

The goal of the work found on this branch is to resurrect the software (Tony
Fisher's `mkfilter` and friends at this time) to be deployed and used locally.

Very quick and dirty instructions:

- make sure a C++ compiler, libgd-dev and libcgicc-dev are installed
- clone this repo
- checkout branch `resurrect/mkfilter`
- build and install the backend binaries from `software/mkfilter/current`
- build and install the frontend from `mkfilter`
- run a simple CGI capable webserver serving `mkfilter`

```
git clone https://github.com/enrikb/cs-www-users-fisher.git
cd cs-www-users-fisher
git checkout resurrect/mkfilter
cd software/mkfilter/current
make install
cd ../../../mkfilter
make install
python3 -m http.server --cgi 8000
```

Then open http://localhost:8000/ and enjoy...

Use at your own risk! The software itself is taken as is and has only been
tweaked minimally to compile and run on a 2020 Linux installation.