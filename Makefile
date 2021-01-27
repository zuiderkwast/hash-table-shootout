##################################################
# User-settable variables

CXX      ?= clang++
CXXFLAGS ?= -O3 -march=native -std=c++14 -DNDEBUG

# LDFLAGS_MALLOC ?=
# LDFLAGS_MALLOC ?= -ljemalloc # much better that glibc's malloc on some workloads
LDFLAGS_MALLOC ?= -ltcmalloc_minimal # often even better than jemalloc

ifndef APPS
APPS += boost_unordered_map
APPS += emilib_hash_map ska_flat_hash_map
APPS += google_sparse_hash_map
APPS += google_dense_hash_map
APPS += google_dense_hash_map_mlf_0_9
APPS += judyHS
APPS += judyL
#APPS += nata88
#APPS += nataF8
APPS += tsl_hopscotch_map
APPS += tsl_hopscotch_map_mlf_0_5
APPS += tsl_hopscotch_map_store_hash
APPS += tsl_robin_map
APPS += tsl_robin_map_mlf_0_9
APPS += tsl_robin_map_store_hash
APPS += tsl_robin_pg_map
APPS += tsl_sparse_map
APPS += tsl_ordered_map
APPS += tsl_array_map
APPS += tsl_array_map_mlf_1_0
APPS += ska_flat_hash_map_power_of_two
APPS += spp_sparse_hash_map
APPS += std_unordered_map
APPS += qt_qhash
endif # APPS

LDFLAGS ?= -lm
LDFLAGS += ${LDFLAGS_MALLOC}

ifeq ($(filter qt_qhash,${APPS}), qt_qhash)
CXXFLAGS_qt_qhash ?= $(shell pkg-config --cflags Qt5Core) -fPIC
LDFLAGS_qt_qhash ?= $(shell pkg-config --libs Qt5Core)
endif
CXXFLAGS_spp_sparse_hash_map            ?= -Isparsepp
CXXFLAGS_emilib_hash_map                ?= -Iemilib
CXXFLAGS_ska_flat_hash_map              ?= -Iflat_hash_map
CXXFLAGS_ska_flat_hash_map_power_of_two ?= ${CXXFLAGS_ska_flat_hash_map}
CXXFLAGS_tsl_hopscotch_map              ?= -Ihopscotch-map
CXXFLAGS_tsl_hopscotch_map_mlf_0_5      ?= ${CXXFLAGS_tsl_hopscotch_map}
CXXFLAGS_tsl_hopscotch_map_store_hash   ?= ${CXXFLAGS_tsl_hopscotch_map}
CXXFLAGS_tsl_robin_map                  ?= -Irobin-map/include
CXXFLAGS_tsl_robin_map_mlf_0_9          ?= ${CXXFLAGS_tsl_robin_map}
CXXFLAGS_tsl_robin_map_store_hash       ?= ${CXXFLAGS_tsl_robin_map}
CXXFLAGS_tsl_robin_pg_map               ?= ${CXXFLAGS_tsl_robin_map}
CXXFLAGS_tsl_sparse_map                 ?= -Isparse-map/include
CXXFLAGS_tsl_ordered_map                ?= -Iordered-map/include
CXXFLAGS_tsl_array_map                  ?= -Iarray-hash/include -std=c++17
CXXFLAGS_tsl_array_map_mlf_1_0          ?= ${CXXFLAGS_tsl_array_map}
CXXFLAGS_judyL                          ?=
CXXFLAGS_judyHS                         ?=
CXXFLAGS_nata88                         ?=
CXXFLAGS_nataF8                         ?= ${CXXFLAGS_nata88}

LDFLAGS_judyL                           ?= -lJudy
LDFLAGS_judyHS                          ?= ${LDFLAGS_judyL}
LDFLAGS_nata88                          ?= -lnata
LDFLAGS_nataF8                          ?= ${LDFLAGS_nata88}

BUILD_DIR ?= ./build

##################################################

EXECUTABLES = $(APPS:%=$(BUILD_DIR)/%)

all: | $(BUILD_DIR) $(EXECUTABLES)

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

$(EXECUTABLES): $(BUILD_DIR)/%: src/%.cc src/template.c
	$(CXX) $(CXXFLAGS) ${CXXFLAGS_${notdir $@}} -o $@ $< ${LDFLAGS} ${LDFLAGS_${notdir $@}}
