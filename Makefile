all: build/stl_unordered_map build/boost_unordered_map build/google_sparse_hash_map build/google_dense_hash_map build/qt_qhash build/spp_sparse_hash_map build/hopscotch_map

build/stl_unordered_map: src/stl_unordered_map.cc Makefile src/template.c
	g++ -O3 -lm src/stl_unordered_map.cc -o build/stl_unordered_map -std=c++11 -DNDEBUG

build/boost_unordered_map: src/boost_unordered_map.cc Makefile src/template.c
	g++ -O3 -lm src/boost_unordered_map.cc -o build/boost_unordered_map -std=c++11 -DNDEBUG

build/google_sparse_hash_map: src/google_sparse_hash_map.cc Makefile src/template.c
	g++ -O3 -lm src/google_sparse_hash_map.cc -o build/google_sparse_hash_map -std=c++11 -DNDEBUG

build/google_dense_hash_map: src/google_dense_hash_map.cc Makefile src/template.c
	g++ -O3 -lm src/google_dense_hash_map.cc -o build/google_dense_hash_map -std=c++11 -DNDEBUG

build/qt_qhash: src/qt_qhash.cc Makefile src/template.c
	g++ -O3 -lm `pkg-config --cflags --libs QtCore` src/qt_qhash.cc -o build/qt_qhash -std=c++11 -DNDEBUG

build/spp_sparse_hash_map: src/spp_sparse_hash_map.cc Makefile src/template.c
	g++ -O3 src/spp_sparse_hash_map.cc -o build/spp_sparse_hash_map -std=c++11 -DNDEBUG

build/hopscotch_map: src/hopscotch_map.cc Makefile src/template.c
	g++ -O3 src/hopscotch_map.cc -o build/hopscotch_map -std=c++11 -DNDEBUG

