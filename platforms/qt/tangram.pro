TEMPLATE = subdirs

SUBDIRS += \
    tangramqt \
    tangramquick \
    tangramquick-demo

tangramquick.subdir = tangramquick
tangramquick-demo.subdir = tangramquick-demo

tangramquick-demo.depends += tangramquick
