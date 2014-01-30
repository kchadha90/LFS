CC = gcc
CXX = g++
PROTOC = protoc
FLAGS = -g -Wall -Werror -Wpointer-arith -Wreturn-type -D_BSD_SOURCE -I. `pkg-config fuse --cflags`
CFLAGS = $(FLAGS) -Wstrict-prototypes -std=gnu99
CXXFLAGS = $(FLAGS) -fno-exceptions -std=gnu++0x `exttool/protobuf/pkg-config.sh --cflags`
LIBS = `pkg-config fuse --libs` `exttool/protobuf/pkg-config.sh --libs`

COBJS = exttool/disk.o fuse/start.o
PROTOHS = log/physical.pb.h file/physical.pb.h dir/physical.pb.h
CXXOBJS = util/logging.o util/time.o util/protobuf.o \
	disk/vdisk.o disk/rodisk.o disk/memdisk.o disk/logging_disk.o \
	log/physical.pb.o log/segio.o log/superblock.o log/checkpoint.o log/seg_summary.o \
	log/blkreader.o log/blkwriter.o log/logcreator.o log/log.o \
	cleaner/seqsegalloc.o cleaner/listsegalloc.o cleaner/segusage.o cleaner/cleaner.o \
	file/physical.pb.o file/inode.o file/file.o file/inode_store.o file/sysfile_creator.o \
	dir/physical.pb.o dir/pathacc.o dir/linear_dir.o dir/file_dir.o \
	fs/fs.o fs/verifier.o \
	fuse/wrap.o fuse/fileops.o fuse/streamops.o fuse/dirops.o fuse/fsops.o \
	command/lfs.o command/mklfs.o command/lfsck.o
TESTOBJS = exttool/gtest.o exttool/gtest_main.o  \
	util/logging_test.o util/cache_test.o util/time_test.o \
	disk/vdisk_test.o disk/memdisk_test.o \
	log/segblknum_test.o log/segio_test.o log/seg_summary_test.o log/blkreaderwriter_test.o \
	log/logcreator_test.o log/log_test.o \
	cleaner/seqsegalloc_test.o \
	file/file_testf.o file/file_test.o file/inode_store_test.o \
	dir/dir_testf.o dir/linear_dir_test.o
PROGRAMS = lfs lfsck mklfs unittest

all: $(PROGRAMS) README.htm

%.htm: %.md
	exttool/Markdown.pl $< > $@

.protobuf: $(PROTOHS)
	touch .protobuf

exttool/disk.o: exttool/disk.c
	$(CC) $(CFLAGS) -Iexttool -c -o $@ $<

%.pb.h: %.proto
	$(PROTOC) --cpp_out=. $<

%.pb.o: %.pb.h
	$(CC) $(CXXFLAGS) -c -o `echo $< | sed 's/.pb.h/.pb.o/'` `echo $< | sed 's/.pb.h/.pb.cc/'`

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cc .protobuf
	$(CXX) $(CXXFLAGS) -c -o $@ $<

lfs: command/main.o $(COBJS) $(CXXOBJS)
	$(CXX) $(CXXFLAGS) -g -o lfs command/main.o $(COBJS) $(CXXOBJS) $(LIBS)

mklfs lfsck: lfs
	if [ -f $@ ]; then rm $@; fi; ln -s lfs $@

unittest: $(TESTOBJS) $(COBJS) $(CXXOBJS)
	$(CXX) $(CXXFLAGS) -g -o unittest $(TESTOBJS) $(COBJS) $(CXXOBJS) $(LIBS)

runtest: unittest
	./unittest

clean:
	rm -f */*.o */*.pb.h */*.pb.cc .protobuf $(PROGRAMS)

