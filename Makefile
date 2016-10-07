all: build/stl_unordered_map build/boost_unordered_map build/google_sparse_hash_map build/google_dense_hash_map build/qt_qhash build/spp_sparse_hash_map build/hopscotch_map

build/stl_unordered_map: src/stl_unordered_map.cc Makefile src/template.c
	g++ -O3 -march=native -std=c++11 -DNDEBUG -lm -o build/stl_unordered_map src/stl_unordered_map.cc

build/boost_unordered_map: src/boost_unordered_map.cc Makefile src/template.c
	g++ -O3 -march=native -std=c++11 -DNDEBUG -lm -o build/boost_unordered_map src/boost_unordered_map.cc

build/google_sparse_hash_map: src/google_sparse_hash_map.cc Makefile src/template.c
	g++ -O3 -march=native -std=c++11 -DNDEBUG -lm -o build/google_sparse_hash_map src/google_sparse_hash_map.cc

build/google_dense_hash_map: src/google_dense_hash_map.cc Makefile src/template.c
	g++ -O3 -march=native -std=c++11 -DNDEBUG -lm -o build/google_dense_hash_map src/google_dense_hash_map.cc

build/qt_qhash: src/qt_qhash.cc Makefile src/template.c
	g++ -O3 -march=native -std=c++11 -DNDEBUG -lm `pkg-config --cflags --libs QtCore` -o build/qt_qhash src/qt_qhash.cc

build/spp_sparse_hash_map: src/spp_sparse_hash_map.cc Makefile src/template.c
	g++ -O3 -march=native -std=c++11 -DNDEBUG -o build/spp_sparse_hash_map src/spp_sparse_hash_map.cc

build/hopscotch_map: src/hopscotch_map.cc Makefile src/template.c
	g++ -O3 -march=native -std=c++11 -DNDEBUG -o build/hopscotch_map src/hopscotch_map.cc


