BUNDLE := bundle
BUNDLE_DIR := .bundle/
GEMFILE_LOCK := Gemfile.lock

JEKYLL := $(BUNDLE) exec jekyll
DEST_DIR := _site/


.PHONY: all build clean distclean install run

all: run

build: $(BUNDLE_DIR)
	$(JEKYLL) build -d $(DEST_DIR)

clean:
	$(JEKYLL) clean -d $(DEST_DIR)

distclean: clean
	$(RM) -rf $(BUNDLE_DIR)
	$(RM) $(GEMFILE_LOCK)

install: $(GEMFILE_LOCK)

run: $(BUNDLE_DIR)
	$(JEKYLL) serve --watch -d $(DEST_DIR)


$(GEMFILE_LOCK):
	$(BUNDLE) install --jobs $(shell nproc) --path $(BUNDLE_DIR)

$(BUNDLE_DIR): $(GEMFILE_LOCK)
