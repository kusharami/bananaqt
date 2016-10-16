TEMPLATE   = subdirs
SUBDIRS   += \
        BananaCore \
        BananaTests

BananaTests.depends = BananaCore
