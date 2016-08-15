all: build/stl_unordered_map build/boost_unordered_map build/google_sparse_hash_map build/google_dense_hash_map build/qt_qhash build/hopscotch_map_8 build/hopscotch_map_32 build/hopscotch_map_64

build/stl_unordered_map: src/stl_unordered_map.cc Makefile src/template.c
	g++ -O2 -lm src/stl_unordered_map.cc -o build/stl_unordered_map -std=c++11 -DNDEBUG

build/boost_unordered_map: src/boost_unordered_map.cc Makefile src/template.c
	g++ -O2 -lm src/boost_unordered_map.cc -o build/boost_unordered_map -std=c++11 -DNDEBUG

build/google_sparse_hash_map: src/google_sparse_hash_map.cc Makefile src/template.c
	g++ -O2 -lm src/google_sparse_hash_map.cc -o build/google_sparse_hash_map -std=c++11 -DNDEBUG

build/google_dense_hash_map: src/google_dense_hash_map.cc Makefile src/template.c
	g++ -O2 -lm src/google_dense_hash_map.cc -o build/google_dense_hash_map -std=c++11 -DNDEBUG

build/qt_qhash: src/qt_qhash.cc Makefile src/template.c
	g++ -O2 -lm `pkg-config --cflags --libs QtCore` src/qt_qhash.cc -o build/qt_qhash -std=c++11 -DNDEBUG

build/hopscotch_map_8: src/hopscotch_map_8.cc Makefile src/template.c
	g++ -O2 -I.. src/hopscotch_map_8.cc -o build/hopscotch_map_8 -std=c++11 -DNDEBUG

build/hopscotch_map_32: src/hopscotch_map_32.cc Makefile src/template.c
	g++ -O2 src/hopscotch_map_32.cc -o build/hopscotch_map_32 -std=c++11 -DNDEBUG

build/hopscotch_map_64: src/hopscotch_map_64.cc Makefile src/template.c
	g++ -O2 src/hopscotch_map_64.cc -o build/hopscotch_map_64 -std=c++11 -DNDEBUG
