TEMPLATE   = subdirs
SUBDIRS   += qt-async-lib\
             tests\
             demo

tests.depends = qt-async-lib
demo.depends = qt-async-lib
