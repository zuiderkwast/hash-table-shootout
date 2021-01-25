##################################################
# User-settable variables

CXX      ?= clang++
CXXFLAGS ?= -O3 -march=native -std=c++14 -DNDEBUG

# LDFLAGS_MALLOC ?=
# LDFLAGS_MALLOC ?= -ljemalloc # much better that glibc's malloc on some workloads
LDFLAGS_MALLOC ?= -ltcmalloc_minimal # often even better than jemalloc

APPS ?= std_unordered_map boost_unordered_map google_sparse_hash_map \
   google_dense_hash_map google_dense_hash_map_mlf_0_9 \
   spp_sparse_hash_map emilib_hash_map ska_flat_hash_map \
   ska_flat_hash_map_power_of_two tsl_hopscotch_map \
   tsl_hopscotch_map_mlf_0_5 tsl_hopscotch_map_store_hash tsl_robin_map \
   tsl_robin_map_mlf_0_9 tsl_robin_map_store_hash tsl_robin_pg_map \
   tsl_sparse_map tsl_ordered_map tsl_array_map tsl_array_map_mlf_1_0 qt_qhash
#APPS += judyL
#APPS += judyHS

LDFLAGS ?= -lm
LDFLAGS += ${LDFLAGS_MALLOC}

ifeq ($(filter qt_qhash,${APPS}), qt_qhash)
CXXFLAGS_qt_qhash ?= $(shell pkg-config --cflags --libs Qt5Core) -fPIC
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
