TEMPLATE   = subdirs
SUBDIRS   += \
    BananaCore \
    BananaUI \
    BananaTests \
    QtnProperty

QtnProperty.file = QtnProperty/QtnProperty/QtnProperty.pro

BananaUI.depends = BananaCore QtnProperty
BananaTests.depends = BananaCore BananaUI
