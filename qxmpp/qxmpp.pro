include(qxmpp.pri)

TEMPLATE = subdirs

SUBDIRS = src

CONFIG += ordered

# Source distribution
QXMPP_ARCHIVE = qxmpp-$$QXMPP_VERSION
dist.commands = \
    $(DEL_FILE) -r $$QXMPP_ARCHIVE && \
    $(MKDIR) $$QXMPP_ARCHIVE && \
    git archive master | tar -x -C $$QXMPP_ARCHIVE && \
    $(COPY_DIR) doc/html $$QXMPP_ARCHIVE/doc && \
    tar czf $${QXMPP_ARCHIVE}.tar.gz $$QXMPP_ARCHIVE && \
    $(DEL_FILE) -r $$QXMPP_ARCHIVE
