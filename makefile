CC = gcc
AR = ar
CHECKMK = checkmk

PKG_VER = 0.02

SRC   = src/mp.c
SRC  += src/mp_trc.c
SRC  += src/mp_rep.c
SRC  += src/mp_os.c
OBJ  = $(SRC:.c=.o)
MP = mp
PKG_NAME = $(MP)-$(PKG_VER).zip

LIB = libmp.a

TST_SRC = ./tst/mptst.c
TST_OBJ = $(TST_SRC:.c=.o)
TST = ./tst/mptst

TST_THRD_SRC = ./tst/mptst_thrd.c
TST_THRD_OBJ = $(TST_THRD_SRC:.c=.o)
TST_THRD = ./tst/mptst_thrd

UT_MP = ./ut/ut_mp
UT_MP_TRC = ./ut/ut_mp_trc
UT_MP_REP = ./ut/ut_mp_rep
UT_MP_PC = ut/ut_mp.check
UT_MP_SRC = $(UT_MP_PC:.check=.c)
UT_MP_TRC_PC = ut/ut_mp_trc.check
UT_MP_TRC_SRC = $(UT_MP_TRC_PC:.check=.c)
UT_MP_REP_PC = ut/ut_mp_rep.check
UT_MP_REP_SRC = $(UT_MP_REP_PC:.check=.c)
UT_MP_OBJ  = $(UT_MP_SRC:.c=.o)
UT_MP_TRC_OBJ += $(UT_MP_TRC_SRC:.c=.o)
UT_MP_REP_OBJ += $(UT_MP_REP_SRC:.c=.o)

XMP_SRC  = ./exm/example01.c
XMP_SRC += ./exm/example02.c
XMP_SRC += ./exm/example03.c
XMP_SRC += ./exm/example04.c

CFLAGS = -Wall -g -Werror -pedantic -std=c99

%.o: %.c
	$(CC) $(CFLAGS) -I./src -c -o $@ $<

all: $(LIB) $(TST) $(TST_THRD)

$(LIB):	$(OBJ)
	$(AR) -cvq $(LIB) $(OBJ)

$(TST):	$(TST_OBJ)
	$(CC) $< -L. -l$(MP) -o $@

$(TST_THRD):	$(TST_THRD_OBJ)
	$(CC) $< -L. -l$(MP) -pthread -o $@

ut: $(UT_MP) $(UT_MP_TRC) $(UT_MP_REP)
	$(UT_MP)
	$(UT_MP_TRC)
	$(UT_MP_REP)
	
$(UT_MP): $(UT_MP_OBJ)
	$(CC) $< -I./src -lcheck -pthread -o $@

$(UT_MP_TRC): $(UT_MP_TRC_OBJ)
	$(CC) $< -I./src -lcheck -o $@

$(UT_MP_REP): $(UT_MP_REP_OBJ)
	$(CC) $< -I./src -lcheck -o $@

$(UT_MP_SRC): $(UT_MP_PC)
	$(CHECKMK) $(UT_MP_PC) > $(UT_MP_SRC)
	
$(UT_MP_TRC_SRC): $(UT_MP_TRC_PC)
	$(CHECKMK) $(UT_MP_TRC_PC) > $(UT_MP_TRC_SRC)
	
$(UT_MP_REP_SRC): $(UT_MP_REP_PC)
	$(CHECKMK) $(UT_MP_REP_PC) > $(UT_MP_REP_SRC)
	
PKG_TMP_DIR = __tmp_mp__
PKG_BASE_DIR = $(MP)

pkg: make_dir \
	copy_src \
	tar_module \
	rm_dir

make_dir:
	@mkdir -p $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/.
	@mkdir -p $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/src
	@mkdir -p $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/ut
	@mkdir -p $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/valgrind
	@mkdir -p $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/tst
	@mkdir -p $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/exm
	@mkdir -p $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/win
	@mkdir -p $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/win/mptst
	@mkdir -p $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/win/mptst_thrd
	@mkdir -p $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/win/lib

copy_src:
	@cp $(SRC) $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/src/.
	@cp src/mp.h $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/src/.
	@cp src/mp_trc.h $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/src/.
	@cp src/mp_os.h $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/src/.
	@cp $(LIB) $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/.
	@cp $(TST_SRC) $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/tst/.
	@cp $(TST_THRD_SRC) $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/tst/.
	@cp $(UT_MP_PC) $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/ut/.
	@cp $(UT_MP_SRC) $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/ut/.
	@cp $(UT_MP_TRC_PC) $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/ut/.
	@cp $(UT_MP_TRC_SRC) $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/ut/.
	@cp $(UT_MP_REP_PC) $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/ut/.
	@cp $(UT_MP_REP_SRC) $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/ut/.
	@cp makefile $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/.
	@cp ChangeLog $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/.
	@cp MANUAL.html $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/.
	@cp valgrind/valgrind.h $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/valgrind/.
	@cp -r ./win/mptst $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/win/.
	@cp -r ./win/mptst_thrd $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/win/.
	@cp -r ./win/lib $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/win/.
	@cp $(XMP_SRC) $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/exm/.
	@cp ./exm/makefile $(PKG_TMP_DIR)/$(PKG_BASE_DIR)/exm/.

tar_module:
	cd $(PKG_TMP_DIR); zip -r ../$(PKG_NAME) $(PKG_BASE_DIR)

rm_dir: 
	rm -rf $(PKG_TMP_DIR)


clean:
	rm -rf $(OBJ) $(TST_OBJ) $(TST_THRD_OBJ) $(UT_MP_OBJ) $(UT_MP_TRC_OBJ) $(UT_MP_REP_OBJ) $(PKG_NAME) $(LIB) $(TST) $(TST_THRD) $(UT_MP) $(UT_MP_TRC) $(UT_MP_REP) memdmp.txt

clean_ut:
	rm -rf $(UT_MP_SRC) $(UT_MP_TRC_SRC) $(UT_MP_REP_SRC)
