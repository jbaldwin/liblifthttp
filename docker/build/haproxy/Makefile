VERSION=0.1

.PHONY: build
build:
	docker build -t jbaldwindh/liblifthttp-automation-haproxy:$(VERSION) -f Dockerfile .

.PHONY: push
push:
	docker push jbaldwindh/liblifthttp-automation-haproxy:$(VERSION)

.PHONY: run
run:
	 docker run -it --network host --rm --name haproxy jbaldwindh/liblifthttp-automation-haproxy:0.1
