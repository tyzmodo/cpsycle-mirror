QT       -= core gui

TARGET = blitz
TEMPLATE = lib

DEFINES += blitz_built

SOURCES += ../src/psycle/plugins/jme/blitzn/aaf.cpp\
../src/psycle/plugins/jme/blitzn/atlantisfilter.cpp\
../src/psycle/plugins/jme/blitzn/blitz.cpp\
../src/psycle/plugins/jme/blitzn/filter.cpp\
../src/psycle/plugins/jme/blitzn/lfo.cpp\
../src/psycle/plugins/jme/blitzn/pwm.cpp\
../src/psycle/plugins/jme/blitzn/voice.cpp\

HEADERS += ../src/psycle/plugins/jme/blitzn/aaf.h\
../src/psycle/plugins/jme/blitzn/atlantisfilter.h\
../src/psycle/plugins/jme/blitzn/blitz.h\
../src/psycle/plugins/jme/blitzn/filter.h\
../src/psycle/plugins/jme/blitzn/lfo.h\
../src/psycle/plugins/jme/blitzn/pwm.h\
../src/psycle/plugins/jme/blitzn/voice.h\

include (common.pri)


#this synth uses its own filter class, making psycle-helpers have namespace issues
#this synth uses its own filter class, making psycle-helpers have namespace issues

HEADERS -= ../../psycle-helpers/src/psycle/helpers/math/sinseq.hpp \
     ../../psycle-helpers/src/psycle/helpers/math/sincos.hpp \
     ../../psycle-helpers/src/psycle/helpers/math/sin.hpp \
     ../../psycle-helpers/src/psycle/helpers/math/math.hpp \
     ../../psycle-helpers/src/psycle/helpers/math/lround.hpp \
     ../../psycle-helpers/src/psycle/helpers/math/lrint.hpp \
     ../../psycle-helpers/src/psycle/helpers/math/log.hpp \
     ../../psycle-helpers/src/psycle/helpers/math/erase_denormals.hpp \
     ../../psycle-helpers/src/psycle/helpers/math/erase_all_nans_infinities_and_denormals.hpp \
     ../../psycle-helpers/src/psycle/helpers/math/constants.hpp \
     ../../psycle-helpers/src/psycle/helpers/math/clip.hpp \
     ../../psycle-helpers/src/psycle/helpers/value_mapper.hpp \
     ../../psycle-helpers/src/psycle/helpers/scale.hpp \
     ../../psycle-helpers/src/psycle/helpers/ring_buffer.hpp \
     ../../psycle-helpers/src/psycle/helpers/riffwave.hpp \
     ../../psycle-helpers/src/psycle/helpers/riff.hpp \
     ../../psycle-helpers/src/psycle/helpers/msriff.hpp \
     ../../psycle-helpers/src/psycle/helpers/mersennetwister.hpp \
     ../../psycle-helpers/src/psycle/helpers/math.hpp \
     ../../psycle-helpers/src/psycle/helpers/hexstring_to_integer.hpp \
     ../../psycle-helpers/src/psycle/helpers/datacompression.hpp \
     ../../psycle-helpers/src/psycle/helpers/hexstring_to_binary.hpp \
     ../../psycle-helpers/src/psycle/helpers/filter.hpp \
     ../../psycle-helpers/src/psycle/helpers/fft.hpp \
     ../../psycle-helpers/src/psycle/helpers/eaiff.hpp \
     ../../psycle-helpers/src/psycle/helpers/abstractiff.hpp \
     ../../psycle-helpers/src/psycle/helpers/dsp.hpp \
     ../../psycle-helpers/src/psycle/helpers/dither.hpp \
     ../../psycle-helpers/src/psycle/helpers/binread.hpp \
     ../../psycle-helpers/src/psycle/helpers/appleaiff.hpp \

SOURCES -=../../psycle-helpers/src/psycle/helpers/riffwave.cpp \
     ../../psycle-helpers/src/psycle/helpers/riff.cpp \
     ../../psycle-helpers/src/psycle/helpers/msriff.cpp \
     ../../psycle-helpers/src/psycle/helpers/mersennetwister.cpp \
     ../../psycle-helpers/src/psycle/helpers/hexstring_to_integer.cpp \
     ../../psycle-helpers/src/psycle/helpers/datacompression.cpp \
     ../../psycle-helpers/src/psycle/helpers/hexstring_to_binary.cpp \
     ../../psycle-helpers/src/psycle/helpers/filter.cpp \
     ../../psycle-helpers/src/psycle/helpers/fft.cpp \
     ../../psycle-helpers/src/psycle/helpers/eaiff.cpp \
     ../../psycle-helpers/src/psycle/helpers/dsp.cpp \
     ../../psycle-helpers/src/psycle/helpers/dither.cpp \
     ../../psycle-helpers/src/psycle/helpers/binread.cpp \
     ../../psycle-helpers/src/psycle/helpers/appleaiff.cpp \
     ../../psycle-helpers/src/psycle/helpers/abstractiff.cpp \