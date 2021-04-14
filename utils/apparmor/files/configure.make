package=libapparmor

configure:
	$(STAGING_DIR_HOST)/bin/aclocal
	$(STAGING_DIR_HOST)/bin/autoconf --force
	$(STAGING_DIR_HOST)/bin/libtoolize --automake -c --force
	$(STAGING_DIR_HOST)/bin/automake -ac
