# when you clone my arduino shared library you also need this other repo. 
# execute this from the checked out library directory, or do it elsewhere and add a symlink in this library to where the checkout is.
git clone https://github.com/980f/ezcpp.git
# added a synonym for ezcpp that lets Arduino include its headers. This will only work for one additional headers library.
ln -s ezcpp src

