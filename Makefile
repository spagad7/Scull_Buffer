all:
	$(MAKE) -C scull_buffer
	$(MAKE) -C producer
	$(MAKE) -C consumer

clean:
	rm -rf scull_buffer/*.o scull_buffer/*~ core .depend scull_buffer/.*.cmd scull_buffer/*.ko scull_buffer/*.mod.c scull_buffer/.tmp_versions producer/producer consumer/consumer
