.PHONY: build
build: build-ubuntu build-fedora

VERSION=0.1

build-ubuntu:
	docker build -t jbaldwindh/liblifthttpbuilder-ubuntu:$(VERSION) -f ubuntu/Dockerfile .

build-fedora:
	docker build -t jbaldwindh/liblifthttpbuilder-fedora:$(VERSION) -f fedora/Dockerfile .

.PHONY: push
push: push-ubuntu push-fedora

push-ubuntu:
	docker push jbaldwindh/liblifthttpbuilder-ubuntu:$(VERSION)

push-fedora:
	docker push jbaldwindh/liblifthttpbuilder-fedora:$(VERSION)
